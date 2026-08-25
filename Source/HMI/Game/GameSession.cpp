// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/GameSession.h"

#include <algorithm>
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
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/AabbVsAabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"
#include "HMI/Game/GameHud.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/MechanismVisuals.h"
#include "HMI/Graphics/Parallax.h"
#include "HMI/Graphics/PlayerSprite.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/PreviousPosition.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/TextRenderer.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"
#include "HMI/Graphics/TileAppearance.h"
#include "HMI/Graphics/TileAutotile.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/PlayerInputMapper.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"

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

// Zoom et marge visuelle communs aux trois modes de cadrage (LOT-64) : une marge UNIQUE, la meme
// quel que soit le mode -- aucune raison visuelle de la faire varier, et une marge par mode se
// verrait comme un saut a chaque changement de cadrage.
constexpr float CAMERA_FIT_MARGIN = 0.92f;
}  // namespace

GameSession::GameSession(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache,
                         int viewportWidth, int viewportHeight, core::Level level,
                         const GameKeyBindings& gameBindings,
                         const GamepadBindings& gamepadBindings, const BitmapFont& font,
                         const Localization* localization)
    : _atlas(atlas),
      _cache(cache),
      _gameBindings(gameBindings),
      _gamepadBindings(gamepadBindings),
      _batch(batch),
      _font(font),
      _localization(localization),
      _camera(viewportWidth, viewportHeight),
      _renderer(batch, atlas, cache) {
    // Images des plans (LOT-69 TACHE-05) : a cote des niveaux, pas sous Assets/ -- un plan est une
    // donnee de niveau, jamais un asset reutilisable.
    _renderer.setPlanesDirectory(executableDirectory() / "Levels" / "Plans");
    loadLevel(std::move(level));
}

// (Re)construit la scene pour un niveau deja charge et valide : monde neuf + grille + personnage
// a l'entree. Coeur commun a la construction et aux rechargements (echec).
void GameSession::loadLevel(core::Level level) {
    // Particules (LOT-53 TACHE-02) : videes AVANT de reinitialiser _world -- les entites qu'elles
    // detiennent encore appartiennent a l'ANCIEN monde, sur le point d'etre remplace ci-dessous.
    // Un ordre inverse laisserait des handles perimes que le nouveau monde (index/generation
    // repartis a zero) pourrait faire coincider avec de toutes autres entites.
    _particles.clear(_world);
    _screenShake = ScreenShakeState{};  // aucune secousse residuelle sur le niveau rechargé
    _world = core::World{};  // repart d'un monde vierge (aucune entite du niveau precedent)
    _loadError.clear();
    // Detection d'evenements (LOT-60 TACHE-03) : un (re)chargement change l'etat sous les pieds de
    // la detection (personnage/mecanismes remis) -- sans ce reset, le premier pas du niveau
    // rechargé se comparerait a l'etat du niveau precedent et fabriquerait de faux evenements.
    // NE PAS vider _lastStepEvents ici : reload() est appele DEPUIS update(), apres que
    // l'evenement Died y a ete pousse -- le vider ici l'effacerait avant que l'appelant ne l'ait
    // jamais vu (mort et rechargement se suivent dans le meme pas).
    _gameEventsInitialized = false;

    _level = std::move(level);              // conserve le niveau pour la simulation et le reset
    const core::Level& levelRef = *_level;  // level est deplace : plus lu au-dela de cette ligne
    _levelWidth = levelRef.tileMap().width();
    _levelHeight = levelRef.tileMap().height();
    // Cadrage de camera (LOT-64) : copie une fois, resolu par LevelLoader -- jamais recalcule
    // ici, la regle de repli vit a un seul endroit (core::resolveCameraFraming).
    _cameraFraming = levelRef.cameraFraming();
    // Partition en salles (LOT-32), a la taille RESOLUE du niveau (LOT-64 : reglable, valeurs par
    // defaut sinon) : reconstruite pour ce niveau, camera immediatement cadree sur la salle de
    // l'ENTREE (pas de salle "en retard" d'une frame apres un (re)chargement) -- meme si le mode
    // retenu n'est pas "par salle", la partition reste bon marche a construire et sert de repere
    // a l'editeur (TACHE-03).
    _roomGrid.emplace(_levelWidth, _levelHeight,
                      _cameraFraming.roomWidthTiles.value_or(core::kDefaultRoomWidthTiles),
                      _cameraFraming.roomHeightTiles.value_or(core::kDefaultRoomHeightTiles));
    _currentRoomIndex = _roomGrid->roomIndexAt(levelRef.entry());
    // Caméra de suivi (LOT-64 TACHE-02) : réinitialisée à chaque chargement, elle démarrera sur le
    // personnage à l'entrée dès le premier `update()` (état non initialisé).
    _followCameraState = FollowCameraState{};
    _previousFollowCenter = core::Vector2{static_cast<float>(levelRef.entry().column) + 0.5f,
                                          static_cast<float>(levelRef.entry().row) + 0.5f};
    // Zone de camera active (mode par salle avec zones dessinees a la main, EX-LVL-007) : resolue
    // ici comme _currentRoomIndex ci-dessus, pour ne pas dependre d'un premier update() avant le
    // premier rendu.
    _currentZoneIndex = _cameraFraming.zones.empty()
                            ? std::nullopt
                            : activeCameraZoneIndex(_cameraFraming.zones, levelRef.entry());
    switch (_cameraFraming.mode) {
        case core::CameraFramingMode::WholeLevel:
            centerCameraOnWholeLevel();
            break;
        case core::CameraFramingMode::PerRoom:
            if (!_cameraFraming.zones.empty()) {
                if (_currentZoneIndex) {
                    centerCameraOnZone(_cameraFraming.zones[*_currentZoneIndex]);
                } else {
                    centerCameraOnWholeLevel();
                }
            } else {
                centerCameraOnRoom(_currentRoomIndex);
            }
            break;
        case core::CameraFramingMode::Follow:
            // Amorcé au premier update() ; place un centre raisonnable (l'entrée) pour un
            // éventuel rendu avant le premier pas fixe (chargement à mi-frame).
            _camera.setCenter(_previousFollowCenter);
            break;
    }
    // La correspondance type -> region d'atlas (rendu) est injectee dans la projection pure, et
    // chaque entite tuile recoit sa marque d'habillage (type + voisinage solide, LOT-42). Le
    // masque ne depend que de la grille du niveau : le calculer ici, une fois, evite de le
    // refaire a chaque image sans rendre la scene dependante du mode de rendu.
    const core::TileMap& sceneMap = levelRef.tileMap();
    core::buildLevelScene(
        _world, levelRef, [this](core::TileType type) { return regionForTile(type); },
        [this, &sceneMap, &levelRef](core::Entity entity, core::TileType type, int column,
                                     int row) {
            _world.addComponent(entity,
                                TileSkinTag{type, solidNeighborMask(sceneMap, column, row),
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
    // Plateformes mobiles (EX-GP-026, LOT-63) : une entite-tuile par plateforme, reperee a sa
    // position de DEPART -- meme patron que les dangers mobiles/blocs ci-dessus (position
    // continue, donc interpolee au rendu via PreviousPosition).
    _platforms.emplace(levelRef);
    _platformEntities.clear();
    for (const core::MovingPlatformConfig& config : levelRef.platformConfigs()) {
        core::Entity platformEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found &&
                    static_cast<int>(transform.position.x) == config.startPosition.column &&
                    static_cast<int>(transform.position.y) == config.startPosition.row) {
                    platformEntity = entity;
                    found = true;
                }
            });
        if (found && _world.hasComponent<core::Transform>(platformEntity)) {
            _world.addComponent(
                platformEntity,
                PreviousPosition{_world.getComponent<core::Transform>(platformEntity).position});
        }
        _platformEntities.push_back(platformEntity);
    }
    // Capacites du tableau (EX-GP-055) : un niveau peut redefinir le nombre de sauts aeriens et de
    // charges de dash, tous deux recharges au contact du sol -- a distinguer des BUDGETS poses sur
    // le personnage dans spawnPlayer, qui se consomment une fois pour toutes. Sans champ, on garde
    // les reglages du moteur. Applique AVANT spawnPlayer, dont la recharge initiale en depend.
    core::PhysicsConfig physicsConfig;
    if (levelRef.airJumps()) {
        physicsConfig.airJumps = *levelRef.airJumps();
    }
    if (levelRef.dashCharges()) {
        physicsConfig.dashCharges = *levelRef.dashCharges();
    }
    _physics.setConfig(physicsConfig);

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
    // Total consommable sur tout le tableau, distinct des CAPACITES rechargees a chaque
    // atterrissage (EX-GP-055), portees par la PhysicsConfig posee dans loadLevel.
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
void GameSession::applyMechanismVisual(core::Entity entity, bool active,
                                       MechanismVisualState& state, const SceneTextures& textures,
                                       float fixedDelta) {
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
    // Axes skin/surcharge (LOT-51) tous deux visibles par defaut : en mode compose, une valeur est
    // TOUJOURS renvoyee. GameSession n'expose pas ces axes -- ce sont des outils d'inspection de
    // l'editeur, sans objet en jeu.
    const TileAppearance appearance =
        resolveTileAppearance(RenderMode::Texture, sprite.region, &tag, textures).value();

    std::string assetPath;
    int width = 0;
    int height = 0;
    if (appearance.source == AppearanceSource::Skin) {
        const SkinTexture& skin = textures.skins[static_cast<std::size_t>(appearance.skinIndex)];
        assetPath = SKINS_SUBDIRECTORY + skin.asset;
        width = skin.width;
        height = skin.height;
    } else if (appearance.source == AppearanceSource::Override) {
        const SkinTexture& object =
            textures.objects[static_cast<std::size_t>(appearance.skinIndex)];
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
        applyMechanismVisual(_doorEntities[index], _mechanisms->isDoorOpen(index),
                             _doorVisuals[index], textures, fixedDelta);
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
        applyMechanismVisual(_dangerBlinkEntities[index], active, _dangerBlinkVisuals[index],
                             textures, fixedDelta);
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

void GameSession::resolveReducedBlockCollision(const core::Aabb& previousBox) {
    core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Vector2 delta = transform.position - previousBox.min;
    if (delta.x == 0.0f && delta.y == 0.0f) {
        return;
    }
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
    if (bestNormal.x == 0.0f && bestNormal.y == 0.0f) {
        return;
    }
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

void GameSession::refreshPlatformVisuals() {
    for (std::size_t index = 0; index < _platformEntities.size(); ++index) {
        const core::Entity platform = _platformEntities[index];
        if (!_world.hasComponent<core::Transform>(platform)) {
            continue;  // entite-tuile non reperee (robustesse) : rien a faire
        }
        core::Transform& transform = _world.getComponent<core::Transform>(platform);
        transform.position = _platforms->boxAt(index).min;
    }
}

std::vector<core::Aabb> GameSession::collectActiveDangerBoxes() {
    std::vector<core::Aabb> boxes;
    boxes.reserve(_dangers->moverCount() + _level->blinkConfigs().size() +
                  _level->dangerLinks().size() + 1);

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
    // Ecrasement par une plateforme mobile (EX-GP-026, LOT-63) : decision de cadrage -- mortel,
    // signale par core::CharacterPhysicsSystem via Player::squished. La boite du personnage
    // lui-meme suffit a declencher Lost via evaluateOutcome, sans nouvelle notion de danger.
    // Ecrasement par une PORTE qui se referme (EX-GP-021, LOT-65 TACHE-06) : meme decision et meme
    // traduction que ci-dessus. Sans cela, le personnage reste encastre dans un mur sans echec
    // possible -- la « situation sans issue » que la conception des niveaux interdit.
    if (_world.hasComponent<core::Player>(_player) &&
        (_world.getComponent<core::Player>(_player).squished || _mechanisms->crushedPlayer())) {
        const core::Transform& squishedTransform = _world.getComponent<core::Transform>(_player);
        const core::Collider& squishedCollider = _world.getComponent<core::Collider>(_player);
        boxes.push_back(
            core::Aabb::fromTopLeftSize(squishedTransform.position, squishedCollider.size));
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
    const core::AtlasRegion proceduralRegion =
        _atlas.playerFrameRegion(proceduralKind, proceduralFrame);
    sprite.region = proceduralRegion;

    core::AtlasRegion textureRegion = proceduralRegion;
    bool usesCharacterSheet = false;
    core::Vector2 imageSizePixels{static_cast<float>(TextureAtlas::PLAYER_FRAME_SIZE),
                                  static_cast<float>(TextureAtlas::PLAYER_FRAME_SIZE)};

    if (const LoadedTexture* sheet =
            _cache.get(PLAYER_SUBDIRECTORY + PLAYER_SHEET_FILE_NAME, AssetFamily::CharacterSheet)) {
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
    // Meme principe pour la camera de suivi (LOT-64 TACHE-02) : fige le centre du pas PRECEDENT
    // avant que ce pas ne le fasse avancer (updateFollowCamera), pour que render() interpole entre
    // les deux comme il le fait deja pour chaque PreviousPosition.
    _previousFollowCenter = _followCameraState.center;
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

// Centre la camera sur le niveau entier (mode WholeLevel, LOT-64) : centre fixe, pose une fois au
// chargement -- symetrique a centerCameraOnRoom, jamais recalcule au pas fixe (voir en-tete).
void GameSession::centerCameraOnWholeLevel() {
    _camera.setCenter(core::Vector2{static_cast<float>(_levelWidth) * 0.5f,
                                    static_cast<float>(_levelHeight) * 0.5f});
}

// Centre la camera sur une zone dessinee a la main (EX-LVL-007, voir en-tete).
void GameSession::centerCameraOnZone(const core::CameraZone& zone) {
    _camera.setCenter(
        core::Vector2{static_cast<float>(zone.x) + static_cast<float>(zone.width) * 0.5f,
                      static_cast<float>(zone.y) + static_cast<float>(zone.height) * 0.5f});
}

// Equivalent de updateCurrentRoom pour les zones dessinees a la main (EX-LVL-007, voir en-tete).
void GameSession::updateCurrentCameraZone() {
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Vector2 center = transform.position + collider.size * 0.5f;
    const core::GridPosition tile{static_cast<int>(std::floor(center.x)),
                                  static_cast<int>(std::floor(center.y))};
    const std::optional<std::size_t> zoneIndex = activeCameraZoneIndex(_cameraFraming.zones, tile);
    if (zoneIndex == _currentZoneIndex) {
        return;
    }
    _currentZoneIndex = zoneIndex;
    if (zoneIndex) {
        centerCameraOnZone(_cameraFraming.zones[*zoneIndex]);
    } else {
        // Aucune zone ne contient le personnage (trou entre les zones dessinees par l'auteur) :
        // repli sur le niveau entier, jamais un etat indefini.
        centerCameraOnWholeLevel();
    }
}

// Avance la camera de suivi d'un pas fixe (mode Follow, LOT-64 TACHE-02, voir en-tete).
void GameSession::updateFollowCamera(float fixedDelta) {
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    // Position SIMULEE du personnage (pas interpolee) : le suivi (zone morte, anticipation,
    // lissage) doit rester deterministe au pas fixe (EX-NFR-002) -- c'est render() qui interpole
    // le CENTRE DE CAMERA resultant entre deux pas, exactement comme il interpole deja la position
    // affichee du personnage (PreviousPosition). Melanger les deux ici desynchroniserait le suivi
    // de la simulation qu'il est cense suivre.
    const core::Vector2 characterCenter = transform.position + collider.size * 0.5f;
    const core::Player& player = _world.getComponent<core::Player>(_player);
    const core::Rect levelBounds{
        core::Vector2{0.0f, 0.0f},
        core::Vector2{static_cast<float>(_levelWidth), static_cast<float>(_levelHeight)}};
    // Cadrage de la camera de suivi : taille reglable par niveau (EX-REN-017, memes champs que la
    // taille de salle du mode par salle -- valeur par defaut si non declaree).
    const core::Vector2 viewHalfExtent{
        static_cast<float>(_cameraFraming.roomWidthTiles.value_or(core::kDefaultRoomWidthTiles)) *
            0.5f,
        static_cast<float>(_cameraFraming.roomHeightTiles.value_or(core::kDefaultRoomHeightTiles)) *
            0.5f};
    _followCameraState = advanceFollowCamera(_followCameraState, characterCenter, player.facing,
                                             levelBounds, viewHalfExtent, fixedDelta);
}

// Selectionne le zoom et le centre effectifs de _camera selon le mode de cadrage resolu (LOT-64,
// voir en-tete). Point d'application UNIQUE du cadrage : les trois modes y sont traites a plat,
// aucune regle de cadrage n'est dupliquee ailleurs dans render().
void GameSession::applyCameraFraming(int viewportWidth, int viewportHeight,
                                     float interpolationAlpha) {
    switch (_cameraFraming.mode) {
        case core::CameraFramingMode::WholeLevel: {
            const float zoom = Camera2D::fitZoom(
                static_cast<float>(viewportWidth), static_cast<float>(viewportHeight),
                static_cast<float>(_levelWidth), static_cast<float>(_levelHeight),
                CAMERA_FIT_MARGIN);
            _camera.setZoom(zoom);
            // Centre deja pose une fois au chargement (centerCameraOnWholeLevel) : fixe, rien a
            // refaire ici.
            break;
        }
        case core::CameraFramingMode::PerRoom: {
            // Rectangle a cadrer : la zone dessinee a la main active (EX-LVL-007), le niveau entier
            // en repli (aucune zone ne contient le personnage), ou la salle de la grille
            // automatique -- selon que _cameraFraming.zones est non vide ou non. Le centre, lui,
            // est deja pose par updateCurrentRoom/updateCurrentCameraZone au franchissement d'une
            // frontiere : rien a refaire ici.
            int width = 0;
            int height = 0;
            if (!_cameraFraming.zones.empty()) {
                if (_currentZoneIndex) {
                    const core::CameraZone& zone = _cameraFraming.zones[*_currentZoneIndex];
                    width = zone.width;
                    height = zone.height;
                } else {
                    width = _levelWidth;
                    height = _levelHeight;
                }
            } else {
                const RoomBounds roomBounds = _roomGrid->roomBounds(_currentRoomIndex);
                width = roomBounds.width;
                height = roomBounds.height;
            }
            const float zoom = Camera2D::fitZoom(
                static_cast<float>(viewportWidth), static_cast<float>(viewportHeight),
                static_cast<float>(width), static_cast<float>(height), CAMERA_FIT_MARGIN);
            _camera.setZoom(zoom);
            break;
        }
        case core::CameraFramingMode::Follow: {
            const float zoom = Camera2D::fitZoom(
                static_cast<float>(viewportWidth), static_cast<float>(viewportHeight),
                static_cast<float>(
                    _cameraFraming.roomWidthTiles.value_or(core::kDefaultRoomWidthTiles)),
                static_cast<float>(
                    _cameraFraming.roomHeightTiles.value_or(core::kDefaultRoomHeightTiles)),
                CAMERA_FIT_MARGIN);
            _camera.setZoom(zoom);
            // Interpole entre le centre du pas fixe PRECEDENT et celui du pas COURANT, comme
            // chaque entite interpole entre PreviousPosition et sa position simulee (EX-ARCH-031)
            // -- sans quoi le personnage, rendu lisse, tremblerait par rapport au contenu cale
            // sur un centre qui ne bouge que par sauts discrets (tache-02, piege documente).
            const core::Vector2 interpolated =
                _previousFollowCenter +
                (_followCameraState.center - _previousFollowCenter) * interpolationAlpha;
            // Alignement pixel (EX-ARCH-022) : sans lui, un centre de camera fractionnaire
            // echantillonne chaque texture entre deux texels et rend tout le pixel art flou
            // (hmi::roundToScreenPixel).
            const float pixelsPerWorldUnit = Camera2D::PIXELS_PER_UNIT * zoom;
            _camera.setCenter(hmi::roundToScreenPixel(interpolated, pixelsPerWorldUnit));
            break;
        }
    }
}

// Simule le personnage d'un pas fixe (mecanismes + physique + animation), puis statue sur l'issue.
core::LevelOutcome GameSession::update(const InputState& input, float fixedDelta) {
    return update(toPlayerInput(input, _gameBindings, _gamepadBindings), fixedDelta);
}

core::LevelOutcome GameSession::update(const core::PlayerInput& intent, float fixedDelta) {
    if (!_level) {
        return core::LevelOutcome::Playing;  // chargement echoue : rien a simuler (etat neutre)
    }

    // 0. Interpolation (EX-ARCH-031) : fige la position COURANTE de chaque entite mobile comme sa
    //    position "precedente" AVANT que ce pas ne la modifie (voir render()).
    snapshotPreviousPositions();

    // 0bis. Particules (LOT-53 TACHE-01/02) : avance celles emises aux pas precedents AVANT que
    // ce pas n'en emette de nouvelles (age puis emet, jamais l'inverse -- une particule tout
    // juste emise ce pas ne doit pas etre vieillie du meme coup).
    _particles.update(_world, fixedDelta);
    // 0ter. Secousse d'ecran (LOT-53 TACHE-03) : decroissance au pas fixe, comme les particules.
    advanceScreenShake(_screenShake, fixedDelta);

    // 1bis. Plateformes mobiles (EX-GP-026, LOT-63) : deplacees EN PREMIER (ordre de resolution
    // documente, TACHE-03) -- portage du personnage et des blocs, puis leur propre physique,
    // consomment sa position DEJA a jour pour ce pas.
    _platforms->update();
    refreshPlatformVisuals();
    const std::vector<core::PlatformSample>& platformSamples = _platforms->samples();

    // 1ter. Blocs poussables (EX-GP-022) : poussee puis chute, resolues AVANT la physique du
    // personnage, avec sa boite TELLE QUE LAISSEE par le pas precedent.
    const core::Transform& previousTransform = _world.getComponent<core::Transform>(_player);
    const core::Collider& previousCollider = _world.getComponent<core::Collider>(_player);
    const core::Aabb previousBox =
        core::Aabb::fromTopLeftSize(previousTransform.position, previousCollider.size);
    _blocks->update(previousBox, intent.moveX, _mechanisms->collisionMap(), platformSamples);
    refreshBlockVisuals();

    // 2. Physique sur la grille des MECANISMES (portes fermees = solides) completee par la position
    //    COURANTE des blocs (resolue ci-dessus).
    const core::TileMap collision = _blocks->collisionMap(_mechanisms->collisionMap());
    // Vitesse verticale AVANT ce pas (LOT-53 TACHE-02) : la physique remet velocity.y a zero au
    // contact du sol -- c'est donc la derniere valeur disponible qui approxime la vitesse
    // d'impact d'un atterrissage survenant CE pas (intensite de la poussiere, cf. plus bas).
    const float previousVerticalVelocity = _world.getComponent<core::Velocity>(_player).value.y;
    _physics.update(_world, collision, intent, fixedDelta, platformSamples);

    // 2a. Détection d'événements (LOT-60 TACHE-03) : l'état du personnage est figé par la
    // physique ci-dessus, avant toute autre système -- c'est l'instantané pertinent pour la
    // détection, prise une seule fois par pas fixe (jamais par image de rendu, EX-REN-021).
    const PlayerEventState currentPlayerEventState =
        PlayerEventState::capture(_world.getComponent<core::Player>(_player));

    // 2bis. Blocs a TAILLE REDUITE (EX-GP-005) : leur boite REELLE (centree, plus petite qu'une
    // case) n'est jamais posee dans `collision` ci-dessus. Composee ici via un balayage boite-boite
    // dedie (core::sweepAabbVsAabb), sur le deplacement REEL obtenu par la physique sur grille.
    resolveReducedBlockCollision(previousBox);

    // 2ter. Animation (EX-REN-012) : derivee de l'etat physique qui vient d'etre mis a jour.
    _animation.update(_world, fixedDelta);
    // 2ter bis. Tuiles animees (LOT-46 TACHE-05) : horloge partagee par asset, au meme pas fixe
    // (jamais au rythme du rendu, pour rester deterministe -- EX-NFR-002).
    updateTileAnimations(fixedDelta);

    // 3. Boite du personnage apres deplacement.
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, collider.size);

    // 3bis. Camera : selon le mode de cadrage resolu du niveau (LOT-64). Bascule de salle nette
    // (LOT-32, EX-REN-015) en mode par salle ; centre fixe en mode niveau entier (pose au
    // chargement, rien a refaire ici) ; suivi avance au pas fixe en mode suivi.
    switch (_cameraFraming.mode) {
        case core::CameraFramingMode::WholeLevel:
            break;
        case core::CameraFramingMode::PerRoom:
            if (!_cameraFraming.zones.empty()) {
                updateCurrentCameraZone();
            } else {
                updateCurrentRoom();
            }
            break;
        case core::CameraFramingMode::Follow:
            updateFollowCamera(fixedDelta);
            break;
    }

    // 4. Mecanismes : contact interrupteurs (front) / poids sur plaque (continu) -> etat des
    // portes. Les blocs poussables comptent comme des poids (EX-GP-025, LOT-65 TACHE-06) : c'est ce
    // qui permet d'en poser un sur une plaque et de repartir, la porte restant ouverte.
    const float playerMass = _world.getComponent<core::Player>(_player).mass;
    std::vector<core::TriggerWeight> blockWeights;
    blockWeights.reserve(_blocks->positions().size());
    for (std::size_t index = 0; index < _blocks->positions().size(); ++index) {
        blockWeights.push_back(
            core::TriggerWeight{.box = _blocks->boxAt(index), .mass = _blocks->massAt(index)});
    }
    _mechanisms->update(box, playerMass, intent.interactPressed, blockWeights);

    // 4a. Détection d'événements, suite de 2a : mécanismes désormais à jour, personnage déjà
    // capturé plus haut (avant que les mécanismes ne puissent influer sur la boîte de collision).
    const MechanismEventState currentMechanismEventState =
        MechanismEventState::capture(*_mechanisms);
    _lastStepEvents.clear();
    if (_gameEventsInitialized) {
        const std::vector<GameEvent> playerEvents =
            detectPlayerEvents(_previousPlayerEventState, currentPlayerEventState);
        _lastStepEvents.insert(_lastStepEvents.end(), playerEvents.begin(), playerEvents.end());

        std::vector<bool> continuousMechanisms;
        continuousMechanisms.reserve(_mechanisms->mechanisms().size());
        for (std::size_t index = 0; index < _mechanisms->mechanisms().size(); ++index) {
            continuousMechanisms.push_back(_mechanisms->isContinuous(index));
        }
        const std::vector<GameEvent> mechanismEvents = detectMechanismEvents(
            _previousMechanismEventState, currentMechanismEventState, continuousMechanisms);
        _lastStepEvents.insert(_lastStepEvents.end(), mechanismEvents.begin(),
                               mechanismEvents.end());
    } else {
        // Tout premier pas apres un (re)chargement : rejoint l'etat courant sans transition, meme
        // principe que MechanismVisualState::initialized (LOT-47).
        _gameEventsInitialized = true;
    }
    _previousPlayerEventState = currentPlayerEventState;
    _previousMechanismEventState = currentMechanismEventState;

    // 4a bis. Particules du personnage (LOT-53 TACHE-02) : trainee de dash CONTINUE (etat
    // COURANT du minuteur, pas un evenement -- emise a chaque pas ou le dash est actif, jamais
    // seulement a son declenchement) et bouffee de poussiere sur l'evenement Landed de ce pas
    // (intensite fonction de previousVerticalVelocity, capturee avant que la physique ne la
    // remette a zero). Reutilise _lastStepEvents deja detecte ci-dessus, sans reimplementer la
    // detection de transition (deja faite deux fois dans le projet, LOT-47/LOT-60).
    {
        const core::Player& particlePlayer = _world.getComponent<core::Player>(_player);
        if (particlePlayer.dashTimer > 0.0f) {
            _particles.emitDashTrail(_world, transform.position, particlePlayer.facing);
        }
        const bool landedThisStep = std::find(_lastStepEvents.begin(), _lastStepEvents.end(),
                                              GameEvent::Landed) != _lastStepEvents.end();
        if (landedThisStep) {
            const float impactSpeed = (std::max)(0.0f, previousVerticalVelocity);
            _particles.emitLanding(_world, transform.position, impactSpeed);
            // Secousse (LOT-53 TACHE-03) reservee a l'atterrissage LOURD : reutilise le seuil
            // d'intensite maximale de la poussiere (core::LANDING_MAX_IMPACT_SPEED) plutot que
            // d'introduire un second seuil -- un atterrissage qui sature deja l'intensite de la
            // poussiere est, par construction, un atterrissage lourd.
            if (impactSpeed >= core::LANDING_MAX_IMPACT_SPEED) {
                triggerScreenShake(_screenShake, LANDING_SHAKE_AMPLITUDE_PIXELS,
                                   SCREEN_SHAKE_DURATION);
            }
        }
    }

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
    // Evenement d'issue (LOT-60 TACHE-03) : calcule et memorise AVANT reload(), qui remet le
    // personnage et les mecanismes a l'etat d'entree -- apres, "Died" ne serait plus observable.
    if (const std::optional<GameEvent> outcomeEvent = detectOutcomeEvent(outcome)) {
        _lastStepEvents.push_back(*outcomeEvent);
    }
    if (outcome == core::LevelOutcome::Lost) {
        // Eclatement a la mort (LOT-53 TACHE-02) : emis AVANT reload(), pour la meme raison que
        // l'evenement Died ci-dessus -- reload() remet le personnage a l'entree, la position de
        // mort ne serait plus observable apres.
        _particles.emitDeath(_world, (box.min + box.max) * 0.5f);
        triggerScreenShake(_screenShake, DEATH_SHAKE_AMPLITUDE_PIXELS, SCREEN_SHAKE_DURATION);
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
    applyCameraFraming(viewportWidth, viewportHeight, interpolationAlpha);
    // Secousse d'ecran (LOT-53 TACHE-03) : decalage courant applique a la CAMERA DE RENDU
    // uniquement (Camera2D::setShakeOffsetPixels), jamais a _center -- sans effet sur le culling
    // ni sur la logique de cadrage (par salle ou de suivi, toutes deux pilotees par la position du
    // personnage, jamais par la secousse).
    _camera.setShakeOffsetPixels(screenShakeOffset(_screenShake));

    // Interpolation de rendu (EX-ARCH-031) entre le pas precedent et le pas courant. La grille de
    // collision des mecanismes (portes) tranche l'ombre d'une porte d'apres son etat COURANT
    // (LOT-55) : TileSkinTag::type reste TileType::Door quel que soit cet etat.
    _renderer.render(_world, _camera, mode, interpolationAlpha, _level->background(), _levelWidth,
                     _levelHeight, _level->textureOverrides(), _tileAnimations, _level->planes(),
                     hmi::planeParallaxActive(_cameraFraming.mode, _level->parallaxEnabled()),
                     _mechanisms ? &_mechanisms->collisionMap() : nullptr);

    renderHud(viewportWidth, viewportHeight);
}

// Compose et soumet l'affichage tete haute, en espace ecran (voir en-tete).
void GameSession::renderHud(int viewportWidth, int viewportHeight) {
    if (_localization == nullptr) {
        return;  // catalogue pas encore charge (demarrage) : pas de HUD plutot qu'un plantage.
    }

    constexpr float HUD_MARGIN = 8.0f;
    constexpr float HUD_SCALE = 1.0f;
    constexpr float HUD_LINE_SPACING = 2.0f;
    // Ombre portee (decalage d'un pixel, noir semi-opaque) : contraste suffisant sur un fond
    // clair comme sur un fond sombre, le fond de niveau etant libre (TACHE-03).
    constexpr core::Color HUD_SHADOW_COLOR{0.0f, 0.0f, 0.0f, 0.75f};
    constexpr core::Color HUD_TEXT_COLOR{1.0f, 1.0f, 1.0f, 1.0f};

    const core::Player& player = _world.getComponent<core::Player>(_player);

    // Le personnage touche-t-il une cle non ramassee (EX-GP-023, LOT-65 TACHE-07) ? La porte
    // qu'ouvre une cle reste FERMEE tant que celle-ci n'est pas ramassee : `isDoorOpen` vaut donc
    // « cle deja prise », et une cle consommee ne doit plus rien afficher.
    const core::Transform& hudTransform = _world.getComponent<core::Transform>(_player);
    const core::Collider& hudCollider = _world.getComponent<core::Collider>(_player);
    const core::Aabb hudBox = core::Aabb::fromTopLeftSize(hudTransform.position, hudCollider.size);
    bool overlappingKey = false;
    for (std::size_t index = 0; index < _mechanisms->mechanisms().size(); ++index) {
        if (!_mechanisms->isKey(index) || _mechanisms->isDoorOpen(index)) {
            continue;
        }
        const core::GridPosition cell = _mechanisms->mechanisms()[index].switchPosition;
        const auto left = static_cast<float>(cell.column);
        const auto top = static_cast<float>(cell.row);
        if (hudBox.min.x < left + 1.0f && hudBox.max.x > left && hudBox.min.y < top + 1.0f &&
            hudBox.max.y > top) {
            overlappingKey = true;
            break;
        }
    }

    const std::vector<std::string> lines =
        gameHudLines(player, _level->name(), *_localization, overlappingKey);

    _hudScene.clear();
    float lineY = HUD_MARGIN;
    for (const std::string& line : lines) {
        composeText(_hudScene, _font, line, HUD_MARGIN + 1.0f, lineY + 1.0f, HUD_SCALE,
                    HUD_SHADOW_COLOR);
        composeText(_hudScene, _font, line, HUD_MARGIN, lineY, HUD_SCALE, HUD_TEXT_COLOR);
        lineY += static_cast<float>(_font.metrics().lineHeight) * HUD_SCALE + HUD_LINE_SPACING;
    }
    _hudScene.sort();
    submitComposedScene(_batch, screenProjectionMatrix(viewportWidth, viewportHeight), _hudScene);
}

}  // namespace hmi
