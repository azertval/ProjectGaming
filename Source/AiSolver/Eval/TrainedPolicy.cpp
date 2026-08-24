// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/TrainedPolicy.h"

#include "AiSolver/Env/ActionDecoding.h"
#include "AiSolver/Env/ActionSpace.h"

namespace aisolver::eval::detail {

std::optional<core::PlayerInput> decodeFromDistribution(const Tensor<float>& distribution,
                                                         ActionDecodingMode mode, Rng& rng,
                                                         bool allowStochastic) {
    if (mode == ActionDecodingMode::Stochastic) {
        if (!allowStochastic) {
            return std::nullopt;
        }
        return toPlayerInput(decodeStochastic(distribution, 1.0f, rng));
    }
    return toPlayerInput(decodeArgmax(distribution));
}

}  // namespace aisolver::eval::detail
