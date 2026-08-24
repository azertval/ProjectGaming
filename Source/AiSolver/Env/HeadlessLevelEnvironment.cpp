// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/HeadlessLevelEnvironment.h"

#include <cmath>
#include <cstddef>

#include "Core/Diagnostics/Assert.h"
#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Levels/DangerGeometry.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/AabbVsAabb.h"
#include "Core/Physics/PhysicsConfig.h"
#include "Core/Physics/PlayerSpawn.h"

namespace aisolver {

namespace {
// Pas fixe déterministe (EX-NFR-002), même constante que STEP dans
// Source/Test/Systeme/test_parcours_complet.cpp et hmi::GameSession (LOT-33).
constexpr float kFixedDelta = 1.0f / 60.0f;
}  // namespace

HeadlessLevelEnvironment::HeadlessLevelEnvironment(EnvironmentConfig config) : _config(config) {}

bool HeadlessLevelEnvironment::reset(const std::filesystem::path& levelPath) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(levelPath);
    if (!loaded.ok()) {
        // Échec récupérable (EX-NFR-040) : état neutre, rien de chargé.
        _loadError = loaded.error;
        _level.reset();
        _physics.reset();
        _blocks.reset();
        _mechanisms.reset();
        _dangers.reset();
        _platforms.reset();
        return false;
    }
    _loadError.clear();
    _world = core::World{};  // monde vierge (aucune entite du niveau precedent), comme GameSession
    _level = loaded.level;
    const core::Level& levelRef = *_level;

    // Capacites du tableau (EX-GP-055), avant le spawn dont la recharge initiale en depend --
    // meme ordre que hmi::GameSession::loadLevel.
    core::PhysicsConfig physicsConfig;
    if (levelRef.airJumps()) {
        physicsConfig.airJumps = *levelRef.airJumps();
    }
    if (levelRef.dashCharges()) {
        physicsConfig.dashCharges = *levelRef.dashCharges();
    }
    _physics.emplace(physicsConfig);
    _blocks.emplace(levelRef);
    _mechanisms.emplace(levelRef);
    _dangers.emplace(levelRef);
    _platforms.emplace(levelRef);

    // Entite joueur : memes quatre composants que spawn() dans test_parcours_complet.cpp, aucun
    // Sprite ni Animation (HeadlessLevelEnvironment ne rend jamais rien).
    _player = _world.createEntity();
    const core::Vector2 size = core::playerSize();
    const core::GridPosition entry = levelRef.entry();
    _world.addComponent(
        _player, core::Transform{core::playerSpawnPosition(entry.column, entry.row), size, 0.0f});
    _world.addComponent(_player, core::Velocity{});
    _world.addComponent(_player, core::Collider{size});
    core::Player playerComponent;
    // Budget de mouvements du tableau (EX-GP-024) : -1 = illimite si le niveau n'en fixe pas.
    playerComponent.jumpsRemaining = levelRef.jumpBudget();
    playerComponent.dashesRemaining = levelRef.dashBudget();
    _world.addComponent(_player, playerComponent);

    _stepIndex = 0;
    _stepsSinceProgress = 0;
    const core::Aabb spawnBox =
        core::Aabb::fromTopLeftSize(core::playerSpawnPosition(entry.column, entry.row), size);
    const core::Vector2 spawnCenter = (spawnBox.min + spawnBox.max) * 0.5f;
    const core::GridPosition exit = levelRef.exit();
    const core::Vector2 exitCenter{static_cast<float>(exit.column) + 0.5f,
                                   static_cast<float>(exit.row) + 0.5f};
    _bestDistanceToExit = (spawnCenter - exitCenter).length();

    return true;
}

bool HeadlessLevelEnvironment::loaded() const {
    return _level.has_value();
}

const std::string& HeadlessLevelEnvironment::loadError() const {
    return _loadError;
}

const core::Level& HeadlessLevelEnvironment::level() const {
    PROJECTGAMING_ASSERT(loaded(), "level() appele sans reset() reussi au prealable");
    return *_level;
}

const core::MechanismController& HeadlessLevelEnvironment::mechanisms() const noexcept {
    PROJECTGAMING_ASSERT(loaded(), "mechanisms() appele sans reset() reussi au prealable");
    return *_mechanisms;
}

const core::DangerController& HeadlessLevelEnvironment::dangers() const noexcept {
    PROJECTGAMING_ASSERT(loaded(), "dangers() appele sans reset() reussi au prealable");
    return *_dangers;
}

bool HeadlessLevelEnvironment::budgetExhausted() const noexcept {
    return _stepIndex >= _config.maxSteps;
}

int HeadlessLevelEnvironment::stepsSinceProgress() const noexcept {
    return _stepsSinceProgress;
}

float HeadlessLevelEnvironment::bestDistanceToExit() const noexcept {
    return _bestDistanceToExit;
}

std::vector<core::Aabb> HeadlessLevelEnvironment::collectActiveDangerBoxes() const {
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

void HeadlessLevelEnvironment::updateProgress(const core::Aabb& playerBox) {
    const core::Vector2 center = (playerBox.min + playerBox.max) * 0.5f;
    const core::GridPosition exit = _level->exit();
    const core::Vector2 exitCenter{static_cast<float>(exit.column) + 0.5f,
                                   static_cast<float>(exit.row) + 0.5f};
    const float distance = (center - exitCenter).length();
    if (_bestDistanceToExit - distance > _config.progressEpsilon) {
        _stepsSinceProgress = 0;
    } else {
        ++_stepsSinceProgress;
    }
    _bestDistanceToExit = (std::min)(_bestDistanceToExit, distance);
}

StepObservation HeadlessLevelEnvironment::step(const core::PlayerInput& input) {
    PROJECTGAMING_ASSERT(loaded(), "step() appele sans reset() reussi au prealable");
    PROJECTGAMING_ASSERT(!budgetExhausted(), "step() appele au-dela du budget de pas");

    // 1. Boite AVANT tout deplacement de ce pas (previousBox), capturee avant que quoi que ce soit
    //    ne bouge -- piege documente dans playLevel()/GameSession::update : la poussee d'un bloc
    //    doit voir la position du personnage telle que laissee par le pas precedent.
    const core::Transform& previousTransform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Aabb previousBox =
        core::Aabb::fromTopLeftSize(previousTransform.position, collider.size);

    // 1bis. Plateformes mobiles (EX-GP-026) : deplacees EN PREMIER, comme hmi::GameSession::update
    // et playLevelTraced() (test_parcours_complet.cpp) -- absentes du perimetre ECRIT de ce lot
    // (redige avant LOT-63), mais necessaires a la fidelite pas-a-pas garantie par TACHE-05 : les
    // deux orchestrations de reference les composent deja.
    _platforms->update();
    const std::vector<core::PlatformSample> platformSamples = _platforms->samples();

    // 2. Mecanismes (lecture de la grille de collision) -> blocs (poussee/chute), avec la boite du
    //    personnage AVANT son propre deplacement de ce pas.
    const core::TileMap mechanismMap = _mechanisms->collisionMap();
    _blocks->update(previousBox, input.moveX, mechanismMap, platformSamples);

    // 3. Physique du personnage sur la grille des mecanismes completee par les blocs.
    const core::TileMap collision = _blocks->collisionMap(mechanismMap);
    _physics->update(_world, collision, input, kFixedDelta, platformSamples);

    // 4. Sweep boite-boite des blocs a taille reduite (EX-GP-005) : leur boite reelle n'est jamais
    //    posee sur la grille de collision ci-dessus, composee ici sur le deplacement REEL obtenu
    //    par la physique sur grille.
    core::Transform& transform = _world.getComponent<core::Transform>(_player);
    core::Velocity& velocity = _world.getComponent<core::Velocity>(_player);
    core::Player& player = _world.getComponent<core::Player>(_player);
    const core::Vector2 delta = transform.position - previousBox.min;
    core::Vector2 bestPosition = transform.position;
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

    // 5. Boite finale du personnage apres deplacement + sweep.
    const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, collider.size);

    // 6. Mecanismes : etat + grille, boite ACTUELLE (pas previousBox) et masse du personnage
    //    (EX-GP-019/025), poids des blocs poussables sur les plaques (EX-GP-025).
    std::vector<core::TriggerWeight> blockWeights;
    blockWeights.reserve(_blocks->positions().size());
    for (std::size_t index = 0; index < _blocks->positions().size(); ++index) {
        blockWeights.push_back(
            core::TriggerWeight{.box = _blocks->boxAt(index), .mass = _blocks->massAt(index)});
    }
    _mechanisms->update(box, player.mass, input.interactPressed, blockWeights);

    // 7. Dangers (EX-GP-051/053) : avance le compteur de pas fixes des dangers mobile/temporise.
    //    Present ici a la difference de l'ancien playLevel() (voir decision de cadrage de l'epic) :
    //    un environnement d'entrainement doit pouvoir faire mourir un agent qui s'aventure dans une
    //    alcove a danger mobile/commute/temporise.
    _dangers->update();

    std::vector<core::Aabb> extraDangerBoxes = collectActiveDangerBoxes();
    // Ecrasement par une plateforme mobile (EX-GP-026) ou par une porte qui se referme (EX-GP-021)
    // : mortel, meme traduction que hmi::GameSession::collectActiveDangerBoxes.
    if (player.squished || _mechanisms->crushedPlayer()) {
        extraDangerBoxes.push_back(box);
    }

    const core::LevelOutcome outcome = core::evaluateOutcome(box, *_level, extraDangerBoxes);

    ++_stepIndex;
    updateProgress(box);

    return StepObservation{outcome, box, player, velocity, _stepIndex};
}

}  // namespace aisolver
