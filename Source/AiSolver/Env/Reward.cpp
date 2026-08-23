// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/Reward.h"

#include "Core/Math/Vector2.h"

namespace aisolver {

namespace {

float distanceToExit(const core::Aabb& box, const core::GridPosition& exit) {
    const core::Vector2 center = (box.min + box.max) * 0.5f;
    const core::Vector2 exitCenter{static_cast<float>(exit.column) + 0.5f,
                                   static_cast<float>(exit.row) + 0.5f};
    return (center - exitCenter).length();
}

}  // namespace

float computeReward(const RewardConfig& config, const core::Aabb& previousBox,
                    const core::Aabb& currentBox, const core::GridPosition& exit,
                    core::LevelOutcome outcome) {
    const float previousDistance = distanceToExit(previousBox, exit);
    const float currentDistance = distanceToExit(currentBox, exit);

    float reward = config.progressScale * (previousDistance - currentDistance);
    if (outcome == core::LevelOutcome::Won) {
        reward += config.completionBonus;
    } else if (outcome == core::LevelOutcome::Lost) {
        reward += config.deathPenalty;
    }
    reward -= config.timePenalty;
    return reward;
}

}  // namespace aisolver
