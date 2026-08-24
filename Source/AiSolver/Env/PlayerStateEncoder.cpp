// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/PlayerStateEncoder.h"

#include <cstddef>

namespace aisolver {

Tensor<float> PlayerStateEncoder::encode(const core::Player& player, const core::Velocity& velocity,
                                         const core::Level& level) const {
    Tensor<float> result({static_cast<std::size_t>(kPlayerStateSize)});

    result.at({0}) = velocity.value.x;
    result.at({1}) = velocity.value.y;
    result.at({2}) = player.grounded ? 1.0f : 0.0f;
    result.at({3}) = player.wallDirection;
    result.at({4}) = player.coyoteTimer / kNominalCoyoteTime;
    result.at({5}) = player.jumpBufferTimer / kNominalJumpBufferTime;
    result.at({6}) = player.wallJumpLockTimer / kNominalWallJumpLockTime;
    result.at({7}) = player.dashTimer / kNominalDashDuration;
    // Meme condition de declenchement qu'un dash reussi dans CharacterPhysicsSystem::update.
    result.at({8}) = (player.dashChargesRemaining > 0 && player.dashTimer <= 0.0f) ? 1.0f : 0.0f;
    result.at({9}) = level.jumpBudget() < 0 ? 1.0f
                                            : static_cast<float>(player.jumpsRemaining) /
                                                  static_cast<float>(level.jumpBudget());
    result.at({10}) = level.dashBudget() < 0 ? 1.0f
                                             : static_cast<float>(player.dashesRemaining) /
                                                   static_cast<float>(level.dashBudget());
    return result;
}

}  // namespace aisolver
