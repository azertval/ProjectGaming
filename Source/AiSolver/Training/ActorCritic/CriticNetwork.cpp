// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ActorCritic/CriticNetwork.h"

#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Nn/Dense.h"
#include "AiSolver/Nn/WeightInit.h"

namespace aisolver::training {

CriticNetwork::CriticNetwork(std::size_t inputSize, std::size_t hiddenSize, Rng& rng) {
    // Meme schema d'initialisation (Xavier) et meme RNG que la politique (evolutionary::
    // policyTopology, LOT-ANNEXE-10) : aucun schema specifique introduit pour le critique.
    _network.addLayer(
        std::make_unique<nn::Dense>(inputSize, hiddenSize, nn::WeightInitScheme::Xavier, rng),
        autodiff::tanhOp);
    // Sortie scalaire non bornee : pas d'activation finale (contrairement au softmax de la
    // politique), la perte d'entrainement (CriticLoss.h) la fait converger vers l'echelle des
    // retours observes.
    _network.addLayer(
        std::make_unique<nn::Dense>(hiddenSize, 1, nn::WeightInitScheme::Xavier, rng), nullptr);
}

autodiff::NodePtr CriticNetwork::forward(const Tensor<float>& observation) {
    const autodiff::NodePtr inputNode = autodiff::variable(observation);
    return _network.forward(inputNode);
}

std::vector<autodiff::NodePtr> CriticNetwork::parameters() const {
    return _network.parameters();
}

}  // namespace aisolver::training
