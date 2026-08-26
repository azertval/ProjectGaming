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

    // Les deux schemas repondent a la MEME question : quelle amplitude donner aux poids pour que
    // la variance du signal soit preservee d'une couche a l'autre ? Trop grands, les activations
    // saturent et les gradients s'annulent ; trop petits, le signal s'eteint en profondeur. Ils
    // ne different que par l'hypothese faite sur l'activation qui suit.
    float* data = weights.data();
    switch (scheme) {
        case WeightInitScheme::Xavier: {
            // Xavier/Glorot, pour une activation symetrique autour de zero (tanh, sigmoide) :
            // loi uniforme sur [-b, b] avec b = racine(6 / (fanIn + fanOut)). Cette borne est
            // celle dont la variance vaut 2 / (fanIn + fanOut) -- le compromis entre preserver
            // la variance a l'aller (qui demande 1 / fanIn) et au retour (1 / fanOut).
            const float bound = std::sqrt(6.0f / static_cast<float>(fanIn + fanOut));
            for (std::size_t i = 0; i < weights.size(); ++i) {
                data[i] = rng.nextFloat(-bound, bound);
            }
            break;
        }
        case WeightInitScheme::He: {
            // He, pour une activation qui annule les valeurs negatives (ReLU) : celle-ci coupe
            // en moyenne la moitie du signal, donc la variance a l'aller vaut fanIn / 2 au lieu
            // de fanIn. D'ou le facteur 2 au numerateur, et fanIn seul -- la retropropagation a
            // travers une ReLU subit la meme coupure, il n'y a pas de compromis a faire.
            const float stddev = std::sqrt(2.0f / static_cast<float>(fanIn));
            for (std::size_t i = 0; i < weights.size(); ++i) {
                data[i] = rng.nextGaussian(0.0f, stddev);
            }
            break;
        }
    }
}

}  // namespace aisolver::nn
