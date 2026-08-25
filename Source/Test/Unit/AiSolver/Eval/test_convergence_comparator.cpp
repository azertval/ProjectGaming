// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_convergence_comparator.cpp
 * @brief Tests unitaires de `ConvergenceComparator` (LOT-ANNEXE-13, TACHE-04) sur des CSV
 * synthétiques, puis vérification qu'il lit sans erreur des CSV réellement produits par
 * `ReinforceTrainer` (LOT-ANNEXE-12) et `ActorCriticTrainer` (LOT-ANNEXE-13, TACHE-03).
 */

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "../Training/TrivialLevelFixture.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Eval/ConvergenceComparator.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Optim/Sgd.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/ActorCritic/ActorCriticTrainer.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/PolicyGradient/ReinforceTrainer.h"

using aisolver::EnvironmentConfig;
using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::Rng;
using aisolver::TrainingStatsRecorder;
using aisolver::eval::analyzeRun;
using aisolver::eval::compareConvergence;
using aisolver::optim::Sgd;
using aisolver::training::ActorCriticConfig;
using aisolver::training::ActorCriticTrainer;
using aisolver::training::CriticNetwork;
using aisolver::training::ReinforceConfig;
using aisolver::training::ReinforceTrainer;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

/// Écrit un CSV synthétique minimal (colonnes `index,bestReward`), une valeur par ligne de
/// `rewards`, pour tester `analyzeRun`/`compareConvergence` sans avoir à exécuter un entraînement.
void writeSyntheticCsv(const std::filesystem::path& path, const std::vector<float>& rewards) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::trunc);
    file << "index,bestReward\n";
    for (std::size_t i = 0; i < rewards.size(); ++i) {
        file << i << ',' << rewards[i] << '\n';
    }
}

std::filesystem::path syntheticDirectory(const char* suffix) {
    const std::filesystem::path path = std::filesystem::temp_directory_path() /
                                       (std::string("aisolver_test_convergence_") + suffix);
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
    std::filesystem::create_directories(path);
    return path;
}

}  // namespace

/**
 * @brief Sur un CSV synthétique de récompense croissante connue, le nombre d'épisodes calculé
 * correspond exactement à la première atteinte du seuil.
 * \castest{<b>analyzeRun : nombre d'épisodes jusqu'au plafond.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. CSV synthétique `[0, 1, 2, ..., 9]`, seuil `5`.<br/>2. `analyzeRun`.<br/>
 * \tattendu `episodesToThreshold == 5` (l'épisode d'index `5` est le premier `>= 5`).}
 */
TEST(ConvergenceComparatorTest, NombreDEpisodesJusquAuPlafond) {
    const std::filesystem::path directory = syntheticDirectory("threshold");
    const std::filesystem::path csvPath = directory / "run.csv";
    writeSyntheticCsv(csvPath, {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f});

    const auto metrics = analyzeRun(csvPath, 5.0f);
    ASSERT_TRUE(metrics.episodesToThreshold.has_value());
    EXPECT_EQ(*metrics.episodesToThreshold, 5);
}

/**
 * @brief Si le plafond n'est jamais atteint, `analyzeRun` ne plante pas et renvoie un indicateur
 * explicite d'absence (pas un nombre d'épisodes erroné).
 * \castest{<b>analyzeRun : plafond jamais atteint.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. CSV synthétique dont aucune valeur n'atteint le seuil.<br/>2. `analyzeRun`.<br/>
 * \tattendu `episodesToThreshold` absent (`std::nullopt`).}
 */
TEST(ConvergenceComparatorTest, PlafondJamaisAtteint) {
    const std::filesystem::path directory = syntheticDirectory("never");
    const std::filesystem::path csvPath = directory / "run.csv";
    writeSyntheticCsv(csvPath, {0.0f, 0.5f, 1.0f, 1.5f});

    const auto metrics = analyzeRun(csvPath, 100.0f);
    EXPECT_FALSE(metrics.episodesToThreshold.has_value());
}

/**
 * @brief L'écart-type calculé à travers plusieurs essais synthétiques correspond à celui d'une
 * distribution à variance connue construite à la main.
 * \castest{<b>compareConvergence : écart-type de fin de run.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Trois CSV synthétiques à récompense finale constante `8`, `10`, `12`.<br/>2.
 * `compareConvergence`, fenêtre couvrant tout le run.<br/>
 * \tattendu Écart-type calculé égal à celui de `{8, 10, 12}` (`~1.633`).}
 */
TEST(ConvergenceComparatorTest, EcartTypeDeFinDeRun) {
    const std::filesystem::path directory = syntheticDirectory("stddev");
    const std::vector<std::filesystem::path> paths{
        directory / "trial_a.csv", directory / "trial_b.csv", directory / "trial_c.csv"};
    writeSyntheticCsv(paths[0], {8.0f, 8.0f, 8.0f});
    writeSyntheticCsv(paths[1], {10.0f, 10.0f, 10.0f});
    writeSyntheticCsv(paths[2], {12.0f, 12.0f, 12.0f});

    const auto report =
        compareConvergence(paths, /*rewardThreshold=*/1000.0f, /*finalWindowSize=*/3);

    const float mean = (8.0f + 10.0f + 12.0f) / 3.0f;
    const float expectedVariance =
        ((8.0f - mean) * (8.0f - mean) + (10.0f - mean) * (10.0f - mean) +
         (12.0f - mean) * (12.0f - mean)) /
        3.0f;
    EXPECT_NEAR(report.finalRewardStdDev, std::sqrt(expectedVariance), 1e-4f);
    EXPECT_EQ(report.trialsReachingThreshold, 0u);
    EXPECT_FALSE(report.meanEpisodesToThreshold.has_value());
}

/**
 * @brief `ConvergenceComparator` lit sans erreur des CSV réellement produits par `ReinforceTrainer`
 * et par `ActorCriticTrainer`, malgré leur provenance différente.
 * \castest{<b>compareConvergence : lecture croisée REINFORCE / acteur-critique.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Run court de chaque algorithme sur le niveau trivial.<br/>2. `compareConvergence` sur
 * chaque CSV produit.<br/>
 * \tattendu Aucune exception ; `totalTrials == 1` des deux côtés.}
 */
TEST(ConvergenceComparatorTest, LectureCroiseeReinforceEtActeurCritique) {
    const TrivialLevelDirectory level("cross_read");
    const ObservationEncoder encoder;
    constexpr int kReducedMaxSteps = 40;
    constexpr std::size_t kEpisodeCount = 15;

    Rng reinforceRng(61);
    auto reinforcePolicy = buildNetwork(policyTopology(encoder.inputSize()), reinforceRng);
    Sgd reinforceOptimizer(0.05f);
    HeadlessLevelEnvironment reinforceEnvironment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    TrainingStatsRecorder reinforceRecorder(level.file("reinforce.csv"));
    ReinforceConfig reinforceConfig;
    reinforceConfig.seedBase = 61;
    ReinforceTrainer reinforceTrainer(*reinforcePolicy, reinforceOptimizer, reinforceEnvironment,
                                      level.levelPath(), reinforceConfig, reinforceRecorder,
                                      "TrivialAI");
    reinforceTrainer.run(kEpisodeCount);

    Rng policyRng(62);
    auto acPolicy = buildNetwork(policyTopology(encoder.inputSize()), policyRng);
    Sgd policyOptimizer(0.05f);
    Rng criticRng(63);
    CriticNetwork critic(encoder.inputSize(), 8, criticRng);
    Sgd criticOptimizer(0.05f);
    HeadlessLevelEnvironment acEnvironment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    TrainingStatsRecorder acRecorder(level.file("actor_critic.csv"));
    ActorCriticConfig acConfig;
    acConfig.seedBase = 62;
    ActorCriticTrainer acTrainer(*acPolicy, policyOptimizer, critic, criticOptimizer, acEnvironment,
                                 level.levelPath(), acConfig, acRecorder, "TrivialAI");
    acTrainer.run(kEpisodeCount);

    const auto reinforceReport =
        compareConvergence({level.file("reinforce.csv")}, /*rewardThreshold=*/1.0f);
    const auto actorCriticReport =
        compareConvergence({level.file("actor_critic.csv")}, /*rewardThreshold=*/1.0f);

    EXPECT_EQ(reinforceReport.totalTrials, 1u);
    EXPECT_EQ(actorCriticReport.totalTrials, 1u);
}

/**
 * @brief Protocole de comparaison de TACHE-04 : plusieurs essais (graines différentes) de
 * `ReinforceTrainer` et `ActorCriticTrainer`, même niveau de contrôle, même budget d'épisodes.
 * Produit un résumé lisible sur la sortie standard (consigné dans `epic.md` une fois exécuté) ;
 * n'impose aucune conclusion figée (mesure honnête, cf. décision de cadrage de l'épic) : vérifie
 * seulement que les deux rapports sont numériquement valides.
 * \castest{<b>Comparaison chiffrée acteur-critique vs REINFORCE brut.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. 4 essais de chaque algorithme, 60 épisodes, niveau trivial.<br/>2.
 * `compareConvergence` sur chaque série.<br/>
 * \tattendu Écart-type fini et positif ou nul des deux côtés.}
 */
TEST(ConvergenceComparatorTest, ComparaisonReinforceVsActorCritiqueSurNiveauDeControle) {
    const TrivialLevelDirectory level("comparison");
    const ObservationEncoder encoder;
    constexpr int kReducedMaxSteps = 40;
    constexpr std::size_t kEpisodeCount = 60;
    constexpr int kTrialCount = 4;
    constexpr float kRewardThreshold = 5.0f;

    std::vector<std::filesystem::path> reinforcePaths;
    std::vector<std::filesystem::path> actorCriticPaths;

    for (int trial = 0; trial < kTrialCount; ++trial) {
        const std::uint64_t seed = 1000 + static_cast<std::uint64_t>(trial);

        Rng reinforceRng(seed);
        auto reinforcePolicy = buildNetwork(policyTopology(encoder.inputSize()), reinforceRng);
        Sgd reinforceOptimizer(0.05f);
        HeadlessLevelEnvironment reinforceEnvironment(
            EnvironmentConfig{.maxSteps = kReducedMaxSteps});
        const std::filesystem::path reinforcePath =
            level.file(("reinforce_" + std::to_string(trial) + ".csv").c_str());
        TrainingStatsRecorder reinforceRecorder(reinforcePath);
        ReinforceConfig reinforceConfig;
        reinforceConfig.seedBase = seed;
        ReinforceTrainer reinforceTrainer(*reinforcePolicy, reinforceOptimizer,
                                          reinforceEnvironment, level.levelPath(), reinforceConfig,
                                          reinforceRecorder, "TrivialAI");
        reinforceTrainer.run(kEpisodeCount);
        reinforcePaths.push_back(reinforcePath);

        Rng policyRng(seed + 500);
        auto acPolicy = buildNetwork(policyTopology(encoder.inputSize()), policyRng);
        Sgd policyOptimizer(0.05f);
        Rng criticRng(seed + 900);
        CriticNetwork critic(encoder.inputSize(), 8, criticRng);
        Sgd criticOptimizer(0.05f);
        HeadlessLevelEnvironment acEnvironment(EnvironmentConfig{.maxSteps = kReducedMaxSteps});
        const std::filesystem::path acPath =
            level.file(("actor_critic_" + std::to_string(trial) + ".csv").c_str());
        TrainingStatsRecorder acRecorder(acPath);
        ActorCriticConfig acConfig;
        acConfig.seedBase = seed;
        ActorCriticTrainer acTrainer(*acPolicy, policyOptimizer, critic, criticOptimizer,
                                     acEnvironment, level.levelPath(), acConfig, acRecorder,
                                     "TrivialAI");
        acTrainer.run(kEpisodeCount);
        actorCriticPaths.push_back(acPath);
    }

    const auto reinforceReport = compareConvergence(reinforcePaths, kRewardThreshold);
    const auto actorCriticReport = compareConvergence(actorCriticPaths, kRewardThreshold);

    std::printf(
        "[LOT-ANNEXE-13 TACHE-04] REINFORCE : %zu/%zu essais atteignent le plafond, "
        "episodesToThreshold moyen = %s, ecart-type fin de run = %f\n",
        reinforceReport.trialsReachingThreshold, reinforceReport.totalTrials,
        reinforceReport.meanEpisodesToThreshold
            ? std::to_string(*reinforceReport.meanEpisodesToThreshold).c_str()
            : "N/A",
        static_cast<double>(reinforceReport.finalRewardStdDev));
    std::printf(
        "[LOT-ANNEXE-13 TACHE-04] ActorCritic : %zu/%zu essais atteignent le plafond, "
        "episodesToThreshold moyen = %s, ecart-type fin de run = %f\n",
        actorCriticReport.trialsReachingThreshold, actorCriticReport.totalTrials,
        actorCriticReport.meanEpisodesToThreshold
            ? std::to_string(*actorCriticReport.meanEpisodesToThreshold).c_str()
            : "N/A",
        static_cast<double>(actorCriticReport.finalRewardStdDev));

    EXPECT_TRUE(std::isfinite(reinforceReport.finalRewardStdDev));
    EXPECT_GE(reinforceReport.finalRewardStdDev, 0.0f);
    EXPECT_TRUE(std::isfinite(actorCriticReport.finalRewardStdDev));
    EXPECT_GE(actorCriticReport.finalRewardStdDev, 0.0f);
}

namespace {

/// Écrit un CSV **tel quel**, sans passer par le formateur : c'est le seul moyen de fabriquer les
/// fichiers malformés qu'un entraînement ne produirait jamais, mais qu'un disque plein, une copie
/// interrompue ou une édition à la main produisent.
void writeRawCsv(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::trunc);
    file << content;
}

}  // namespace

/**
 * @brief Un CSV absent ou illisible rend des métriques neutres, sans exception ni valeur inventée.
 * \castest{<b>analyzeRun : fichier absent -> métriques neutres.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `analyzeRun` sur un chemin qui n'existe pas.<br/>
 * \tattendu Aucune exception ; `episodesToThreshold` absent et `finalWindowMeanReward` nul.}
 */
TEST(ConvergenceComparatorTest, FichierAbsentRendDesMetriquesNeutres) {
    const std::filesystem::path directory = syntheticDirectory("absent");
    const aisolver::eval::RunConvergenceMetrics metrics =
        analyzeRun(directory / "inexistant.csv", 5.0f, 10);

    EXPECT_FALSE(metrics.episodesToThreshold.has_value());
    EXPECT_FLOAT_EQ(metrics.finalWindowMeanReward, 0.0f);
}

/**
 * @brief Un en-tête qui ne déclare pas `bestReward` rend des métriques neutres : la série est
 * refusée en bloc plutôt que lue sur la mauvaise colonne.
 *
 * C'est la distinction qui compte : lire silencieusement une autre colonne produirait des chiffres
 * plausibles et faux, bien pires qu'une absence de résultat.
 * \castest{<b>analyzeRun : en-tête sans colonne bestReward -> métriques neutres.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. CSV dont l'en-tête est `index,meanReward` (pas de `bestReward`), avec des lignes de
 * données valides.<br/>2. `analyzeRun`.<br/>
 * \tattendu `episodesToThreshold` absent et `finalWindowMeanReward` nul : aucune colonne voisine
 * n'est lue à la place.}
 */
TEST(ConvergenceComparatorTest, EnTeteSansColonneBestRewardRendDesMetriquesNeutres) {
    const std::filesystem::path csv = syntheticDirectory("entete") / "run.csv";
    writeRawCsv(csv, "index,meanReward\n0,10\n1,20\n2,30\n");

    const aisolver::eval::RunConvergenceMetrics metrics = analyzeRun(csv, 5.0f, 10);

    EXPECT_FALSE(metrics.episodesToThreshold.has_value());
    EXPECT_FLOAT_EQ(metrics.finalWindowMeanReward, 0.0f);
}

/**
 * @brief Une ligne tronquée est ignorée, et **seulement elle** : les lignes valides du même
 * fichier restent lues.
 * \castest{<b>analyzeRun : ligne tronquée ignorée, le reste du fichier reste lu.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. CSV `index,bestReward` dont la deuxième ligne de données n'a qu'un champ.<br/>
 * 2. `analyzeRun` avec un seuil de 15.<br/>
 * \tattendu Les deux lignes valides (10 et 20) sont retenues : moyenne 15, seuil atteint au
 * deuxième élément lu.}
 */
TEST(ConvergenceComparatorTest, LigneTronqueeIgnoreeSansPerdreLeResteDuFichier) {
    const std::filesystem::path csv = syntheticDirectory("tronquee") / "run.csv";
    writeRawCsv(csv, "index,bestReward\n0,10\n1\n2,20\n");

    const aisolver::eval::RunConvergenceMetrics metrics = analyzeRun(csv, 15.0f, 10);

    EXPECT_FLOAT_EQ(metrics.finalWindowMeanReward, 15.0f);
    ASSERT_TRUE(metrics.episodesToThreshold.has_value());
    EXPECT_EQ(*metrics.episodesToThreshold, 1);
}

/**
 * @brief Un champ non numérique est ignoré, et **seulement lui** : les lignes valides du même
 * fichier restent lues.
 * \castest{<b>analyzeRun : champ non numérique ignoré, le reste du fichier reste lu.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. CSV `index,bestReward` dont la deuxième ligne porte `abc` en `bestReward`.<br/>
 * 2. `analyzeRun` avec un seuil de 15.<br/>
 * \tattendu Les deux lignes valides (10 et 20) sont retenues : moyenne 15, seuil atteint au
 * deuxième élément lu.}
 */
TEST(ConvergenceComparatorTest, ChampNonNumeriqueIgnoreSansPerdreLeResteDuFichier) {
    const std::filesystem::path csv = syntheticDirectory("non_numerique") / "run.csv";
    writeRawCsv(csv, "index,bestReward\n0,10\nabc,abc\n2,20\n");

    const aisolver::eval::RunConvergenceMetrics metrics = analyzeRun(csv, 15.0f, 10);

    EXPECT_FLOAT_EQ(metrics.finalWindowMeanReward, 15.0f);
    ASSERT_TRUE(metrics.episodesToThreshold.has_value());
    EXPECT_EQ(*metrics.episodesToThreshold, 1);
}
