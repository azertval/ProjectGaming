// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Dqn/QNetwork.h"

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Nn/Dense.h"
#include "AiSolver/Nn/WeightInit.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

QNetwork::QNetwork(std::size_t inputSize, std::size_t hiddenSize, Rng& rng) {
    _network.addLayer(
        std::make_unique<nn::Dense>(inputSize, hiddenSize, nn::WeightInitScheme::Xavier, rng),
        autodiff::tanhOp);
    // Sortie non bornee (valeurs Q(s,a), pas une distribution) : pas d'activation finale, meme
    // decision de cadrage que CriticNetwork (LOT-ANNEXE-13).
    _network.addLayer(
        std::make_unique<nn::Dense>(hiddenSize, actionCount(), nn::WeightInitScheme::Xavier, rng),
        nullptr);
}

autodiff::NodePtr QNetwork::forward(const Tensor<float>& observation) {
    const autodiff::NodePtr inputNode = autodiff::variable(observation);
    return _network.forward(inputNode);
}

std::vector<autodiff::NodePtr> QNetwork::parameters() const {
    return _network.parameters();
}

void QNetwork::copyWeightsFrom(const QNetwork& source) {
    const std::vector<autodiff::NodePtr> sourceParameters = source.parameters();
    const std::vector<autodiff::NodePtr> targetParameters = parameters();
    PROJECTGAMING_ASSERT(sourceParameters.size() == targetParameters.size(),
                         "QNetwork::copyWeightsFrom : topologie source/cible incompatible");
    for (std::size_t index = 0; index < sourceParameters.size(); ++index) {
        targetParameters[index]->value = sourceParameters[index]->value.clone();
    }
}

}  // namespace aisolver::training
