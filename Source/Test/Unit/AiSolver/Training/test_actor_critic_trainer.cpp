// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_actor_critic_trainer.cpp
 * @brief Test de bout en bout de `ActorCriticTrainer` (LOT-ANNEXE-13, TACHE-03).
 *
 * Niveau de contrôle : le même corridor trivial que `test_reinforce_trainer.cpp`
 * (LOT-ANNEXE-11/12), condition nécessaire pour que la comparaison de LOT-ANNEXE-13 TACHE-04 porte
 * sur exactement le même niveau des deux côtés.
 */

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Optim/Sgd.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/ActorCritic/ActorCriticTrainer.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::Rng;
using aisolver::TrainingStatsRecorder;
using aisolver::optim::Sgd;
using aisolver::training::ActorCriticConfig;
using aisolver::training::ActorCriticTrainer;
using aisolver::training::CriticNetwork;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

constexpr int kReducedMaxSteps = 40;
constexpr std::size_t kEpisodeCount = 80;
constexpr std::size_t kCriticHiddenSize = 8;

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

}  // namespace

/**
 * @brief Sur le niveau de contrôle trivial, l'erreur quadratique moyenne du critique (moyenne des
 * `(valeur_t - retour_t)^2` sur l'épisode) diminue entre le début et la fin d'un run
 * d'entraînement.
 * \castest{<b>ActorCriticTrainer : convergence du critique.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `ActorCriticTrainer` sur le niveau trivial, `80` épisodes.<br/>2. Comparer l'erreur
 * quadratique moyenne du critique des 10 premiers et des 10 derniers épisodes.<br/>
 * \tattendu Erreur moyenne des 10 derniers épisodes strictement inférieure à celle des 10
 * premiers.}
 */
TEST(ActorCriticTrainerTest, ConvergenceDuCritique) {
    const TrivialLevelDirectory level("convergence");
    const ObservationEncoder encoder;

    Rng policyRng(21);
    auto policy = buildNetwork(policyTopology(encoder.inputSize()), policyRng);
    Sgd policyOptimizer(0.05f);

    Rng criticRng(22);
    CriticNetwork critic(encoder.inputSize(), kCriticHiddenSize, criticRng);
    Sgd criticOptimizer(0.05f);

    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    TrainingStatsRecorder recorder(level.file("stats.csv"));
    const std::filesystem::path criticLossCsv = level.file("critic_loss.csv");

    ActorCriticConfig config;
    config.gamma = 0.95f;
    config.seedBase = 21;

    ActorCriticTrainer trainer(*policy, policyOptimizer, critic, criticOptimizer, environment,
                               level.levelPath(), config, recorder, "TrivialAI", criticLossCsv);
    trainer.run(kEpisodeCount);

    const std::vector<std::string> lines = splitLines(readWholeFile(criticLossCsv));
    ASSERT_EQ(lines.size(), kEpisodeCount + 1);

    const auto criticLossOf = [](const std::string& line) {
        const std::size_t comma = line.find(',');
        return std::stof(line.substr(comma + 1));
    };

    float firstTen = 0.0f;
    for (std::size_t i = 1; i <= 10; ++i) {
        firstTen += criticLossOf(lines[i]);
    }
    firstTen /= 10.0f;

    float lastTen = 0.0f;
    for (std::size_t i = lines.size() - 10; i < lines.size(); ++i) {
        lastTen += criticLossOf(lines[i]);
    }
    lastTen /= 10.0f;

    EXPECT_LT(lastTen, firstTen);
}

/**
 * @brief Une exécution où l'optimiseur du critique est volontairement figé (`updateCritic =
 * false`) laisse néanmoins l'optimiseur de la politique continuer à produire des mises à jour
 * normales : les deux réseaux/graphes sont bien séparés.
 * \castest{<b>ActorCriticTrainer : indépendance des deux optimisations.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Run avec `updateCritic = false`.<br/>2. Comparer les poids du critique avant/après
 * (inchangés) et ceux de la politique (changés).<br/>
 * \tattendu Poids du critique strictement identiques ; au moins un poids de politique modifié.}
 */
TEST(ActorCriticTrainerTest, IndependanceDesDeuxOptimisations) {
    const TrivialLevelDirectory level("independance");
    const ObservationEncoder encoder;

    Rng policyRng(31);
    auto policy = buildNetwork(policyTopology(encoder.inputSize()), policyRng);
    Sgd policyOptimizer(0.05f);

    Rng criticRng(32);
    CriticNetwork critic(encoder.inputSize(), kCriticHiddenSize, criticRng);
    Sgd criticOptimizer(0.05f);

    const auto criticWeightsBefore = critic.parameters().front()->value.clone();
    const auto policyWeightsBefore = policy->parameters().front()->value.clone();

    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    TrainingStatsRecorder recorder(level.file("stats.csv"));

    ActorCriticConfig config;
    config.seedBase = 31;
    ActorCriticTrainer trainer(*policy, policyOptimizer, critic, criticOptimizer, environment,
                               level.levelPath(), config, recorder, "TrivialAI");
    trainer.run(20, /*updateCritic=*/false);

    const auto& criticWeightsAfter = critic.parameters().front()->value;
    ASSERT_EQ(criticWeightsBefore.size(), criticWeightsAfter.size());
    for (std::size_t i = 0; i < criticWeightsBefore.size(); ++i) {
        EXPECT_FLOAT_EQ(criticWeightsBefore.data()[i], criticWeightsAfter.data()[i]);
    }

    const auto& policyWeightsAfter = policy->parameters().front()->value;
    bool anyChanged = false;
    for (std::size_t i = 0; i < policyWeightsBefore.size(); ++i) {
        if (policyWeightsBefore.data()[i] != policyWeightsAfter.data()[i]) {
            anyChanged = true;
        }
    }
    EXPECT_TRUE(anyChanged);
}

/**
 * @brief Après un run complet, le gradient accumulé sur les poids de la politique et du critique
 * est nul pour les deux réseaux (remise à zéro entre épisodes, ni fuite d'un épisode à l'autre ni
 * entre acteur et critique).
 * \castest{<b>ActorCriticTrainer : remise à zéro des gradients des deux réseaux.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Run complet (10 épisodes).<br/>2. Lire le gradient de chaque paramètre des deux
 * réseaux.<br/>
 * \tattendu Gradient nul sur tous les paramètres, politique et critique.}
 */
TEST(ActorCriticTrainerTest, RemiseAZeroDesGradientsDesDeuxReseaux) {
    const TrivialLevelDirectory level("zerograd");
    const ObservationEncoder encoder;

    Rng policyRng(41);
    auto policy = buildNetwork(policyTopology(encoder.inputSize()), policyRng);
    Sgd policyOptimizer(0.05f);

    Rng criticRng(42);
    CriticNetwork critic(encoder.inputSize(), kCriticHiddenSize, criticRng);
    Sgd criticOptimizer(0.05f);

    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    TrainingStatsRecorder recorder(level.file("stats.csv"));

    ActorCriticConfig config;
    config.seedBase = 41;
    ActorCriticTrainer trainer(*policy, policyOptimizer, critic, criticOptimizer, environment,
                               level.levelPath(), config, recorder, "TrivialAI");
    trainer.run(10);

    for (const auto& parameter : policy->parameters()) {
        for (std::size_t i = 0; i < parameter->grad.size(); ++i) {
            EXPECT_FLOAT_EQ(parameter->grad.data()[i], 0.0f);
        }
    }
    for (const auto& parameter : critic.parameters()) {
        for (std::size_t i = 0; i < parameter->grad.size(); ++i) {
            EXPECT_FLOAT_EQ(parameter->grad.data()[i], 0.0f);
        }
    }
}

/**
 * @brief Le CSV secondaire de perte du critique contient exactement `episodeCount` lignes (plus
 * l'en-tête) et des valeurs numériquement plausibles (finies).
 * \castest{<b>ActorCriticTrainer : CSV de perte du critique bien formé.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Run de `20` épisodes avec chemin de CSV de perte du critique.<br/>2. Lire le
 * fichier.<br/>
 * \tattendu `21` lignes, toutes les valeurs de perte finies et positives ou nulles.}
 */
TEST(ActorCriticTrainerTest, CsvDePerteDuCritiqueBienForme) {
    const TrivialLevelDirectory level("csv_critic");
    const ObservationEncoder encoder;

    Rng policyRng(51);
    auto policy = buildNetwork(policyTopology(encoder.inputSize()), policyRng);
    Sgd policyOptimizer(0.05f);

    Rng criticRng(52);
    CriticNetwork critic(encoder.inputSize(), kCriticHiddenSize, criticRng);
    Sgd criticOptimizer(0.05f);

    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    TrainingStatsRecorder recorder(level.file("stats.csv"));
    const std::filesystem::path criticLossCsv = level.file("critic_loss.csv");

    ActorCriticConfig config;
    config.seedBase = 51;
    ActorCriticTrainer trainer(*policy, policyOptimizer, critic, criticOptimizer, environment,
                               level.levelPath(), config, recorder, "TrivialAI", criticLossCsv);
    trainer.run(20);

    const std::vector<std::string> lines = splitLines(readWholeFile(criticLossCsv));
    ASSERT_EQ(lines.size(), 21u);
    EXPECT_EQ(lines[0], "index,criticLoss");
    for (std::size_t i = 1; i < lines.size(); ++i) {
        const std::size_t comma = lines[i].find(',');
        ASSERT_NE(comma, std::string::npos);
        const float value = std::stof(lines[i].substr(comma + 1));
        EXPECT_TRUE(std::isfinite(value));
        EXPECT_GE(value, 0.0f);
    }
}
