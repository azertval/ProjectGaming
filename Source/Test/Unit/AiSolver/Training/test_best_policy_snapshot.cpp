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

/**
 * @brief Une réussite l'emporte sur un échec, quelle que soit la récompense.
 * \castest{<b>Une réussite l'emporte sur un échec, quelle que soit la récompense.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Comparer une réussite très mal récompensée (-100) et lente (500 pas) à un échec très
 * bien récompensé (1000) et rapide (10 pas).<br/>
 * \tattendu La réussite gagne dans les deux sens de la comparaison : résoudre le tableau prime sur
 * toute récompense, sans quoi un run conserverait un modèle qui échoue mieux qu'un modèle qui
 * réussit.}
 */
TEST(PolicyScore, UneReussiteBatToujoursUnEchecMemeMoinsRecompensee) {
    const PolicyScore won{.solved = true, .reward = -100.0f, .stepCount = 500};
    const PolicyScore lost{.solved = false, .reward = 1000.0f, .stepCount = 10};

    EXPECT_TRUE(won.betterThan(lost));
    EXPECT_FALSE(lost.betterThan(won));
}

/**
 * @brief À réussite égale, le trajet le plus court gagne.
 * \castest{<b>À réussite égale, le trajet le plus court gagne.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Comparer deux réussites : l'une en 120 pas moins récompensée, l'autre en 300 pas
 * mieux récompensée.<br/>
 * \tattendu La plus courte gagne : entre deux politiques qui résolvent, c'est le nombre de pas qui
 * départage, pas la récompense accumulée.}
 */
TEST(PolicyScore, EntreDeuxReussitesLaPlusCourteGagne) {
    const PolicyScore quick{.solved = true, .reward = 1.0f, .stepCount = 120};
    const PolicyScore slow{.solved = true, .reward = 9.0f, .stepCount = 300};

    EXPECT_TRUE(quick.betterThan(slow));
    EXPECT_FALSE(slow.betterThan(quick));
}

/**
 * @brief À échec égal, la mieux récompensée gagne.
 * \castest{<b>À échec égal, la mieux récompensée gagne.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Comparer deux échecs de même longueur, l'un récompensé 4, l'autre -2.<br/>
 * \tattendu Le mieux récompensé gagne : faute de réussite, la récompense reste le seul signal de
 * progrès disponible.}
 */
TEST(PolicyScore, EntreDeuxEchecsLaMieuxRecompenseeGagne) {
    const PolicyScore near{.solved = false, .reward = 4.0f, .stepCount = 300};
    const PolicyScore far{.solved = false, .reward = -2.0f, .stepCount = 300};

    EXPECT_TRUE(near.betterThan(far));
    EXPECT_FALSE(far.betterThan(near));
}

/**
 * @brief Un cliché neuf est vide et ne restaure rien.
 * \castest{<b>Un cliché neuf est vide et ne restaure rien.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un cliché sans lui soumettre le moindre candidat.<br/>2. Interroger son
 * état et tenter une restauration.<br/>
 * \tattendu Aucun cliché, aucun rejeu, et `restore` échoue sans toucher au réseau — un run
 * interrompu avant son premier épisode ne doit rien écraser.}
 */
TEST(BestPolicySnapshot, VideTantQuAucunCandidatNAEteSoumis) {
    BestPolicySnapshot snapshot;
    const std::unique_ptr<Network> network = makeNetwork(7);

    EXPECT_FALSE(snapshot.hasSnapshot());
    EXPECT_FALSE(snapshot.bestReplay().has_value());
    EXPECT_FALSE(snapshot.restore(*network));
}

/**
 * @brief Le premier candidat est toujours retenu, même s'il échoue.
 * \castest{<b>Le premier candidat est retenu, même s'il échoue.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Soumettre un unique candidat dont l'épisode expire (`TimedOut`).<br/>
 * \tattendu Il est retenu : il n'y a rien de mieux à conserver. Le cliché existe et son score n'est
 * pas marqué comme résolu.}
 */
TEST(BestPolicySnapshot, LePremierCandidatEstToujoursRetenu) {
    BestPolicySnapshot snapshot;
    const std::unique_ptr<Network> network = makeNetwork(7);

    EXPECT_TRUE(snapshot.consider(*network, makeReplay(EpisodeStatus::TimedOut, -5.0f, 40)));
    EXPECT_TRUE(snapshot.hasSnapshot());
    EXPECT_FALSE(snapshot.bestScore().solved);
}

/**
 * @brief Un candidat moins bon ne remplace pas le cliché.
 * \castest{<b>Un candidat moins bon ne remplace pas le cliché.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Soumettre une génération gagnante.<br/>2. Dégrader les poids en place, puis soumettre
 * un échec mieux récompensé.<br/>3. Restaurer.<br/>
 * \tattendu Le second candidat est refusé, le score conservé reste celui de la réussite, et la
 * restauration rend exactement les poids gagnants.}
 */
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

/**
 * @brief Un run qui finit mal restaure sa meilleure génération.
 * \castest{<b>Un run qui finit mal restaure sa meilleure génération.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Faire gagner une génération intermédiaire.<br/>2. Dégrader les poids et faire expirer
 * la dernière génération.<br/>3. Restaurer et relire le rejeu conservé.<br/>
 * \tattendu Les poids gagnants reviennent, et le rejeu conservé est bien celui de la génération
 * gagnante — c'est le défaut que ce mécanisme corrige : un run ne doit plus publier son DERNIER
 * modèle, mais son MEILLEUR.}
 */
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

/**
 * @brief Une réussite plus courte remplace la précédente.
 * \castest{<b>Une réussite plus courte remplace la précédente.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Conserver une réussite en 300 pas.<br/>2. Soumettre une réussite en 150 pas avec
 * d'autres poids.<br/>3. Écraser les poids courants puis restaurer.<br/>
 * \tattendu Le cliché bascule sur la réussite courte et la restauration rend ses poids à elle.}
 */
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

/**
 * @brief Le cliché est une copie profonde, et reste réutilisable après restauration.
 * \castest{<b>Le cliché est une copie profonde, réutilisable.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Conserver un réseau gagnant.<br/>2. Continuer à mettre à jour ses poids EN
 * PLACE.<br/>3. Restaurer deux fois de suite, en réécrivant les poids entre les deux.<br/>
 * \tattendu Les poids restaurés sont ceux capturés, inchangés par les mises à jour ultérieures ; et
 * `restore` ne consomme pas le cliché, qui reste utilisable.}
 */
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

/**
 * @brief Une topologie incompatible est refusée sans toucher au réseau cible.
 * \castest{<b>Une topologie incompatible est refusée, réseau intact.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Conserver un cliché sur un réseau donné.<br/>2. Tenter de le restaurer dans un réseau
 * de couche cachée plus large.<br/>
 * \tattendu `restore` renvoie faux et laisse le réseau cible strictement intact — jamais un
 * chargement partiel, qui produirait un agent silencieusement corrompu.}
 */
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
