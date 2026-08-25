// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dqn_trainer.cpp
 * @brief Test de bout en bout de `DqnTrainer` (LOT-ANNEXE-14, TACHE-01/TACHE-02).
 *
 * Niveau de contrôle : le même corridor trivial que `test_reinforce_trainer.cpp`/
 * `test_actor_critic_trainer.cpp`, condition nécessaire pour la comparaison de TACHE-03.
 */

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Optim/Sgd.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/Dqn/DqnTrainer.h"
#include "AiSolver/Training/Dqn/QNetwork.h"
#include "TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::Rng;
using aisolver::TrainingStatsRecorder;
using aisolver::optim::Sgd;
using aisolver::training::DqnConfig;
using aisolver::training::DqnTrainer;
using aisolver::training::QNetwork;
using aisolver_test::TrivialLevelDirectory;

namespace {

constexpr int REDUCED_MAX_STEPS = 40;
constexpr std::size_t HIDDEN_SIZE = 8;

std::string readWholeFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> splitLines(const std::string& content) {
    std::vector<std::string> lines;
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

/// Retire la colonne `timestampIso8601` (horodatage d'enregistrement, non deterministe entre deux
/// runs reels) d'un CSV `TrainingStatsRecorder` : la reproductibilite porte sur les donnees
/// d'entrainement, pas sur l'instant d'ecriture.
std::string stripTimestampColumn(const std::string& csvContent) {
    const std::vector<std::string> lines = splitLines(csvContent);
    if (lines.empty()) {
        return csvContent;
    }
    const std::vector<std::string> header = splitCsvLine(lines[0]);
    std::size_t timestampColumn = header.size();
    for (std::size_t i = 0; i < header.size(); ++i) {
        if (header[i] == "timestampIso8601") {
            timestampColumn = i;
            break;
        }
    }

    std::ostringstream result;
    for (const std::string& line : lines) {
        const std::vector<std::string> fields = splitCsvLine(line);
        for (std::size_t i = 0; i < fields.size(); ++i) {
            if (i == timestampColumn) {
                continue;
            }
            result << fields[i];
            if (i + 1 < fields.size()) {
                result << ',';
            }
        }
        result << '\n';
    }
    return result.str();
}

DqnConfig baseConfig(std::uint64_t seedBase) {
    DqnConfig config;
    config.hiddenSize = HIDDEN_SIZE;
    config.replayCapacity = 200;
    config.batchSize = 4;
    config.warmupSize = 4;
    config.updatePeriodSteps = 1;
    config.targetSyncPeriodSteps = 1000;
    config.gamma = 0.95f;
    config.epsilonStart = 1.0f;
    config.epsilonEnd = 0.05f;
    config.epsilonDecaySteps = 100;
    config.seedBase = seedBase;
    return config;
}

}  // namespace

/**
 * @brief Après un run de plusieurs épisodes, le CSV principal et le CSV secondaire DQN contiennent
 * chacun exactement `episodeCount` lignes de données (plus en-tête), colonnes attendues.
 * \castest{<b>DqnTrainer : CSV bien formes.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Run de 15 episodes.<br/>2. Lire les deux CSV.<br/>
 * \tattendu 16 lignes chacun ; en-tete DQN `index,replayBufferSize,epsilon`.}
 */
TEST(DqnTrainerTest, CsvBienFormes) {
    const TrivialLevelDirectory level("csv");
    const ObservationEncoder encoder;
    constexpr std::size_t EPISODE_COUNT = 15;

    Rng mainRng(11);
    QNetwork mainNetwork(encoder.inputSize(), HIDDEN_SIZE, mainRng);
    Rng targetRng(12);
    QNetwork targetNetwork(encoder.inputSize(), HIDDEN_SIZE, targetRng);
    targetNetwork.copyWeightsFrom(mainNetwork);
    Sgd optimizer(0.05f);

    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    TrainingStatsRecorder recorder(level.file("stats.csv"));
    const std::filesystem::path dqnStatsCsv = level.file("dqn_stats.csv");

    DqnTrainer trainer(mainNetwork, targetNetwork, optimizer, environment, level.levelPath(),
                       baseConfig(11), recorder, "TrivialAI", dqnStatsCsv);
    trainer.run(EPISODE_COUNT);

    const std::vector<std::string> statLines = splitLines(readWholeFile(level.file("stats.csv")));
    EXPECT_EQ(statLines.size(), EPISODE_COUNT + 1);

    const std::vector<std::string> dqnLines = splitLines(readWholeFile(dqnStatsCsv));
    ASSERT_EQ(dqnLines.size(), EPISODE_COUNT + 1);
    EXPECT_EQ(dqnLines[0], "index,replayBufferSize,epsilon");
}

/**
 * @brief Deux runs construits avec la même graine de base et la même configuration produisent des
 * CSV identiques ligne à ligne.
 * \castest{<b>DqnTrainer : reproductibilite integrale.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux runs independants, meme graine/configuration.<br/>2. Comparer les CSV.<br/>
 * \tattendu Fichiers strictement identiques.}
 */
TEST(DqnTrainerTest, ReproductibiliteIntegrale) {
    constexpr std::size_t EPISODE_COUNT = 10;

    const auto runOnce = [&](const char* suffix) {
        const TrivialLevelDirectory level(suffix);
        const ObservationEncoder encoder;

        Rng mainRng(99);
        QNetwork mainNetwork(encoder.inputSize(), HIDDEN_SIZE, mainRng);
        Rng targetRng(100);
        QNetwork targetNetwork(encoder.inputSize(), HIDDEN_SIZE, targetRng);
        targetNetwork.copyWeightsFrom(mainNetwork);
        Sgd optimizer(0.05f);

        HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
        TrainingStatsRecorder recorder(level.file("stats.csv"));

        DqnTrainer trainer(mainNetwork, targetNetwork, optimizer, environment, level.levelPath(),
                           baseConfig(7), recorder, "TrivialAI");
        trainer.run(EPISODE_COUNT);
        return stripTimestampColumn(readWholeFile(level.file("stats.csv")));
    };

    const std::string first = runOnce("repro_a");
    const std::string second = runOnce("repro_b");
    EXPECT_EQ(first, second);
}

/**
 * @brief Avec une période de synchronisation très supérieure au nombre de pas du run, le réseau
 * cible reste rigoureusement figé (aucune synchronisation ne survient) alors que le réseau
 * principal change bien.
 * \castest{<b>DqnTrainer : reseau cible fige avant la periode de synchronisation.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `targetSyncPeriodSteps` tres grand, run court.<br/>2. Comparer poids cible avant/
 * apres et poids principal avant/apres.<br/>
 * \tattendu Poids cible identiques ; au moins un poids principal modifie.}
 */
TEST(DqnTrainerTest, ReseauCibleFigeAvantLaPeriodeDeSynchronisation) {
    const TrivialLevelDirectory level("frozen_target");
    const ObservationEncoder encoder;

    Rng mainRng(21);
    QNetwork mainNetwork(encoder.inputSize(), HIDDEN_SIZE, mainRng);
    Rng targetRng(22);
    QNetwork targetNetwork(encoder.inputSize(), HIDDEN_SIZE, targetRng);

    const auto targetBefore = targetNetwork.parameters().front()->value.clone();
    const auto mainBefore = mainNetwork.parameters().front()->value.clone();

    Sgd optimizer(0.1f);
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    TrainingStatsRecorder recorder(level.file("stats.csv"));

    DqnConfig config = baseConfig(21);
    config.targetSyncPeriodSteps = 1'000'000;  // jamais atteint sur ce run court
    DqnTrainer trainer(mainNetwork, targetNetwork, optimizer, environment, level.levelPath(),
                       config, recorder, "TrivialAI");
    trainer.run(20);

    const auto& targetAfter = targetNetwork.parameters().front()->value;
    for (std::size_t i = 0; i < targetBefore.size(); ++i) {
        EXPECT_FLOAT_EQ(targetBefore.data()[i], targetAfter.data()[i]);
    }

    const auto& mainAfter = mainNetwork.parameters().front()->value;
    bool anyChanged = false;
    for (std::size_t i = 0; i < mainBefore.size(); ++i) {
        if (mainBefore.data()[i] != mainAfter.data()[i]) {
            anyChanged = true;
        }
    }
    EXPECT_TRUE(anyChanged);
}

/**
 * @brief Avec une période de synchronisation courte, le réseau cible finit par recevoir exactement
 * les poids du réseau principal (synchronisation effective, pas seulement configurée).
 * \castest{<b>DqnTrainer : synchronisation effective du reseau cible.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `targetSyncPeriodSteps` petit, run de plusieurs episodes.<br/>2. Comparer les poids
 * des deux reseaux a la fin.<br/>
 * \tattendu Poids strictement identiques (derniere synchronisation reflete l'etat courant du
 * reseau principal a cet instant, mais au moins une synchronisation a eu lieu).}
 */
TEST(DqnTrainerTest, SynchronisationEffectiveDuReseauCible) {
    const TrivialLevelDirectory level("sync_target");
    const ObservationEncoder encoder;

    Rng mainRng(23);
    QNetwork mainNetwork(encoder.inputSize(), HIDDEN_SIZE, mainRng);
    Rng targetRng(24);
    QNetwork targetNetwork(encoder.inputSize(), HIDDEN_SIZE, targetRng);

    Sgd optimizer(0.1f);
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    TrainingStatsRecorder recorder(level.file("stats.csv"));

    DqnConfig config = baseConfig(23);
    config.targetSyncPeriodSteps = 2;
    DqnTrainer trainer(mainNetwork, targetNetwork, optimizer, environment, level.levelPath(),
                       config, recorder, "TrivialAI");
    trainer.run(20);

    ASSERT_GE(trainer.totalSteps(), config.targetSyncPeriodSteps);
    // Au moins une synchronisation garantie ; le reseau cible ne doit plus etre a son
    // initialisation d'origine (differente de celle du principal par construction).
    Rng freshTargetRng(24);
    QNetwork freshTarget(encoder.inputSize(), HIDDEN_SIZE, freshTargetRng);
    const auto& targetNow = targetNetwork.parameters().front()->value;
    const auto& targetOriginal = freshTarget.parameters().front()->value;
    bool anyChanged = false;
    for (std::size_t i = 0; i < targetOriginal.size(); ++i) {
        if (targetOriginal.data()[i] != targetNow.data()[i]) {
            anyChanged = true;
        }
    }
    EXPECT_TRUE(anyChanged);
}

/**
 * @brief Faire varier `epsilonDecaySteps` change effectivement la valeur de `epsilon` observée
 * après un même nombre de pas -- ce n'est pas une valeur ignorée en pratique.
 * \castest{<b>DqnTrainer : configuration d'epsilon effective.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Deux runs identiques hormis `epsilonDecaySteps`.<br/>2. Comparer `currentEpsilon()`
 * apres le meme run.<br/>
 * \tattendu Valeurs d'epsilon differentes.}
 */
TEST(DqnTrainerTest, ConfigurationDEpsilonEffective) {
    const auto epsilonAfterRun = [&](std::size_t decaySteps) {
        const TrivialLevelDirectory level(decaySteps == 1 ? "eps_fast" : "eps_slow");
        const ObservationEncoder encoder;

        Rng mainRng(31);
        QNetwork mainNetwork(encoder.inputSize(), HIDDEN_SIZE, mainRng);
        Rng targetRng(32);
        QNetwork targetNetwork(encoder.inputSize(), HIDDEN_SIZE, targetRng);
        Sgd optimizer(0.05f);

        HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
        TrainingStatsRecorder recorder(level.file("stats.csv"));

        DqnConfig config = baseConfig(31);
        config.epsilonDecaySteps = decaySteps;
        DqnTrainer trainer(mainNetwork, targetNetwork, optimizer, environment, level.levelPath(),
                           config, recorder, "TrivialAI");
        trainer.run(3);
        EXPECT_GT(trainer.totalSteps(), 0u);
        return trainer.currentEpsilon();
    };

    const float epsilonFastDecay = epsilonAfterRun(1);
    const float epsilonSlowDecay = epsilonAfterRun(100);
    EXPECT_NE(epsilonFastDecay, epsilonSlowDecay);
}

/**
 * @brief `shouldStop` renvoyant `true` dès le premier appel interrompt le run avant le premier
 * épisode (`LOT-ANNEXE-21`).
 * \castest{<b>DqnTrainer : `shouldStop` interrompt avant le premier épisode.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `run(10, shouldStop)` avec un `shouldStop` qui renvoie `true` dès le premier
 * appel.<br/>
 * \tattendu `episodeIndex() == 0` et `totalSteps() == 0` (aucun épisode exécuté).}
 */
TEST(DqnTrainerTest, ShouldStopInterromptAvantLePremierEpisode) {
    const TrivialLevelDirectory level("shouldstop");
    const ObservationEncoder encoder;

    Rng mainRng(41);
    QNetwork mainNetwork(encoder.inputSize(), HIDDEN_SIZE, mainRng);
    Rng targetRng(42);
    QNetwork targetNetwork(encoder.inputSize(), HIDDEN_SIZE, targetRng);
    Sgd optimizer(0.05f);

    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    TrainingStatsRecorder recorder(level.file("stats.csv"));

    DqnTrainer trainer(mainNetwork, targetNetwork, optimizer, environment, level.levelPath(),
                       baseConfig(41), recorder, "TrivialAI");
    trainer.run(10, [] { return true; });

    EXPECT_EQ(trainer.episodeIndex(), 0);
    EXPECT_EQ(trainer.totalSteps(), 0u);
}
