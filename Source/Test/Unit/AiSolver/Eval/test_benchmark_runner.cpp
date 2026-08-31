// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_benchmark_runner.cpp
 * @brief Tests unitaires de `BenchmarkRunner`/`deriveSeed`/`BenchmarkResult` (LOT-ANNEXE-15,
 * TACHE-01/TACHE-03, `EX-IA-016`).
 */

#include <memory>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "../Training/TrivialLevelFixture.h"
#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Eval/AdvancedAlgorithmTrainedPolicy.h"
#include "AiSolver/Eval/BenchmarkRunner.h"
#include "AiSolver/Eval/EvolutionaryTrainedPolicy.h"
#include "AiSolver/Eval/TrainedPolicy.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Training/Dqn/QNetwork.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

using aisolver::Action;
using aisolver::actionCount;
using aisolver::Direction;
using aisolver::indexOf;
using aisolver::Rng;
using aisolver::Tensor;
using aisolver::eval::ActionDecodingMode;
using aisolver::eval::AdvancedAlgorithmTrainedPolicy;
using aisolver::eval::BenchmarkConfig;
using aisolver::eval::BenchmarkResult;
using aisolver::eval::BenchmarkRunner;
using aisolver::eval::deriveSeed;
using aisolver::eval::EpisodeOutcome;
using aisolver::eval::EvolutionaryTrainedPolicy;
using aisolver::eval::TrainedPolicy;
using aisolver::training::QNetwork;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

constexpr std::size_t INPUT_SIZE = 1;  // Ignoree par les politiques factices de ce fichier.

// Taille reelle du vecteur d'observation produit par ObservationEncoder (LOT-ANNEXE-06) : tout
// reseau passe a BenchmarkRunner::run/runWithNoise doit avoir ete construit avec cette taille
// d'entree, puisque BenchmarkRunner encode toujours l'observation reelle de l'environnement --
// contrairement aux tests qui appellent TrainedPolicy::selectAction directement avec un tenseur
// factice de taille arbitraire (INPUT_SIZE ci-dessus).
std::size_t realObservationInputSize() {
    return aisolver::ObservationEncoder().inputSize();
}

// Force un reseau "presque one-hot" sur l'action donnee (meme patron que
// constantActionNetwork/test_trajectory_collector.cpp) : deterministe et fiable en test, sans
// entrainement reel.
std::unique_ptr<aisolver::nn::Network> constantActionNetwork(std::size_t inputSize,
                                                             std::size_t actionIndex) {
    Rng initRng(1);
    auto network =
        aisolver::training::evolutionary::buildNetwork(policyTopology(inputSize), initRng);
    auto params = network->parameters();
    float* outputWeights = params[2]->value.data();
    for (std::size_t i = 0; i < params[2]->value.size(); ++i) {
        outputWeights[i] = 0.0f;
    }
    float* outputBias = params[3]->value.data();
    for (std::size_t i = 0; i < params[3]->value.size(); ++i) {
        outputBias[i] = (i == actionIndex) ? 10.0f : -10.0f;
    }
    return network;
}

std::size_t rightActionIndex() {
    return indexOf(Action{Direction::Right, false, false, false});
}

// Politique factice ignorant l'observation, retourne toujours la meme entree joueur -- sert a
// isoler la mecanique de BenchmarkRunner (troncature, reproductibilite) de tout reseau reel.
class ScriptedTrainedPolicy : public TrainedPolicy {
public:
    explicit ScriptedTrainedPolicy(core::PlayerInput fixedInput) : _fixedInput(fixedInput) {}

    [[nodiscard]] std::optional<core::PlayerInput> selectAction(const Tensor<float>&,
                                                                ActionDecodingMode, Rng&) override {
        return _fixedInput;
    }

private:
    core::PlayerInput _fixedInput;
};

}  // namespace

/**
 * \castest{deriveSeed produit des graines distinctes par repetition.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Deriver 10 graines a partir d'une meme base, indices 0 a 9.<br/>
 * \tattendu Les 10 graines sont deux a deux distinctes.}
 */
TEST(DeriveSeedTest, ProduitDesGrainesDistinctes) {
    std::set<std::uint64_t> seeds;
    for (int i = 0; i < 10; ++i) {
        seeds.insert(deriveSeed(123, i));
    }
    EXPECT_EQ(seeds.size(), 10u);
}

/**
 * \castest{Integration : une politique scriptee triviale reussit a tous les coups.<br/>
 * \tcat Unitaire (integration niveau reel) · AiSolver Eval<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Politique constante "aller a droite" sur le niveau trivial reel (fichier JSON sur
 * disque).<br/>2. BenchmarkRunner::run, 5 repetitions.<br/>
 * \tattendu successRate() == 1.0.}
 */
TEST(BenchmarkRunnerTest, IntegrationPolitiqueScripteeReussitToujours) {
    const TrivialLevelDirectory level("scripted_always_wins");
    core::PlayerInput rightInput;
    rightInput.moveX = 1.0f;
    ScriptedTrainedPolicy policy(rightInput);

    BenchmarkConfig config;
    config.repetitions = 5;
    config.maxStepsPerEpisode = 100;
    config.decodingMode = ActionDecodingMode::Argmax;

    const BenchmarkResult result = BenchmarkRunner::run(policy, level.levelPath(), config);
    ASSERT_EQ(result.episodes.size(), 5u);
    EXPECT_DOUBLE_EQ(result.successRate(), 1.0);
}

/**
 * \castest{Troncature au budget de pas, sans boucle infinie.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Politique constante immobile sur le niveau trivial.<br/>2.
 * maxStepsPerEpisode = 25.<br/>
 * \tattendu L'episode se termine avec outcome == Playing (timeout) et stepCount == 25.}
 */
TEST(BenchmarkRunnerTest, TroncatureSansBoucleInfinie) {
    const TrivialLevelDirectory level("scripted_never_wins");
    ScriptedTrainedPolicy policy(
        core::PlayerInput{});  // Immobile : ne peut jamais atteindre la sortie.

    BenchmarkConfig config;
    config.repetitions = 1;
    config.maxStepsPerEpisode = 25;
    config.decodingMode = ActionDecodingMode::Argmax;

    const BenchmarkResult result = BenchmarkRunner::run(policy, level.levelPath(), config);
    ASSERT_EQ(result.episodes.size(), 1u);
    EXPECT_EQ(result.episodes.front().outcome, core::LevelOutcome::Playing);
    EXPECT_EQ(result.episodes.front().stepCount, 25);
}

/**
 * \castest{Reproductibilite stricte a graine de base fixee.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. BenchmarkRunner::run deux fois, meme config, meme politique.<br/>
 * \tattendu Issues et nombres de pas identiques, repetition par repetition.}
 */
TEST(BenchmarkRunnerTest, ReproductibiliteStricte) {
    const TrivialLevelDirectory level("reproducibility");
    auto network = constantActionNetwork(realObservationInputSize(), rightActionIndex());
    EvolutionaryTrainedPolicy policy(*network);

    BenchmarkConfig config;
    config.repetitions = 4;
    config.maxStepsPerEpisode = 100;
    config.decodingMode = ActionDecodingMode::Argmax;

    const BenchmarkResult first = BenchmarkRunner::run(policy, level.levelPath(), config);
    const BenchmarkResult second = BenchmarkRunner::run(policy, level.levelPath(), config);

    ASSERT_EQ(first.episodes.size(), second.episodes.size());
    for (std::size_t i = 0; i < first.episodes.size(); ++i) {
        EXPECT_EQ(first.episodes[i].outcome, second.episodes[i].outcome);
        EXPECT_EQ(first.episodes[i].stepCount, second.episodes[i].stepCount);
    }
}

/**
 * \castest{Un modele evolutionniste (Argmax) produit une variance nulle entre repetitions.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. EvolutionaryTrainedPolicy, 6 repetitions, mode Argmax.<br/>
 * \tattendu Toutes les repetitions ont la meme issue et le meme nombre de pas ; stepVariance() ==
 * 0.}
 */
TEST(BenchmarkRunnerTest, ModeleEvolutionnisteVarianceNulle) {
    const TrivialLevelDirectory level("evolutionary_zero_variance");
    auto network = constantActionNetwork(realObservationInputSize(), rightActionIndex());
    EvolutionaryTrainedPolicy policy(*network);

    BenchmarkConfig config;
    config.repetitions = 6;
    config.maxStepsPerEpisode = 100;
    config.rngSeedBase = 0;
    config.decodingMode = ActionDecodingMode::Argmax;

    const BenchmarkResult result = BenchmarkRunner::run(policy, level.levelPath(), config);
    ASSERT_EQ(result.episodes.size(), 6u);
    for (const EpisodeOutcome& episode : result.episodes) {
        EXPECT_EQ(episode.outcome, result.episodes.front().outcome);
        EXPECT_EQ(episode.stepCount, result.episodes.front().stepCount);
    }
    EXPECT_DOUBLE_EQ(result.stepVariance(), 0.0);
}

/**
 * \castest{Refus explicite du mode Stochastic pour l'evolutionniste.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. EvolutionaryTrainedPolicy::selectAction en mode Stochastic.<br/>
 * \tattendu std::nullopt retourne (erreur recuperable, pas de plantage).}
 */
TEST(BenchmarkRunnerTest, RefusExpliciteDuModeStochastiquePourEvolutionniste) {
    Rng initRng(1);
    auto network = buildNetwork(policyTopology(INPUT_SIZE), initRng);
    EvolutionaryTrainedPolicy policy(*network);
    Rng rng(2);
    const Tensor<float> observation({INPUT_SIZE, 1});
    EXPECT_FALSE(policy.selectAction(observation, ActionDecodingMode::Stochastic, rng).has_value());
}

/**
 * \castest{Refus explicite du mode Stochastic pour l'algorithme avance (DQN).<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. AdvancedAlgorithmTrainedPolicy::selectAction en mode Stochastic.<br/>
 * \tattendu std::nullopt retourne (Q(s,a) n'est pas une distribution de probabilite).}
 */
TEST(BenchmarkRunnerTest, RefusExpliciteDuModeStochastiquePourAlgorithmeAvance) {
    Rng rng(3);
    QNetwork qNetwork(INPUT_SIZE, QNetwork::DEFAULT_HIDDEN_SIZE, rng);
    AdvancedAlgorithmTrainedPolicy policy(qNetwork);
    Rng actionRng(4);
    const Tensor<float> observation({INPUT_SIZE, 1});
    EXPECT_FALSE(
        policy.selectAction(observation, ActionDecodingMode::Stochastic, actionRng).has_value());
}

/**
 * \castest{Distinction meanStepsAll / meanStepsOnSuccess.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Melanger episodes reussis (peu de pas) et timeouts (plafond de pas) dans un
 * BenchmarkResult.<br/>
 * \tattendu meanStepsAll() et meanStepsOnSuccess() different, valeurs verifiees independamment.}
 */
TEST(BenchmarkResultTest, DistinctionMeanStepsAllEtOnSuccess) {
    BenchmarkResult result;
    result.episodes.push_back(EpisodeOutcome{core::LevelOutcome::Won, 10});
    result.episodes.push_back(EpisodeOutcome{core::LevelOutcome::Won, 20});
    result.episodes.push_back(EpisodeOutcome{core::LevelOutcome::Playing, 100});  // Timeout.
    result.episodes.push_back(EpisodeOutcome{core::LevelOutcome::Playing, 100});  // Timeout.

    EXPECT_DOUBLE_EQ(result.meanStepsOnSuccess(), 15.0);
    EXPECT_DOUBLE_EQ(result.meanStepsAll(), 57.5);
    EXPECT_NE(result.meanStepsAll(), result.meanStepsOnSuccess());
    EXPECT_DOUBLE_EQ(result.successRate(), 0.5);
}

/**
 * \castest{Convergence du taux de reussite vers la probabilite theorique.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Simuler 4000 episodes via une piece biaisee (p=0.3), graine fixee.<br/>2. Calculer
 * successRate().<br/>
 * \tattendu successRate() proche de 0.3, tolerance 0.02.}
 */
TEST(BenchmarkResultTest, ConvergenceTauxDeReussiteVersProbabiliteTheorique) {
    // Episodes simules directement (pieces biaisees), sans passer par l'environnement reel : la
    // convergence testee porte sur l'agregation de BenchmarkResult::successRate(), pas sur la
    // simulation physique -- deterministe malgre le tirage simule (graine fixee).
    constexpr double THEORETICAL_PROBABILITY = 0.3;
    constexpr int SAMPLE_COUNT = 4000;
    Rng rng(2024);

    BenchmarkResult result;
    result.episodes.reserve(SAMPLE_COUNT);
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        const bool won = rng.nextFloat() < static_cast<float>(THEORETICAL_PROBABILITY);
        result.episodes.push_back(
            EpisodeOutcome{won ? core::LevelOutcome::Won : core::LevelOutcome::Lost, 10});
    }

    EXPECT_NEAR(result.successRate(), THEORETICAL_PROBABILITY, 0.02);
}

/**
 * \castest{Observateur de repetition : progression puis interruption.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Politique scriptee sur le niveau trivial, 10 repetitions demandees.<br/>2.
 * `onRepetition` enregistre chaque appel et renvoie `false` apres la troisieme.<br/>
 * \tattendu Exactement 3 notifications, numerotees 1 a 3 et portant le total demande, et un
 * resultat ne comptant que les 3 repetitions jouees -- sans quoi une IHM ne pourrait ni afficher
 * l'avancement d'une campagne, ni l'annuler.}
 */
TEST(BenchmarkRunnerTest, ObservateurDeRepetitionInterromptLaCampagne) {
    const TrivialLevelDirectory level("observed_campaign");
    core::PlayerInput rightInput;
    rightInput.moveX = 1.0f;
    ScriptedTrainedPolicy policy(rightInput);

    BenchmarkConfig config;
    config.repetitions = 10;
    config.maxStepsPerEpisode = 100;
    config.decodingMode = ActionDecodingMode::Argmax;

    std::vector<std::pair<int, int>> notifications;
    const BenchmarkResult result = BenchmarkRunner::run(
        policy, level.levelPath(), config, [&notifications](int completed, int total) {
            notifications.emplace_back(completed, total);
            return completed < 3;
        });

    ASSERT_EQ(notifications.size(), 3u);
    for (std::size_t index = 0; index < notifications.size(); ++index) {
        EXPECT_EQ(notifications[index].first, static_cast<int>(index) + 1);
        EXPECT_EQ(notifications[index].second, 10);
    }
    EXPECT_EQ(result.episodes.size(), 3u);
}

/**
 * \castest{Absence d'observateur : comportement inchange.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Meme campagne, sans passer d'observateur.<br/>
 * \tattendu Les 10 repetitions demandees sont jouees : le parametre ajoute est bien optionnel.}
 */
TEST(BenchmarkRunnerTest, SansObservateurLaCampagneVaJusquAuBout) {
    const TrivialLevelDirectory level("unobserved_campaign");
    core::PlayerInput rightInput;
    rightInput.moveX = 1.0f;
    ScriptedTrainedPolicy policy(rightInput);

    BenchmarkConfig config;
    config.repetitions = 10;
    config.maxStepsPerEpisode = 100;
    config.decodingMode = ActionDecodingMode::Argmax;

    const BenchmarkResult result = BenchmarkRunner::run(policy, level.levelPath(), config);
    EXPECT_EQ(result.episodes.size(), 10u);
}
