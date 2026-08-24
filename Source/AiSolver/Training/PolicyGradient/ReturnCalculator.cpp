// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/PolicyGradient/ReturnCalculator.h"

#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

std::vector<float> computeReturns(const Trajectory& trajectory, float gamma) {
    PROJECTGAMING_ASSERT(gamma >= 0.0f && gamma <= 1.0f,
                         "computeReturns : gamma doit etre dans [0, 1]");

    std::vector<float> returns(trajectory.steps.size(), 0.0f);
    float runningReturn = 0.0f;
    for (std::size_t index = trajectory.steps.size(); index-- > 0;) {
        runningReturn = trajectory.steps[index].reward + gamma * runningReturn;
        returns[index] = runningReturn;
    }
    return returns;
}

}  // namespace aisolver::training
