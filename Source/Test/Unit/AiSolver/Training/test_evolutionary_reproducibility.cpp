// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_evolutionary_reproducibility.cpp
 * @brief Reproductibilité stricte à seed fixée et sensibilité effective à la seed (LOT-ANNEXE-10,
 * TACHE-05).
 */

#include <cstdint>
#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryTrainer.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::TrainingStatsRecorder;
using aisolver::training::evolutionary::EvolutionaryConfig;
using aisolver::training::evolutionary::EvolutionaryTrainer;
using aisolver::training::evolutionary::policyTopology;

namespace {

/// Répertoire temporaire du test courant, nettoyé à la destruction (RAII), même patron que
/// Source/Test/Unit/AiSolver/Stats/test_training_stats_recorder.cpp.
class TempDirectory {
public:
    TempDirectory()
        : _path(std::filesystem::temp_directory_path() /
                "aisolver_test_evolutionary_reproducibility") {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
        std::filesystem::create_directories(_path);
    }
    ~TempDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }

    [[nodiscard]] std::filesystem::path file(const char* name) const {
        return _path / name;
    }

private:
    std::filesystem::path _path;
};

std::filesystem::path levelPath(const char* file) {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
}

// Population/generations reduites : quelques secondes suffisent a exercer plusieurs generations
// completes de croisement/mutation, sans viser la convergence (voir
// test_evolutionary_non_regression.cpp).
EvolutionaryConfig testConfig() {
    EvolutionaryConfig config;
    config.populationSize = 6;
    return config;
}

constexpr int GENERATIONS = 4;

struct RunResult {
    std::vector<float> bestFitnessPerGeneration;
    std::vector<float> finalBestWeights;
};

RunResult runTrainer(std::uint64_t seed, const TempDirectory& tempDir, const char* csvName) {
    TrainingStatsRecorder recorder(tempDir.file(csvName));
    // Budget de pas reduit (vs. 3000 par defaut) : borne le cout pire cas d'une evaluation
    // n'atteignant ni victoire ni blocage franc, pour un budget de test de quelques secondes.
    HeadlessLevelEnvironment env(aisolver::EnvironmentConfig{.maxSteps = 80});
    const ObservationEncoder encoder;

    EvolutionaryTrainer trainer(policyTopology(encoder.inputSize()), testConfig(), env,
                                levelPath("demo-deplacement.json"), seed, recorder,
                                "demo-deplacement");

    RunResult result;
    for (int generation = 0; generation < GENERATIONS; ++generation) {
        trainer.runGeneration();
        result.bestFitnessPerGeneration.push_back(trainer.bestIndividual().fitness);
    }
    for (const auto& parameter : trainer.bestIndividual().network().parameters()) {
        for (std::size_t i = 0; i < parameter->value.size(); ++i) {
            result.finalBestWeights.push_back(parameter->value.data()[i]);
        }
    }
    return result;
}

}  // namespace

/**
 * @brief Deux instances complètes d'`EvolutionaryTrainer`, mêmes paramètres et même seed,
 * produisent des poids finaux et un historique de fitness strictement identiques.
 * \castest{<b>EvolutionaryTrainer : reproductibilité stricte à seed fixée.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux `EvolutionaryTrainer` identiques (même config, même seed, même niveau), 6
 * générations chacun.<br/>2. Comparer bit-à-bit l'historique de meilleur fitness et les poids
 * finaux du meilleur individu.<br/>
 * \tattendu Égalité stricte, génération par génération et poids par poids.}
 */
TEST(EvolutionaryReproducibilityTest, ReproductibiliteStricteASeedFixee) {
    TempDirectory tempDir;
    const RunResult first = runTrainer(1234, tempDir, "run_a.csv");
    const RunResult second = runTrainer(1234, tempDir, "run_b.csv");

    ASSERT_EQ(first.bestFitnessPerGeneration.size(), second.bestFitnessPerGeneration.size());
    for (std::size_t i = 0; i < first.bestFitnessPerGeneration.size(); ++i) {
        EXPECT_EQ(first.bestFitnessPerGeneration[i], second.bestFitnessPerGeneration[i]);
    }

    ASSERT_EQ(first.finalBestWeights.size(), second.finalBestWeights.size());
    for (std::size_t i = 0; i < first.finalBestWeights.size(); ++i) {
        EXPECT_EQ(first.finalBestWeights[i], second.finalBestWeights[i]);
    }
}

/**
 * @brief Deux seeds différentes produisent des résultats différents (garde-fou contre un
 * algorithme qui ignorerait la seed).
 * \castest{<b>EvolutionaryTrainer : sensibilité effective à la seed.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux `EvolutionaryTrainer` identiques sauf la seed, 6 générations chacun.<br/>2.
 * Comparer l'historique de meilleur fitness et les poids finaux.<br/>
 * \tattendu Au moins une différence (historique ou poids).}
 */
TEST(EvolutionaryReproducibilityTest, SensibiliteEffectiveALaSeed) {
    TempDirectory tempDir;
    const RunResult first = runTrainer(1234, tempDir, "run_seed_a.csv");
    const RunResult second = runTrainer(5678, tempDir, "run_seed_b.csv");

    bool anyDifferent = false;
    if (first.bestFitnessPerGeneration != second.bestFitnessPerGeneration) {
        anyDifferent = true;
    }
    if (first.finalBestWeights != second.finalBestWeights) {
        anyDifferent = true;
    }
    EXPECT_TRUE(anyDifferent);
}
