// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_return_calculator.cpp
 * @brief Tests unitaires de `computeReturns` (LOT-ANNEXE-12, TACHE-02).
 */

#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Training/PolicyGradient/ReturnCalculator.h"

using aisolver::training::computeReturns;
using aisolver::training::Trajectory;
using aisolver::training::TrajectoryStep;

namespace {

Trajectory trajectoryFromRewards(const std::vector<float>& rewards) {
    Trajectory trajectory;
    for (float reward : rewards) {
        TrajectoryStep step;
        step.reward = reward;
        trajectory.steps.push_back(step);
    }
    return trajectory;
}

}  // namespace

/**
 * @brief Avec `gamma = 1` (pas d'actualisation), le retour au premier pas égale la somme brute de
 * toutes les récompenses, et le retour au dernier pas égale sa propre récompense.
 * \castest{<b>computeReturns : gamma = 1 (Monte-Carlo brut).</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Trajectoire `[1, 2, 3, 4]`.<br/>2. `computeReturns(trajectory, 1.0f)`.<br/>
 * \tattendu `returns[0] == 10`, `returns[3] == 4`.}
 */
TEST(ReturnCalculatorTest, GammaUnSommeBrute) {
    const Trajectory trajectory = trajectoryFromRewards({1.0f, 2.0f, 3.0f, 4.0f});
    const std::vector<float> returns = computeReturns(trajectory, 1.0f);

    ASSERT_EQ(returns.size(), 4u);
    EXPECT_FLOAT_EQ(returns[0], 10.0f);
    EXPECT_FLOAT_EQ(returns[3], 4.0f);
}

/**
 * @brief Avec `gamma = 0` (myope), le retour à chaque pas égale exactement sa récompense immédiate.
 * \castest{<b>computeReturns : gamma = 0 (myope).</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Trajectoire `[1, 2, 3, 4]`.<br/>2. `computeReturns(trajectory, 0.0f)`.<br/>
 * \tattendu `returns == rewards`, pas à pas.}
 */
TEST(ReturnCalculatorTest, GammaZeroMyope) {
    const std::vector<float> rewards{1.0f, 2.0f, 3.0f, 4.0f};
    const Trajectory trajectory = trajectoryFromRewards(rewards);
    const std::vector<float> returns = computeReturns(trajectory, 0.0f);

    ASSERT_EQ(returns.size(), rewards.size());
    for (std::size_t i = 0; i < rewards.size(); ++i) {
        EXPECT_FLOAT_EQ(returns[i], rewards[i]);
    }
}

/**
 * @brief Avec un `gamma` intermédiaire, le retour à chaque pas suit exactement la récurrence
 * `G_t = reward_t + gamma * G_{t+1}`, vérifiée à la main sur une trajectoire connue.
 * \castest{<b>computeReturns : récurrence à gamma intermédiaire.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Trajectoire `[1, 1, 1]`, `gamma = 0.9`.<br/>2. `computeReturns`.<br/>
 * \tattendu `returns == [1 + 0.9 + 0.81, 1 + 0.9, 1]`, à la tolérance flottante usuelle.}
 */
TEST(ReturnCalculatorTest, RecurrenceAGammaIntermediaire) {
    const Trajectory trajectory = trajectoryFromRewards({1.0f, 1.0f, 1.0f});
    const std::vector<float> returns = computeReturns(trajectory, 0.9f);

    ASSERT_EQ(returns.size(), 3u);
    EXPECT_NEAR(returns[2], 1.0f, 1e-6f);
    EXPECT_NEAR(returns[1], 1.0f + 0.9f, 1e-6f);
    EXPECT_NEAR(returns[0], 1.0f + 0.9f + 0.81f, 1e-6f);
}

/**
 * @brief Une trajectoire d'un seul pas produit un retour égal à sa récompense, sans récursion à
 * tort.
 * \castest{<b>computeReturns : trajectoire d'un seul pas.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Trajectoire `[5]`.<br/>2. `computeReturns(trajectory, 0.5f)`.<br/>
 * \tattendu `returns == [5]`.}
 */
TEST(ReturnCalculatorTest, TrajectoireDUnSeulPas) {
    const Trajectory trajectory = trajectoryFromRewards({5.0f});
    const std::vector<float> returns = computeReturns(trajectory, 0.5f);

    ASSERT_EQ(returns.size(), 1u);
    EXPECT_FLOAT_EQ(returns[0], 5.0f);
}

/**
 * @brief Le vecteur renvoyé a exactement la même taille que `trajectory.steps`, quelle que soit la
 * longueur de l'épisode.
 * \castest{<b>computeReturns : monotonie de la longueur.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Trajectoires de longueurs 0, 1, 7.<br/>2. `computeReturns` sur chacune.<br/>
 * \tattendu Longueur du vecteur retour toujours égale à la longueur de la trajectoire d'entrée.}
 */
TEST(ReturnCalculatorTest, MonotonieDeLaLongueur) {
    EXPECT_EQ(computeReturns(trajectoryFromRewards({}), 0.9f).size(), 0u);
    EXPECT_EQ(computeReturns(trajectoryFromRewards({1.0f}), 0.9f).size(), 1u);
    EXPECT_EQ(
        computeReturns(trajectoryFromRewards({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f}), 0.9f)
            .size(),
        7u);
}
