// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ActorCritic/ActorCriticLoss.h"

#include "AiSolver/Training/PolicyGradientLoss.h"

namespace aisolver::training {

autodiff::NodePtr computeActorCriticLoss(nn::Network& policy, const Trajectory& trajectory,
                                         const std::vector<float>& advantages) {
    return computeWeightedPolicyGradientLoss(policy, trajectory, advantages);
}

}  // namespace aisolver::training
