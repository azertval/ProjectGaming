// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_episode.cpp
 * @brief Tests unitaires de aisolver::classifyEpisode (LOT-ANNEXE-08, TACHE-02).
 */

#include <gtest/gtest.h>

#include "AiSolver/Env/Episode.h"
#include "Core/Levels/LevelOutcome.h"

/**
 * @brief `Won`/`Lost` dominent toujours, même au-delà du plafond de pas ou du seuil de blocage.
 * \castest{<b>`Won`/`Lost` dominent toujours les critères artificiels.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `classifyEpisode(Won, ...)` avec `stepIndex`/`stepsSinceProgress` au-delà des deux
 * plafonds.<br/>2. Idem avec `Lost`.<br/>
 * \tattendu Les deux appels renvoient respectivement `Won` et `Lost`, jamais `TimedOut`/`Stuck`.}
 */
TEST(EpisodeTest, VictoireEtDefaitePrimentSurLesCriteresArtificiels) {
    EXPECT_EQ(aisolver::classifyEpisode(core::LevelOutcome::Won, 1000, 1000, 100, 50),
              aisolver::EpisodeStatus::Won);
    EXPECT_EQ(aisolver::classifyEpisode(core::LevelOutcome::Lost, 1000, 1000, 100, 50),
              aisolver::EpisodeStatus::Lost);
}

/**
 * @brief `TimedOut` se déclenche exactement au plafond de pas, jamais avant ni après.
 * \castest{<b>`TimedOut` se déclenche exactement au plafond de pas.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `stepIndex = hardStepBudget - 1`.<br/>2. `stepIndex = hardStepBudget`.<br/>
 * \tattendu `Ongoing` avant, `TimedOut` exactement au plafond.}
 */
TEST(EpisodeTest, TimedOutDeclencheExactementAuPlafond) {
    constexpr int BUDGET = 100;
    EXPECT_EQ(aisolver::classifyEpisode(core::LevelOutcome::Playing, BUDGET - 1, 0, BUDGET, 50),
              aisolver::EpisodeStatus::Ongoing);
    EXPECT_EQ(aisolver::classifyEpisode(core::LevelOutcome::Playing, BUDGET, 0, BUDGET, 50),
              aisolver::EpisodeStatus::TimedOut);
}

/**
 * @brief `Stuck` se déclenche exactement au seuil de stagnation, indépendamment du plafond dur.
 * \castest{<b>`Stuck` se déclenche exactement au seuil de stagnation, bien avant le plafond
 * dur.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `stepsSinceProgress = threshold - 1`, `stepIndex` loin du plafond.<br/>2.
 * `stepsSinceProgress = threshold`.<br/>
 * \tattendu `Ongoing` avant, `Stuck` exactement au seuil.}
 */
TEST(EpisodeTest, StuckDeclencheExactementAuSeuilIndependammentDuPlafond) {
    constexpr int THRESHOLD = 30;
    constexpr int BUDGET = 10000;
    EXPECT_EQ(
        aisolver::classifyEpisode(core::LevelOutcome::Playing, 5, THRESHOLD - 1, BUDGET, THRESHOLD),
        aisolver::EpisodeStatus::Ongoing);
    EXPECT_EQ(
        aisolver::classifyEpisode(core::LevelOutcome::Playing, 5, THRESHOLD, BUDGET, THRESHOLD),
        aisolver::EpisodeStatus::Stuck);
}

/**
 * @brief `Ongoing` dans tous les autres cas : progression en cours, sous les deux plafonds.
 * \castest{<b>`Ongoing` sous les deux plafonds.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `classifyEpisode(Playing, ...)` avec `stepIndex`/`stepsSinceProgress` loin des deux
 * plafonds.<br/>
 * \tattendu `Ongoing`.}
 */
TEST(EpisodeTest, OngoingSousLesDeuxPlafonds) {
    EXPECT_EQ(aisolver::classifyEpisode(core::LevelOutcome::Playing, 5, 2, 1000, 50),
              aisolver::EpisodeStatus::Ongoing);
}

/**
 * @brief Un agent qui progresse lentement mais sûrement dépasse le seuil de stagnation sans
 *        jamais déclencher `Stuck`, tant qu'il progresse régulièrement.
 * \castest{<b>Une progression régulière (jamais bloquée) ne déclenche jamais `Stuck`, même en
 * approchant le plafond dur.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `classifyEpisode` avec `stepsSinceProgress = 0` (progression a chaque pas) et
 * `stepIndex` proche mais sous le plafond dur.<br/>
 * \tattendu `Ongoing`, jamais `Stuck`.}
 */
TEST(EpisodeTest, ProgressionReguliereJamaisStuck) {
    constexpr int BUDGET = 1000;
    EXPECT_EQ(aisolver::classifyEpisode(core::LevelOutcome::Playing, BUDGET - 1, 0, BUDGET, 50),
              aisolver::EpisodeStatus::Ongoing);
}
