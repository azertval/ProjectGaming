// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Advanced/DqnLoss.h"

#include <algorithm>

#include "AiSolver/Math/Autodiff/Ops.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

namespace {

float maxQValue(const Tensor<float>& qValues) {
    const float* data = qValues.data();
    return *std::max_element(data, data + qValues.size());
}

}  // namespace

autodiff::NodePtr computeDqnLoss(QNetwork& mainNetwork, QNetwork& targetNetwork,
                                 const std::vector<Transition>& batch, float gamma) {
    PROJECTGAMING_ASSERT(!batch.empty(), "computeDqnLoss : le mini-lot ne doit pas etre vide");

    autodiff::NodePtr totalLoss;
    for (const Transition& transition : batch) {
        const autodiff::NodePtr predictedQ = mainNetwork.forward(transition.observation);
        const autodiff::NodePtr selectedQ =
            autodiff::selectIndex(predictedQ, transition.actionIndex);

        // Cible detachee du graphe : le reseau cible n'est jamais retropropage (meme convention que
        // computeAdvantages/CriticLoss, LOT-ANNEXE-13).
        float targetValue = transition.reward;
        if (!transition.done) {
            const autodiff::NodePtr nextQ = targetNetwork.forward(transition.nextObservation);
            targetValue += gamma * maxQValue(nextQ->value);
        }

        const autodiff::NodePtr error = autodiff::addScalar(selectedQ, -targetValue);
        const autodiff::NodePtr squaredError = autodiff::multiply(error, error);
        totalLoss = totalLoss ? autodiff::add(totalLoss, squaredError) : squaredError;
    }

    return autodiff::multiplyScalar(totalLoss, 1.0f / static_cast<float>(batch.size()));
}

}  // namespace aisolver::training
