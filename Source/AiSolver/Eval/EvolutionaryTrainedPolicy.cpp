// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/EvolutionaryTrainedPolicy.h"

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Math/Autodiff/Node.h"

namespace aisolver::eval {

std::optional<core::PlayerInput> EvolutionaryTrainedPolicy::selectAction(
    const Tensor<float>& observation, ActionDecodingMode mode, Rng& rng) {
    if (!supportsMode(mode)) {
        return std::nullopt;
    }
    const autodiff::NodePtr inputNode = autodiff::variable(observation);
    const autodiff::NodePtr outputNode = _network.forward(inputNode);
    const Tensor<float> distribution = outputNode->value.view({actionCount()});
    return detail::decodeFromDistribution(distribution, mode, rng, /*allowStochastic=*/false);
}

}  // namespace aisolver::eval
