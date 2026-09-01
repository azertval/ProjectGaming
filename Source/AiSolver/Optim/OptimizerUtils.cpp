// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Optim/OptimizerUtils.h"

#include <cmath>
#include <cstddef>

namespace aisolver::optim {

float clipGradientNorm(const std::vector<autodiff::NodePtr>& parameters, float maxNorm) {
    double sumOfSquares = 0.0;
    for (const autodiff::NodePtr& parameter : parameters) {
        const Tensor<float>& gradient = parameter->grad;
        for (std::size_t index = 0; index < gradient.size(); ++index) {
            const double value = gradient.data()[index];
            sumOfSquares += value * value;
        }
    }
    const auto norm = static_cast<float>(std::sqrt(sumOfSquares));
    if (maxNorm <= 0.0f || norm <= maxNorm || norm == 0.0f) {
        return norm;
    }
    const float scale = maxNorm / norm;
    for (const autodiff::NodePtr& parameter : parameters) {
        Tensor<float>& gradient = parameter->grad;
        for (std::size_t index = 0; index < gradient.size(); ++index) {
            gradient.data()[index] *= scale;
        }
    }
    return norm;
}

void zeroGrad(const std::vector<autodiff::NodePtr>& parameters) {
    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
}

}  // namespace aisolver::optim
