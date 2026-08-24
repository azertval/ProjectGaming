// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trained_policy.cpp
 * @brief Tests unitaires des quatre adaptateurs `TrainedPolicy` (LOT-ANNEXE-15, TACHE-01,
 * `EX-IA-016`).
 */

#include <memory>

#include <gtest/gtest.h>

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Eval/ActorCriticTrainedPolicy.h"
#include "AiSolver/Eval/AdvancedAlgorithmTrainedPolicy.h"
#include "AiSolver/Eval/EvolutionaryTrainedPolicy.h"
#include "AiSolver/Eval/ReinforceTrainedPolicy.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Training/Advanced/QNetwork.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

using aisolver::Action;
using aisolver::actionCount;
using aisolver::Direction;
using aisolver::indexOf;
using aisolver::Rng;
using aisolver::Tensor;
using aisolver::eval::ActionDecodingMode;
using aisolver::eval::ActorCriticTrainedPolicy;
using aisolver::eval::AdvancedAlgorithmTrainedPolicy;
using aisolver::eval::EvolutionaryTrainedPolicy;
using aisolver::eval::ReinforceTrainedPolicy;
using aisolver::training::QNetwork;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;

namespace {

constexpr std::size_t kInputSize = 5;

Tensor<float> zeroObservation() {
    return Tensor<float>({kInputSize, 1});
}

}  // namespace

/**
 * \castest{EvolutionaryTrainedPolicy n'accepte que Argmax.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une politique evolutionniste.<br/>2. Appeler selectAction en Argmax puis
 * en Stochastic.<br/>
 * \tattendu Argmax retourne une valeur, Stochastic retourne std::nullopt.}
 */
TEST(TrainedPolicyTest, EvolutionaryAcceptelArgmaxSeulement) {
    Rng rng(1);
    auto network = buildNetwork(policyTopology(kInputSize), rng);
    EvolutionaryTrainedPolicy policy(*network);

    EXPECT_TRUE(policy.supportsMode(ActionDecodingMode::Argmax));
    EXPECT_FALSE(policy.supportsMode(ActionDecodingMode::Stochastic));

    Rng actionRng(2);
    const auto argmaxResult = policy.selectAction(zeroObservation(), ActionDecodingMode::Argmax, actionRng);
    EXPECT_TRUE(argmaxResult.has_value());

    const auto stochasticResult =
        policy.selectAction(zeroObservation(), ActionDecodingMode::Stochastic, actionRng);
    EXPECT_FALSE(stochasticResult.has_value());
}

/**
 * \castest{ReinforceTrainedPolicy accepte Argmax et Stochastic.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Moyen<br/>
 * \tetapes 1. Construire une politique REINFORCE.<br/>2. Appeler selectAction dans les deux
 * modes.<br/>
 * \tattendu Les deux appels retournent une valeur.}
 */
TEST(TrainedPolicyTest, ReinforceAccepteLesDeuxModes) {
    Rng rng(3);
    auto network = buildNetwork(policyTopology(kInputSize), rng);
    ReinforceTrainedPolicy policy(*network);

    EXPECT_TRUE(policy.supportsMode(ActionDecodingMode::Argmax));
    EXPECT_TRUE(policy.supportsMode(ActionDecodingMode::Stochastic));

    Rng actionRng(4);
    EXPECT_TRUE(
        policy.selectAction(zeroObservation(), ActionDecodingMode::Argmax, actionRng).has_value());
    EXPECT_TRUE(
        policy.selectAction(zeroObservation(), ActionDecodingMode::Stochastic, actionRng).has_value());
}

/**
 * \castest{ActorCriticTrainedPolicy accepte Argmax et Stochastic, sans charger le critique.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Moyen<br/>
 * \tetapes 1. Construire une politique acteur-critique a partir du seul acteur.<br/>2. Appeler
 * selectAction dans les deux modes.<br/>
 * \tattendu Les deux appels retournent une valeur.}
 */
TEST(TrainedPolicyTest, ActorCriticAccepteLesDeuxModes) {
    Rng rng(5);
    auto actor = buildNetwork(policyTopology(kInputSize), rng);
    ActorCriticTrainedPolicy policy(*actor);

    Rng actionRng(6);
    EXPECT_TRUE(
        policy.selectAction(zeroObservation(), ActionDecodingMode::Argmax, actionRng).has_value());
    EXPECT_TRUE(
        policy.selectAction(zeroObservation(), ActionDecodingMode::Stochastic, actionRng).has_value());
}

/**
 * \castest{AdvancedAlgorithmTrainedPolicy (DQN) n'accepte que Argmax.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une politique DQN a partir du reseau principal.<br/>2. Appeler
 * selectAction en Argmax puis en Stochastic.<br/>
 * \tattendu Argmax retourne une valeur, Stochastic retourne std::nullopt (Q(s,a) n'est pas une
 * distribution de probabilite).}
 */
TEST(TrainedPolicyTest, AlgorithmeAvanceAcceptelArgmaxSeulement) {
    Rng rng(7);
    QNetwork qNetwork(kInputSize, QNetwork::kDefaultHiddenSize, rng);
    AdvancedAlgorithmTrainedPolicy policy(qNetwork);

    EXPECT_TRUE(policy.supportsMode(ActionDecodingMode::Argmax));
    EXPECT_FALSE(policy.supportsMode(ActionDecodingMode::Stochastic));

    Rng actionRng(8);
    EXPECT_TRUE(
        policy.selectAction(zeroObservation(), ActionDecodingMode::Argmax, actionRng).has_value());
    EXPECT_FALSE(
        policy.selectAction(zeroObservation(), ActionDecodingMode::Stochastic, actionRng).has_value());
}

/**
 * \castest{Le decodage Argmax evolutionniste ignore rng.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Appeler selectAction en Argmax avec deux Rng de graines differentes.<br/>
 * \tattendu Les deux actions retournees sont identiques.}
 */
TEST(TrainedPolicyTest, EvolutionaryArgmaxEstDeterministe) {
    Rng initRng(9);
    auto network = buildNetwork(policyTopology(kInputSize), initRng);
    EvolutionaryTrainedPolicy policy(*network);

    Rng rngA(100);
    Rng rngB(200);  // graine differente : Argmax n'utilise jamais rng, doit rester identique.
    const auto first = policy.selectAction(zeroObservation(), ActionDecodingMode::Argmax, rngA);
    const auto second = policy.selectAction(zeroObservation(), ActionDecodingMode::Argmax, rngB);
    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(first->moveX, second->moveX);
    EXPECT_EQ(first->jumpPressed, second->jumpPressed);
}
