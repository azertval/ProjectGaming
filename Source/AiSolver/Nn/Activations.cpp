// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Nn/Activations.h"

#include <cmath>

#include "AiSolver/Math/TensorOps.h"

namespace aisolver::nn {

autodiff::NodePtr sigmoid(const autodiff::NodePtr& a) {
    return autodiff::unaryOp(
        a,
        [](const Tensor<float>& value) {
            return aisolver::detail::elementwiseUnary(value, [](float x) { return 1.0f / (1.0f + std::exp(-x)); });
        },
        [](const Tensor<float>& outputValue, const Tensor<float>& outputGrad, const Tensor<float>&) {
            return aisolver::detail::elementwise(
                outputGrad, outputValue, [](float g, float s) { return g * s * (1.0f - s); });
        });
}

autodiff::NodePtr softmax(const autodiff::NodePtr& a) {
    return autodiff::unaryOp(
        a,
        [](const Tensor<float>& value) {
            // Stabilite numerique : soustraire le maximum ne change pas le resultat mathematique
            // (softmax(x) == softmax(x - max(x))) mais evite un debordement de exp() sur de grands
            // logits (cf. TACHE-02, points d'attention).
            const float maxValue = aisolver::max(value);
            const Tensor<float> shifted =
                aisolver::detail::elementwiseUnary(value, [maxValue](float x) { return x - maxValue; });
            const Tensor<float> exponentials =
                aisolver::detail::elementwiseUnary(shifted, [](float x) { return std::exp(x); });
            const float sumOfExponentials = aisolver::sum(exponentials);
            return aisolver::detail::elementwiseUnary(
                exponentials, [sumOfExponentials](float e) { return e / sumOfExponentials; });
        },
        [](const Tensor<float>& outputValue, const Tensor<float>& outputGrad, const Tensor<float>&) {
            // Produit jacobien-vecteur direct, sans materialiser la jacobienne n x n :
            // grad_i = softmax_i * (outputGrad_i - sum_j(outputGrad_j * softmax_j)).
            const float dot = aisolver::sum(aisolver::multiply(outputGrad, outputValue));
            return aisolver::detail::elementwise(
                outputValue, outputGrad, [dot](float softmaxI, float gradI) { return softmaxI * (gradI - dot); });
        });
}

}  // namespace aisolver::nn
