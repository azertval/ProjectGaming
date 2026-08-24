// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/ActorCriticTrainedPolicy.h"

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Math/Autodiff/Node.h"

namespace aisolver::eval {

std::optional<core::PlayerInput> ActorCriticTrainedPolicy::selectAction(
    const Tensor<float>& observation, ActionDecodingMode mode, Rng& rng) {
    const autodiff::NodePtr inputNode = autodiff::variable(observation);
    const autodiff::NodePtr outputNode = _actor.forward(inputNode);
    const Tensor<float> distribution = outputNode->value.view({actionCount()});
    return detail::decodeFromDistribution(distribution, mode, rng, /*allowStochastic=*/true);
}

}  // namespace aisolver::eval
