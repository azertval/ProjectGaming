// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/PolicyGradientLoss.h"

#include <cstddef>
#include <utility>
#include <vector>

#include "AiSolver/Math/Autodiff/Ops.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

autodiff::NodePtr computeWeightedPolicyGradientLoss(nn::Network& policy,
                                                    const Trajectory& trajectory,
                                                    const std::vector<float>& weights,
                                                    float entropyCoefficient) {
    PROJECTGAMING_ASSERT(
        !trajectory.steps.empty(),
        "computeWeightedPolicyGradientLoss : la trajectoire ne doit pas etre vide");
    PROJECTGAMING_ASSERT(weights.size() == trajectory.steps.size(),
                         "computeWeightedPolicyGradientLoss : weights doit avoir la meme longueur "
                         "que la trajectoire");

    // Pertes par pas accumulees a plat, puis reduites en ARBRE (ci-dessous) plutot qu'en chaine.
    // Une chaine d'`add` a une profondeur egale au nombre de pas : au-dela de quelques milliers,
    // ni la retropropagation ni meme la DESTRUCTION du graphe (recursive par les `shared_ptr`
    // parents) ne tiennent dans la pile d'un fil. Un episode compte desormais plusieurs milliers
    // de pas -- la reduction en arbre ramene cette profondeur a son logarithme.
    std::vector<autodiff::NodePtr> stepLosses;
    stepLosses.reserve(trajectory.steps.size());
    for (std::size_t index = 0; index < trajectory.steps.size(); ++index) {
        const TrajectoryStep& step = trajectory.steps[index];

        const autodiff::NodePtr inputNode = autodiff::variable(step.observation);
        const autodiff::NodePtr outputNode = policy.forward(inputNode);
        const autodiff::NodePtr actionProbability =
            autodiff::selectIndex(outputNode, step.actionIndex);
        const autodiff::NodePtr logProbability = autodiff::logOp(actionProbability);
        // -log(pi(a_t|s_t)) * weights[t] : le poids (retour ou avantage) est une grandeur detachee
        // (float), jamais un noeud.
        autodiff::NodePtr stepLoss = autodiff::multiplyScalar(logProbability, -weights[index]);

        if (entropyCoefficient > 0.0f) {
            // -beta * H(pi) = +beta * somme(p_i log p_i) : minimiser la perte revient donc a
            // MAXIMISER l'entropie de la distribution, ce qui garde d'autres actions que la
            // favorite echantillonnables. Le plancher de `logOp` rend le terme fini meme quand une
            // probabilite s'annule en flottant (`p log p -> 0`, sa derivee `log p + 1` reste
            // bornee).
            const autodiff::NodePtr negativeEntropy =
                autodiff::sumAll(autodiff::multiply(outputNode, autodiff::logOp(outputNode)));
            stepLoss = autodiff::add(stepLoss,
                                     autodiff::multiplyScalar(negativeEntropy, entropyCoefficient));
        }

        stepLosses.push_back(std::move(stepLoss));
    }

    // Reduction en arbre : on somme les voisins deux a deux jusqu'a n'avoir qu'un noeud.
    while (stepLosses.size() > 1) {
        std::size_t write = 0;
        for (std::size_t read = 0; read + 1 < stepLosses.size(); read += 2) {
            stepLosses[write++] = autodiff::add(stepLosses[read], stepLosses[read + 1]);
        }
        if (stepLosses.size() % 2 == 1) {
            stepLosses[write++] = stepLosses.back();
        }
        stepLosses.resize(write);
    }

    // Moyenne (pas somme brute) : l'amplitude de la perte ne doit pas dependre de la longueur
    // variable de l'episode (decision de cadrage de LOT-ANNEXE-12, reprise telle quelle ici).
    return autodiff::multiplyScalar(stepLosses.front(),
                                    1.0f / static_cast<float>(trajectory.steps.size()));
}

}  // namespace aisolver::training
