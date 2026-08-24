// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reward_episode.cpp
 * @brief Scénarios combinés récompense + classification d'épisode (LOT-ANNEXE-08, TACHE-03) : mort
 * immédiate, complétion immédiate, stagnation prolongée.
 */

#include <gtest/gtest.h>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/GridDistanceField.h"
#include "AiSolver/Env/Reward.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/TileMap.h"
#include "Core/Physics/Aabb.h"

namespace {

core::Aabb boxAt(float x, float y) {
    return core::Aabb::fromTopLeftSize(core::Vector2{x, y}, core::Vector2{1.0f, 1.0f});
}

/// Grille carrée entièrement vide (aucune case solide), assez grande pour couvrir les sorties
/// utilisées par ce fichier.
core::TileMap openMap(int size) {
    return core::TileMap(size, size);
}

}  // namespace

/**
 * @brief Une mort au premier pas produit une récompense cumulée dominée par `deathPenalty` et un
 *        épisode classé `Lost`.
 * \castest{<b>Mort immédiate : récompense dominée par `deathPenalty`, `classifyEpisode == Lost`.
 * </b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Un seul pas, issue `Lost`, sans aucune progression.<br/>
 * \tattendu La récompense cumulée est strictement négative, `classifyEpisode(...) == Lost`.}
 */
TEST(RewardEpisodeTest, MortImmediateRecompenseDomineeParLaPenaliteEtEpisodeLost) {
    const aisolver::RewardConfig config;
    const core::TileMap map = openMap(60);
    const core::GridPosition exit{50, 50};
    const aisolver::GridDistanceField distanceField(map, exit);

    const float reward = aisolver::computeReward(config, distanceField, boxAt(0.0f, 0.0f),
                                                 boxAt(0.0f, 0.0f), core::LevelOutcome::Lost);
    EXPECT_LT(reward, 0.0f);
    EXPECT_NEAR(reward, config.deathPenalty - config.timePenalty, 1e-6f);

    const aisolver::EpisodeStatus status =
        aisolver::classifyEpisode(core::LevelOutcome::Lost, 1, 0, 3000, 100);
    EXPECT_EQ(status, aisolver::EpisodeStatus::Lost);
}

/**
 * @brief Un personnage qui commence déjà sur la sortie (cas synthétique) produit une récompense
 *        dominée par `completionBonus` et un épisode classé `Won`.
 * \castest{<b>Complétion immédiate : récompense dominée par `completionBonus`, `classifyEpisode
 * == Won`.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Un seul pas, boîte déjà centrée sur la sortie, issue `Won`.<br/>
 * \tattendu La récompense cumulée est strictement positive et dominée par `completionBonus`,
 * `classifyEpisode(...) == Won`.}
 */
TEST(RewardEpisodeTest, CompletionImmediateRecompenseDomineeParLeBonusEtEpisodeWon) {
    const aisolver::RewardConfig config;
    const core::TileMap map = openMap(30);
    const core::GridPosition exit{10, 10};
    const aisolver::GridDistanceField distanceField(map, exit);
    const core::Aabb onExit = boxAt(10.0f, 10.0f);

    const float reward =
        aisolver::computeReward(config, distanceField, onExit, onExit, core::LevelOutcome::Won);
    EXPECT_GT(reward, 0.0f);
    EXPECT_NEAR(reward, config.completionBonus - config.timePenalty, 1e-6f);

    const aisolver::EpisodeStatus status =
        aisolver::classifyEpisode(core::LevelOutcome::Won, 1, 0, 3000, 100);
    EXPECT_EQ(status, aisolver::EpisodeStatus::Won);
}

/**
 * @brief Une position figée artificiellement (aucune progression) mène à `Stuck` exactement au
 *        seuil configuré, jamais `Ongoing` au-delà.
 * \castest{<b>Stagnation prolongée : `classifyEpisode == Stuck` exactement au seuil.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Simule `stuckThreshold` pas consécutifs sans amélioration de distance (position
 * figée).<br/>2. Un pas de plus.<br/>
 * \tattendu `Ongoing` à chaque pas sous le seuil, `Stuck` au pas du seuil et à tout pas
 * ultérieur.}
 */
TEST(RewardEpisodeTest, StagnationProlongeeMeneAStuckAuSeuil) {
    constexpr int STUCK_THRESHOLD = 60;
    constexpr int HARD_BUDGET = 3000;

    for (int stepsSinceProgress = 0; stepsSinceProgress < STUCK_THRESHOLD; ++stepsSinceProgress) {
        const aisolver::EpisodeStatus status =
            aisolver::classifyEpisode(core::LevelOutcome::Playing, stepsSinceProgress,
                                      stepsSinceProgress, HARD_BUDGET, STUCK_THRESHOLD);
        EXPECT_EQ(status, aisolver::EpisodeStatus::Ongoing) << "pas " << stepsSinceProgress;
    }

    const aisolver::EpisodeStatus status =
        aisolver::classifyEpisode(core::LevelOutcome::Playing, STUCK_THRESHOLD, STUCK_THRESHOLD,
                                  HARD_BUDGET, STUCK_THRESHOLD);
    EXPECT_EQ(status, aisolver::EpisodeStatus::Stuck);

    const aisolver::EpisodeStatus statusAfter =
        aisolver::classifyEpisode(core::LevelOutcome::Playing, STUCK_THRESHOLD + 1,
                                  STUCK_THRESHOLD + 1, HARD_BUDGET, STUCK_THRESHOLD);
    EXPECT_EQ(statusAfter, aisolver::EpisodeStatus::Stuck);
}
