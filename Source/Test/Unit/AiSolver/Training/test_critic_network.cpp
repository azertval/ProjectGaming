// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_critic_network.cpp
 * @brief Tests unitaires de `CriticNetwork` (LOT-ANNEXE-13, TACHE-01).
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::training::CriticNetwork;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;

namespace {

constexpr std::size_t kInputSize = 5;
constexpr std::size_t kHiddenSize = 4;

Tensor<float> observationOf(float a, float b, float c, float d, float e) {
    Tensor<float> observation({kInputSize, 1});
    observation.at({0, 0}) = a;
    observation.at({1, 0}) = b;
    observation.at({2, 0}) = c;
    observation.at({3, 0}) = d;
    observation.at({4, 0}) = e;
    return observation;
}

}  // namespace

/**
 * @brief `forward` renvoie toujours un unique scalaire (forme `[1]`), quelle que soit
 * l'observation.
 * \castest{<b>CriticNetwork : forme de sortie scalaire.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un critique.<br/>2. `forward` sur deux observations distinctes.<br/>
 * \tattendu `value.size() == 1` dans les deux cas.}
 */
TEST(CriticNetworkTest, FormeDeSortieScalaire) {
    Rng rng(1);
    CriticNetwork critic(kInputSize, kHiddenSize, rng);

    const auto first = critic.forward(observationOf(0.1f, 0.2f, -0.1f, 0.0f, 0.3f));
    const auto second = critic.forward(observationOf(-0.5f, 0.4f, 0.2f, -0.2f, 0.1f));
    EXPECT_EQ(first->value.size(), 1u);
    EXPECT_EQ(second->value.size(), 1u);
}

/**
 * @brief Deux appels à `forward` avec la même observation et les mêmes poids renvoient exactement
 * la même valeur.
 * \castest{<b>CriticNetwork : déterminisme à poids fixés.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire un critique.<br/>2. `forward` deux fois sur la même observation.<br/>
 * \tattendu Valeurs strictement égales.}
 */
TEST(CriticNetworkTest, DeterminismeAPoidsFixes) {
    Rng rng(2);
    CriticNetwork critic(kInputSize, kHiddenSize, rng);
    const Tensor<float> observation = observationOf(0.3f, -0.1f, 0.2f, 0.4f, -0.3f);

    const float first = critic.forward(observation)->value.data()[0];
    const float second = critic.forward(observation)->value.data()[0];
    EXPECT_FLOAT_EQ(first, second);
}

/**
 * @brief Une perturbation des poids du critique change la valeur de sortie pour une même
 * observation (le réseau n'est pas dégénéré après construction).
 * \castest{<b>CriticNetwork : sensibilité aux poids.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `forward` sur une observation.<br/>2. Perturber un poids.<br/>3. `forward` de
 * nouveau.<br/>
 * \tattendu Les deux valeurs diffèrent.}
 */
TEST(CriticNetworkTest, SensibiliteAuxPoids) {
    Rng rng(3);
    CriticNetwork critic(kInputSize, kHiddenSize, rng);
    const Tensor<float> observation = observationOf(0.1f, 0.1f, 0.1f, 0.1f, 0.1f);

    const float before = critic.forward(observation)->value.data()[0];

    const auto parameters = critic.parameters();
    ASSERT_FALSE(parameters.empty());
    parameters.front()->value.data()[0] += 10.0f;

    const float after = critic.forward(observation)->value.data()[0];
    EXPECT_NE(before, after);
}

/**
 * @brief Construire et faire varier un `CriticNetwork` n'affecte en rien les sorties d'un
 * `nn::Network` de politique construit séparément sur la même observation (pas de poids partagé
 * accidentel).
 * \castest{<b>CriticNetwork : indépendance du réseau de politique.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire une politique et un critique, RNG distincts.<br/>2. `forward` de la
 * politique.<br/>3. Perturber le critique.<br/>4. `forward` de la politique à nouveau.<br/>
 * \tattendu Sortie de la politique strictement identique avant/après.}
 */
TEST(CriticNetworkTest, IndependanceDuReseauDePolitique) {
    Rng policyRng(4);
    auto policy = buildNetwork(policyTopology(kInputSize, kHiddenSize), policyRng);
    Rng criticRng(5);
    CriticNetwork critic(kInputSize, kHiddenSize, criticRng);

    const Tensor<float> observation = observationOf(0.2f, -0.2f, 0.1f, 0.0f, 0.4f);
    const auto policyInput = aisolver::autodiff::variable(observation);
    const auto before = policy->forward(policyInput)->value.clone();

    const auto criticParameters = critic.parameters();
    ASSERT_FALSE(criticParameters.empty());
    criticParameters.front()->value.data()[0] += 100.0f;
    [[maybe_unused]] const auto criticOutput = critic.forward(observation);

    const auto policyInputAgain = aisolver::autodiff::variable(observation);
    const auto after = policy->forward(policyInputAgain)->value;

    ASSERT_EQ(before.size(), after.size());
    for (std::size_t i = 0; i < before.size(); ++i) {
        EXPECT_FLOAT_EQ(before.data()[i], after.data()[i]);
    }
}
