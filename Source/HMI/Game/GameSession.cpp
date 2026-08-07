#include "HMI/Game/GameSession.h"

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Levels/DangerGeometry.h"
#include "Core/Levels/LevelScene.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/AabbVsAabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/MechanismVisuals.h"
#include "HMI/Graphics/PlayerSprite.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/PreviousPosition.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"
#include "HMI/Graphics/TileAppearance.h"
#include "HMI/Graphics/TileAutotile.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/PlayerInputMapper.h"

namespace hmi {

namespace {
// Nombre d'images procedurales d'un PlayerClipKind (ProceduralAtlas.h) : seule source de verite
// deja etablie par LOT-18/LOT-46, reprise ici pour borner animation.frameIndex (Core) au cycle
// procedural, quel que soit le nombre d'images du clip Core resolu (LOT-48).
int proceduralFrameCount(PlayerClipKind kind) {
    switch (kind) {
        case PlayerClipKind::Idle:
            return PLAYER_IDLE_FRAME_COUNT;
        case PlayerClipKind::Run:
            return PLAYER_RUN_FRAME_COUNT;
        case PlayerClipKind::Jump:
            return PLAYER_JUMP_FRAME_COUNT;
    }
    return 1;
}

// Traduit un NOM de clip resolu par Core (core::playerClipSet()->clipAt(...).name) en identite de
// clip cote presentation (hmi::PlayerClipKind, ProceduralAtlas.h). L'atlas procedural ne sait
// dessiner que trois poses (Idle/Run/Jump, LOT-18) : les quatre clips LOT-48 (fall/land/wallslide/
// dash) retombent sur le plus proche via la MEME chaine de repli qu'une spritesheet externe
// partielle (hmi::resolveDeclaredPlayerClip, hmi::proceduralPlayerClipNames) -- l'atlas procedural
// est traite comme une spritesheet qui n'en declare que trois.
PlayerClipKind proceduralClipKindFor(const std::string& clipName) {
    const std::string resolved = resolveDeclaredPlayerClip(proceduralPlayerClipNames(), clipName);
    if (resolved == "run") {
        return PlayerClipKind::Run;
    }
    if (resolved == "jump") {
        return PlayerClipKind::Jump;
    }
    return PlayerClipKind::Idle;
}
}  // namespace

GameSession::GameSession(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache,
                         int viewportWidth, int viewportHeight, core::Level level,
                         const GameKeyBindings& gameBindings,
                         const GamepadBindings& gamepadBindings)
    : _atlas(atlas),
      _cache(cache),
      _gameBindings(gameBindings),
      _gamepadBindings(gamepadBindings),
      _camera(viewportWidth, viewportHeight),
      _renderer(batch, atlas, cache) {
    loadLevel(std::move(level));
}

// (Re)construit la scene pour un niveau deja charge et valide : monde neuf + grille + personnage
// a l'entree. Coeur commun a la construction et aux rechargements (echec).
void GameSession::loadLevel(core::Level level) {
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
    // La correspondance type -> region d'atlas (rendu) est injectee dans la projection pure, et
    // chaque entite tuile recoit sa marque d'habillage (type + voisinage solide, LOT-42). Le
    // masque ne depend que de la grille du niveau : le calculer ici, une fois, evite de le
    // refaire a chaque image sans rendre la scene dependante du mode de rendu.
    const core::TileMap& sceneMap = levelRef.tileMap();
    core::buildLevelScene(
        _world, levelRef, [this](core::TileType type) { return regionForTile(type); },
        [this, &sceneMap, &levelRef](core::Entity entity, core::TileType type, int column,
                                     int row) {
            _world.addComponent(
                entity, TileSkinTag{type, solidNeighborMask(sceneMap, column, row),
                                    textureOverrideAt(levelRef.textureOverrides(),
                                                       core::GridPosition{column, row})});
        });
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
    _doorVisuals.assign(_doorEntities.size(), MechanismVisualState{});
    // Repere l'entite-tuile du DECLENCHEUR (interrupteur/plaque de pression) de chaque mecanisme,
    // meme ordre que _doorEntities (LOT-47 : le declencheur change aussi d'apparence).
    _switchEntities.clear();
    for (const core::Mechanism& mechanism : _mechanisms->mechanisms()) {
        core::Entity switchEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found &&
                    static_cast<int>(transform.position.x) == mechanism.switchPosition.column &&
                    static_cast<int>(transform.position.y) == mechanism.switchPosition.row) {
                    switchEntity = entity;
                    found = true;
                }
            });
        _switchEntities.push_back(switchEntity);
    }
    _switchVisuals.assign(_switchEntities.size(), MechanismVisualState{});
    // Dangers mobile/temporise (EX-GP-051/053) : compteur de pas fixes a zero pour ce niveau.
    _dangers.emplace(levelRef);
    // Repere l'entite-tuile de chaque danger mobile (a sa position de DEPART) pour la repositionner
    // chaque pas (refreshDangerVisuals).
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
        // Danger mobile = mouvement continu : interpolé au rendu (EX-ARCH-031). PreviousPosition
        // initialisee a sa position de depart pour ne pas "glisser" depuis l'origine a la 1re
        // frame.
        if (found && _world.hasComponent<core::Transform>(moverEntity)) {
            _world.addComponent(
                moverEntity,
                PreviousPosition{_world.getComponent<core::Transform>(moverEntity).position});
        }
        _moverEntities.push_back(moverEntity);
    }
    _dangerMoverVisuals.assign(_moverEntities.size(), MechanismVisualState{});
    // Dangers commute/temporise (EX-GP-052/053) : une entite-tuile par danger, dont l'apparence
    // suit desormais l'etat actif/inactif (LOT-47, updateMechanismVisuals).
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
    _dangerSwitchedVisuals.assign(_dangerSwitchedEntities.size(), MechanismVisualState{});
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
    _dangerBlinkVisuals.assign(_dangerBlinkEntities.size(), MechanismVisualState{});
    // Blocs poussables (EX-GP-022) : une entite-tuile par bloc, reperee a sa position de depart.
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
        // Bloc poussable = mouvement (poussee/chute) : interpole au rendu (EX-ARCH-031), comme le
        // danger mobile ci-dessus. PreviousPosition initialisee a sa position de depart.
        if (found && _world.hasComponent<core::Transform>(blockEntity)) {
            _world.addComponent(
                blockEntity,
                PreviousPosition{_world.getComponent<core::Transform>(blockEntity).position});
        }
        _blockEntities.push_back(blockEntity);
    }
    spawnPlayer(levelRef.entry());
    HMI_LOG_INFO("Niveau charge : " + levelRef.name() + " (" + std::to_string(_levelWidth) + "x" +
                 std::to_string(_levelHeight) + ")");
}

void GameSession::reload() {
    if (_level) {
        loadLevel(*_level);  // recharge depuis le Level en memoire (perso a l'entree, etat remis)
    }
}

// Fait apparaitre le personnage humanoide (0,4x0,8), centre dans la tuile d'entree.
void GameSession::spawnPlayer(core::GridPosition entry) {
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
    core::Animation animation;
    animation.clips = core::playerClipSet();
    animation.clipIndex = core::PLAYER_CLIP_IDLE;
    _world.addComponent(_player, animation);
    core::Sprite sprite;
    sprite.region = _atlas.playerFrameRegion(PlayerClipKind::Idle, 0);
    sprite.tint = core::Color{1.0f, 1.0f, 1.0f, 1.0f};
    _world.addComponent(_player, sprite);
    // Habillage du personnage (LOT-48) : resolu chaque image par refreshPlayerSprite, valeurs
    // par defaut sans effet tant qu'un premier appel n'a pas eu lieu (RenderMode::Physique inchange
    // entre-temps, puisqu'il ne consulte que core::Sprite::region ci-dessus).
    _world.addComponent(_player, PlayerSpriteTag{});
    // Calque de dessin nomme (LOT-40, EX-REN-014) : le personnage passe devant les tuiles parce
    // qu'il est sur RenderLayer::Player, plus parce qu'on lui aurait attribue un entier plus grand.
    _world.addComponent(_player, RenderLayerTag{RenderLayer::Player});
    _world.addComponent(_player,
                        PreviousPosition{core::playerSpawnPosition(entry.column, entry.row)});
}

// Applique la correspondance etat -> clip a UNE entite-tuile de mecanisme (voir en-tete).
void GameSession::applyMechanismVisual(core::Entity entity, bool active, MechanismVisualState& state,
                                       const SceneTextures& textures, float fixedDelta) {
    if (!_world.hasComponent<TileSkinTag>(entity) || !_world.hasComponent<core::Sprite>(entity)) {
        return;  // entite-tuile non reperee (robustesse) : rien a faire, meme garde que le reste.
    }
    TileSkinTag& tag = _world.getComponent<TileSkinTag>(entity);
    // Par defaut : pas d'image par instance -- repli sur l'horloge partagee par asset (LOT-46) ou
    // sur l'image entiere, selon ce que resolveTileAppearance decide plus bas au rendu.
    tag.animatedFrame.reset();

    if (!isStatefulMechanism(tag.type)) {
        return;
    }

    // Asset effectivement lie a CETTE tuile, via le point de resolution UNIQUE (LOT-41) -- jamais
    // duplique ici : la hierarchie surcharge (LOT-45) > skin de type (LOT-42) > damier reste celle
    // du rendu. On ignore la region retournee (calculee sans connaitre l'etat), seuls la source et
    // l'index servent a retrouver l'asset et ses dimensions.
    const core::Sprite& sprite = _world.getComponent<core::Sprite>(entity);
    const TileAppearance appearance =
        resolveTileAppearance(RenderMode::Texture, sprite.region, &tag, textures);

    std::string assetPath;
    int width = 0;
    int height = 0;
    if (appearance.source == AppearanceSource::Skin) {
        const SkinTexture& skin = textures.skins[static_cast<std::size_t>(appearance.skinIndex)];
        assetPath = SKINS_SUBDIRECTORY + skin.asset;
        width = skin.width;
        height = skin.height;
    } else if (appearance.source == AppearanceSource::Override) {
        const SkinTexture& object = textures.objects[static_cast<std::size_t>(appearance.skinIndex)];
        assetPath = OBJECTS_SUBDIRECTORY + object.asset;
        width = object.width;
        height = object.height;
    } else {
        return;  // damier de repli (aucun asset assigne/charge) : rien a animer, deja journalise.
    }

    const AnimationDescription* description = _cache.getAnimation(assetPath, width, height);
    if (description == nullptr) {
        return;  // pas de fichier d'animation : image fixe, cas legitime et silencieux (LOT-46).
    }

    // Decision (transition/etat cible/repli) et progression : logique pure, testee hors GPU
    // (hmi::MechanismVisuals, LOT-47 TACHE-02) -- cette fonction ne fait que lui fournir l'asset
    // effectivement lie et ecrire le resultat sur la marque de presentation de la tuile.
    tag.animatedFrame = advanceMechanismVisual(state, *description, tag.type, active, assetPath,
                                               fixedDelta, _warnedMissingMechanismClips);
}

// Apparence des mecanismes pilotee par leur etat logique, au pas fixe (voir en-tete).
void GameSession::updateMechanismVisuals(float fixedDelta) {
    // Textures resolues UNE fois pour ce pas : c'est precisement ce pas qui calcule l'image par
    // instance de chaque mecanisme (tileAnimations vide -- l'horloge partagee par asset, LOT-46,
    // n'a rien a apporter ici, seule la resolution asset/dimensions de sceneTextures sert).
    const SceneTextures textures =
        sceneTextures(_atlas, _cache, _tileSkins, _tileSkinSet, _level->textureOverrides(), {});

    for (std::size_t index = 0; index < _doorEntities.size(); ++index) {
        applyMechanismVisual(_doorEntities[index], _mechanisms->isDoorOpen(index), _doorVisuals[index],
                             textures, fixedDelta);
    }
    for (std::size_t index = 0; index < _switchEntities.size(); ++index) {
        applyMechanismVisual(_switchEntities[index], _mechanisms->isDoorOpen(index),
                             _switchVisuals[index], textures, fixedDelta);
    }
    const std::vector<core::DangerLink>& links = _level->dangerLinks();
    for (std::size_t index = 0; index < _dangerSwitchedEntities.size(); ++index) {
        const bool active = _mechanisms->isDangerActive(links[index].dangerPosition);
        applyMechanismVisual(_dangerSwitchedEntities[index], active, _dangerSwitchedVisuals[index],
                             textures, fixedDelta);
    }
    const std::vector<core::DangerBlinkConfig>& blinkConfigs = _level->blinkConfigs();
    for (std::size_t index = 0; index < _dangerBlinkEntities.size(); ++index) {
        const bool active = _dangers->isBlinkActive(blinkConfigs[index].position);
        applyMechanismVisual(_dangerBlinkEntities[index], active, _dangerBlinkVisuals[index], textures,
                             fixedDelta);
    }
    for (std::size_t index = 0; index < _moverEntities.size(); ++index) {
        // Danger mobile : un seul clip (l'etat est porte par la position, pas par ce booleen) --
        // "actif" constant n'y declenche donc jamais de changement au-dela du calcul initial.
        applyMechanismVisual(_moverEntities[index], true, _dangerMoverVisuals[index], textures,
                             fixedDelta);
    }
}

// Modulation d'opacite de diagnostic, reservee au mode Physique (voir en-tete).
void GameSession::refreshMechanismDiagnosticTint(RenderMode mode) {
    constexpr float INACTIVE_ALPHA = 0.35f;
    constexpr float ACTIVE_ALPHA = 1.0f;
    constexpr float DOOR_OPEN_ALPHA = 0.25f;
    constexpr float DOOR_CLOSED_ALPHA = 1.0f;

    for (std::size_t index = 0; index < _doorEntities.size(); ++index) {
        const core::Entity door = _doorEntities[index];
        if (!_world.hasComponent<core::Sprite>(door)) {
            continue;  // porte non reperee (robustesse) : rien a faire.
        }
        const float alpha = mechanismDiagnosticAlpha(mode, _mechanisms->isDoorOpen(index),
                                                      DOOR_OPEN_ALPHA, DOOR_CLOSED_ALPHA);
        _world.getComponent<core::Sprite>(door).tint = core::Color{1.0f, 1.0f, 1.0f, alpha};
    }

    const std::vector<core::DangerLink>& links = _level->dangerLinks();
    for (std::size_t index = 0; index < _dangerSwitchedEntities.size(); ++index) {
        const core::Entity entity = _dangerSwitchedEntities[index];
        if (!_world.hasComponent<core::Sprite>(entity)) {
            continue;
        }
        const bool active = _mechanisms->isDangerActive(links[index].dangerPosition);
        const float alpha = mechanismDiagnosticAlpha(mode, active, ACTIVE_ALPHA, INACTIVE_ALPHA);
        _world.getComponent<core::Sprite>(entity).tint = core::Color{1.0f, 1.0f, 1.0f, alpha};
    }

    const std::vector<core::DangerBlinkConfig>& blinkConfigs = _level->blinkConfigs();
    for (std::size_t index = 0; index < _dangerBlinkEntities.size(); ++index) {
        const core::Entity entity = _dangerBlinkEntities[index];
        if (!_world.hasComponent<core::Sprite>(entity)) {
            continue;
        }
        const bool active = _dangers->isBlinkActive(blinkConfigs[index].position);
        const float alpha = mechanismDiagnosticAlpha(mode, active, ACTIVE_ALPHA, INACTIVE_ALPHA);
        _world.getComponent<core::Sprite>(entity).tint = core::Color{1.0f, 1.0f, 1.0f, alpha};
    }
}

void GameSession::refreshBlockVisuals() {
    const std::vector<core::GridPosition>& positions = _blocks->positions();
    const std::vector<float>& scales = _blocks->scales();
    for (std::size_t index = 0; index < _blockEntities.size(); ++index) {
        const core::Entity block = _blockEntities[index];
        if (!_world.hasComponent<core::Transform>(block)) {
            continue;  // entite-tuile non reperee (robustesse) : rien a faire
        }
        core::Transform& transform = _world.getComponent<core::Transform>(block);
        const float scale = scales[index];
        const float margin = (1.0f - scale) * 0.5f;
        transform.position = core::Vector2{static_cast<float>(positions[index].column) + margin,
                                           static_cast<float>(positions[index].row) + margin};
        transform.scale = core::Vector2{scale, scale};
    }
}

void GameSession::refreshDangerVisuals() {
    for (std::size_t index = 0; index < _moverEntities.size(); ++index) {
        const core::Entity mover = _moverEntities[index];
        if (!_world.hasComponent<core::Transform>(mover)) {
            continue;  // entite-tuile non reperee (robustesse) : rien a faire
        }
        core::Transform& transform = _world.getComponent<core::Transform>(mover);
        transform.position = _dangers->moverBox(index).min;
    }
}

std::vector<core::Aabb> GameSession::collectActiveDangerBoxes() const {
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

// Resout l'apparence du personnage pour l'image courante : region procedurale (Physique, et repli
// de Texture) + habillage Texture (spritesheet externe si chargee, LOT-48).
void GameSession::refreshPlayerSprite() {
    const core::Animation& animation = _world.getComponent<core::Animation>(_player);
    const core::Player& player = _world.getComponent<core::Player>(_player);
    core::Sprite& sprite = _world.getComponent<core::Sprite>(_player);
    PlayerSpriteTag& tag = _world.getComponent<PlayerSpriteTag>(_player);

    const std::string clipName =
        animation.clips ? std::string(animation.clips->clipAt(animation.clipIndex).name) : "idle";

    // Region PROCEDURALE : comportement de RenderMode::Physique strictement inchange depuis avant
    // LOT-48 (core::Sprite::region), et repli de RenderMode::Texture en l'absence de spritesheet
    // externe (AC#2 du lot) -- calculee une seule fois, partagee par les deux usages.
    const PlayerClipKind proceduralKind = proceduralClipKindFor(clipName);
    const int proceduralFrame = animation.frameIndex % proceduralFrameCount(proceduralKind);
    const core::AtlasRegion proceduralRegion = _atlas.playerFrameRegion(proceduralKind, proceduralFrame);
    sprite.region = proceduralRegion;

    core::AtlasRegion textureRegion = proceduralRegion;
    bool usesCharacterSheet = false;
    core::Vector2 imageSizePixels{static_cast<float>(TextureAtlas::PLAYER_FRAME_SIZE),
                                  static_cast<float>(TextureAtlas::PLAYER_FRAME_SIZE)};

    if (const LoadedTexture* sheet = _cache.get(PLAYER_SUBDIRECTORY + PLAYER_SHEET_FILE_NAME,
                                                AssetFamily::CharacterSheet)) {
        if (const AnimationDescription* description = _cache.getAnimation(
                PLAYER_SUBDIRECTORY + PLAYER_SHEET_FILE_NAME, sheet->width, sheet->height)) {
            // Noms effectivement declares par CETTE spritesheet (peut-etre partielle) : le repli
            // (chute -> saut, atterrissage -> repos, ...) est le meme mecanisme que pour l'atlas
            // procedural ci-dessus, seul l'ensemble declare differe.
            std::vector<std::string> declaredNames;
            declaredNames.reserve(static_cast<std::size_t>(description->clips.clipCount()));
            for (int index = 0; index < description->clips.clipCount(); ++index) {
                declaredNames.emplace_back(description->clips.clipAt(index).name);
            }
            const std::string resolvedName = resolveDeclaredPlayerClip(declaredNames, clipName);
            const int resolvedIndex = description->clips.indexOf(resolvedName);
            const core::AnimationClip& sheetClip =
                description->clips.clipAt(resolvedIndex >= 0 ? resolvedIndex : 0);
            const int frameSheetIndex =
                sheetClip.frames.empty()
                    ? 0
                    : sheetClip.frames[static_cast<std::size_t>(animation.frameIndex) %
                                       sheetClip.frames.size()];
            textureRegion = AnimationCatalog::frameRegion(*description, frameSheetIndex);
            usesCharacterSheet = true;
            imageSizePixels = core::Vector2{static_cast<float>(description->frameWidth),
                                            static_cast<float>(description->frameHeight)};
        }
    }

    const PlayerSpriteQuad quad = computePlayerSpriteQuad(imageSizePixels, core::playerSize());
    tag.textureRegion = textureRegion;
    tag.usesCharacterSheet = usesCharacterSheet;
    tag.quadOffset = quad.offset;
    tag.quadSize = quad.size;
    // Orientation (LOT-48 TACHE-03) : sens du deplacement, maintenu par la physique
    // (core::Player::facing), sans que le rendu n'ait a le recalculer.
    tag.flipHorizontal = player.facing < 0.0f;
}

// Avance l'horloge d'animation partagee des tuiles animees, au pas fixe (LOT-46 TACHE-05).
void GameSession::updateTileAnimations(float fixedDelta) {
    advanceTileAnimations(_tileSkins, _tileSkinSet, _cache, fixedDelta, _tileAnimations,
                          _warnedExcludedAnimations);
}

void GameSession::snapshotPreviousPositions() {
    _world.view<core::Transform, PreviousPosition>().each(
        [](core::Entity, const core::Transform& transform, PreviousPosition& previous) {
            previous.value = transform.position;
        });
}

void GameSession::centerCameraOnRoom(core::GridPosition roomIndex) {
    const RoomBounds bounds = _roomGrid->roomBounds(roomIndex);
    _camera.setCenter(
        core::Vector2{static_cast<float>(bounds.column) + static_cast<float>(bounds.width) * 0.5f,
                      static_cast<float>(bounds.row) + static_cast<float>(bounds.height) * 0.5f});
}

void GameSession::updateCurrentRoom() {
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

// Simule le personnage d'un pas fixe (mecanismes + physique + animation), puis statue sur l'issue.
core::LevelOutcome GameSession::update(const InputState& input, float fixedDelta) {
    if (!_level) {
        return core::LevelOutcome::Playing;  // chargement echoue : rien a simuler (etat neutre)
    }

    // 0. Interpolation (EX-ARCH-031) : fige la position COURANTE de chaque entite mobile comme sa
    //    position "precedente" AVANT que ce pas ne la modifie (voir render()).
    snapshotPreviousPositions();

    // 1. Entrees -> intention.
    const core::PlayerInput intent = toPlayerInput(input, _gameBindings, _gamepadBindings);

    // 1bis. Blocs poussables (EX-GP-022) : poussee puis chute, resolues AVANT la physique du
    // personnage, avec sa boite TELLE QUE LAISSEE par le pas precedent.
    const core::Transform& previousTransform = _world.getComponent<core::Transform>(_player);
    const core::Collider& previousCollider = _world.getComponent<core::Collider>(_player);
    const core::Aabb previousBox =
        core::Aabb::fromTopLeftSize(previousTransform.position, previousCollider.size);
    _blocks->update(previousBox, intent.moveX, _mechanisms->collisionMap());
    refreshBlockVisuals();

    // 2. Physique sur la grille des MECANISMES (portes fermees = solides) completee par la position
    //    COURANTE des blocs (resolue ci-dessus).
    const core::TileMap collision = _blocks->collisionMap(_mechanisms->collisionMap());
    _physics.update(_world, collision, intent, fixedDelta);

    // 2bis. Blocs a TAILLE REDUITE (EX-GP-005) : leur boite REELLE (centree, plus petite qu'une
    // case) n'est jamais posee dans `collision` ci-dessus. Composee ici via un balayage boite-boite
    // dedie (core::sweepAabbVsAabb), sur le deplacement REEL obtenu par la physique sur grille.
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

    // 2ter. Animation (EX-REN-012) : derivee de l'etat physique qui vient d'etre mis a jour.
    _animation.update(_world, fixedDelta);
    // 2ter bis. Tuiles animees (LOT-46 TACHE-05) : horloge partagee par asset, au meme pas fixe
    // (jamais au rythme du rendu, pour rester deterministe -- EX-NFR-002).
    updateTileAnimations(fixedDelta);

    // 3. Boite du personnage apres deplacement.
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, collider.size);

    // 3bis. Camera : bascule de salle (LOT-32, EX-REN-015) -- coupure nette si franchissement.
    updateCurrentRoom();

    // 4. Mecanismes : contact interrupteurs (front) / poids sur plaque (continu) -> etat des
    // portes.
    const float playerMass = _world.getComponent<core::Player>(_player).mass;
    _mechanisms->update(box, playerMass);

    // 4bis. Dangers mobile/temporise (EX-GP-051/053) : avance le compteur deterministe, puis
    // replace les sprites des dangers mobiles (position simulee, pas un artifice visuel).
    _dangers->update();
    refreshDangerVisuals();

    // 4ter. Apparence des mecanismes pilotee par l'etat logique (LOT-47, EX-REN-006) : porte,
    // declencheur, dangers commute/temporise/mobile -- correspondance + transitions, au meme pas
    // fixe que tout ce qui precede (la simulation, elle, n'en depend jamais).
    updateMechanismVisuals(fixedDelta);

    // 5. Issue du niveau. Sur echec : rechargement complet depuis le Level en memoire. Sur reussite
    // :
    //    l'appelant decide (enchainer, revenir au menu, terminer un essai...).
    const core::LevelOutcome outcome =
        core::evaluateOutcome(box, *_level, collectActiveDangerBoxes());
    if (outcome == core::LevelOutcome::Lost) {
        reload();
    }
    return outcome;
}

// Dessine le niveau charge (rien si le chargement a echoue : l'appelant gere l'affichage d'erreur).
void GameSession::render(int viewportWidth, int viewportHeight, RenderMode mode,
                         float interpolationAlpha) {
    if (!_level) {
        return;
    }
    refreshPlayerSprite();
    // Modulation d'opacite de diagnostic, mode Physique uniquement (LOT-47 TACHE-03) : decision
    // purement visuelle et dependante du mode courant, elle vit ici plutot qu'au pas fixe.
    refreshMechanismDiagnosticTint(mode);

    _camera.setViewportSize(viewportWidth, viewportHeight);
    // Zoom pour faire tenir la SALLE COURANTE dans la fenetre (LOT-32, EX-REN-015).
    const RoomBounds roomBounds = _roomGrid->roomBounds(_currentRoomIndex);
    const float zoom = Camera2D::fitZoom(
        static_cast<float>(viewportWidth), static_cast<float>(viewportHeight),
        static_cast<float>(roomBounds.width), static_cast<float>(roomBounds.height), 0.92f);
    _camera.setZoom(zoom);

    // Interpolation de rendu (EX-ARCH-031) entre le pas precedent et le pas courant.
    _renderer.render(_world, _camera, mode, interpolationAlpha, _level->background(), _levelWidth,
                     _levelHeight, _level->textureOverrides(), _tileAnimations);
}

}  // namespace hmi
