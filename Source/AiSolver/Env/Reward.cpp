// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/Reward.h"

#include <cmath>

#include "Core/Diagnostics/Assert.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Math/Vector2.h"

namespace aisolver {

namespace {

core::GridPosition cellOf(const core::Aabb& box) {
    const core::Vector2 center = (box.min + box.max) * 0.5f;
    return core::GridPosition{static_cast<int>(std::floor(center.x)),
                              static_cast<int>(std::floor(center.y))};
}

}  // namespace

float computeReward(const RewardConfig& config, const GridDistanceField& distanceField,
                    const core::Aabb& previousBox, const core::Aabb& currentBox,
                    core::LevelOutcome outcome) {
    const core::GridPosition previousCell = cellOf(previousBox);
    const core::GridPosition currentCell = cellOf(currentBox);

    // Progression nulle des qu'une des deux cases est inatteignable : `distance()` y rend une
    // sentinelle (`largeur x hauteur`), et la difference entre une distance reelle et cette
    // sentinelle vaudrait des centaines de points sur un seul pas -- de quoi ecraser a elle seule
    // le retour de tout un episode, dans un sens comme dans l'autre. Un pas dont on ne sait pas
    // dire s'il rapproche ne rapporte rien ; il reste soumis a la penalite de temps.
    float reward = 0.0f;
    if (distanceField.isReachable(previousCell) && distanceField.isReachable(currentCell)) {
        const float previousDistance = static_cast<float>(distanceField.distance(previousCell));
        const float currentDistance = static_cast<float>(distanceField.distance(currentCell));
        reward = config.progressScale * (previousDistance - currentDistance);
    }
    if (outcome == core::LevelOutcome::Won) {
        reward += config.completionBonus;
    } else if (outcome == core::LevelOutcome::Lost) {
        reward += config.deathPenalty;
    }
    reward -= config.timePenalty;
    return reward;
}

GridDistanceField buildObjectiveDistanceField(const core::Level& level,
                                              const core::MechanismController& mechanisms) {
    return buildObjectiveDistanceField(level, mechanisms, mechanisms.collisionMap());
}

GridDistanceField buildObjectiveDistanceField(const core::Level& level,
                                              const core::MechanismController& mechanisms,
                                              const core::TileMap& collision) {
    const std::vector<core::Mechanism>& links = mechanisms.mechanisms();
    std::vector<core::GridPosition> targets;
    targets.reserve(1 + links.size());  // la sortie, plus au plus un declencheur par mecanisme
    targets.push_back(level.exit());
    for (std::size_t index = 0; index < links.size(); ++index) {
        if (!mechanisms.isDoorOpen(index)) {
            targets.push_back(links[index].switchPosition);
        }
    }
    return GridDistanceField(collision, targets);
}

const GridDistanceField& ObjectiveDistanceFieldCache::field(
    const core::Level& level, const core::MechanismController& mechanisms,
    const core::TileMap& collision, const std::vector<core::GridPosition>& blockPositions) {
    const std::size_t mechanismCount = mechanisms.mechanisms().size();

    bool stillValid = _field.has_value() && _doorsOpen.size() == mechanismCount &&
                      _blockPositions == blockPositions;
    for (std::size_t index = 0; stillValid && index < mechanismCount; ++index) {
        stillValid = _doorsOpen[index] == mechanisms.isDoorOpen(index);
    }

    if (!stillValid) {
        _doorsOpen.resize(mechanismCount);
        for (std::size_t index = 0; index < mechanismCount; ++index) {
            _doorsOpen[index] = mechanisms.isDoorOpen(index);
        }
        _blockPositions = blockPositions;
        _field = buildObjectiveDistanceField(level, mechanisms, collision);
        ++_revision;
    }
    return *_field;
}

const GridDistanceField& ObjectiveDistanceFieldCache::field(
    const core::Level& level, const core::MechanismController& mechanisms) {
    return field(level, mechanisms, mechanisms.collisionMap(), {});
}

const GridDistanceField& ObjectiveDistanceFieldCache::lastField() const {
    PROJECTGAMING_ASSERT(_field.has_value(),
                         "ObjectiveDistanceFieldCache::lastField : field() doit avoir ete appele");
    return *_field;
}

}  // namespace aisolver
