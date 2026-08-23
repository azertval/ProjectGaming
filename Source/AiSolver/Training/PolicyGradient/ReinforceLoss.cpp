// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/PolicyGradient/ReinforceLoss.h"

#include "AiSolver/Math/Autodiff/Ops.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

autodiff::NodePtr computeReinforceLoss(nn::Network& policy, const Trajectory& trajectory,
                                       const std::vector<float>& returns) {
    PROJECTGAMING_ASSERT(!trajectory.steps.empty(),
                         "computeReinforceLoss : la trajectoire ne doit pas etre vide");
    PROJECTGAMING_ASSERT(returns.size() == trajectory.steps.size(),
                         "computeReinforceLoss : returns doit avoir la meme longueur que la "
                         "trajectoire");

    autodiff::NodePtr totalLoss;
    for (std::size_t index = 0; index < trajectory.steps.size(); ++index) {
        const TrajectoryStep& step = trajectory.steps[index];

        const autodiff::NodePtr inputNode = autodiff::variable(step.observation);
        const autodiff::NodePtr outputNode = policy.forward(inputNode);
        const autodiff::NodePtr actionProbability =
            autodiff::selectIndex(outputNode, step.actionIndex);
        const autodiff::NodePtr logProbability = autodiff::logOp(actionProbability);
        // -log(pi(a_t|s_t)) * G_t : le retour est une grandeur detachee (float), jamais un noeud.
        const autodiff::NodePtr stepLoss = autodiff::multiplyScalar(logProbability, -returns[index]);

        totalLoss = totalLoss ? autodiff::add(totalLoss, stepLoss) : stepLoss;
    }

    // Moyenne (pas somme brute) : l'amplitude de la perte ne doit pas dependre de la longueur
    // variable de l'episode (decision de cadrage de l'epic).
    return autodiff::multiplyScalar(totalLoss, 1.0f / static_cast<float>(trajectory.steps.size()));
}

}  // namespace aisolver::training
