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
#include "AiSolver/Optim/Adam.h"
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
using aisolver::optim::Adam;
using aisolver::optim::Sgd;
using aisolver::training::ReinforceConfig;
using aisolver::training::ReinforceTrainer;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

// Budget de pas reduit : le corridor trivial se resout en un pas une fois l'action decouverte, le
// reste du budget ne sert qu'a borner le cout pire cas d'une politique encore peu entrainee.
constexpr int REDUCED_MAX_STEPS = 40;
constexpr std::size_t EPISODE_COUNT = 80;

/// Budget de pas du corridor long : sept cases a franchir, vingt images par case a `moveSpeed = 3`
/// -- moins de deux cents images ne laisseraient meme pas au trace parfait le temps d'arriver.
constexpr int LONG_CORRIDOR_MAX_STEPS = 400;
/// Episodes du test de progression : assez pour qu'une tendance emerge du bruit d'echantillonnage.
constexpr std::size_t PROGRESSION_EPISODE_COUNT = 300;

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
                  const std::filesystem::path& csvPath, std::size_t episodeCount = EPISODE_COUNT,
                  int maxSteps = REDUCED_MAX_STEPS) {
    const ObservationEncoder encoder;
    Rng networkRng(seedBase);
    auto policy = buildNetwork(policyTopology(encoder.inputSize()), networkRng);
    // Meme optimiseur et meme taux que la configuration livree (`cli::TrainingConfig`) : ce test
    // mesure la progression de l'entrainement tel qu'il est reellement execute, pas d'un reglage
    // fige a l'ecriture du test.
    Adam optimizer(0.003f);
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = maxSteps});
    TrainingStatsRecorder recorder(csvPath);

    ReinforceConfig config;
    config.gamma = 0.95f;
    config.seedBase = seedBase;

    ReinforceTrainer trainer(*policy, optimizer, environment, level.levelPath(), config, recorder,
                             "TrivialAI");
    trainer.run(episodeCount);
}

}  // namespace

/**
 * @brief Sur le niveau de contrôle trivial, la récompense moyenne glissante des derniers épisodes
 * d'un run est significativement supérieure à celle des premiers — signe d'un apprentissage
 * effectivement dirigé par le gradient, pas seulement d'une absence de crash.
 * \castest{<b>ReinforceTrainer : progression de la récompense sur le niveau de contrôle.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `ReinforceTrainer` sur le corridor long, `300` épisodes.<br/>2. Comparer la
 * récompense moyenne du premier et du dernier cinquième du run (colonne `bestReward` du CSV, un
 * seul épisode par ligne).<br/>
 * \tattendu Moyenne du dernier cinquième strictement supérieure à celle du premier.}
 */
TEST(ReinforceTrainerTest, ProgressionDeLaRecompenseSurLeNiveauDeControle) {
    // Corridor long, pas le corridor a deux cases : ce dernier est deja termine quatre fois sur
    // cinq par une politique tiree au hasard, et une progression ne s'y mesure pas -- il n'y a
    // presque rien a gagner, et la variance de dix episodes depasse le gain possible.
    const TrivialLevelDirectory level("progression", aisolver_test::LONG_CORRIDOR_LEVEL_JSON);
    const std::filesystem::path csvPath = level.file("stats.csv");
    runReinforce(level, 12345, csvPath, PROGRESSION_EPISODE_COUNT, LONG_CORRIDOR_MAX_STEPS);

    const std::vector<std::string> lines = splitLines(readWholeFile(csvPath));
    ASSERT_EQ(lines.size(), PROGRESSION_EPISODE_COUNT + 1);  // + 1 ligne d'en-tete.

    // bestReward est la 2e colonne (index, bestReward, meanReward, ...) -- meme ordre que
    // TrainingStatsRow (Stats/TrainingStatsRecorder.h).
    const auto rewardOf = [](const std::string& line) {
        const std::size_t firstComma = line.find(',');
        const std::size_t secondComma = line.find(',', firstComma + 1);
        return std::stof(line.substr(firstComma + 1, secondComma - firstComma - 1));
    };

    // Un cinquieme du run de chaque cote, pas dix episodes : l'echantillonnage est stochastique,
    // et dix episodes varient plus que ce qu'un entrainement court peut gagner.
    const std::size_t window = PROGRESSION_EPISODE_COUNT / 5;

    float firstWindow = 0.0f;
    for (std::size_t i = 1; i <= window; ++i) {
        firstWindow += rewardOf(lines[i]);
    }
    firstWindow /= static_cast<float>(window);

    float lastWindow = 0.0f;
    for (std::size_t i = lines.size() - window; i < lines.size(); ++i) {
        lastWindow += rewardOf(lines[i]);
    }
    lastWindow /= static_cast<float>(window);

    EXPECT_GT(lastWindow, firstWindow);
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
    EXPECT_EQ(lines.size(), EPISODE_COUNT + 1);
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
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
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

/**
 * @brief `shouldStop` renvoyant `true` dès le premier appel interrompt le run avant le premier
 * épisode (`LOT-ANNEXE-21`).
 * \castest{<b>ReinforceTrainer : `shouldStop` interrompt avant le premier épisode.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `run(10, shouldStop)` avec un `shouldStop` qui renvoie `true` dès le premier
 * appel.<br/>
 * \tattendu `episodeIndex() == 0` (aucun épisode exécuté), aucune ligne journalisée au-delà de
 * l'en-tête.}
 */
TEST(ReinforceTrainerTest, ShouldStopInterromptAvantLePremierEpisode) {
    const TrivialLevelDirectory level("shouldstop");
    const ObservationEncoder encoder;
    Rng networkRng(13);
    auto policy = buildNetwork(policyTopology(encoder.inputSize()), networkRng);
    Sgd optimizer(0.05f);
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    const std::filesystem::path csvPath = level.file("stats.csv");
    TrainingStatsRecorder recorder(csvPath);

    ReinforceConfig config;
    config.seedBase = 13;
    ReinforceTrainer trainer(*policy, optimizer, environment, level.levelPath(), config, recorder,
                             "TrivialAI");
    trainer.run(10, [] { return true; });

    EXPECT_EQ(trainer.episodeIndex(), 0);
    const std::vector<std::string> lines = splitLines(readWholeFile(csvPath));
    EXPECT_EQ(lines.size(), 1u);  // en-tete seul.
}
