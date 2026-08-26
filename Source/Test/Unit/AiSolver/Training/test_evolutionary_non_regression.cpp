// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_evolutionary_non_regression.cpp
 * @brief Non-régression de la croissance du fitness sur un niveau trivial (LOT-ANNEXE-10,
 * TACHE-05).
 */

#include <filesystem>
#include <limits>

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
                "aisolver_test_evolutionary_non_regression") {
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

// Population/generations/budget de pas choisis experimentalement (voir corps du test) : cette
// configuration reduite suffit pour que l'algorithme decouvre une politique qui progresse
// reellement sur demo-deplacement.json (aucun obstacle, seule la mecanique "avancer" est a
// apprendre depuis une politique initialement aleatoire), en ~14 s mesurees localement (build
// Debug) -- tient dans un budget CI raisonnable sans viser la victoire complete du niveau.
constexpr std::size_t POPULATION_SIZE = 10;
constexpr int GENERATIONS = 10;
constexpr int MAX_STEPS_PER_EPISODE = 80;
// Seuil minimal documente : nettement superieur a ce qu'obtiendrait une politique qui ne progresse
// jamais (fitness proche de 0, dominee par la penalite de temps -0.01/pas sur quelques dizaines de
// pas avant blocage), signe d'une progression reelle vers la sortie, sans exiger la victoire
// complete dans le budget de pas reduit ci-dessus -- mesure locale (seed 4242, configuration
// ci-dessus) : ~11.2, marge confortable au-dessus du seuil retenu.
constexpr float MINIMUM_FITNESS_THRESHOLD = 5.0f;

}  // namespace

/**
 * @brief Sur `demo-deplacement.json`, le meilleur fitness croît de façon significative sur une
 * fenêtre de générations fixée, franchit un seuil documenté, et ne régresse jamais d'une génération
 * à l'autre.
 * \castest{<b>EvolutionaryTrainer : croissance du fitness sur niveau trivial.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `EvolutionaryTrainer` sur `demo-deplacement.json`, population `POPULATION_SIZE`,
 * seed fixée.<br/>
 * 2. Enchaîner `GENERATIONS` générations, en observant `bestIndividual().fitness`
 * après chacune.<br/>
 * \tattendu Le meilleur fitness ne régresse jamais d'une génération à l'autre ; le meilleur fitness
 * final dépasse `MINIMUM_FITNESS_THRESHOLD`.}
 */
TEST(EvolutionaryNonRegressionTest, CroissanceDuFitnessSurNiveauTrivial) {
    TempDirectory tempDir;
    TrainingStatsRecorder recorder(tempDir.file("stats.csv"));
    // Budget de pas reduit (vs. 3000 par defaut) : borne le cout pire cas d'une evaluation
    // n'atteignant ni victoire ni blocage franc, pour un budget de test de quelques secondes.
    HeadlessLevelEnvironment env(aisolver::EnvironmentConfig{.maxSteps = MAX_STEPS_PER_EPISODE});
    const ObservationEncoder encoder;

    EvolutionaryConfig config;
    config.populationSize = POPULATION_SIZE;

    EvolutionaryTrainer trainer(policyTopology(encoder.inputSize()), config, env,
                                levelPath("demo-deplacement.json"), 4242, recorder,
                                "demo-deplacement");

    float previousBest = -std::numeric_limits<float>::infinity();
    for (int generation = 0; generation < GENERATIONS; ++generation) {
        trainer.runGeneration();
        const float currentBest = trainer.bestIndividual().fitness;
        EXPECT_GE(currentBest, previousBest) << "generation " << generation;
        previousBest = currentBest;
    }

    EXPECT_GT(previousBest, MINIMUM_FITNESS_THRESHOLD);
}
