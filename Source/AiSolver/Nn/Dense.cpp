// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Nn/Dense.h"

#include "AiSolver/Math/Autodiff/Ops.h"

namespace aisolver::nn {

namespace {

Tensor<float> makeWeights(std::size_t inputSize, std::size_t outputSize, WeightInitScheme scheme, Rng& rng) {
    Tensor<float> weights({outputSize, inputSize});
    initializeWeights(weights, scheme, rng);
    return weights;
}

Tensor<float> makeBias(std::size_t outputSize) {
    return Tensor<float>({outputSize, 1});
}

}  // namespace

Dense::Dense(std::size_t inputSize, std::size_t outputSize, WeightInitScheme scheme, Rng& rng)
    : _weights(autodiff::variable(makeWeights(inputSize, outputSize, scheme, rng))),
      _bias(autodiff::variable(makeBias(outputSize))) {}

autodiff::NodePtr Dense::forward(const autodiff::NodePtr& input) {
    return autodiff::add(autodiff::matmul(_weights, input), _bias);
}

std::vector<autodiff::NodePtr> Dense::parameters() const {
    return {_weights, _bias};
}

const Tensor<float>& Dense::weights() const {
    return _weights->value;
}

const Tensor<float>& Dense::bias() const {
    return _bias->value;
}

}  // namespace aisolver::nn
