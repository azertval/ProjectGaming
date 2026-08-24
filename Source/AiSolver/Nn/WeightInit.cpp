// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Nn/WeightInit.h"

#include <cmath>

#include "Core/Diagnostics/Assert.h"

namespace aisolver::nn {

void initializeWeights(Tensor<float>& weights, WeightInitScheme scheme, Rng& rng) {
    PROJECTGAMING_ASSERT(weights.rank() == 2,
                         "initializeWeights() : les poids doivent etre de rang 2");
    const std::size_t fanOut = weights.shape()[0];
    const std::size_t fanIn = weights.shape()[1];

    float* data = weights.data();
    switch (scheme) {
        case WeightInitScheme::Xavier: {
            const float bound = std::sqrt(6.0f / static_cast<float>(fanIn + fanOut));
            for (std::size_t i = 0; i < weights.size(); ++i) {
                data[i] = rng.nextFloat(-bound, bound);
            }
            break;
        }
        case WeightInitScheme::He: {
            const float stddev = std::sqrt(2.0f / static_cast<float>(fanIn));
            for (std::size_t i = 0; i < weights.size(); ++i) {
                data[i] = rng.nextGaussian(0.0f, stddev);
            }
            break;
        }
    }
}

}  // namespace aisolver::nn
