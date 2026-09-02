// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_best_policy_snapshot.cpp
 * @brief Conservation du meilleur réseau d'un run par gradient (`BestPolicySnapshot`).
 *
 * Défaut couvert : les familles pg/ac/avance entraînent un réseau **en place** et sauvegardaient
 * donc l'état du dernier épisode ; un dernier épisode raté écrasait un modèle qui réussissait le
 * niveau quelques générations plus tôt.
 */

#include <cstddef>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/BestPolicySnapshot.h"
#include "AiSolver/Training/DeterministicReplay.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

using aisolver::EpisodeStatus;
using aisolver::Rng;
using aisolver::nn::Network;
using aisolver::training::BestPolicySnapshot;
using aisolver::training::DeterministicReplayResult;
using aisolver::training::PolicyScore;

namespace {

constexpr std::size_t INPUT_SIZE = 6;
constexpr std::size_t HIDDEN_SIZE = 4;

std::unique_ptr<Network> makeNetwork(std::uint64_t seed) {
    Rng rng(seed);
    return aisolver::training::evolutionary::buildNetwork(
        aisolver::training::evolutionary::policyTopology(INPUT_SIZE, HIDDEN_SIZE), rng);
}

/// Rejeu synthétique : seuls le statut, la récompense et la longueur sont notés.
DeterministicReplayResult makeReplay(EpisodeStatus status, float reward, std::size_t stepCount) {
    DeterministicReplayResult replay;
    replay.status = status;
    replay.finalReward = reward;
    replay.steps.resize(stepCount);
    return replay;
}

/// Somme de tous les paramètres : signature scalaire suffisante pour distinguer deux jeux de poids.
float parameterSum(const Network& network) {
    float total = 0.0f;
    for (const aisolver::autodiff::NodePtr& parameter : network.parameters()) {
        for (std::size_t index = 0; index < parameter->value.size(); ++index) {
            total += parameter->value.data()[index];
        }
    }
    return total;
}

/// Remplace tous les poids par @p value : simule une mise à jour de gradient qui dégrade le réseau.
void fillParameters(Network& network, float value) {
    for (const aisolver::autodiff::NodePtr& parameter : network.parameters()) {
        for (std::size_t index = 0; index < parameter->value.size(); ++index) {
            parameter->value.data()[index] = value;
        }
    }
}

}  // namespace

TEST(PolicyScore, UneReussiteBatToujoursUnEchecMemeMoinsRecompensee) {
    const PolicyScore won{.solved = true, .reward = -100.0f, .stepCount = 500};
    const PolicyScore lost{.solved = false, .reward = 1000.0f, .stepCount = 10};

    EXPECT_TRUE(won.betterThan(lost));
    EXPECT_FALSE(lost.betterThan(won));
}

TEST(PolicyScore, EntreDeuxReussitesLaPlusCourteGagne) {
    const PolicyScore quick{.solved = true, .reward = 1.0f, .stepCount = 120};
    const PolicyScore slow{.solved = true, .reward = 9.0f, .stepCount = 300};

    EXPECT_TRUE(quick.betterThan(slow));
    EXPECT_FALSE(slow.betterThan(quick));
}

TEST(PolicyScore, EntreDeuxEchecsLaMieuxRecompenseeGagne) {
    const PolicyScore near{.solved = false, .reward = 4.0f, .stepCount = 300};
    const PolicyScore far{.solved = false, .reward = -2.0f, .stepCount = 300};

    EXPECT_TRUE(near.betterThan(far));
    EXPECT_FALSE(far.betterThan(near));
}

TEST(BestPolicySnapshot, VideTantQuAucunCandidatNAEteSoumis) {
    BestPolicySnapshot snapshot;
    const std::unique_ptr<Network> network = makeNetwork(7);

    EXPECT_FALSE(snapshot.hasSnapshot());
    EXPECT_FALSE(snapshot.bestReplay().has_value());
    EXPECT_FALSE(snapshot.restore(*network));
}

TEST(BestPolicySnapshot, LePremierCandidatEstToujoursRetenu) {
    BestPolicySnapshot snapshot;
    const std::unique_ptr<Network> network = makeNetwork(7);

    EXPECT_TRUE(snapshot.consider(*network, makeReplay(EpisodeStatus::TimedOut, -5.0f, 40)));
    EXPECT_TRUE(snapshot.hasSnapshot());
    EXPECT_FALSE(snapshot.bestScore().solved);
}

TEST(BestPolicySnapshot, UnCandidatMoinsBonNeRemplacePasLeCliche) {
    BestPolicySnapshot snapshot;
    const std::unique_ptr<Network> network = makeNetwork(7);
    const float winningSum = parameterSum(*network);

    ASSERT_TRUE(snapshot.consider(*network, makeReplay(EpisodeStatus::Won, 12.0f, 100)));

    // Le run continue et degrade les poids, puis echoue sur son dernier episode.
    fillParameters(*network, 0.5f);
    EXPECT_FALSE(snapshot.consider(*network, makeReplay(EpisodeStatus::Lost, 30.0f, 20)));

    EXPECT_TRUE(snapshot.bestScore().solved);
    EXPECT_EQ(snapshot.bestScore().stepCount, 100);
    ASSERT_TRUE(snapshot.restore(*network));
    EXPECT_FLOAT_EQ(parameterSum(*network), winningSum);
}

TEST(BestPolicySnapshot, RestaureLesPoidsDeLaMeilleureGenerationApresUnDernierEpisodeRate) {
    BestPolicySnapshot snapshot;
    const std::unique_ptr<Network> network = makeNetwork(7);

    // Generation intermediaire gagnante : c'est elle que le run doit sauvegarder.
    fillParameters(*network, 0.25f);
    const float winningSum = parameterSum(*network);
    ASSERT_TRUE(snapshot.consider(*network, makeReplay(EpisodeStatus::Won, 10.0f, 200)));

    // Derniere generation : echec, poids differents -- exactement ce qui etait sauvegarde avant.
    fillParameters(*network, -1.0f);
    const float failingSum = parameterSum(*network);
    ASSERT_NE(winningSum, failingSum);
    EXPECT_FALSE(snapshot.consider(*network, makeReplay(EpisodeStatus::TimedOut, -3.0f, 400)));

    ASSERT_TRUE(snapshot.restore(*network));
    EXPECT_FLOAT_EQ(parameterSum(*network), winningSum);
    ASSERT_TRUE(snapshot.bestReplay().has_value());
    EXPECT_EQ(snapshot.bestReplay()->status, EpisodeStatus::Won);
    EXPECT_EQ(snapshot.bestReplay()->steps.size(), 200u);
}

TEST(BestPolicySnapshot, UneReussitePlusCourteRemplaceLaPrecedente) {
    BestPolicySnapshot snapshot;
    const std::unique_ptr<Network> network = makeNetwork(7);

    fillParameters(*network, 0.25f);
    ASSERT_TRUE(snapshot.consider(*network, makeReplay(EpisodeStatus::Won, 10.0f, 300)));

    fillParameters(*network, 0.75f);
    const float shorterSum = parameterSum(*network);
    EXPECT_TRUE(snapshot.consider(*network, makeReplay(EpisodeStatus::Won, 8.0f, 150)));

    EXPECT_EQ(snapshot.bestScore().stepCount, 150);
    fillParameters(*network, 0.0f);
    ASSERT_TRUE(snapshot.restore(*network));
    EXPECT_FLOAT_EQ(parameterSum(*network), shorterSum);
}

TEST(BestPolicySnapshot, LeClicheEstUneCopieProfondeInsensibleAuxMisesAJourUlterieures) {
    BestPolicySnapshot snapshot;
    const std::unique_ptr<Network> network = makeNetwork(7);

    fillParameters(*network, 2.0f);
    const float capturedSum = parameterSum(*network);
    ASSERT_TRUE(snapshot.consider(*network, makeReplay(EpisodeStatus::Won, 1.0f, 50)));

    // Les poids continuent d'etre mis a jour EN PLACE : le cliche ne doit pas suivre.
    fillParameters(*network, -7.0f);
    ASSERT_TRUE(snapshot.restore(*network));
    EXPECT_FLOAT_EQ(parameterSum(*network), capturedSum);

    // Et il reste reutilisable apres restauration (restore ne consomme pas le cliche).
    fillParameters(*network, 3.0f);
    ASSERT_TRUE(snapshot.restore(*network));
    EXPECT_FLOAT_EQ(parameterSum(*network), capturedSum);
}

TEST(BestPolicySnapshot, RefuseUneTopologieIncompatibleSansToucherAuReseau) {
    BestPolicySnapshot snapshot;
    const std::unique_ptr<Network> trained = makeNetwork(7);
    ASSERT_TRUE(snapshot.consider(*trained, makeReplay(EpisodeStatus::Won, 1.0f, 50)));

    Rng otherRng(11);
    const std::unique_ptr<Network> other = aisolver::training::evolutionary::buildNetwork(
        aisolver::training::evolutionary::policyTopology(INPUT_SIZE, HIDDEN_SIZE + 3), otherRng);
    const float untouchedSum = parameterSum(*other);

    EXPECT_FALSE(snapshot.restore(*other));
    EXPECT_FLOAT_EQ(parameterSum(*other), untouchedSum);
}
