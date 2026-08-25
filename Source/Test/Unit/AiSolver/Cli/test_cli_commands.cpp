// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cli_commands.cpp
 * @brief Tests d'intégration de `runTrain`/`runEvaluate`/`runExportReplay` (LOT-ANNEXE-19,
 * TACHE-01, `EX-IA-020`), bout en bout sur le niveau de contrôle trivial.
 */

#include <filesystem>
#include <sstream>

#include <gtest/gtest.h>

#include "../Training/TrivialLevelFixture.h"
#include "AiSolver/Cli/Commands.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Eval/ActionDecodingMode.h"
#include "AiSolver/Eval/BenchmarkConfig.h"
#include "AiSolver/Eval/BenchmarkRunner.h"
#include "AiSolver/Eval/EvolutionaryTrainedPolicy.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/Serialization.h"
#include "AiSolver/Replay/ReplayFile.h"
#include "AiSolver/Replay/ReplayValidation.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

using namespace aisolver::cli;
using aisolver_test::TrivialLevelDirectory;

namespace {

std::filesystem::path findFile(const std::filesystem::path& root, const char* name) {
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::recursive_directory_iterator(root)) {
        if (entry.path().filename() == name) {
            return entry.path();
        }
    }
    return {};
}

}  // namespace

/**
 * @brief `runTrain` avec `--algo evo` sur un niveau trivial produit stats, modele, configuration
 * resolue et rejeu.
 * \castest{`runTrain` evolutionniste -> CSV, modele, config, rejeu.<br/>
 * \tcat Integration · AiSolver Cli<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `TrainArgs` (`algorithmId=evo`, niveau trivial, population reduite).<br/>2.
 * `runTrain`.<br/>
 * \tattendu Code de sortie `0` ; `stats.csv`, `model.bin`, `config.json` et `replay.json`
 * (niveau resolu) presents dans le dossier de run.}
 */
TEST(CliCommandsTest, RunTrainEvoProduitStatsModeleEtRejeu) {
    const TrivialLevelDirectory level("cli_train_evo");
    TrainArgs args;
    args.level = level.levelPath();
    args.algorithmId = "evo";
    args.seed = 1;
    args.runsRoot = level.file("runs");
    args.populationSize = 8;

    EXPECT_EQ(runTrain(args), 0);

    EXPECT_FALSE(findFile(args.runsRoot, "stats.csv").empty());
    EXPECT_FALSE(findFile(args.runsRoot, "model.bin").empty());
    EXPECT_FALSE(findFile(args.runsRoot, "config.json").empty());
    // Niveau trivial : resolu de facon quasi certaine par l'evolutionniste.
    EXPECT_FALSE(findFile(args.runsRoot, "replay.json").empty());
}

/**
 * @brief `runTrain` rejette un chemin de niveau introuvable sans planter.
 * \castest{Niveau introuvable -> rejet propre.<br/>
 * \tcat Integration · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `TrainArgs` avec un chemin de niveau inexistant.<br/>2. `runTrain`.<br/>
 * \tattendu Code de sortie non nul, aucune exception.}
 */
TEST(CliCommandsTest, RunTrainRejetteUnNiveauIntrouvable) {
    TrainArgs args;
    args.level = "chemin/vers/rien.json";
    args.algorithmId = "evo";
    EXPECT_NE(runTrain(args), 0);
}

/**
 * @brief `runEvaluate` produit un taux de reussite numeriquement identique a un appel direct a
 * `BenchmarkRunner::run` dans les memes conditions.
 * \castest{`runEvaluate` vs appel direct -> resultat identique.<br/>
 * \tcat Integration · AiSolver Cli<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Entrainement `runTrain` (`evo`) pour obtenir un modele.<br/>2. `runEvaluate`
 * sur ce modele, taux de reussite capture sur `stdout`.<br/>3. Appel direct a
 * `BenchmarkRunner::run` avec les memes repetitions/mode de decodage.<br/>
 * \tattendu Les deux taux de reussite sont strictement egaux (`EXPECT_DOUBLE_EQ`).}
 */
TEST(CliCommandsTest, RunEvaluateCoherentAvecUnAppelDirectAuHarnais) {
    const TrivialLevelDirectory level("cli_evaluate");
    TrainArgs trainArgs;
    trainArgs.level = level.levelPath();
    trainArgs.algorithmId = "evo";
    trainArgs.seed = 2;
    trainArgs.runsRoot = level.file("runs");
    trainArgs.populationSize = 8;
    ASSERT_EQ(runTrain(trainArgs), 0);

    const std::filesystem::path modelPath = findFile(trainArgs.runsRoot, "model.bin");
    ASSERT_FALSE(modelPath.empty());

    EvaluateArgs evalArgs;
    evalArgs.model = modelPath;
    evalArgs.algorithmId = "evo";
    evalArgs.level = level.levelPath();
    evalArgs.repetitions = 4;

    std::ostringstream captured;
    std::streambuf* previousCout = std::cout.rdbuf(captured.rdbuf());
    const int code = runEvaluate(evalArgs);
    std::cout.rdbuf(previousCout);
    ASSERT_EQ(code, 0);

    double cliSuccessRate = -1.0;
    {
        const std::string output = captured.str();
        const std::string marker = "taux de reussite=";
        const std::size_t position = output.find(marker);
        ASSERT_NE(position, std::string::npos) << output;
        cliSuccessRate = std::stod(output.substr(position + marker.size()));
    }

    // Appel direct de reference, memes conditions (repetitions, Argmax) que runEvaluate.
    aisolver::Rng rng(0);
    const auto topology = aisolver::training::evolutionary::policyTopology(
        aisolver::ObservationEncoder().inputSize());
    auto network = aisolver::training::evolutionary::buildNetwork(topology, rng);
    ASSERT_TRUE(aisolver::nn::loadWeights(*network, modelPath));
    aisolver::eval::EvolutionaryTrainedPolicy policy(*network);
    aisolver::eval::BenchmarkConfig benchmarkConfig;
    benchmarkConfig.repetitions = 4;
    benchmarkConfig.decodingMode = aisolver::eval::ActionDecodingMode::Argmax;
    const aisolver::eval::BenchmarkResult direct =
        aisolver::eval::BenchmarkRunner::run(policy, level.levelPath(), benchmarkConfig);

    EXPECT_DOUBLE_EQ(cliSuccessRate, direct.successRate());
}

/**
 * @brief `runExportReplay` produit un fichier de rejeu valide pour un modele entraine.
 * \castest{`runExportReplay` -> rejeu valide.<br/>
 * \tcat Integration · AiSolver Cli<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Entrainement `runTrain` (`evo`) pour obtenir un modele.<br/>2.
 * `runExportReplay` sur ce modele.<br/>3. `readReplay` puis `validateReplay` sur le
 * fichier produit.<br/>
 * \tattendu Code de sortie `0` ; `validateReplay` renvoie `std::nullopt` ; `algorithmId ==
 * "evo"`.}
 */
TEST(CliCommandsTest, RunExportReplayProduitUnRejeuValide) {
    const TrivialLevelDirectory level("cli_export_replay");
    TrainArgs trainArgs;
    trainArgs.level = level.levelPath();
    trainArgs.algorithmId = "evo";
    trainArgs.seed = 3;
    trainArgs.runsRoot = level.file("runs");
    trainArgs.populationSize = 8;
    ASSERT_EQ(runTrain(trainArgs), 0);

    const std::filesystem::path modelPath = findFile(trainArgs.runsRoot, "model.bin");
    ASSERT_FALSE(modelPath.empty());

    ExportReplayArgs exportArgs;
    exportArgs.model = modelPath;
    exportArgs.algorithmId = "evo";
    exportArgs.level = level.levelPath();
    exportArgs.output = level.file("exported_replay.json");
    exportArgs.seed = 3;

    ASSERT_EQ(runExportReplay(exportArgs), 0);

    const aisolver::ReplayLoadResult loaded = aisolver::readReplay(exportArgs.output);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const std::optional<aisolver::ReplayValidationError> validation =
        aisolver::validateReplay(*loaded.replay, level.levelPath().parent_path());
    EXPECT_FALSE(validation.has_value());
    EXPECT_EQ(loaded.replay->algorithmId, "evo");
}
