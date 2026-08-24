// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Nn/Activations.h"
#include "AiSolver/Nn/Dense.h"

namespace aisolver::training::evolutionary {

std::unique_ptr<nn::Network> buildNetwork(const NetworkTopology& topology, Rng& rng) {
    auto network = std::make_unique<nn::Network>();
    for (const LayerTopology& layer : topology) {
        network->addLayer(
            std::make_unique<nn::Dense>(layer.inputSize, layer.outputSize, layer.initScheme, rng),
            layer.activation);
    }
    return network;
}

NetworkTopology policyTopology(std::size_t inputSize, std::size_t hiddenSize) {
    return NetworkTopology{
        LayerTopology{inputSize, hiddenSize, nn::WeightInitScheme::Xavier, autodiff::tanhOp},
        LayerTopology{hiddenSize, actionCount(), nn::WeightInitScheme::Xavier, nn::softmax},
    };
}

}  // namespace aisolver::training::evolutionary
