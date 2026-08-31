// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/PolicyGradient/ReinforceLoss.h"

#include "AiSolver/Training/PolicyGradientLoss.h"

namespace aisolver::training {

autodiff::NodePtr computeReinforceLoss(nn::Network& policy, const Trajectory& trajectory,
                                       const std::vector<float>& returns,
                                       float entropyCoefficient) {
    return computeWeightedPolicyGradientLoss(policy, trajectory, returns, entropyCoefficient);
}

}  // namespace aisolver::training
