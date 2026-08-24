// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cross_level_benchmark.cpp
 * @brief Tests unitaires de `runCrossLevelCampaign`/`writeCrossLevelCsv` (LOT-ANNEXE-16, TACHE-01,
 * `EX-IA-017`), et campagne réelle de transfert (TACHE-02) dont les chiffres affichés par
 * `CrossLevelTransferTest.CampagneReelleDeuxPairesDeNiveaux` sont ceux consignés dans
 * `Documentation/Lot-Annexe/LOT-ANNEXE-16-evaluation-hors-niveau/resultats-transfert.md`.
 */

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Eval/CrossLevelBenchmark.h"
#include "AiSolver/Eval/EvolutionaryTrainedPolicy.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/LevelTrainingSession.h"
#include "AiSolver/Training/TrainingResult.h"
#include "Core/Physics/PlayerInput.h"
#include "../Training/TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::ObservationEncoder;
using aisolver::eval::ActionDecodingMode;
using aisolver::eval::BenchmarkConfig;
using aisolver::eval::CrossLevelBenchmarkResult;
using aisolver::eval::CrossLevelPair;
using aisolver::eval::EvolutionaryTrainedPolicy;
using aisolver::eval::runCrossLevelCampaign;
using aisolver::eval::writeCrossLevelCsv;
using aisolver::training::LevelTrainingSession;
using aisolver::training::StoppingConfig;
using aisolver::training::TrainingResult;
using aisolver::training::evolutionary::EvolutionaryConfig;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

// Politique factice ignorant l'observation, retourne toujours la meme entree joueur -- meme patron
// que test_benchmark_runner.cpp (LOT-ANNEXE-15), sert a isoler runCrossLevelCampaign de tout reseau
// reel pour les tests structurels.
class ScriptedPolicy : public aisolver::eval::TrainedPolicy {
public:
    explicit ScriptedPolicy(core::PlayerInput fixedInput) : _fixedInput(fixedInput) {}

    [[nodiscard]] std::optional<core::PlayerInput> selectAction(const aisolver::Tensor<float>&,
                                                                 ActionDecodingMode,
                                                                 aisolver::Rng&) override {
        return _fixedInput;
    }

private:
    core::PlayerInput _fixedInput;
};

// Corridor plus long (3 pas vers la droite au lieu d'un seul) : meme mecanique que
// TrivialLevelDirectory (deplacement lateral pur, aucun saut), geometrie jamais vue a
// l'entrainement -- paire "mecanique partagee" de la campagne de TACHE-02.
inline constexpr const char* kTrivialLongLevelJson = R"({
  "name": "TrivialLong",
  "width": 6,
  "height": 3,
  "tiles": [
    {"x": 0, "y": 1, "type": "solid"},
    {"x": 1, "y": 1, "type": "entry"},
    {"x": 2, "y": 1, "type": "solid"},
    {"x": 3, "y": 1, "type": "solid"},
    {"x": 4, "y": 1, "type": "exit"},
    {"x": 5, "y": 1, "type": "solid"},
    {"x": 0, "y": 2, "type": "solid"},
    {"x": 1, "y": 2, "type": "solid"},
    {"x": 2, "y": 2, "type": "solid"},
    {"x": 3, "y": 2, "type": "solid"},
    {"x": 4, "y": 2, "type": "solid"},
    {"x": 5, "y": 2, "type": "solid"}
  ]
})";

// Corridor avec une breche d'une case (aucun sol en x=2, ni au niveau du sol) : franchissable
// uniquement en sautant -- une mecanique jamais requise par TrivialLevelDirectory (deplacement pur)
// -- paire "mecanique differente" de la campagne de TACHE-02.
inline constexpr const char* kGapLevelJson = R"({
  "name": "Gap",
  "width": 6,
  "height": 3,
  "tiles": [
    {"x": 0, "y": 1, "type": "solid"},
    {"x": 1, "y": 1, "type": "entry"},
    {"x": 3, "y": 1, "type": "solid"},
    {"x": 4, "y": 1, "type": "exit"},
    {"x": 5, "y": 1, "type": "solid"},
    {"x": 0, "y": 2, "type": "solid"},
    {"x": 1, "y": 2, "type": "solid"},
    {"x": 3, "y": 2, "type": "solid"},
    {"x": 4, "y": 2, "type": "solid"},
    {"x": 5, "y": 2, "type": "solid"}
  ]
})";

class NamedLevelDirectory {
public:
    NamedLevelDirectory(const char* suffix, const char* json)
        : _path(std::filesystem::temp_directory_path() /
                (std::string("aisolver_test_cross_level_") + suffix)) {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
        std::filesystem::create_directories(_path);
        std::ofstream file(levelPath(), std::ios::binary | std::ios::trunc);
        file << json;
    }
    ~NamedLevelDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }

    [[nodiscard]] std::filesystem::path levelPath() const {
        return _path / "level.json";
    }
    [[nodiscard]] std::filesystem::path file(const char* name) const {
        return _path / name;
    }

private:
    std::filesystem::path _path;
};

}  // namespace

/**
 * \castest{Un CrossLevelBenchmarkResult ecrit deux colonnes de niveau distinctes, jamais
 * fusionnees.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire un CrossLevelBenchmarkResult (trainedOnLevel="A", executedOnLevel="B").
 * <br/>2. writeCrossLevelCsv.<br/>
 * \tattendu La ligne CSV contient "A" et "B" dans deux champs separes.}
 */
TEST(CrossLevelBenchmarkTest, DistingueLesDeuxNiveauxDansLeRapport) {
    core::LevelOutcome outcome = core::LevelOutcome::Won;
    aisolver::eval::BenchmarkResult result;
    result.episodes.push_back(aisolver::eval::EpisodeOutcome{outcome, 5});

    std::vector<CrossLevelBenchmarkResult> results;
    results.push_back(CrossLevelBenchmarkResult{"A", "B", result});

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "aisolver_test_cross_level_report.csv";
    writeCrossLevelCsv(results, path);

    std::ifstream file(path);
    std::string header;
    std::getline(file, header);
    EXPECT_EQ(header, "trainedOnLevel,executedOnLevel,successRate,meanStepCount,stepCountStdDev");
    std::string row;
    std::getline(file, row);
    EXPECT_NE(row.find("A,B,"), std::string::npos);

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

/**
 * \castest{runCrossLevelCampaign produit un resultat par paire, dans l'ordre fourni.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Trois paires distinctes, meme politique scriptee, meme niveau trivial.<br/>2.
 * runCrossLevelCampaign.<br/>
 * \tattendu Trois resultats, trainedOnLevel/executedOnLevel corrects et dans l'ordre.}
 */
TEST(CrossLevelBenchmarkTest, CampagneMultiPairesRespecteLOrdre) {
    const TrivialLevelDirectory level("multi_pairs");
    core::PlayerInput rightInput;
    rightInput.moveX = 1.0f;
    ScriptedPolicy policy(rightInput);

    BenchmarkConfig config;
    config.repetitions = 2;
    config.maxStepsPerEpisode = 50;
    config.decodingMode = ActionDecodingMode::Argmax;

    const std::vector<CrossLevelPair> pairs{
        CrossLevelPair{policy, "TrainA", level.levelPath(), "ExecX"},
        CrossLevelPair{policy, "TrainB", level.levelPath(), "ExecY"},
        CrossLevelPair{policy, "TrainC", level.levelPath(), "ExecZ"},
    };

    const std::vector<CrossLevelBenchmarkResult> results = runCrossLevelCampaign(pairs, config);

    ASSERT_EQ(results.size(), 3u);
    EXPECT_EQ(results[0].trainedOnLevel, "TrainA");
    EXPECT_EQ(results[0].executedOnLevel, "ExecX");
    EXPECT_EQ(results[1].trainedOnLevel, "TrainB");
    EXPECT_EQ(results[1].executedOnLevel, "ExecY");
    EXPECT_EQ(results[2].trainedOnLevel, "TrainC");
    EXPECT_EQ(results[2].executedOnLevel, "ExecZ");
    for (const CrossLevelBenchmarkResult& result : results) {
        EXPECT_EQ(result.result.episodes.size(), 2u);
    }
}

/**
 * @brief Campagne reelle de transfert (TACHE-02) : un modele evolutionniste entraine uniquement sur
 * le niveau trivial (deplacement lateral pur, un pas), execute sur deux niveaux jamais vus -- l'un
 * partageant la meme mecanique (corridor plus long), l'autre en exigeant une differente (breche a
 * sauter). Attente déclarée avant mesure (décision de cadrage de l'épic, `EX-IA-017`) : le régime
 * d'entraînement niveau-par-niveau n'exerce aucune pression de généralisation, un transfert faible
 * est attendu et n'est pas un échec du programme. Ce test ne fige aucun seuil de réussite -- seule
 * la validité structurelle des résultats est vérifiée ; les valeurs numériques imprimées ici sont
 * celles recopiées dans `resultats-transfert.md`.
 * \castest{Campagne de transfert reelle sur deux paires de niveaux illustratives.<br/>
 * \tcat Unitaire (integration niveaux reels) · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Entrainement evolutionniste jusqu'a resolution stable sur le niveau trivial.<br/>2.
 * Execution croisee (20 repetitions) sur un niveau de meme mecanique et un niveau de mecanique
 * differente.<br/>
 * \tattendu Deux resultats, taux de reussite dans [0, 1], pas moyen fini et positif.}
 */
TEST(CrossLevelTransferTest, CampagneReelleDeuxPairesDeNiveaux) {
    const TrivialLevelDirectory trainingLevel("transfer_training");
    const NamedLevelDirectory longLevel("transfer_long", kTrivialLongLevelJson);
    const NamedLevelDirectory gapLevel("transfer_gap", kGapLevelJson);
    const ObservationEncoder encoder;
    constexpr int kReducedMaxSteps = 50;

    EvolutionaryConfig trainingConfig;
    trainingConfig.populationSize = 32;
    StoppingConfig stopping;
    stopping.requiredConsecutiveSuccesses = 3;
    stopping.maxGenerations = 200;

    LevelTrainingSession session(trainingLevel.levelPath(), policyTopology(encoder.inputSize()),
                                 trainingConfig, stopping, 7777, trainingLevel.file("stats.csv"),
                                 EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    TrainingResult trainingResult = session.run();
    ASSERT_TRUE(trainingResult.solved);

    EvolutionaryTrainedPolicy policy(trainingResult.bestIndividual.network());

    BenchmarkConfig benchmarkConfig;
    benchmarkConfig.repetitions = 20;
    benchmarkConfig.maxStepsPerEpisode = 100;
    benchmarkConfig.decodingMode = ActionDecodingMode::Argmax;

    const std::vector<CrossLevelPair> pairs{
        CrossLevelPair{policy, "Trivial", longLevel.levelPath(), "TrivialLong"},
        CrossLevelPair{policy, "Trivial", gapLevel.levelPath(), "Gap"},
    };
    const std::vector<CrossLevelBenchmarkResult> results =
        runCrossLevelCampaign(pairs, benchmarkConfig);

    ASSERT_EQ(results.size(), 2u);
    for (const CrossLevelBenchmarkResult& result : results) {
        const double successRate = result.result.successRate();
        EXPECT_GE(successRate, 0.0);
        EXPECT_LE(successRate, 1.0);
        EXPECT_TRUE(std::isfinite(result.result.meanStepsAll()));
        std::printf("[LOT-ANNEXE-16 TACHE-02] %s -> %s : successRate=%.3f meanStepsAll=%.2f\n",
                    result.trainedOnLevel.c_str(), result.executedOnLevel.c_str(), successRate,
                    result.result.meanStepsAll());
        std::fflush(stdout);
    }
}
