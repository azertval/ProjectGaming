// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dqn_loss.cpp
 * @brief Gradient checking de `computeDqnLoss` (LOT-ANNEXE-14, TACHE-01), par rapport aux poids du
 * réseau principal uniquement -- même méthodologie que `test_actor_critic_loss.cpp`
 * (LOT-ANNEXE-13).
 */

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Training/Advanced/DqnLoss.h"
#include "AiSolver/Training/Advanced/QNetwork.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::training::computeDqnLoss;
using aisolver::training::QNetwork;
using aisolver::training::Transition;

namespace {

constexpr std::size_t kInputSize = 3;
constexpr std::size_t kHiddenSize = 4;
constexpr float kGamma = 0.9f;

Tensor<float> observationOf(float a, float b, float c) {
    Tensor<float> observation({kInputSize, 1});
    observation.at({0, 0}) = a;
    observation.at({1, 0}) = b;
    observation.at({2, 0}) = c;
    return observation;
}

Transition transitionAt(std::size_t actionIndex, const Tensor<float>& observation, float reward,
                        const Tensor<float>& nextObservation, bool done = false) {
    Transition transition;
    transition.observation = observation;
    transition.actionIndex = actionIndex;
    transition.reward = reward;
    transition.nextObservation = nextObservation;
    transition.done = done;
    return transition;
}

/// Même méthode que `test_actor_critic_loss.cpp` : différences finies centrées sur chaque
/// poids/biais du réseau principal, réseau cible tenu fixe.
float maxAbsoluteGradientError(QNetwork& mainNetwork, QNetwork& targetNetwork,
                               const std::vector<Transition>& batch, float epsilon = 1e-3f) {
    const std::vector<aisolver::autodiff::NodePtr> parameters = mainNetwork.parameters();

    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
    const aisolver::autodiff::NodePtr loss =
        computeDqnLoss(mainNetwork, targetNetwork, batch, kGamma);
    aisolver::autodiff::backward(loss);

    std::vector<Tensor<float>> analyticGrads;
    analyticGrads.reserve(parameters.size());
    for (const auto& parameter : parameters) {
        analyticGrads.push_back(parameter->grad.clone());
    }

    float maxError = 0.0f;
    for (std::size_t paramIndex = 0; paramIndex < parameters.size(); ++paramIndex) {
        auto& parameter = parameters[paramIndex];
        for (std::size_t elementIndex = 0; elementIndex < parameter->value.size(); ++elementIndex) {
            const float original = parameter->value.data()[elementIndex];

            parameter->value.data()[elementIndex] = original + epsilon;
            const float lossPlus =
                computeDqnLoss(mainNetwork, targetNetwork, batch, kGamma)->value.data()[0];

            parameter->value.data()[elementIndex] = original - epsilon;
            const float lossMinus =
                computeDqnLoss(mainNetwork, targetNetwork, batch, kGamma)->value.data()[0];

            parameter->value.data()[elementIndex] = original;

            const float numericGrad = (lossPlus - lossMinus) / (2.0f * epsilon);
            const float analyticGrad = analyticGrads[paramIndex].data()[elementIndex];
            const float error = std::abs(numericGrad - analyticGrad);
            if (error > maxError) {
                maxError = error;
            }
        }
    }
    return maxError;
}

}  // namespace

/**
 * @brief Sur un mini-lot de 3 transitions non terminales, le gradient de `backward()` par rapport
 * au réseau principal correspond au gradient par différences finies.
 * \castest{<b>computeDqnLoss : gradient checking par rapport au reseau principal.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Reseaux principal/cible minuscules, mini-lot de 3 transitions.<br/>2. Comparer
 * gradients analytique et numerique.<br/>
 * \tattendu Ecart maximal `< 1e-2`.}
 */
TEST(DqnLossTest, GradientCheckingParRapportAuReseauPrincipal) {
    Rng mainRng(1);
    QNetwork mainNetwork(kInputSize, kHiddenSize, mainRng);
    Rng targetRng(2);
    QNetwork targetNetwork(kInputSize, kHiddenSize, targetRng);

    std::vector<Transition> batch;
    batch.push_back(
        transitionAt(0, observationOf(0.1f, 0.2f, 0.3f), 1.0f, observationOf(0.0f, 0.1f, -0.2f)));
    batch.push_back(
        transitionAt(5, observationOf(-0.4f, 0.0f, 0.2f), -0.5f, observationOf(0.3f, -0.1f, 0.1f)));
    batch.push_back(transitionAt(12, observationOf(0.3f, -0.1f, -0.2f), 0.2f,
                                 observationOf(-0.2f, 0.2f, 0.0f)));

    const float maxError = maxAbsoluteGradientError(mainNetwork, targetNetwork, batch);
    EXPECT_LT(maxError, 1e-2f);
}

/**
 * @brief Une transition terminale (`done == true`) réduit la cible de Bellman à la seule récompense
 * immédiate, sans dépendre de la valeur du réseau cible sur l'observation suivante.
 * \castest{<b>computeDqnLoss : transition terminale ignore le reseau cible.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Un mini-lot d'une transition, `done = true`.<br/>2. Comparer la perte a l'erreur
 * quadratique calculee a la main a partir de la seule recompense.<br/>
 * \tattendu Perte egale (a la precision flottante pres) a `(Q(s,a) - reward)^2`.}
 */
TEST(DqnLossTest, TransitionTerminaleIgnoreLeReseauCible) {
    Rng mainRng(3);
    QNetwork mainNetwork(kInputSize, kHiddenSize, mainRng);
    Rng targetRng(4);
    QNetwork targetNetwork(kInputSize, kHiddenSize, targetRng);

    const Tensor<float> observation = observationOf(0.2f, -0.1f, 0.4f);
    const Tensor<float> nextObservation = observationOf(0.5f, 0.5f, 0.5f);
    constexpr std::size_t kActionIndex = 6;
    constexpr float kReward = 0.75f;

    const float predictedQ = mainNetwork.forward(observation)->value.data()[kActionIndex];
    const float expectedLoss = (predictedQ - kReward) * (predictedQ - kReward);

    std::vector<Transition> batch;
    batch.push_back(
        transitionAt(kActionIndex, observation, kReward, nextObservation, /*done=*/true));
    const float actualLoss =
        computeDqnLoss(mainNetwork, targetNetwork, batch, kGamma)->value.data()[0];

    EXPECT_NEAR(actualLoss, expectedLoss, 1e-4f);
}

/**
 * @brief Le réseau cible n'accumule jamais de gradient : sa valeur n'est utilisée que comme
 * constante détachée dans la cible de Bellman.
 * \castest{<b>computeDqnLoss : le reseau cible n'accumule aucun gradient.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `backward()` sur la perte d'un mini-lot non terminal.<br/>2. Lire le gradient de
 * chaque parametre du reseau cible.<br/>
 * \tattendu Gradient nul sur tous les parametres du reseau cible.}
 */
TEST(DqnLossTest, LeReseauCibleNAccumuleAucunGradient) {
    Rng mainRng(5);
    QNetwork mainNetwork(kInputSize, kHiddenSize, mainRng);
    Rng targetRng(6);
    QNetwork targetNetwork(kInputSize, kHiddenSize, targetRng);

    std::vector<Transition> batch;
    batch.push_back(
        transitionAt(0, observationOf(0.1f, 0.1f, 0.1f), 1.0f, observationOf(0.2f, 0.2f, 0.2f)));

    for (const auto& parameter : mainNetwork.parameters()) {
        parameter->zeroGrad();
    }
    for (const auto& parameter : targetNetwork.parameters()) {
        parameter->zeroGrad();
    }

    const auto loss = computeDqnLoss(mainNetwork, targetNetwork, batch, kGamma);
    aisolver::autodiff::backward(loss);

    for (const auto& parameter : targetNetwork.parameters()) {
        for (std::size_t i = 0; i < parameter->grad.size(); ++i) {
            EXPECT_FLOAT_EQ(parameter->grad.data()[i], 0.0f);
        }
    }
}
