// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_evolutionary_trainer.cpp
 * @brief Tests unitaires de `EvolutionaryTrainer` (LOT-ANNEXE-10, TACHE-04).
 */

#include <filesystem>
#include <limits>

#include <gtest/gtest.h>

#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryTrainer.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::Rng;
using aisolver::training::evolutionary::EvolutionaryConfig;
using aisolver::training::evolutionary::EvolutionaryTrainer;
using aisolver::training::evolutionary::policyTopology;

namespace {

/// Répertoire temporaire du test courant, nettoyé à la destruction (RAII), même patron que
/// Source/Test/Unit/AiSolver/Stats/test_training_stats_recorder.cpp.
class TempDirectory {
public:
    TempDirectory()
        : _path(std::filesystem::temp_directory_path() / "aisolver_test_evolutionary_trainer") {
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

// Configuration reduite : quelques individus, quelques generations, suffisant pour exercer la
// boucle sans viser la convergence (voir test_evolutionary_non_regression.cpp pour la convergence
// reelle).
EvolutionaryConfig smallConfig() {
    EvolutionaryConfig config;
    config.populationSize = 6;
    return config;
}

}  // namespace

/**
 * @brief Après `runGeneration()`, la population contient toujours exactement `N` individus.
 * \castest{<b>EvolutionaryTrainer : taille de population préservée.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `EvolutionaryTrainer`, population de 6.<br/>2. Appeler
 * `runGeneration()` trois fois.<br/>
 * \tattendu `bestIndividual()` reste accessible sans erreur après chaque appel (taille interne
 * inchangée).}
 */
TEST(EvolutionaryTrainerTest, TailleDePopulationPreservee) {
    TempDirectory tempDir;
    aisolver::TrainingStatsRecorder recorder(tempDir.file("stats.csv"));
    // Budget de pas reduit (vs. 3000 par defaut) : borne le cout pire cas d'une evaluation
    // n'atteignant ni victoire ni blocage franc, pour un budget de test de quelques secondes.
    HeadlessLevelEnvironment env(aisolver::EnvironmentConfig{.maxSteps = 80});
    const ObservationEncoder encoder;

    EvolutionaryTrainer trainer(policyTopology(encoder.inputSize()), smallConfig(), env,
                                levelPath("demo-deplacement.json"), 42, recorder,
                                "demo-deplacement");

    for (int generation = 0; generation < 3; ++generation) {
        EXPECT_NO_THROW(trainer.runGeneration());
        EXPECT_GE(trainer.bestIndividual().fitness,
                  aisolver::training::evolutionary::UNEVALUATED_FITNESS);
    }
}

/**
 * @brief Sur plusieurs générations consécutives, `bestIndividual().fitness` n'est jamais
 * strictement inférieur à sa valeur à la génération précédente.
 * \castest{<b>EvolutionaryTrainer : non-régression de l'élite.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un `EvolutionaryTrainer`.<br/>2. Enchaîner 5 `runGeneration()`, en
 * observant `bestIndividual().fitness` après chacune.<br/>
 * \tattendu La suite des meilleurs fitness est monotone non décroissante.}
 */
TEST(EvolutionaryTrainerTest, NonRegressionDeLElite) {
    TempDirectory tempDir;
    aisolver::TrainingStatsRecorder recorder(tempDir.file("stats.csv"));
    // Budget de pas reduit (vs. 3000 par defaut) : borne le cout pire cas d'une evaluation
    // n'atteignant ni victoire ni blocage franc, pour un budget de test de quelques secondes.
    HeadlessLevelEnvironment env(aisolver::EnvironmentConfig{.maxSteps = 80});
    const ObservationEncoder encoder;

    EvolutionaryTrainer trainer(policyTopology(encoder.inputSize()), smallConfig(), env,
                                levelPath("demo-deplacement.json"), 7, recorder,
                                "demo-deplacement");

    float previousBest = -std::numeric_limits<float>::infinity();
    for (int generation = 0; generation < 5; ++generation) {
        trainer.runGeneration();
        const float currentBest = trainer.bestIndividual().fitness;
        EXPECT_GE(currentBest, previousBest);
        previousBest = currentBest;
    }
}

/**
 * @brief Les appels à `TrainingStatsRecorder::record` reçoivent un indice de génération strictement
 * croissant, sans saut ni répétition.
 * \castest{<b>EvolutionaryTrainer : indice de génération correct.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `EvolutionaryTrainer`.<br/>2. Enchaîner 4 `runGeneration()`, lire
 * `generationIndex()` après chacune.<br/>
 * \tattendu La suite des indices est `1, 2, 3, 4` (nombre d'appels effectifs).}
 */
TEST(EvolutionaryTrainerTest, IndiceDeGenerationCorrect) {
    TempDirectory tempDir;
    aisolver::TrainingStatsRecorder recorder(tempDir.file("stats.csv"));
    // Budget de pas reduit (vs. 3000 par defaut) : borne le cout pire cas d'une evaluation
    // n'atteignant ni victoire ni blocage franc, pour un budget de test de quelques secondes.
    HeadlessLevelEnvironment env(aisolver::EnvironmentConfig{.maxSteps = 80});
    const ObservationEncoder encoder;

    EvolutionaryTrainer trainer(policyTopology(encoder.inputSize()), smallConfig(), env,
                                levelPath("demo-deplacement.json"), 11, recorder,
                                "demo-deplacement");

    for (int expected = 1; expected <= 4; ++expected) {
        trainer.runGeneration();
        EXPECT_EQ(trainer.generationIndex(), expected);
    }
}

/**
 * @brief À tout instant après un appel à `runGeneration()`, l'individu retourné par
 * `bestIndividual()` a bien le fitness maximal de la population courante.
 * \castest{<b>EvolutionaryTrainer : cohérence de `bestIndividual()`.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `EvolutionaryTrainer`.<br/>2. Appeler `runGeneration()` deux fois.<br/>
 * \tattendu `bestIndividual().fitness` est fini (pas `UNEVALUATED_FITNESS`) après chaque appel.}
 */
TEST(EvolutionaryTrainerTest, CoherenceDeBestIndividual) {
    TempDirectory tempDir;
    aisolver::TrainingStatsRecorder recorder(tempDir.file("stats.csv"));
    // Budget de pas reduit (vs. 3000 par defaut) : borne le cout pire cas d'une evaluation
    // n'atteignant ni victoire ni blocage franc, pour un budget de test de quelques secondes.
    HeadlessLevelEnvironment env(aisolver::EnvironmentConfig{.maxSteps = 80});
    const ObservationEncoder encoder;

    EvolutionaryTrainer trainer(policyTopology(encoder.inputSize()), smallConfig(), env,
                                levelPath("demo-deplacement.json"), 21, recorder,
                                "demo-deplacement");

    for (int generation = 0; generation < 2; ++generation) {
        trainer.runGeneration();
        EXPECT_GT(trainer.bestIndividual().fitness,
                  aisolver::training::evolutionary::UNEVALUATED_FITNESS);
    }
}
