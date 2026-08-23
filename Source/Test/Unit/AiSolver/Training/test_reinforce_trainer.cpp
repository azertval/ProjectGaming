// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reinforce_trainer.cpp
 * @brief Test de bout en bout de `ReinforceTrainer` (LOT-ANNEXE-12, TACHE-04/TACHE-05).
 *
 * Niveau de contrôle : le corridor trivial de `TrivialLevelFixture.h` (même fixture que
 * LOT-ANNEXE-11), pour que la comparaison chiffrée envisagée par LOT-ANNEXE-13 (TACHE-04) porte sur
 * exactement le même niveau que l'évolutionniste.
 */

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
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/PolicyGradient/ReinforceTrainer.h"
#include "TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::Rng;
using aisolver::TrainingStatsRecorder;
using aisolver::optim::Sgd;
using aisolver::training::ReinforceConfig;
using aisolver::training::ReinforceTrainer;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

// Budget de pas reduit : le corridor trivial se resout en un pas une fois l'action decouverte, le
// reste du budget ne sert qu'a borner le cout pire cas d'une politique encore peu entrainee.
constexpr int kReducedMaxSteps = 40;
constexpr std::size_t kEpisodeCount = 80;

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

/// Retire la colonne `timestampIso8601` (9e, 0-indexee) d'une ligne CSV : seule colonne qui varie
/// legitimement entre deux runs par ailleurs identiques (horloge murale), aucune des donnees de
/// nom de niveau ne contient de virgule ici (pas de re-analyse CSV complete necessaire).
std::string stripTimestampColumn(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ',')) {
        fields.push_back(field);
    }
    if (fields.size() > 9) {
        fields.erase(fields.begin() + 9);
    }
    std::string result;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) {
            result += ',';
        }
        result += fields[i];
    }
    return result;
}

/// Exécute un run complet de `ReinforceTrainer` sur le niveau trivial, avec une seed de base fixée.
void runReinforce(const TrivialLevelDirectory& level, std::uint64_t seedBase,
                  const std::filesystem::path& csvPath) {
    const ObservationEncoder encoder;
    Rng networkRng(seedBase);
    auto policy = buildNetwork(policyTopology(encoder.inputSize()), networkRng);
    Sgd optimizer(0.05f);
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    TrainingStatsRecorder recorder(csvPath);

    ReinforceConfig config;
    config.gamma = 0.95f;
    config.seedBase = seedBase;

    ReinforceTrainer trainer(*policy, optimizer, environment, level.levelPath(), config, recorder,
                             "TrivialAI");
    trainer.run(kEpisodeCount);
}

}  // namespace

/**
 * @brief Sur le niveau de contrôle trivial, la récompense moyenne glissante des derniers épisodes
 * d'un run est significativement supérieure à celle des premiers — signe d'un apprentissage
 * effectivement dirigé par le gradient, pas seulement d'une absence de crash.
 * \castest{<b>ReinforceTrainer : progression de la récompense sur le niveau de contrôle.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `ReinforceTrainer` sur le niveau trivial, `80` épisodes.<br/>2. Comparer la récompense
 * moyenne des 10 premiers et des 10 derniers épisodes (colonne `bestReward` du CSV, un seul épisode
 * par ligne).<br/>
 * \tattendu Moyenne des 10 derniers épisodes strictement supérieure à celle des 10 premiers.}
 */
TEST(ReinforceTrainerTest, ProgressionDeLaRecompenseSurLeNiveauDeControle) {
    const TrivialLevelDirectory level("progression");
    const std::filesystem::path csvPath = level.file("stats.csv");
    runReinforce(level, 12345, csvPath);

    const std::vector<std::string> lines = splitLines(readWholeFile(csvPath));
    ASSERT_EQ(lines.size(), kEpisodeCount + 1);  // + 1 ligne d'en-tete.

    // bestReward est la 2e colonne (index, bestReward, meanReward, ...) -- meme ordre que
    // TrainingStatsRow (Stats/TrainingStatsRecorder.h).
    const auto rewardOf = [](const std::string& line) {
        const std::size_t firstComma = line.find(',');
        const std::size_t secondComma = line.find(',', firstComma + 1);
        return std::stof(line.substr(firstComma + 1, secondComma - firstComma - 1));
    };

    float firstTen = 0.0f;
    for (std::size_t i = 1; i <= 10; ++i) {
        firstTen += rewardOf(lines[i]);
    }
    firstTen /= 10.0f;

    float lastTen = 0.0f;
    for (std::size_t i = lines.size() - 10; i < lines.size(); ++i) {
        lastTen += rewardOf(lines[i]);
    }
    lastTen /= 10.0f;

    EXPECT_GT(lastTen, firstTen);
}

/**
 * @brief Après un run court, le fichier CSV produit contient exactement `episodeCount` lignes (plus
 * l'en-tête), colonnes cohérentes avec les CSV des autres lots (LOT-ANNEXE-09/10).
 * \castest{<b>ReinforceTrainer : CSV bien formé.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Run de `80` épisodes.<br/>2. Compter les lignes du CSV.<br/>
 * \tattendu Exactement `80 + 1` lignes (en-tête incluse).}
 */
TEST(ReinforceTrainerTest, CsvBienForme) {
    const TrivialLevelDirectory level("csv");
    const std::filesystem::path csvPath = level.file("stats.csv");
    runReinforce(level, 7, csvPath);

    const std::vector<std::string> lines = splitLines(readWholeFile(csvPath));
    EXPECT_EQ(lines.size(), kEpisodeCount + 1);
}

/**
 * @brief Deux runs avec la même graine de base et la même configuration produisent des CSV
 * identiques ligne à ligne.
 * \castest{<b>ReinforceTrainer : reproductibilité intégrale.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux runs identiques (même seed de base).<br/>2. Comparer les CSV produits.<br/>
 * \tattendu Contenu strictement identique.}
 */
TEST(ReinforceTrainerTest, ReproductibiliteIntegrale) {
    const TrivialLevelDirectory level("reproductibilite");
    const std::filesystem::path csvA = level.file("run_a.csv");
    const std::filesystem::path csvB = level.file("run_b.csv");

    runReinforce(level, 999, csvA);
    runReinforce(level, 999, csvB);

    const std::vector<std::string> linesA = splitLines(readWholeFile(csvA));
    const std::vector<std::string> linesB = splitLines(readWholeFile(csvB));
    ASSERT_EQ(linesA.size(), linesB.size());
    for (std::size_t i = 0; i < linesA.size(); ++i) {
        EXPECT_EQ(stripTimestampColumn(linesA[i]), stripTimestampColumn(linesB[i]));
    }
}

/**
 * @brief Après un run complet, le gradient accumulé sur les poids du réseau de politique est nul
 * (la boucle remet à zéro après chaque `step()`, prête pour un épisode suivant hypothétique).
 * \castest{<b>ReinforceTrainer : remise à zéro des gradients entre épisodes.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Run complet.<br/>2. Lire le gradient de chaque paramètre du réseau de politique.<br/>
 * \tattendu Gradient nul sur tous les paramètres.}
 */
TEST(ReinforceTrainerTest, RemiseAZeroDesGradientsEntreEpisodes) {
    const TrivialLevelDirectory level("zerograd");
    const ObservationEncoder encoder;
    Rng networkRng(3);
    auto policy = buildNetwork(policyTopology(encoder.inputSize()), networkRng);
    Sgd optimizer(0.05f);
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    TrainingStatsRecorder recorder(level.file("stats.csv"));

    ReinforceConfig config;
    config.seedBase = 3;
    ReinforceTrainer trainer(*policy, optimizer, environment, level.levelPath(), config, recorder,
                             "TrivialAI");
    trainer.run(10);

    for (const auto& parameter : policy->parameters()) {
        for (std::size_t i = 0; i < parameter->grad.size(); ++i) {
            EXPECT_FLOAT_EQ(parameter->grad.data()[i], 0.0f);
        }
    }
}

/**
 * @brief Un épisode de longueur minimale (échec dès le premier pas, cas limite du corridor trivial
 * borné) ne fait pas planter la boucle et journalise correctement une trajectoire courte.
 * \castest{<b>ReinforceTrainer : robustesse à un épisode très court.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Budget de pas réduit à `1` (force une fin d'épisode dès le premier pas si non
 * résolu).<br/>2. `run(5)`.<br/>
 * \tattendu Aucune exception ; CSV de `5 + 1` lignes.}
 */
TEST(ReinforceTrainerTest, RobustesseAUnEpisodeTresCourt) {
    const TrivialLevelDirectory level("court");
    const ObservationEncoder encoder;
    Rng networkRng(11);
    auto policy = buildNetwork(policyTopology(encoder.inputSize()), networkRng);
    Sgd optimizer(0.05f);
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = 1});
    const std::filesystem::path csvPath = level.file("stats.csv");
    TrainingStatsRecorder recorder(csvPath);

    ReinforceConfig config;
    config.seedBase = 11;
    ReinforceTrainer trainer(*policy, optimizer, environment, level.levelPath(), config, recorder,
                             "TrivialAI");
    EXPECT_NO_THROW(trainer.run(5));

    const std::vector<std::string> lines = splitLines(readWholeFile(csvPath));
    EXPECT_EQ(lines.size(), 6u);
}
