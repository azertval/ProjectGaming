// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reward.cpp
 * @brief Tests unitaires de aisolver::computeReward (LOT-ANNEXE-08, TACHE-01).
 */

#include <cmath>

#include <gtest/gtest.h>

#include "AiSolver/Env/Reward.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/Aabb.h"

namespace {

core::Aabb boxAt(float x, float y) {
    return core::Aabb::fromTopLeftSize(core::Vector2{x, y}, core::Vector2{1.0f, 1.0f});
}

}  // namespace

/**
 * @brief Se rapprocher de la sortie produit une récompense de progression strictement positive.
 * \castest{<b>Se rapprocher de la sortie produit une récompense strictement positive.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `previousBox` a distance 10 de la sortie, `currentBox` a distance 9.<br/>
 * \tattendu La récompense est strictement positive.}
 */
TEST(RewardTest, ProgressionPositive) {
    const aisolver::RewardConfig config;
    const core::GridPosition exit{0, 0};
    const float reward =
        aisolver::computeReward(config, boxAt(10.0f, 0.0f), boxAt(9.0f, 0.0f), exit,
                                core::LevelOutcome::Playing);
    EXPECT_GT(reward, 0.0f);
}

/**
 * @brief Reculer ou stagner ne produit jamais une récompense de progression positive.
 * \castest{<b>Reculer ou stagner ne produit jamais une récompense positive.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Recul : `previousBox` plus proche que `currentBox`.<br/>2. Stagnation : boîtes
 * identiques.<br/>
 * \tattendu La récompense est négative ou nulle dans les deux cas.}
 */
TEST(RewardTest, RegressionOuStagnationJamaisPositive) {
    const aisolver::RewardConfig config;
    const core::GridPosition exit{0, 0};

    const float regression =
        aisolver::computeReward(config, boxAt(9.0f, 0.0f), boxAt(10.0f, 0.0f), exit,
                                core::LevelOutcome::Playing);
    EXPECT_LE(regression, 0.0f);

    const float stagnation =
        aisolver::computeReward(config, boxAt(9.0f, 0.0f), boxAt(9.0f, 0.0f), exit,
                                core::LevelOutcome::Playing);
    EXPECT_LE(stagnation, 0.0f);
}

/**
 * @brief Le bonus de complétion domine la pénalité de temps cumulée sur un épisode long.
 * \castest{<b>Le bonus de complétion domine sur un épisode `Won` face à un `Lost`/`Playing` de
 * même progression partielle.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Cumule la récompense de plusieurs centaines de pas identiques, le dernier en
 * `Won`.<br/>2. Meme cumul, dernier pas en `Lost`.<br/>3. Meme cumul, dernier pas en
 * `Playing` (interrompu).<br/>
 * \tattendu La récompense cumulée de l'épisode `Won` est strictement supérieure aux deux
 * autres.}
 */
TEST(RewardTest, BonusDeCompletionDomine) {
    const aisolver::RewardConfig config;
    const core::GridPosition exit{0, 0};
    constexpr int STEPS = 500;

    auto cumulative = [&](core::LevelOutcome finalOutcome) {
        float total = 0.0f;
        for (int step = 0; step < STEPS; ++step) {
            const bool last = (step == STEPS - 1);
            total += aisolver::computeReward(config, boxAt(0.0f, 0.0f), boxAt(0.0f, 0.0f), exit,
                                              last ? finalOutcome : core::LevelOutcome::Playing);
        }
        return total;
    };

    const float won = cumulative(core::LevelOutcome::Won);
    const float lost = cumulative(core::LevelOutcome::Lost);
    const float interrupted = cumulative(core::LevelOutcome::Playing);

    EXPECT_GT(won, lost);
    EXPECT_GT(won, interrupted);
}

/**
 * @brief Les valeurs par défaut de `RewardConfig` ne produisent ni `NaN` ni valeur aberrante.
 * \castest{<b>Les valeurs par défaut de `RewardConfig` restent finies sur des distances/durées
 * typiques.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `computeReward` avec `RewardConfig{}` sur des boîtes et une sortie plausibles pour
 * `Playing`, `Won` et `Lost`.<br/>
 * \tattendu Chaque récompense est finie (ni `NaN` ni infinie).}
 */
TEST(RewardTest, ValeursParDefautPlausibles) {
    const aisolver::RewardConfig config;
    const core::GridPosition exit{20, 10};
    for (const core::LevelOutcome outcome :
        {core::LevelOutcome::Playing, core::LevelOutcome::Won, core::LevelOutcome::Lost}) {
        const float reward =
            aisolver::computeReward(config, boxAt(0.0f, 0.0f), boxAt(1.0f, 0.5f), exit, outcome);
        EXPECT_TRUE(std::isfinite(reward));
    }
}
