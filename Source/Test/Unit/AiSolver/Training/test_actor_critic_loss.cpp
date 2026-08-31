// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_actor_critic_loss.cpp
 * @brief Gradient checking de `computeActorCriticLoss` (LOT-ANNEXE-13, TACHE-02), par rapport aux
 * poids de la politique uniquement -- même méthodologie que `test_reinforce_loss.cpp`
 * (LOT-ANNEXE-12).
 */

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/ActorCritic/ActorCriticLoss.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::training::computeActorCriticLoss;
using aisolver::training::Trajectory;
using aisolver::training::TrajectoryStep;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;

namespace {

constexpr std::size_t INPUT_SIZE = 3;
constexpr std::size_t HIDDEN_SIZE = 4;

std::unique_ptr<aisolver::nn::Network> tinyPolicy(std::uint64_t seed) {
    Rng rng(seed);
    return buildNetwork(policyTopology(INPUT_SIZE, HIDDEN_SIZE), rng);
}

Tensor<float> observationOf(float a, float b, float c) {
    Tensor<float> observation({INPUT_SIZE, 1});
    observation.at({0, 0}) = a;
    observation.at({1, 0}) = b;
    observation.at({2, 0}) = c;
    return observation;
}

TrajectoryStep stepAt(std::size_t actionIndex, const Tensor<float>& observation) {
    TrajectoryStep step;
    step.observation = observation;
    step.actionIndex = actionIndex;
    return step;
}

/// Même méthode que `test_reinforce_loss.cpp` : différences finies centrées sur chaque
/// poids/biais de la politique.
float maxAbsoluteGradientError(aisolver::nn::Network& policy, const Trajectory& trajectory,
                               const std::vector<float>& advantages, float epsilon = 1e-3f) {
    const std::vector<aisolver::autodiff::NodePtr> parameters = policy.parameters();

    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
    const aisolver::autodiff::NodePtr loss = computeActorCriticLoss(policy, trajectory, advantages);
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
                computeActorCriticLoss(policy, trajectory, advantages)->value.data()[0];

            parameter->value.data()[elementIndex] = original - epsilon;
            const float lossMinus =
                computeActorCriticLoss(policy, trajectory, advantages)->value.data()[0];

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
 * @brief Sur une trajectoire de 3 pas à avantages de signes mélangés, le gradient de `backward()`
 * correspond au gradient par différences finies.
 * \castest{<b>computeActorCriticLoss : gradient checking par rapport à la politique.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Réseau minuscule, 3 pas, avantages `[2, -1, 0.5]`.<br/>2. Comparer gradients
 * analytique et numérique.<br/>
 * \tattendu Écart maximal `< 1e-2`.}
 */
TEST(ActorCriticLossTest, GradientCheckingParRapportALaPolitique) {
    auto policy = tinyPolicy(1);
    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(0, observationOf(0.1f, 0.2f, 0.3f)));
    trajectory.steps.push_back(stepAt(5, observationOf(-0.4f, 0.0f, 0.2f)));
    trajectory.steps.push_back(stepAt(12, observationOf(0.3f, -0.1f, -0.2f)));
    const std::vector<float> advantages{2.0f, -1.0f, 0.5f};

    const float maxError = maxAbsoluteGradientError(*policy, trajectory, advantages);
    EXPECT_LT(maxError, 1e-2f);
}

/**
 * @brief Un avantage nul en entrée produit un gradient rigoureusement nul, tout comme pour
 * `computeReinforceLoss` avec un retour nul (même formule factorisée).
 * \castest{<b>computeActorCriticLoss : avantage nul, gradient nul.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Réseau minuscule, trajectoire d'un pas, avantage `0`.<br/>2. `backward()`.<br/>
 * \tattendu Gradient nul sur tous les paramètres.}
 */
TEST(ActorCriticLossTest, AvantageNulGradientNul) {
    auto policy = tinyPolicy(2);
    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(7, observationOf(0.2f, 0.3f, -0.1f)));
    const std::vector<float> advantages{0.0f};

    const std::vector<aisolver::autodiff::NodePtr> parameters = policy->parameters();
    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
    // Terme d'entropie desactive : c'est la nullite du gradient du terme PONDERE qui est en jeu
    // ici, et l'entropie ne depend pas des avantages (elle a son propre test,
    // test_policy_gradient_loss.cpp).
    const aisolver::autodiff::NodePtr loss =
        computeActorCriticLoss(*policy, trajectory, advantages, 0.0f);
    aisolver::autodiff::backward(loss);

    for (const auto& parameter : parameters) {
        for (std::size_t i = 0; i < parameter->grad.size(); ++i) {
            EXPECT_FLOAT_EQ(parameter->grad.data()[i], 0.0f);
        }
    }
}
