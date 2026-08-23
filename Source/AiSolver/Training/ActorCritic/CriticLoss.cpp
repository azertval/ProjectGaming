// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ActorCritic/CriticLoss.h"

#include "AiSolver/Math/Autodiff/Ops.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

autodiff::NodePtr computeCriticLoss(CriticNetwork& critic, const Trajectory& trajectory,
                                    const std::vector<float>& returns) {
    PROJECTGAMING_ASSERT(!trajectory.steps.empty(),
                         "computeCriticLoss : la trajectoire ne doit pas etre vide");
    PROJECTGAMING_ASSERT(returns.size() == trajectory.steps.size(),
                         "computeCriticLoss : returns doit avoir la meme longueur que la "
                         "trajectoire");

    autodiff::NodePtr totalLoss;
    for (std::size_t index = 0; index < trajectory.steps.size(); ++index) {
        const autodiff::NodePtr estimatedValue =
            critic.forward(trajectory.steps[index].observation);
        const autodiff::NodePtr error = autodiff::addScalar(estimatedValue, -returns[index]);
        const autodiff::NodePtr squaredError = autodiff::multiply(error, error);

        totalLoss = totalLoss ? autodiff::add(totalLoss, squaredError) : squaredError;
    }

    return autodiff::multiplyScalar(totalLoss, 1.0f / static_cast<float>(trajectory.steps.size()));
}

}  // namespace aisolver::training
