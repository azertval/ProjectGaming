// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deterministic_replay.cpp
 * @brief Rejeu déterministe du meilleur individu (LOT-ANNEXE-11, TACHE-02).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Training/DeterministicReplay.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/LevelTrainingSession.h"
#include "AiSolver/Training/TrainingResult.h"
#include "Core/Physics/PlayerInput.h"
#include "TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::EpisodeStatus;
using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::training::DeterministicReplayResult;
using aisolver::training::LevelTrainingSession;
using aisolver::training::replayBestIndividual;
using aisolver::training::StoppingConfig;
using aisolver::training::TrainingResult;
using aisolver::training::evolutionary::EvolutionaryConfig;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

// Budget de pas reduit (vs. 3000 par defaut) : le niveau trivial se resout en quelques pas.
constexpr int kReducedMaxSteps = 50;

/// Entraîne un individu qui résout à coup sûr le niveau trivial (mêmes paramètres que
/// `LevelTrainingSessionTest.ArretParResolutionStable`), utilisé comme fixture par les tests
/// ci-dessous : ce lot ne réimplémente jamais l'algorithme évolutionniste, seulement son usage.
TrainingResult trainSolvedIndividual(const TrivialLevelDirectory& level) {
    const ObservationEncoder encoder;
    EvolutionaryConfig config;
    config.populationSize = 32;
    StoppingConfig stopping;
    stopping.requiredConsecutiveSuccesses = 3;
    stopping.maxGenerations = 200;

    LevelTrainingSession session(level.levelPath(), policyTopology(encoder.inputSize()), config,
                                 stopping, 4242, level.file("stats.csv"),
                                 EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    return session.run();
}

bool sameSteps(const std::vector<core::PlayerInput>& lhs, const std::vector<core::PlayerInput>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (std::size_t index = 0; index < lhs.size(); ++index) {
        const core::PlayerInput& a = lhs[index];
        const core::PlayerInput& b = rhs[index];
        if (a.moveX != b.moveX || a.jumpPressed != b.jumpPressed || a.jumpHeld != b.jumpHeld ||
            a.moveY != b.moveY || a.dashPressed != b.dashPressed) {
            return false;
        }
    }
    return true;
}

}  // namespace

/**
 * @brief Le rejeu d'un individu qui a résolu le niveau pendant l'entraînement produit, sur une
 * **nouvelle** instance d'environnement, une séquence qui atteint bien la sortie.
 * \castest{<b>replayBestIndividual : rejeu indépendant réussi.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Entraîner un individu jusqu'à résolution du niveau trivial.<br/>2. Le rejouer sur une
 * nouvelle instance `HeadlessLevelEnvironment`.<br/>
 * \tattendu Le statut du rejeu est `EpisodeStatus::Won`.}
 */
TEST(DeterministicReplayTest, RejeuIndependantReussi) {
    const TrivialLevelDirectory level("rejeu-reussi");
    TrainingResult training = trainSolvedIndividual(level);
    ASSERT_TRUE(training.solved);

    HeadlessLevelEnvironment freshEnvironment;
    const DeterministicReplayResult replay =
        replayBestIndividual(training.bestIndividual, freshEnvironment, level.levelPath());

    EXPECT_EQ(replay.status, aisolver::EpisodeStatus::Won);
    EXPECT_FALSE(replay.steps.empty());
}

/**
 * @brief Deux rejeux successifs du même individu sur le même niveau produisent une séquence
 * d'actions strictement identique.
 * \castest{<b>replayBestIndividual : déterminisme du rejeu.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Entraîner un individu.<br/>2. Le rejouer deux fois, sur deux environnements
 * distincts.<br/>
 * \tattendu Les deux séquences d'actions sont strictement identiques.}
 */
TEST(DeterministicReplayTest, DeterminismeDuRejeu) {
    const TrivialLevelDirectory level("determinisme");
    TrainingResult training = trainSolvedIndividual(level);
    ASSERT_TRUE(training.solved);

    HeadlessLevelEnvironment firstEnvironment;
    HeadlessLevelEnvironment secondEnvironment;
    const DeterministicReplayResult firstReplay =
        replayBestIndividual(training.bestIndividual, firstEnvironment, level.levelPath());
    const DeterministicReplayResult secondReplay =
        replayBestIndividual(training.bestIndividual, secondEnvironment, level.levelPath());

    EXPECT_EQ(firstReplay.status, secondReplay.status);
    EXPECT_FLOAT_EQ(firstReplay.finalReward, secondReplay.finalReward);
    EXPECT_TRUE(sameSteps(firstReplay.steps, secondReplay.steps));
}

/**
 * @brief La séquence produite ne dépasse jamais le budget de pas configuré de l'environnement.
 * \castest{<b>replayBestIndividual : borne de longueur.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Entraîner un individu.<br/>2. Le rejouer sur un environnement au budget de pas
 * réduit et connu.<br/>
 * \tattendu `steps.size()` ne dépasse jamais ce budget.}
 */
TEST(DeterministicReplayTest, BorneDeLongueur) {
    const TrivialLevelDirectory level("borne-longueur");
    TrainingResult training = trainSolvedIndividual(level);
    ASSERT_TRUE(training.solved);

    constexpr int kMaxSteps = 50;
    HeadlessLevelEnvironment boundedEnvironment(EnvironmentConfig{.maxSteps = kMaxSteps});
    const DeterministicReplayResult replay =
        replayBestIndividual(training.bestIndividual, boundedEnvironment, level.levelPath());

    EXPECT_LE(replay.steps.size(), static_cast<std::size_t>(kMaxSteps));
}

/**
 * @brief Le rejeu d'un individu qui ne peut pas progresser (budget de pas nul) produit une
 * séquence exploitable sans exception ni plantage, correctement marquée comme non résolue.
 * \castest{<b>replayBestIndividual : rejeu d'un individu non résolvant.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Entraîner un individu.<br/>2. Le rejouer sur un environnement au budget de pas
 * nul.<br/>
 * \tattendu Aucune exception ; `status != Won` ; la séquence produite est vide (aucun pas
 * simulable).}
 */
TEST(DeterministicReplayTest, RejeuDUnIndividuNonResolvant) {
    const TrivialLevelDirectory level("non-resolvant");
    TrainingResult training = trainSolvedIndividual(level);
    ASSERT_TRUE(training.solved);

    HeadlessLevelEnvironment zeroBudgetEnvironment(EnvironmentConfig{.maxSteps = 0});
    const DeterministicReplayResult replay = replayBestIndividual(
        training.bestIndividual, zeroBudgetEnvironment, level.levelPath());

    EXPECT_NE(replay.status, aisolver::EpisodeStatus::Won);
    EXPECT_TRUE(replay.steps.empty());
}
