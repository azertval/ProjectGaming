// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/AdvancedAlgorithmTrainedPolicy.h"

#include "AiSolver/Env/ActionSpace.h"

namespace aisolver::eval {

std::optional<core::PlayerInput> AdvancedAlgorithmTrainedPolicy::selectAction(
    const Tensor<float>& observation, ActionDecodingMode mode, Rng& rng) {
    if (!supportsMode(mode)) {
        return std::nullopt;
    }
    const autodiff::NodePtr outputNode = _mainNetwork.forward(observation);
    const Tensor<float> qValues = outputNode->value.view({actionCount()});
    return detail::decodeFromDistribution(qValues, mode, rng, /*allowStochastic=*/false);
}

}  // namespace aisolver::eval
