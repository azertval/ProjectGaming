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
                       int viewportHeight, std::vector<std::filesystem::path> levels)
    : _atlas(atlas),
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
                       int viewportHeight, core::Level level)
    : _atlas(atlas), _camera(viewportWidth, viewportHeight), _renderer(batch, atlas) {
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

    _level = std::move(level);  // conserve le niveau pour la simulation et le reset
    const core::Level& levelRef = *_level;  // level est deplace : plus lu au-dela de cette ligne
    _levelWidth = levelRef.tileMap().width();
    _levelHeight = levelRef.tileMap().height();
    _camera.setCenter(core::Vector2{static_cast<float>(_levelWidth) * 0.5f,
                                    static_cast<float>(_levelHeight) * 0.5f});
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

// Met a jour la region d'atlas du sprite du personnage depuis son etat d'animation courant.
void GameScreen::refreshPlayerSprite() {
    const core::Animation& animation = _world.getComponent<core::Animation>(_player);
    core::Sprite& sprite = _world.getComponent<core::Sprite>(_player);
    sprite.region = _atlas.playerFrameRegion(animation.clip, animation.frameIndex);
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
    const core::PlayerInput intent = toPlayerInput(input);

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
                if (result.normal.x != 0.0f &&
                    std::fabs(result.position.x - previousBox.min.x) <
                        std::fabs(bestPosition.x - previousBox.min.x)) {
                    bestPosition.x = result.position.x;
                    bestNormal.x = result.normal.x;
                }
                if (result.normal.y != 0.0f &&
                    std::fabs(result.position.y - previousBox.min.y) <
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

    // 4. Mecanismes : contact interrupteurs (front) / poids sur plaque de pression (continu,
    //    EX-GP-025) -> etat des portes (pour le pas suivant + visuel).
    const float playerMass = _world.getComponent<core::Player>(_player).mass;
    _mechanisms->update(box, playerMass);
    refreshDoorVisuals();

    // 5. Issue du niveau.
    switch (core::evaluateOutcome(box, *_level)) {
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

    // Zoom pour faire tenir le niveau ENTIER dans la fenetre (LOT-16, EX-REN-013) : entier
    // (nettete pixel art) tant qu'il tient a l'echelle x1, fractionnaire au-dela pour qu'aucune
    // zone ne reste hors champ — la camera cadre le niveau, elle ne suit pas le personnage.
    const float zoom =
        Camera2D::fitZoom(static_cast<float>(context.viewportWidth),
                          static_cast<float>(context.viewportHeight),
                          static_cast<float>(_levelWidth), static_cast<float>(_levelHeight), 0.92f);
    _camera.setZoom(zoom);

    _renderer.render(_world, _camera);
}

}  // namespace hmi
