// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/PlayerStateEncoder.h"

#include <cstddef>

namespace aisolver {

namespace {

/// Part restante d'un budget de mouvements, dans `[0, 1]`.
///
/// Un budget **nul** -- « aucun dash autorise sur ce tableau », que sept niveaux livres declarent
/// -- rendait `0 / 0`, donc `NaN`. Un seul `NaN` dans le vecteur d'observation contamine toute la
/// propagation avant : `tanh(NaN)` puis un `softmax` entierement `NaN`, dont `decodeStochastic`
/// tirait invariablement la derniere action de l'espace. L'agent rejouait alors exactement le meme
/// episode a chaque fois, quelle que soit sa graine et quel que soit son entrainement -- mesure :
/// **2 trajectoires distinctes sur 1 500 episodes** sur chacun de ces sept niveaux, contre
/// plusieurs centaines partout ailleurs.
/// @param remaining Mouvements restants.
/// @param budget Budget total du niveau ; negatif = illimite, `0` = interdit.
float normalizedBudget(int remaining, int budget) {
    if (budget < 0) {
        return 1.0f;  // illimite : rien ne s'epuise jamais.
    }
    if (budget == 0) {
        return 0.0f;  // interdit : il n'en reste rien, et il n'en restera jamais rien.
    }
    return static_cast<float>(remaining) / static_cast<float>(budget);
}

}  // namespace

Tensor<float> PlayerStateEncoder::encode(const core::Player& player, const core::Velocity& velocity,
                                         const core::Level& level) const {
    Tensor<float> result({static_cast<std::size_t>(PLAYER_STATE_SIZE)});

    result.at({0}) = velocity.value.x;
    result.at({1}) = velocity.value.y;
    result.at({2}) = player.grounded ? 1.0f : 0.0f;
    result.at({3}) = player.wallDirection;
    result.at({4}) = player.coyoteTimer / NOMINAL_COYOTE_TIME;
    result.at({5}) = player.jumpBufferTimer / NOMINAL_JUMP_BUFFER_TIME;
    result.at({6}) = player.wallJumpLockTimer / NOMINAL_WALL_JUMP_LOCK_TIME;
    result.at({7}) = player.dashTimer / NOMINAL_DASH_DURATION;
    // Meme condition de declenchement qu'un dash reussi dans CharacterPhysicsSystem::update.
    result.at({8}) = (player.dashChargesRemaining > 0 && player.dashTimer <= 0.0f) ? 1.0f : 0.0f;
    result.at({9}) = normalizedBudget(player.jumpsRemaining, level.jumpBudget());
    result.at({10}) = normalizedBudget(player.dashesRemaining, level.dashBudget());
    return result;
}

}  // namespace aisolver
