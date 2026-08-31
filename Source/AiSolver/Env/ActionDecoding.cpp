// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/ActionDecoding.h"

#include <cmath>
#include <optional>

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

    // p_i^(1/temperature), puis renormalisation. A temperature 1 l'exposant vaut 1 : la
    // ponderation est l'identite, et la distribution d'entree sert directement de poids -- ni
    // parcours ni allocation, alors que c'est le seul reglage utilise a l'entrainement.
    std::optional<Tensor<float>> reweighted;
    if (temperature != 1.0f) {
        const float inverseTemperature = 1.0f / temperature;
        reweighted = detail::elementwiseUnary(distribution, [inverseTemperature](float p) {
            return std::pow(p, inverseTemperature);
        });
    }
    const Tensor<float>& weighted = reweighted ? *reweighted : distribution;
    const float total = sum(weighted);
    // Une distribution non finie ne se tire pas : toute comparaison a `NaN` etant fausse, la
    // roulette ci-dessous tomberait systematiquement sur la DERNIERE action, silencieusement et
    // pour tout le reste de l'entrainement. C'est ce qui arrivait quand une entree d'observation
    // valait `NaN` (voir `PlayerStateEncoder`) -- une erreur de programmation en amont, pas une
    // entree utilisateur : elle doit s'entendre.
    PROJECTGAMING_ASSERT(std::isfinite(total) && total > 0.0f,
                         "decodeStochastic : la distribution doit etre finie et de somme positive");

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
