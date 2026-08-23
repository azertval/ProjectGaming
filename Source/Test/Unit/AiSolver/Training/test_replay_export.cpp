// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_replay_export.cpp
 * @brief Export au format de rejeu v1 et point d'entrée minimal (LOT-ANNEXE-11, TACHE-03).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Replay/ReplayFile.h"
#include "AiSolver/Training/DeterministicReplay.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/LevelTrainingSession.h"
#include "AiSolver/Training/ReplayExport.h"
#include "AiSolver/Training/TrainingResult.h"
#include "TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::ReplayLoadResult;
using aisolver::readReplay;
using aisolver::training::DeterministicReplayResult;
using aisolver::training::exportReplay;
using aisolver::training::LevelTrainingSession;
using aisolver::training::replayBestIndividual;
using aisolver::training::ReplayExportError;
using aisolver::training::ReplayExportResult;
using aisolver::training::StoppingConfig;
using aisolver::training::TrainAndExportOutcome;
using aisolver::training::trainLevelAndExportReplay;
using aisolver::training::TrainingResult;
using aisolver::training::evolutionary::EvolutionaryConfig;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

// Budget de pas reduit (vs. 3000 par defaut) : le niveau trivial se resout en quelques pas.
constexpr int kReducedMaxSteps = 50;

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

}  // namespace

/**
 * @brief L'export d'une séquence résolue produit un fichier conforme au format v1, relu à
 * l'identique par `readReplay`.
 * \castest{<b>exportReplay : export réussi et round-trip.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Entraîner et rejouer un individu résolu.<br/>2. Exporter, puis relire le fichier
 * produit.<br/>
 * \tattendu L'export réussit ; la séquence relue est identique à celle exportée.}
 */
TEST(ReplayExportTest, ExportReussiEtRoundTrip) {
    const TrivialLevelDirectory level("export-roundtrip");
    TrainingResult training = trainSolvedIndividual(level);
    ASSERT_TRUE(training.solved);

    HeadlessLevelEnvironment environment;
    const DeterministicReplayResult replay =
        replayBestIndividual(training.bestIndividual, environment, level.levelPath());
    ASSERT_EQ(replay.status, aisolver::EpisodeStatus::Won);

    const std::filesystem::path outputPath = level.file("replay.json");
    const ReplayExportResult exportResult =
        exportReplay(replay, training.solved, level.levelPath(), outputPath, "evolutionnaire", 4242);

    ASSERT_TRUE(exportResult.exported);
    EXPECT_EQ(exportResult.error, ReplayExportError::None);

    const ReplayLoadResult loaded = readReplay(outputPath);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    EXPECT_EQ(loaded.replay->steps.size(), replay.steps.size());
    EXPECT_EQ(loaded.replay->algorithmName, "evolutionnaire");
    EXPECT_EQ(loaded.replay->seed, 4242u);
}

/**
 * @brief Une tentative d'export d'un résultat non résolu échoue explicitement, sans écrire de
 * fichier.
 * \castest{<b>exportReplay : refus d'export sur échec.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire un rejeu quelconque, `solved = false`.<br/>2. Tenter l'export.<br/>
 * \tattendu L'export échoue (`ReplayExportError::NotSolved`) ; aucun fichier n'est créé.}
 */
TEST(ReplayExportTest, RefusDExportSurEchec) {
    const TrivialLevelDirectory level("export-refus");
    const std::filesystem::path outputPath = level.file("replay.json");

    DeterministicReplayResult replay;
    replay.status = aisolver::EpisodeStatus::TimedOut;

    const ReplayExportResult exportResult =
        exportReplay(replay, /*solved=*/false, level.levelPath(), outputPath, "evolutionnaire", 1);

    EXPECT_FALSE(exportResult.exported);
    EXPECT_EQ(exportResult.error, ReplayExportError::NotSolved);
    EXPECT_FALSE(std::filesystem::exists(outputPath));
}

/**
 * @brief Le fichier exporté référence le nom du niveau source utilisé pour l'entraînement.
 * \castest{<b>exportReplay : référence au bon niveau.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Entraîner, rejouer et exporter sur le niveau trivial.<br/>2. Relire le fichier.<br/>
 * \tattendu `ReplayFile::levelPath` correspond au nom de fichier du niveau source.}
 */
TEST(ReplayExportTest, ReferenceAuBonNiveau) {
    const TrivialLevelDirectory level("export-reference");
    TrainingResult training = trainSolvedIndividual(level);
    ASSERT_TRUE(training.solved);

    HeadlessLevelEnvironment environment;
    const DeterministicReplayResult replay =
        replayBestIndividual(training.bestIndividual, environment, level.levelPath());
    const std::filesystem::path outputPath = level.file("replay.json");
    ASSERT_TRUE(
        exportReplay(replay, training.solved, level.levelPath(), outputPath, "evolutionnaire", 4242)
            .exported);

    const ReplayLoadResult loaded = readReplay(outputPath);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    EXPECT_EQ(loaded.replay->levelPath, level.levelPath().filename().string());
}

/**
 * @brief Sur le niveau trivial et un plafond de générations réduit, un appel unique au point
 * d'entrée minimal produit un fichier de rejeu valide sur disque.
 * \castest{<b>trainLevelAndExportReplay : bout en bout.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Appeler `trainLevelAndExportReplay` sur le niveau trivial.<br/>
 * \tattendu L'entraînement résout le niveau ; l'export réussit ; le fichier de rejeu existe et se
 * relit.}
 */
TEST(ReplayExportTest, PointDEntreeMinimalBoutEnBout) {
    const TrivialLevelDirectory level("bout-en-bout");
    const ObservationEncoder encoder;

    EvolutionaryConfig config;
    config.populationSize = 32;
    StoppingConfig stopping;
    stopping.requiredConsecutiveSuccesses = 3;
    stopping.maxGenerations = 200;

    const std::filesystem::path outputPath = level.file("replay.json");
    const TrainAndExportOutcome outcome =
        trainLevelAndExportReplay(level.levelPath(), policyTopology(encoder.inputSize()), config,
                                  stopping, 4242, level.file("stats.csv"), outputPath,
                                  EnvironmentConfig{.maxSteps = kReducedMaxSteps});

    EXPECT_TRUE(outcome.trainingResult.solved);
    EXPECT_TRUE(outcome.exportResult.exported);
    ASSERT_TRUE(std::filesystem::exists(outputPath));

    const ReplayLoadResult loaded = readReplay(outputPath);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    EXPECT_FALSE(loaded.replay->steps.empty());
}
