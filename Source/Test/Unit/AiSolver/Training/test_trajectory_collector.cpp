// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trajectory_collector.cpp
 * @brief Tests unitaires de `TrajectoryCollector::collectEpisode` (LOT-ANNEXE-12, TACHE-01).
 */

#include <cmath>
#include <set>

#include <gtest/gtest.h>

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Nn/Activations.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/PolicyGradient/TrajectoryCollector.h"
#include "TrivialLevelFixture.h"

using aisolver::actionCount;
using aisolver::EnvironmentConfig;
using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::Rng;
using aisolver::training::Trajectory;
using aisolver::training::TrajectoryCollector;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {

// Budget de pas volontairement tres reduit : le niveau trivial (corridor de deux cases) n'a besoin
// que de quelques pas pour etre resolu ou pour atteindre le plafond de securite.
constexpr int REDUCED_MAX_STEPS = 30;

// Reseau "presque constant" (mais pas totalement degenere) : biais tres favorable a une action,
// poids nuls -- meme patron que constantActionIndividual (test_fitness_evaluator.cpp), pour un
// episode qui atteint fiablement le plafond de pas dur sans jamais progresser ni se resoudre.
std::unique_ptr<aisolver::nn::Network> constantActionNetwork(std::size_t inputSize,
                                                             std::size_t actionIndex) {
    Rng initRng(1);
    auto network =
        aisolver::training::evolutionary::buildNetwork(policyTopology(inputSize), initRng);
    auto params = network->parameters();
    // policyTopology : deux couches, poids/biais de la couche de sortie sont les deux derniers
    // elements de parameters() (Dense::parameters() = {poids, biais}, Network::parameters()
    // concatene dans l'ordre d'ajout des couches).
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

}  // namespace

/**
 * @brief Un épisode qui n'atteint jamais l'issue (`TimedOut`) produit une trajectoire de longueur
 * exactement égale au budget de pas dur configuré.
 * \castest{<b>TrajectoryCollector : longueur cohérente avec un timeout.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Politique à action constante « ne rien faire ».<br/>2. Collecter un épisode sur le
 * niveau trivial, budget de pas réduit.<br/>
 * \tattendu `trajectory.status == TimedOut`, `trajectory.steps.size() == maxSteps`.}
 */
TEST(TrajectoryCollectorTest, LongueurCoherenteAvecTimeout) {
    const TrivialLevelDirectory level("timeout");
    const ObservationEncoder encoder;
    const std::size_t doNothingIndex =
        aisolver::indexOf(aisolver::Action{aisolver::Direction::None, false, false, false});
    auto network = constantActionNetwork(encoder.inputSize(), doNothingIndex);

    HeadlessLevelEnvironment env(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    ASSERT_TRUE(env.reset(level.levelPath()));

    TrajectoryCollector collector;
    Rng rng(1);
    const Trajectory trajectory = collector.collectEpisode(env, *network, rng);

    EXPECT_EQ(trajectory.status, aisolver::EpisodeStatus::TimedOut);
    EXPECT_EQ(trajectory.steps.size(), static_cast<std::size_t>(REDUCED_MAX_STEPS));
}

/**
 * @brief Un épisode qui se résout avant le plafond produit une trajectoire strictement plus courte
 * que le budget de pas dur.
 * \castest{<b>TrajectoryCollector : longueur cohérente avec une victoire rapide.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Politique à action constante « avancer à droite ».<br/>2. Collecter un épisode sur le
 * niveau trivial (résoluble en 1 pas).<br/>
 * \tattendu `trajectory.status == Won`, `trajectory.steps.size() < maxSteps`.}
 */
TEST(TrajectoryCollectorTest, LongueurCoherenteAvecVictoireRapide) {
    const TrivialLevelDirectory level("victoire");
    const ObservationEncoder encoder;
    const std::size_t moveRightIndex =
        aisolver::indexOf(aisolver::Action{aisolver::Direction::Right, false, false, false});
    auto network = constantActionNetwork(encoder.inputSize(), moveRightIndex);

    HeadlessLevelEnvironment env(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    ASSERT_TRUE(env.reset(level.levelPath()));

    TrajectoryCollector collector;
    Rng rng(1);
    const Trajectory trajectory = collector.collectEpisode(env, *network, rng);

    EXPECT_EQ(trajectory.status, aisolver::EpisodeStatus::Won);
    EXPECT_LT(trajectory.steps.size(), static_cast<std::size_t>(REDUCED_MAX_STEPS));
}

/**
 * @brief Chaque log-probabilité stockée est `<= 0` et correspond bien à la probabilité de l'action
 * enregistrée dans le même pas (recalcul indépendant à la tolérance flottante usuelle).
 * \castest{<b>TrajectoryCollector : log-probabilités valides.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Politique à action constante.<br/>2. Collecter un épisode.<br/>
 * \tattendu Toutes les `logProbability <= 0` ; `exp(logProbability)` proche de la probabilité
 * attendue pour l'action constante (`~1`).}
 */
TEST(TrajectoryCollectorTest, LogProbabilitesValides) {
    const TrivialLevelDirectory level("logprob");
    const ObservationEncoder encoder;
    const std::size_t moveRightIndex =
        aisolver::indexOf(aisolver::Action{aisolver::Direction::Right, false, false, false});
    auto network = constantActionNetwork(encoder.inputSize(), moveRightIndex);

    HeadlessLevelEnvironment env(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    ASSERT_TRUE(env.reset(level.levelPath()));

    TrajectoryCollector collector;
    Rng rng(1);
    const Trajectory trajectory = collector.collectEpisode(env, *network, rng);

    ASSERT_FALSE(trajectory.steps.empty());
    for (const auto& step : trajectory.steps) {
        EXPECT_LE(step.logProbability, 0.0f);
    }
    // Biais tres favorable (10 contre -10) : la probabilite de l'action jouee au premier pas doit
    // etre tres proche de 1 (log-probabilite proche de 0).
    EXPECT_NEAR(std::exp(trajectory.steps.front().logProbability), 1.0f, 1e-3f);
}

/**
 * @brief À graine identique et même politique, deux collectes indépendantes produisent des
 * trajectoires strictement identiques pas à pas.
 * \castest{<b>TrajectoryCollector : déterminisme à graine fixée.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Même politique, deux `Rng` initialisés à la même graine.<br/>2. Collecter deux
 * épisodes sur deux environnements distincts.<br/>
 * \tattendu Actions, log-probabilités et récompenses identiques à chaque pas.}
 */
TEST(TrajectoryCollectorTest, DeterminismeAGraineFixee) {
    const TrivialLevelDirectory level("determinisme");
    const ObservationEncoder encoder;

    // Reseau proche de l'uniforme (poids initiaux) : rend l'echantillonnage stochastique
    // effectivement sensible a la graine du Rng, condition necessaire pour que ce test soit
    // significatif (un reseau a action constante rendrait la trajectoire triviale meme sans
    // determinisme correct du Rng).
    Rng topologyRng(99);
    auto networkA = aisolver::training::evolutionary::buildNetwork(
        policyTopology(encoder.inputSize()), topologyRng);
    Rng topologyRngCopy(99);
    auto networkB = aisolver::training::evolutionary::buildNetwork(
        policyTopology(encoder.inputSize()), topologyRngCopy);

    HeadlessLevelEnvironment envA(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    HeadlessLevelEnvironment envB(EnvironmentConfig{.maxSteps = REDUCED_MAX_STEPS});
    ASSERT_TRUE(envA.reset(level.levelPath()));
    ASSERT_TRUE(envB.reset(level.levelPath()));

    TrajectoryCollector collector;
    Rng rngA(2024);
    Rng rngB(2024);
    const Trajectory trajectoryA = collector.collectEpisode(envA, *networkA, rngA);
    const Trajectory trajectoryB = collector.collectEpisode(envB, *networkB, rngB);

    ASSERT_EQ(trajectoryA.steps.size(), trajectoryB.steps.size());
    EXPECT_EQ(trajectoryA.status, trajectoryB.status);
    for (std::size_t i = 0; i < trajectoryA.steps.size(); ++i) {
        EXPECT_EQ(trajectoryA.steps[i].actionIndex, trajectoryB.steps[i].actionIndex);
        EXPECT_FLOAT_EQ(trajectoryA.steps[i].logProbability, trajectoryB.steps[i].logProbability);
        EXPECT_FLOAT_EQ(trajectoryA.steps[i].reward, trajectoryB.steps[i].reward);
    }
}

/**
 * @brief Sur plusieurs épisodes d'une politique proche de l'uniforme (poids initiaux), plusieurs
 * actions distinctes de l'espace d'action sont effectivement échantillonnées (l'échantillonnage
 * n'est jamais dégénéré, contrairement à un décodage par argmax).
 * \castest{<b>TrajectoryCollector : couverture non dégénérée de l'espace d'action.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Politique proche de l'uniforme.<br/>2. Collecter une trentaine d'épisodes courts,
 * cumuler les indices d'action rencontrés.<br/>
 * \tattendu Au moins 8 indices d'action distincts sur les 48 de l'espace.}
 */
TEST(TrajectoryCollectorTest, CouvertureNonDegenereeDeLEspaceDAction) {
    const TrivialLevelDirectory level("couverture");
    const ObservationEncoder encoder;
    Rng topologyRng(7);
    auto network = aisolver::training::evolutionary::buildNetwork(
        policyTopology(encoder.inputSize()), topologyRng);

    // Budget de pas tres reduit : seul le nombre d'episodes compte pour la diversite observee, pas
    // la longueur individuelle de chacun (le corridor trivial ne demande qu'un pas pour resoudre un
    // deplacement a droite, mais la politique quasi-uniforme errera sur les autres actions avant).
    HeadlessLevelEnvironment env(EnvironmentConfig{.maxSteps = 40});

    TrajectoryCollector collector;
    std::set<std::size_t> observedActions;
    Rng rng(555);
    for (int episode = 0; episode < 40; ++episode) {
        ASSERT_TRUE(env.reset(level.levelPath()));
        const Trajectory trajectory = collector.collectEpisode(env, *network, rng);
        for (const auto& step : trajectory.steps) {
            observedActions.insert(step.actionIndex);
        }
    }

    EXPECT_GE(observedActions.size(), 8u);
    EXPECT_LE(observedActions.size(), actionCount());
}
