// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/ActionDecoding.h"

#include <cmath>

#include "AiSolver/Math/TensorOps.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver {

namespace {

void checkDistributionShape([[maybe_unused]] const Tensor<float>& distribution) {
    PROJECTGAMING_ASSERT(distribution.rank() == 1 && distribution.shape()[0] == actionCount(),
                         "decodeArgmax/decodeStochastic : distribution de forme incorrecte "
                         "(attendu un vecteur de taille actionCount())");
}

}  // namespace

Action decodeArgmax(const Tensor<float>& distribution) {
    checkDistributionShape(distribution);

    std::size_t bestIndex = 0;
    float bestValue = distribution.data()[0];
    for (std::size_t index = 1; index < actionCount(); ++index) {
        const float value = distribution.data()[index];
        if (value > bestValue) {
            bestValue = value;
            bestIndex = index;
        }
    }
    return actionAt(bestIndex);
}

Action decodeStochastic(const Tensor<float>& distribution, float temperature, Rng& rng) {
    checkDistributionShape(distribution);
    PROJECTGAMING_ASSERT(temperature > 0.0f, "decodeStochastic : temperature doit etre positive");

    // p_i^(1/temperature), puis renormalisation -- temperature = 1.0 laisse la distribution
    // inchangee (exposant 1).
    const float inverseTemperature = 1.0f / temperature;
    Tensor<float> weighted = detail::elementwiseUnary(
        distribution, [inverseTemperature](float p) { return std::pow(p, inverseTemperature); });
    const float total = sum(weighted);

    // Methode de la roulette : tirage uniforme dans [0, 1), parcours de la somme cumulee des
    // probabilites normalisees. Le dernier indice recoit tout reliquat numerique (la somme
    // cumulee peut ne pas atteindre exactement `target` a la derniere iteration en float).
    const float target = rng.nextFloat(0.0f, 1.0f) * total;
    float cumulative = 0.0f;
    for (std::size_t index = 0; index + 1 < actionCount(); ++index) {
        cumulative += weighted.data()[index];
        if (cumulative >= target) {
            return actionAt(index);
        }
    }
    return actionAt(actionCount() - 1);
}

}  // namespace aisolver
