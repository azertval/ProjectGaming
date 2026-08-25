// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_advantage_calculator.cpp
 * @brief Tests unitaires de `computeAdvantages` (LOT-ANNEXE-13, TACHE-02).
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Rng.h"
#include "AiSolver/Training/ActorCritic/AdvantageCalculator.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::training::computeAdvantages;
using aisolver::training::CriticNetwork;
using aisolver::training::Trajectory;
using aisolver::training::TrajectoryStep;

namespace {

constexpr std::size_t INPUT_SIZE = 3;
constexpr std::size_t HIDDEN_SIZE = 4;

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

}  // namespace

/**
 * @brief Un pas dont le retour dépasse la valeur estimée produit un avantage positif ; l'inverse
 * produit un avantage négatif.
 * \castest{<b>computeAdvantages : signe de l'avantage.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Critique quelconque, deux pas de retours connus.<br/>2. Comparer chaque avantage à la
 * valeur estimée correspondante.<br/>
 * \tattendu Signe de `advantage[i]` cohérent avec `returns[i] - valeur_estimee[i]`.}
 */
TEST(AdvantageCalculatorTest, SigneDeLAvantage) {
    Rng rng(1);
    CriticNetwork critic(INPUT_SIZE, HIDDEN_SIZE, rng);

    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(0, observationOf(0.1f, 0.2f, 0.3f)));
    trajectory.steps.push_back(stepAt(1, observationOf(-0.2f, 0.1f, 0.0f)));

    const float valueA = critic.forward(trajectory.steps[0].observation)->value.data()[0];
    const float valueB = critic.forward(trajectory.steps[1].observation)->value.data()[0];
    const std::vector<float> returns{valueA + 5.0f, valueB - 5.0f};

    const std::vector<float> advantages = computeAdvantages(returns, critic, trajectory);
    ASSERT_EQ(advantages.size(), 2u);
    EXPECT_GT(advantages[0], 0.0f);
    EXPECT_LT(advantages[1], 0.0f);
}

/**
 * @brief Si le retour observé est exactement la valeur estimée pour chaque pas, l'avantage
 * correspondant est nul.
 * \castest{<b>computeAdvantages : avantage nul dégénéré.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Retours construits égaux à la valeur estimée courante.<br/>2.
 * `computeAdvantages`.<br/>
 * \tattendu Tous les avantages sont nuls (à la précision flottante près).}
 */
TEST(AdvantageCalculatorTest, AvantageNulDegenere) {
    Rng rng(2);
    CriticNetwork critic(INPUT_SIZE, HIDDEN_SIZE, rng);

    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(3, observationOf(0.2f, -0.1f, 0.4f)));
    trajectory.steps.push_back(stepAt(7, observationOf(0.0f, 0.3f, -0.2f)));
    trajectory.steps.push_back(stepAt(2, observationOf(-0.1f, -0.1f, 0.1f)));

    std::vector<float> returns;
    for (const TrajectoryStep& step : trajectory.steps) {
        returns.push_back(critic.forward(step.observation)->value.data()[0]);
    }

    const std::vector<float> advantages = computeAdvantages(returns, critic, trajectory);
    for (const float advantage : advantages) {
        EXPECT_NEAR(advantage, 0.0f, 1e-5f);
    }
}
