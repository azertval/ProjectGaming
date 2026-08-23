// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Nn/Network.h"

#include <utility>

namespace aisolver::nn {

void Network::addLayer(std::unique_ptr<Dense> layer, ActivationFn activation) {
    _layers.push_back(Layer{std::move(layer), std::move(activation)});
}

autodiff::NodePtr Network::forward(const autodiff::NodePtr& input) {
    autodiff::NodePtr current = input;
    for (const Layer& layer : _layers) {
        current = layer.dense->forward(current);
        if (layer.activation) {
            current = layer.activation(current);
        }
    }
    return current;
}

std::vector<autodiff::NodePtr> Network::parameters() const {
    std::vector<autodiff::NodePtr> allParameters;
    allParameters.reserve(_layers.size() * 2);
    for (const Layer& layer : _layers) {
        const std::vector<autodiff::NodePtr> layerParameters = layer.dense->parameters();
        allParameters.insert(allParameters.end(), layerParameters.begin(), layerParameters.end());
    }
    return allParameters;
}

std::size_t Network::layerCount() const {
    return _layers.size();
}

}  // namespace aisolver::nn
