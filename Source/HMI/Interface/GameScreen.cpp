#include "HMI/Interface/GameScreen.h"

#include <cmath>
#include <cstddef>
#include <string>

#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Levels/DangerGeometry.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/LevelScene.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/AabbVsAabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/PlayerInputMapper.h"
#include "HMI/Interface/RenderContext.h"

namespace hmi {

// Construit l'ecran et charge le premier niveau de la sequence.
GameScreen::GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                       int viewportHeight, std::vector<std::filesystem::path> levels,
                       const GameKeyBindings& gameBindings, const GamepadBindings& gamepadBindings)
    : _atlas(atlas),
      _gameBindings(gameBindings),
      _gamepadBindings(gamepadBindings),
      _camera(viewportWidth, viewportHeight),
      _renderer(batch, atlas),
      _sequence(LevelSequence(std::move(levels))) {
    if (_sequence->empty()) {
        _loadError = "Aucun niveau a charger.";  // robustesse : sequence vide -> etat neutre
        HMI_LOG_WARNING(_loadError);
        return;
    }
    loadLevel(_sequence->current());
}

// Construit l'ecran pour un niveau unique deja en memoire (essai immediat de l'editeur, LOT-15) :
// pas de sequence/fichier, la sortie termine l'essai au lieu d'enchainer (voir update()).
GameScreen::GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                       int viewportHeight, core::Level level, const GameKeyBindings& gameBindings,
                       const GamepadBindings& gamepadBindings)
    : _atlas(atlas),
      _gameBindings(gameBindings),
      _gamepadBindings(gamepadBindings),
      _camera(viewportWidth, viewportHeight),
      _renderer(batch, atlas) {
    loadLevel(std::move(level));
}

// Charge le niveau path depuis un fichier, puis delegue a loadLevel(core::Level) ; echec
// recuperable (EX-NFR-040) : _level reste vide, _loadError est renseigne.
void GameScreen::loadLevel(const std::filesystem::path& path) {
    core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
    if (!result.ok()) {
        _world = core::World{};  // repart d'un monde vierge (etat neutre)
        _level.reset();
        _loadError = result.error;
        HMI_LOG_WARNING("Echec du chargement du niveau : " + result.error);
        return;
    }
    loadLevel(std::move(*result.level));
}

// (Re)construit la scene pour un niveau deja charge et valide : monde neuf + grille + personnage
// a l'entree. Coeur commun aux deux constructeurs et aux rechargements (echec, niveau suivant).
void GameScreen::loadLevel(core::Level level) {
    _world = core::World{};  // repart d'un monde vierge (aucune entite du niveau precedent)
    _loadError.clear();

    _level = std::move(level);              // conserve le niveau pour la simulation et le reset
    const core::Level& levelRef = *_level;  // level est deplace : plus lu au-dela de cette ligne
    _levelWidth = levelRef.tileMap().width();
    _levelHeight = levelRef.tileMap().height();
    // Partition en salles (LOT-32) : reconstruite pour ce niveau, camera immediatement cadree sur
    // la salle de l'ENTREE (pas de salle "en retard" d'une frame apres un (re)chargement).
    _roomGrid.emplace(_levelWidth, _levelHeight);
    _currentRoomIndex = _roomGrid->roomIndexAt(levelRef.entry());
    centerCameraOnRoom(_currentRoomIndex);
    // La correspondance type -> region d'atlas (rendu) est injectee dans la projection pure.
    core::buildLevelScene(_world, levelRef,
                          [this](core::TileType type) { return regionForTile(type, _atlas); });
    // Mecanismes : etat interrupteurs/portes + grille de collision (portes fermees = solides).
    _mechanisms.emplace(levelRef);
    // Repere l'entite-tuile de chaque porte (avant le spawn du perso) pour le retour visuel d'etat.
    _doorEntities.clear();
    for (const core::Mechanism& mechanism : _mechanisms->mechanisms()) {
        core::Entity doorEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found &&
                    static_cast<int>(transform.position.x) == mechanism.doorPosition.column &&
                    static_cast<int>(transform.position.y) == mechanism.doorPosition.row) {
                    doorEntity = entity;
                    found = true;
                }
            });
        _doorEntities.push_back(doorEntity);
    }
    // Dangers mobile/temporise (EX-GP-051/053) : compteur de pas fixes a zero pour ce niveau.
    _dangers.emplace(levelRef);
    // Repere l'entite-tuile de chaque danger mobile (a sa position de DEPART, avant tout
    // deplacement) pour pouvoir la repositionner chaque pas (refreshDangerVisuals) -- sans quoi la
    // tuile resterait affichee a sa position de depart alors que sa boite mortelle reelle bouge.
    _moverEntities.clear();
    for (const core::DangerMoverConfig& config : levelRef.moverConfigs()) {
        core::Entity moverEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found &&
                    static_cast<int>(transform.position.x) == config.startPosition.column &&
                    static_cast<int>(transform.position.y) == config.startPosition.row) {
                    moverEntity = entity;
                    found = true;
                }
            });
        _moverEntities.push_back(moverEntity);
    }
    // Dangers commute/temporise (EX-GP-052/053) : meme principe que les portes ci-dessus, une
    // entite-tuile par danger, pour pouvoir teinter selon son etat actif/inactif
    // (refreshDangerStateVisuals) -- sans quoi l'activation ne se verrait jamais.
    _dangerSwitchedEntities.clear();
    for (const core::DangerLink& link : levelRef.dangerLinks()) {
        core::Entity dangerEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found &&
                    static_cast<int>(transform.position.x) == link.dangerPosition.column &&
                    static_cast<int>(transform.position.y) == link.dangerPosition.row) {
                    dangerEntity = entity;
                    found = true;
                }
            });
        _dangerSwitchedEntities.push_back(dangerEntity);
    }
    _dangerBlinkEntities.clear();
    for (const core::DangerBlinkConfig& config : levelRef.blinkConfigs()) {
        core::Entity dangerEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found && static_cast<int>(transform.position.x) == config.position.column &&
                    static_cast<int>(transform.position.y) == config.position.row) {
                    dangerEntity = entity;
                    found = true;
                }
            });
        _dangerBlinkEntities.push_back(dangerEntity);
    }
    // Blocs poussables (EX-GP-022) : memes principes que les portes ci-dessus, une entite-tuile
    // par bloc, reperee a sa position de depart.
    _blocks.emplace(levelRef);
    _blockEntities.clear();
    for (const core::GridPosition& position : _blocks->positions()) {
        core::Entity blockEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found && static_cast<int>(transform.position.x) == position.column &&
                    static_cast<int>(transform.position.y) == position.row) {
                    blockEntity = entity;
                    found = true;
                }
            });
        _blockEntities.push_back(blockEntity);
    }
    spawnPlayer(levelRef.entry());
    HMI_LOG_INFO("Niveau charge : " + levelRef.name() + " (" + std::to_string(_levelWidth) + "x" +
                 std::to_string(_levelHeight) + ")");
}

// Fait apparaitre le personnage humanoide (0,4x0,8), centre dans la tuile d'entree (voir en-tete).
void GameScreen::spawnPlayer(core::GridPosition entry) {
    _player = _world.createEntity();
    const core::Vector2 size = core::playerSize();  // collision ET rendu partagent la meme taille
    _world.addComponent(
        _player, core::Transform{core::playerSpawnPosition(entry.column, entry.row), size, 0.0f});
    _world.addComponent(_player, core::Velocity{});
    _world.addComponent(_player, core::Collider{size});
    // Budget de mouvements du tableau (EX-GP-024) : -1 = illimite si le niveau n'en fixe pas.
    core::Player playerComponent;
    playerComponent.jumpsRemaining = _level->jumpBudget();
    playerComponent.dashesRemaining = _level->dashBudget();
    _world.addComponent(_player, playerComponent);
    // Etat d'animation (EX-REN-012) : demarre en repos, image 0 (valeurs par defaut) ; fait
    // evoluer par AnimationSystem chaque pas fixe (voir update()).
    _world.addComponent(_player, core::Animation{});
    // Sprite du personnage : silhouette humanoide animee (EX-REN-011), couche haute (dessine
    // par-dessus les tuiles). La taille a l'ecran suit l'echelle du Transform. La region initiale
    // correspond a l'etat d'animation par defaut ; refreshPlayerSprite() la tient a jour ensuite.
    core::Sprite sprite;
    sprite.region = _atlas.playerFrameRegion(core::AnimationClip::Idle, 0);
    sprite.layer = 100;
    sprite.tint = core::Color{1.0f, 1.0f, 1.0f, 1.0f};
    _world.addComponent(_player, sprite);
}

// Met a jour la teinte des sprites de portes selon leur etat (ouverte attenuee / fermee opaque).
void GameScreen::refreshDoorVisuals() {
    for (std::size_t index = 0; index < _doorEntities.size(); ++index) {
        const core::Entity door = _doorEntities[index];
        if (!_world.hasComponent<core::Sprite>(door)) {
            continue;  // porte non reperee (robustesse) : rien a faire
        }
        const float alpha = _mechanisms->isDoorOpen(index) ? 0.25f : 1.0f;
        _world.getComponent<core::Sprite>(door).tint = core::Color{1.0f, 1.0f, 1.0f, alpha};
    }
}

// Replace le sprite de chaque bloc a sa position courante (poussee/chute, EX-GP-022).
void GameScreen::refreshBlockVisuals() {
    const std::vector<core::GridPosition>& positions = _blocks->positions();
    const std::vector<float>& scales = _blocks->scales();
    for (std::size_t index = 0; index < _blockEntities.size(); ++index) {
        const core::Entity block = _blockEntities[index];
        if (!_world.hasComponent<core::Transform>(block)) {
            continue;  // entite-tuile non reperee (robustesse) : rien a faire
        }
        core::Transform& transform = _world.getComponent<core::Transform>(block);
        // Centre le sprite dans sa case selon son facteur de taille (EX-GP-005) : marge nulle
        // pour un bloc plein (comportement inchange), sinon exactement la meme marge que la
        // boite de collision reelle (BlockController::boxAt) -- coherence stricte visuel/collision.
        const float scale = scales[index];
        const float margin = (1.0f - scale) * 0.5f;
        transform.position = core::Vector2{static_cast<float>(positions[index].column) + margin,
                                           static_cast<float>(positions[index].row) + margin};
        transform.scale = core::Vector2{scale, scale};
    }
}

// Replace le sprite de chaque danger mobile a sa position courante (EX-GP-051) -- meme principe
// que refreshBlockVisuals ci-dessus : sans cette mise a jour, la tuile resterait affichee a sa
// position de depart alors que sa boite mortelle reelle (_dangers->moverBox) se deplace bien.
void GameScreen::refreshDangerVisuals() {
    for (std::size_t index = 0; index < _moverEntities.size(); ++index) {
        const core::Entity mover = _moverEntities[index];
        if (!_world.hasComponent<core::Transform>(mover)) {
            continue;  // entite-tuile non reperee (robustesse) : rien a faire
        }
        core::Transform& transform = _world.getComponent<core::Transform>(mover);
        transform.position = _dangers->moverBox(index).min;
    }
}

// Teinte chaque danger commute/temporise selon son etat courant -- alpha attenue (inoffensif) ou
// opaque (mortel), meme principe que refreshDoorVisuals ci-dessus.
void GameScreen::refreshDangerStateVisuals() {
    constexpr float INACTIVE_ALPHA = 0.35f;
    constexpr float ACTIVE_ALPHA = 1.0f;

    const std::vector<core::DangerLink>& links = _level->dangerLinks();
    for (std::size_t index = 0; index < _dangerSwitchedEntities.size(); ++index) {
        const core::Entity entity = _dangerSwitchedEntities[index];
        if (!_world.hasComponent<core::Sprite>(entity)) {
            continue;  // entite-tuile non reperee (robustesse) : rien a faire
        }
        const bool active = _mechanisms->isDangerActive(links[index].dangerPosition);
        _world.getComponent<core::Sprite>(entity).tint =
            core::Color{1.0f, 1.0f, 1.0f, active ? ACTIVE_ALPHA : INACTIVE_ALPHA};
    }

    const std::vector<core::DangerBlinkConfig>& blinkConfigs = _level->blinkConfigs();
    for (std::size_t index = 0; index < _dangerBlinkEntities.size(); ++index) {
        const core::Entity entity = _dangerBlinkEntities[index];
        if (!_world.hasComponent<core::Sprite>(entity)) {
            continue;
        }
        const bool active = _dangers->isBlinkActive(blinkConfigs[index].position);
        _world.getComponent<core::Sprite>(entity).tint =
            core::Color{1.0f, 1.0f, 1.0f, active ? ACTIVE_ALPHA : INACTIVE_ALPHA};
    }
}

// Assemble les boites actuellement mortelles des dangers a etat (mobile/commute/temporise,
// EX-GP-051/052/053) : Core/Levels ne connaissant pas Core/Gameplay, cette composition revient a
// l'appelant (voir en-tete de core::LevelOutcome.h).
std::vector<core::Aabb> GameScreen::collectActiveDangerBoxes() const {
    std::vector<core::Aabb> boxes;
    boxes.reserve(_dangers->moverCount() + _level->blinkConfigs().size() +
                  _level->dangerLinks().size());

    for (std::size_t index = 0; index < _dangers->moverCount(); ++index) {
        boxes.push_back(_dangers->moverBox(index));
    }
    for (const core::DangerBlinkConfig& config : _level->blinkConfigs()) {
        if (_dangers->isBlinkActive(config.position)) {
            boxes.push_back(core::dangerHitbox(core::TileType::DangerBlink, config.position.column,
                                               config.position.row));
        }
    }
    for (const core::DangerLink& link : _level->dangerLinks()) {
        if (_mechanisms->isDangerActive(link.dangerPosition)) {
            boxes.push_back(core::dangerHitbox(core::TileType::DangerSwitched,
                                               link.dangerPosition.column,
                                               link.dangerPosition.row));
        }
    }
    return boxes;
}

// Met a jour la region d'atlas du sprite du personnage depuis son etat d'animation courant.
void GameScreen::refreshPlayerSprite() {
    const core::Animation& animation = _world.getComponent<core::Animation>(_player);
    core::Sprite& sprite = _world.getComponent<core::Sprite>(_player);
    sprite.region = _atlas.playerFrameRegion(animation.clip, animation.frameIndex);
}

// Centre la camera sur le rectangle de la salle roomIndex (LOT-32) -- coupure nette : seul
// l'appelant decide QUAND recentrer (chargement, changement de salle), jamais chaque frame.
void GameScreen::centerCameraOnRoom(core::GridPosition roomIndex) {
    const RoomBounds bounds = _roomGrid->roomBounds(roomIndex);
    _camera.setCenter(
        core::Vector2{static_cast<float>(bounds.column) + static_cast<float>(bounds.width) * 0.5f,
                      static_cast<float>(bounds.row) + static_cast<float>(bounds.height) * 0.5f});
}

// Determine la salle contenant le personnage ; recentre la camera SEULEMENT si elle a change
// depuis le dernier pas (EX-REN-015) -- une camera suiveuse en continu a ete explicitement ecartee
// (epic LOT-32, meme raisonnement que LOT-16).
void GameScreen::updateCurrentRoom() {
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Vector2 center = transform.position + collider.size * 0.5f;
    const core::GridPosition tile{static_cast<int>(std::floor(center.x)),
                                  static_cast<int>(std::floor(center.y))};
    const core::GridPosition roomIndex = _roomGrid->roomIndexAt(tile);
    if (roomIndex != _currentRoomIndex) {
        _currentRoomIndex = roomIndex;
        centerCameraOnRoom(_currentRoomIndex);
    }
}

// Simule le personnage d'un pas fixe (mecanismes + physique + animation), puis statue sur
// l'issue du niveau.
ScreenTransition GameScreen::update(const InputState& input, float fixedDelta) {
    if (input.keyPressed(Key::Escape)) {
        return ScreenTransition::switchTo(ScreenId::Menu);
    }
    if (!_level) {
        return ScreenTransition::none();  // chargement echoue : rien a simuler
    }

    // 1. Entrees -> intention.
    const core::PlayerInput intent = toPlayerInput(input, _gameBindings, _gamepadBindings);

    // 1bis. Blocs poussables (EX-GP-022) : poussee puis chute, resolues AVANT la physique du
    // personnage, avec sa boite TELLE QUE LAISSEE par le pas precedent — pour qu'un bloc qui vient
    // de se degager ne bloque jamais le personnage sur ce meme pas.
    const core::Transform& previousTransform = _world.getComponent<core::Transform>(_player);
    const core::Collider& previousCollider = _world.getComponent<core::Collider>(_player);
    const core::Aabb previousBox =
        core::Aabb::fromTopLeftSize(previousTransform.position, previousCollider.size);
    _blocks->update(previousBox, intent.moveX, _mechanisms->collisionMap());
    refreshBlockVisuals();

    // 2. Physique sur la grille des MECANISMES (portes fermees = solides) completee par la
    //    position COURANTE des blocs (resolue ci-dessus).
    const core::TileMap collision = _blocks->collisionMap(_mechanisms->collisionMap());
    _physics.update(_world, collision, intent, fixedDelta);

    // 2bis. Blocs a TAILLE REDUITE (EX-GP-005) : leur boite REELLE (centree, plus petite qu'une
    // case) n'est jamais posee dans `collision` ci-dessus (BlockController::collisionMap) -- sinon
    // sa case entiere bloquerait a tort l'espace vide qui l'entoure. Composee ici via un balayage
    // boite-boite dedie (core::sweepAabbVsAabb), sur le deplacement REEL obtenu par la physique sur
    // grille : la restriction la plus stricte des deux l'emporte, jamais l'inverse (cette passe ne
    // peut que reduire encore le deplacement, jamais l'etendre -- voir AabbVsAabb.h).
    {
        core::Transform& transform = _world.getComponent<core::Transform>(_player);
        const core::Vector2 delta = transform.position - previousBox.min;
        if (delta.x != 0.0f || delta.y != 0.0f) {
            core::Vector2 bestPosition = transform.position;  // depart : resultat de la grille
            core::Vector2 bestNormal{};
            const std::vector<float>& scales = _blocks->scales();
            for (std::size_t index = 0; index < scales.size(); ++index) {
                if (scales[index] >= 1.0f) {
                    continue;  // bloc plein : deja resolu par le balayage sur grille ci-dessus
                }
                const core::SweepResult result =
                    core::sweepAabbVsAabb(previousBox, delta, _blocks->boxAt(index));
                if (result.normal.x != 0.0f && std::fabs(result.position.x - previousBox.min.x) <
                                                   std::fabs(bestPosition.x - previousBox.min.x)) {
                    bestPosition.x = result.position.x;
                    bestNormal.x = result.normal.x;
                }
                if (result.normal.y != 0.0f && std::fabs(result.position.y - previousBox.min.y) <
                                                   std::fabs(bestPosition.y - previousBox.min.y)) {
                    bestPosition.y = result.position.y;
                    bestNormal.y = result.normal.y;
                }
            }
            if (bestNormal.x != 0.0f || bestNormal.y != 0.0f) {
                transform.position = bestPosition;
                core::Velocity& velocity = _world.getComponent<core::Velocity>(_player);
                core::Player& player = _world.getComponent<core::Player>(_player);
                if (bestNormal.x != 0.0f) {
                    velocity.value.x = 0.0f;
                }
                if (bestNormal.y != 0.0f) {
                    velocity.value.y = 0.0f;
                    if (bestNormal.y < 0.0f) {
                        player.grounded = true;  // pose sur le dessus d'un bloc reduit
                    }
                }
            }
        }
    }

    // 2ter. Animation (EX-REN-012) : derivee de l'etat physique (Player::grounded, Velocity) qui
    // vient d'etre mis a jour pour CE pas — doit s'executer apres la physique, jamais avant.
    _animation.update(_world, fixedDelta);

    // 3. Boite du personnage apres deplacement.
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, collider.size);

    // 3bis. Camera : bascule de salle (LOT-32, EX-REN-015) -- coupure nette si le personnage vient
    // de franchir une frontiere de salle, sans effet sinon.
    updateCurrentRoom();

    // 4. Mecanismes : contact interrupteurs (front) / poids sur plaque de pression (continu,
    //    EX-GP-025) -> etat des portes (pour le pas suivant + visuel).
    const float playerMass = _world.getComponent<core::Player>(_player).mass;
    _mechanisms->update(box, playerMass);
    refreshDoorVisuals();

    // 4bis. Dangers mobile/temporise (EX-GP-051/053) : avance le compteur de pas fixes qui pilote
    // leur position/activation (purement deterministe, EX-NFR-002), puis replace les sprites des
    // dangers mobiles sur leur position ainsi mise a jour.
    _dangers->update();
    refreshDangerVisuals();
    // Dangers commute/temporise (EX-GP-052/053) : teinte selon l'etat courant (mecanismes deja mis
    // a jour juste au-dessus pour les commutes ; le controleur de dangers, pour les temporises).
    refreshDangerStateVisuals();

    // 5. Issue du niveau.
    switch (core::evaluateOutcome(box, *_level, collectActiveDangerBoxes())) {
        case core::LevelOutcome::Won:
            if (_sequence && _sequence->hasNext()) {
                // Enchaine le niveau suivant : on reste sur l'ecran de jeu (EX-LVL-011).
                _sequence->advance();
                HMI_LOG_INFO("Niveau termine : passage au niveau suivant.");
                loadLevel(_sequence->current());
                return ScreenTransition::none();
            }
            // Dernier niveau de la sequence franchi, ou niveau unique en memoire (essai immediat,
            // LOT-15, pas de sequence) : retour au titre, sans enchainement.
            HMI_LOG_INFO("Niveau/sequence termine(e) : retour au menu.");
            return ScreenTransition::switchTo(ScreenId::Menu);
        case core::LevelOutcome::Lost:
            // Echec : rechargement COMPLET du niveau (perso a l'entree, mecanismes et budget
            // remis) depuis le Level deja en memoire — pas de nouvelle lecture disque, valable
            // aussi bien en mode sequence qu'en mode niveau unique en memoire.
            loadLevel(*_level);
            break;
        case core::LevelOutcome::Playing:
            break;
    }
    return ScreenTransition::none();
}

// Dessine le niveau charge, ou un etat neutre si le chargement a echoue.
void GameScreen::render(RenderContext& context) {
    if (!_loadError.empty()) {
        // Etat d'erreur : message centre a l'ecran.
        const char* message = "Niveau indisponible";
        constexpr float scale = 4.0f;
        const float x =
            (static_cast<float>(context.viewportWidth) - context.font.textWidth(message, scale)) *
            0.5f;
        const float y =
            (static_cast<float>(context.viewportHeight) - context.font.lineHeight(scale)) * 0.5f;
        const DirectX::XMFLOAT4X4 projection =
            BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
        context.spriteBatch.begin(projection, context.font.textureView());
        context.font.drawText(context.spriteBatch, message, x, y, scale,
                              core::Color{0.90f, 0.55f, 0.55f, 1.0f});
        context.spriteBatch.end();
        return;
    }

    refreshPlayerSprite();

    _camera.setViewportSize(context.viewportWidth, context.viewportHeight);

    // Zoom pour faire tenir la SALLE COURANTE dans la fenetre (LOT-32, EX-REN-015) : entier
    // (nettete pixel art) tant qu'elle tient a l'echelle x1, fractionnaire au-dela pour qu'aucune
    // zone ne reste hors champ. Un niveau qui tient dans une seule salle degenere exactement au
    // cadrage "niveau entier" de LOT-16 (EX-REN-013) : meme formule, applique au rectangle de la
    // salle courante plutot qu'au niveau entier -- la camera ne suit jamais le personnage en
    // continu, seul updateCurrentRoom() la recentre, a la bascule de salle.
    const RoomBounds roomBounds = _roomGrid->roomBounds(_currentRoomIndex);
    const float zoom = Camera2D::fitZoom(
        static_cast<float>(context.viewportWidth), static_cast<float>(context.viewportHeight),
        static_cast<float>(roomBounds.width), static_cast<float>(roomBounds.height), 0.92f);
    _camera.setZoom(zoom);

    _renderer.render(_world, _camera);
}

}  // namespace hmi
