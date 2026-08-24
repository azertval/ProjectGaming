// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_critic_loss.cpp
 * @brief Gradient checking de `computeCriticLoss` (LOT-ANNEXE-13, TACHE-03).
 */

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Training/ActorCritic/CriticLoss.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::training::computeCriticLoss;
using aisolver::training::CriticNetwork;
using aisolver::training::Trajectory;
using aisolver::training::TrajectoryStep;

namespace {

constexpr std::size_t kInputSize = 3;
constexpr std::size_t kHiddenSize = 4;

Tensor<float> observationOf(float a, float b, float c) {
    Tensor<float> observation({kInputSize, 1});
    observation.at({0, 0}) = a;
    observation.at({1, 0}) = b;
    observation.at({2, 0}) = c;
    return observation;
}

TrajectoryStep stepAt(const Tensor<float>& observation) {
    TrajectoryStep step;
    step.observation = observation;
    return step;
}

}  // namespace

/**
 * @brief Le gradient de `computeCriticLoss` rétropropagé correspond au gradient par différences
 * finies, sur une trajectoire de plusieurs pas et des retours variés.
 * \castest{<b>computeCriticLoss : gradient checking.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Critique minuscule, 4 pas, retours variés.<br/>2. Comparer gradient analytique et
 * différences finies.<br/>
 * \tattendu Écart maximal `< 1e-2`.}
 */
TEST(CriticLossTest, GradientChecking) {
    Rng rng(1);
    CriticNetwork critic(kInputSize, kHiddenSize, rng);

    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(observationOf(0.1f, 0.2f, 0.3f)));
    trajectory.steps.push_back(stepAt(observationOf(-0.4f, 0.0f, 0.2f)));
    trajectory.steps.push_back(stepAt(observationOf(0.3f, -0.1f, -0.2f)));
    trajectory.steps.push_back(stepAt(observationOf(-0.1f, 0.4f, 0.1f)));
    const std::vector<float> returns{2.0f, -1.0f, 0.5f, 1.2f};

    const std::vector<aisolver::autodiff::NodePtr> parameters = critic.parameters();
    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
    const aisolver::autodiff::NodePtr loss = computeCriticLoss(critic, trajectory, returns);
    aisolver::autodiff::backward(loss);

    std::vector<Tensor<float>> analyticGrads;
    analyticGrads.reserve(parameters.size());
    for (const auto& parameter : parameters) {
        analyticGrads.push_back(parameter->grad.clone());
    }

    constexpr float epsilon = 1e-3f;
    float maxError = 0.0f;
    for (std::size_t paramIndex = 0; paramIndex < parameters.size(); ++paramIndex) {
        auto& parameter = parameters[paramIndex];
        for (std::size_t elementIndex = 0; elementIndex < parameter->value.size(); ++elementIndex) {
            const float original = parameter->value.data()[elementIndex];

            parameter->value.data()[elementIndex] = original + epsilon;
            const float lossPlus = computeCriticLoss(critic, trajectory, returns)->value.data()[0];

            parameter->value.data()[elementIndex] = original - epsilon;
            const float lossMinus = computeCriticLoss(critic, trajectory, returns)->value.data()[0];

            parameter->value.data()[elementIndex] = original;

            const float numericGrad = (lossPlus - lossMinus) / (2.0f * epsilon);
            const float analyticGrad = analyticGrads[paramIndex].data()[elementIndex];
            const float error = std::abs(numericGrad - analyticGrad);
            if (error > maxError) {
                maxError = error;
            }
        }
    }
    EXPECT_LT(maxError, 1e-2f);
}

/**
 * @brief Si la valeur estimée est exactement égale au retour cible pour chaque pas, la perte du
 * critique est nulle.
 * \castest{<b>computeCriticLoss : perte nulle quand la prédiction est exacte.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Retours construits égaux à la valeur estimée courante.<br/>2.
 * `computeCriticLoss`.<br/>
 * \tattendu Perte proche de `0`.}
 */
TEST(CriticLossTest, PerteNulleQuandLaPredictionEstExacte) {
    Rng rng(2);
    CriticNetwork critic(kInputSize, kHiddenSize, rng);

    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(observationOf(0.2f, -0.1f, 0.4f)));
    trajectory.steps.push_back(stepAt(observationOf(0.0f, 0.3f, -0.2f)));

    std::vector<float> returns;
    for (const TrajectoryStep& step : trajectory.steps) {
        returns.push_back(critic.forward(step.observation)->value.data()[0]);
    }

    const aisolver::autodiff::NodePtr loss = computeCriticLoss(critic, trajectory, returns);
    EXPECT_NEAR(loss->value.data()[0], 0.0f, 1e-6f);
}
