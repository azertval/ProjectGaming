// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/Reward.h"

#include <cmath>

#include "Core/Levels/GridPosition.h"
#include "Core/Math/Vector2.h"

namespace aisolver {

namespace {

float distanceToExit(const core::Aabb& box, const GridDistanceField& distanceField) {
    const core::Vector2 center = (box.min + box.max) * 0.5f;
    const core::GridPosition cell{static_cast<int>(std::floor(center.x)),
                                  static_cast<int>(std::floor(center.y))};
    return static_cast<float>(distanceField.distance(cell));
}

}  // namespace

float computeReward(const RewardConfig& config, const GridDistanceField& distanceField,
                    const core::Aabb& previousBox, const core::Aabb& currentBox,
                    core::LevelOutcome outcome) {
    const float previousDistance = distanceToExit(previousBox, distanceField);
    const float currentDistance = distanceToExit(currentBox, distanceField);

    float reward = config.progressScale * (previousDistance - currentDistance);
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
    std::vector<core::GridPosition> targets{level.exit()};
    const std::vector<core::Mechanism>& links = mechanisms.mechanisms();
    for (std::size_t index = 0; index < links.size(); ++index) {
        if (!mechanisms.isDoorOpen(index)) {
            targets.push_back(links[index].switchPosition);
        }
    }
    return GridDistanceField(mechanisms.collisionMap(), targets);
}

}  // namespace aisolver
