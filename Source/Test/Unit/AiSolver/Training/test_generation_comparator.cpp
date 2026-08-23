// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_generation_comparator.cpp
 * @brief Tests unitaires de `compareGenerations`/`evolutionaryEpisodeBudget` (LOT-ANNEXE-14,
 * TACHE-03), puis comparaison chiffrée réelle à quatre séries (évolutionniste, REINFORCE,
 * acteur-critique, DQN) qui clôt l'exigence ferme de la génération 3.
 */

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Optim/Sgd.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/Advanced/DqnTrainer.h"
#include "AiSolver/Training/Advanced/GenerationComparator.h"
#include "AiSolver/Training/Advanced/QNetwork.h"
#include "AiSolver/Training/ActorCritic/ActorCriticTrainer.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryConfig.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryTrainer.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/PolicyGradient/ReinforceTrainer.h"
#include "TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::Rng;
using aisolver::TrainingStatsRecorder;
using aisolver::optim::Sgd;
using aisolver::training::ActorCriticConfig;
using aisolver::training::ActorCriticTrainer;
using aisolver::training::compareConvergence;
using aisolver::training::compareGenerations;
using aisolver::training::CriticNetwork;
using aisolver::training::DqnConfig;
using aisolver::training::DqnTrainer;
using aisolver::training::evolutionaryEpisodeBudget;
using aisolver::training::GenerationComparisonResult;
using aisolver::training::NamedSeries;
using aisolver::training::QNetwork;
using aisolver::training::ReinforceConfig;
using aisolver::training::ReinforceTrainer;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::EvolutionaryConfig;
using aisolver::training::evolutionary::EvolutionaryTrainer;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

void writeSyntheticCsv(const std::filesystem::path& path, const std::vector<float>& rewards,
                       const char* extraColumnName = nullptr, float extraColumnValue = 0.0f) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::trunc);
    if (extraColumnName) {
        file << "index,bestReward," << extraColumnName << '\n';
    } else {
        file << "index,bestReward\n";
    }
    for (std::size_t i = 0; i < rewards.size(); ++i) {
        file << i << ',' << rewards[i];
        if (extraColumnName) {
            file << ',' << extraColumnValue;
        }
        file << '\n';
    }
}

std::filesystem::path syntheticDirectory(const char* suffix) {
    const std::filesystem::path path = std::filesystem::temp_directory_path() /
                                       (std::string("aisolver_test_generation_comparator_") +
                                        suffix);
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
    std::filesystem::create_directories(path);
    return path;
}

}  // namespace

/**
 * @brief Appliqué aux deux mêmes séries qu'un `ConvergenceComparator` direct, le comparateur
 * généralisé produit un résultat identique (non-régression du cas à deux séries).
 * \castest{<b>compareGenerations : non-regression du cas a deux series.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux series synthetiques.<br/>2. `compareConvergence` direct vs `compareGenerations`.<br/>
 * \tattendu Memes metriques (episodesToThreshold, ecart-type) des deux cotes.}
 */
TEST(GenerationComparatorTest, NonRegressionDuCasADeuxSeries) {
    const std::filesystem::path directory = syntheticDirectory("non_regression");
    const std::filesystem::path pathA = directory / "a.csv";
    const std::filesystem::path pathB = directory / "b.csv";
    writeSyntheticCsv(pathA, {0.0f, 2.0f, 4.0f, 6.0f, 8.0f});
    writeSyntheticCsv(pathB, {1.0f, 1.0f, 5.0f, 5.0f, 9.0f});

    const auto directReportA = compareConvergence({pathA}, 5.0f);
    const auto directReportB = compareConvergence({pathB}, 5.0f);

    const std::vector<NamedSeries> series{{"A", {pathA}}, {"B", {pathB}}};
    const std::vector<GenerationComparisonResult> results = compareGenerations(series, 5.0f);

    ASSERT_EQ(results.size(), 2u);
    ASSERT_TRUE(results[0].report.has_value());
    ASSERT_TRUE(results[1].report.has_value());
    EXPECT_EQ(results[0].report->meanEpisodesToThreshold, directReportA.meanEpisodesToThreshold);
    EXPECT_EQ(results[0].report->trialsReachingThreshold, directReportA.trialsReachingThreshold);
    EXPECT_FLOAT_EQ(results[0].report->finalRewardStdDev, directReportA.finalRewardStdDev);
    EXPECT_EQ(results[1].report->meanEpisodesToThreshold, directReportB.meanEpisodesToThreshold);
    EXPECT_EQ(results[1].report->trialsReachingThreshold, directReportB.trialsReachingThreshold);
    EXPECT_FLOAT_EQ(results[1].report->finalRewardStdDev, directReportB.finalRewardStdDev);
}

/**
 * @brief Le comparateur généralisé lit correctement des CSV aux colonnes spécifiques différentes,
 * sans supposer un schéma unique au-delà des colonnes communes déjà standardisées.
 * \castest{<b>compareGenerations : lecture uniforme de CSV heterogenes.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Deux series avec des colonnes supplementaires differentes.<br/>2.
 * `compareGenerations`.<br/>
 * \tattendu Les deux series sont lues sans erreur, resultats presents.}
 */
TEST(GenerationComparatorTest, LectureUniformeDeCsvHeterogenes) {
    const std::filesystem::path directory = syntheticDirectory("heterogeneous");
    const std::filesystem::path pathEvo = directory / "evo.csv";
    const std::filesystem::path pathDqn = directory / "dqn.csv";
    writeSyntheticCsv(pathEvo, {1.0f, 3.0f, 5.0f}, "successRate", 0.5f);
    writeSyntheticCsv(pathDqn, {2.0f, 4.0f, 6.0f}, "epsilon", 0.1f);

    const std::vector<NamedSeries> series{{"Evolutionary", {pathEvo}}, {"Dqn", {pathDqn}}};
    const std::vector<GenerationComparisonResult> results = compareGenerations(series, 100.0f);

    ASSERT_EQ(results.size(), 2u);
    EXPECT_TRUE(results[0].report.has_value());
    EXPECT_TRUE(results[1].report.has_value());
}

/**
 * @brief Une série sans CSV (vide) est signalée explicitement (résultat absent), sans faire
 * échouer ni fausser silencieusement le reste de la comparaison.
 * \castest{<b>compareGenerations : robustesse a une serie manquante ou vide.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Trois series, l'une vide.<br/>2. `compareGenerations`.<br/>
 * \tattendu Rapport absent pour la serie vide, present pour les deux autres.}
 */
TEST(GenerationComparatorTest, RobustesseAUneSerieManquanteOuVide) {
    const std::filesystem::path directory = syntheticDirectory("missing");
    const std::filesystem::path pathA = directory / "a.csv";
    const std::filesystem::path pathC = directory / "c.csv";
    writeSyntheticCsv(pathA, {1.0f, 2.0f});
    writeSyntheticCsv(pathC, {3.0f, 4.0f});

    const std::vector<NamedSeries> series{
        {"A", {pathA}}, {"B (absente)", {}}, {"C", {pathC}}};
    const std::vector<GenerationComparisonResult> results = compareGenerations(series, 100.0f);

    ASSERT_EQ(results.size(), 3u);
    EXPECT_TRUE(results[0].report.has_value());
    EXPECT_FALSE(results[1].report.has_value());
    EXPECT_TRUE(results[2].report.has_value());
}

/**
 * @brief La conversion générations/individus vers un nombre d'épisodes de jeu équivalent est
 * cohérente avec le budget déclaré pour les autres approches (un individu évalué == un épisode).
 * \castest{<b>evolutionaryEpisodeBudget : conversion de budget equitable.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `generationCount = 15`, `populationSize = 8`.<br/>2.
 * `evolutionaryEpisodeBudget`.<br/>
 * \tattendu `120` (== 15 * 8).}
 */
TEST(GenerationComparatorTest, ConversionDeBudgetEquitable) {
    EXPECT_EQ(evolutionaryEpisodeBudget(15, 8), 120u);
    EXPECT_EQ(evolutionaryEpisodeBudget(0, 8), 0u);
    EXPECT_EQ(evolutionaryEpisodeBudget(15, 0), 0u);
}

/**
 * @brief Comparaison chiffrée à quatre séries (TACHE-03) : évolutionniste (LOT-ANNEXE-10),
 * REINFORCE (LOT-ANNEXE-12), acteur-critique (LOT-ANNEXE-13) et DQN (ce lot, TACHE-01), même niveau
 * de contrôle, même budget d'épisodes de jeu (conversion équitable pour l'évolutionniste). Mesure
 * honnête (décision de cadrage de l'épic) : n'impose aucune conclusion figée, vérifie seulement que
 * les quatre rapports sont numériquement valides. Le résumé imprimé ici est celui consigné dans
 * `epic.md`.
 * \castest{<b>Comparaison chiffree a quatre series (cloture generation 3).</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. 3 essais de chaque approche, budget de jeu equivalent, niveau trivial.<br/>2.
 * `compareGenerations` sur les quatre series.<br/>
 * \tattendu Les quatre rapports sont presents, ecart-type fini et positif ou nul.}
 */
TEST(GenerationComparatorTest, ComparaisonAQuatreSeriesClotureGeneration3) {
    const TrivialLevelDirectory level("four_way");
    const ObservationEncoder encoder;
    constexpr int kReducedMaxSteps = 40;
    constexpr int kTrialCount = 3;
    constexpr std::size_t kEpisodeBudget = 120;
    constexpr std::size_t kPopulationSize = 8;
    constexpr std::size_t kGenerationCount = kEpisodeBudget / kPopulationSize;  // 15
    constexpr float kRewardThreshold = 5.0f;

    ASSERT_EQ(evolutionaryEpisodeBudget(kGenerationCount, kPopulationSize), kEpisodeBudget);

    std::vector<std::filesystem::path> evoPaths;
    std::vector<std::filesystem::path> reinforcePaths;
    std::vector<std::filesystem::path> acPaths;
    std::vector<std::filesystem::path> dqnPaths;

    for (int trial = 0; trial < kTrialCount; ++trial) {
        const std::uint64_t seed = 2000 + static_cast<std::uint64_t>(trial);
        const std::string suffix = std::to_string(trial);

        // --- Evolutionniste : kGenerationCount generations x kPopulationSize individus.
        const std::filesystem::path evoPath = level.file(("evo_" + suffix + ".csv").c_str());
        TrainingStatsRecorder evoRecorder(evoPath);
        EvolutionaryConfig evoConfig;
        evoConfig.populationSize = kPopulationSize;
        HeadlessLevelEnvironment evoEnvironment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
        EvolutionaryTrainer evoTrainer(policyTopology(encoder.inputSize()), evoConfig,
                                      evoEnvironment, level.levelPath(), seed, evoRecorder,
                                      "TrivialAI");
        for (std::size_t generation = 0; generation < kGenerationCount; ++generation) {
            evoTrainer.runGeneration();
        }
        evoPaths.push_back(evoPath);

        // --- REINFORCE : kEpisodeBudget episodes.
        Rng reinforceRng(seed);
        auto reinforcePolicy = buildNetwork(policyTopology(encoder.inputSize()), reinforceRng);
        Sgd reinforceOptimizer(0.05f);
        HeadlessLevelEnvironment reinforceEnvironment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
        const std::filesystem::path reinforcePath =
            level.file(("reinforce_" + suffix + ".csv").c_str());
        TrainingStatsRecorder reinforceRecorder(reinforcePath);
        ReinforceConfig reinforceConfig;
        reinforceConfig.seedBase = seed;
        ReinforceTrainer reinforceTrainer(*reinforcePolicy, reinforceOptimizer,
                                          reinforceEnvironment, level.levelPath(),
                                          reinforceConfig, reinforceRecorder, "TrivialAI");
        reinforceTrainer.run(kEpisodeBudget);
        reinforcePaths.push_back(reinforcePath);

        // --- Acteur-critique : kEpisodeBudget episodes.
        Rng acPolicyRng(seed + 500);
        auto acPolicy = buildNetwork(policyTopology(encoder.inputSize()), acPolicyRng);
        Sgd acPolicyOptimizer(0.05f);
        Rng criticRng(seed + 900);
        CriticNetwork critic(encoder.inputSize(), 8, criticRng);
        Sgd criticOptimizer(0.05f);
        HeadlessLevelEnvironment acEnvironment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
        const std::filesystem::path acPath = level.file(("ac_" + suffix + ".csv").c_str());
        TrainingStatsRecorder acRecorder(acPath);
        ActorCriticConfig acConfig;
        acConfig.seedBase = seed;
        ActorCriticTrainer acTrainer(*acPolicy, acPolicyOptimizer, critic, criticOptimizer,
                                    acEnvironment, level.levelPath(), acConfig, acRecorder,
                                    "TrivialAI");
        acTrainer.run(kEpisodeBudget);
        acPaths.push_back(acPath);

        // --- DQN : kEpisodeBudget episodes.
        Rng dqnMainRng(seed + 1300);
        QNetwork dqnMain(encoder.inputSize(), 8, dqnMainRng);
        Rng dqnTargetRng(seed + 1700);
        QNetwork dqnTarget(encoder.inputSize(), 8, dqnTargetRng);
        Sgd dqnOptimizer(0.05f);
        HeadlessLevelEnvironment dqnEnvironment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
        const std::filesystem::path dqnPath = level.file(("dqn_" + suffix + ".csv").c_str());
        TrainingStatsRecorder dqnRecorder(dqnPath);
        DqnConfig dqnConfig;
        dqnConfig.hiddenSize = 8;
        dqnConfig.replayCapacity = 500;
        dqnConfig.batchSize = 16;
        dqnConfig.warmupSize = 16;
        dqnConfig.epsilonDecaySteps = 200;
        dqnConfig.seedBase = seed;
        DqnTrainer dqnTrainer(dqnMain, dqnTarget, dqnOptimizer, dqnEnvironment, level.levelPath(),
                              dqnConfig, dqnRecorder, "TrivialAI");
        dqnTrainer.run(kEpisodeBudget);
        dqnPaths.push_back(dqnPath);
    }

    const std::vector<NamedSeries> series{
        {"Evolutionniste (LOT-ANNEXE-10)", evoPaths},
        {"REINFORCE (LOT-ANNEXE-12)", reinforcePaths},
        {"Acteur-critique (LOT-ANNEXE-13)", acPaths},
        {"DQN (LOT-ANNEXE-14)", dqnPaths},
    };
    const std::vector<GenerationComparisonResult> results =
        compareGenerations(series, kRewardThreshold);

    ASSERT_EQ(results.size(), 4u);
    for (const GenerationComparisonResult& result : results) {
        ASSERT_TRUE(result.report.has_value()) << result.name;
        EXPECT_TRUE(std::isfinite(result.report->finalRewardStdDev)) << result.name;
        EXPECT_GE(result.report->finalRewardStdDev, 0.0f) << result.name;
        std::printf(
            "[LOT-ANNEXE-14 TACHE-03] %-32s : %zu/%zu essais atteignent le plafond (seuil=%.1f), "
            "episodesToThreshold moyen = %s, ecart-type fin de run = %f\n",
            result.name.c_str(), result.report->trialsReachingThreshold, result.report->totalTrials,
            static_cast<double>(kRewardThreshold),
            result.report->meanEpisodesToThreshold
                ? std::to_string(*result.report->meanEpisodesToThreshold).c_str()
                : "N/A",
            static_cast<double>(result.report->finalRewardStdDev));
    }
}
