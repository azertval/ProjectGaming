// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_policy_gradient_loss.cpp
 * @brief Tests unitaires de `computeWeightedPolicyGradientLoss` (`LOT-ANNEXE-13`, TACHE-02).
 *
 * C'est la **primitive partagée** : `computeReinforceLoss` (poids = retours) et
 * `computeActorCriticLoss` (poids = avantages) s'y ramènent tous deux. Ses deux consommateurs
 * étaient éprouvés, la formule commune ne l'était pas — or c'est elle qui porte le contrat :
 * linéarité en le poids, moyenne et non somme, indépendance de la log-probabilité détachée.
 */

#include <cmath>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"
#include "AiSolver/Training/PolicyGradientLoss.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::training::computeWeightedPolicyGradientLoss;
using aisolver::training::Trajectory;
using aisolver::training::TrajectoryStep;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;

namespace {

constexpr std::size_t INPUT_SIZE = 3;
constexpr std::size_t HIDDEN_SIZE = 4;

std::unique_ptr<aisolver::nn::Network> tinyPolicy(std::uint64_t seed) {
    Rng rng(seed);
    return buildNetwork(policyTopology(INPUT_SIZE, HIDDEN_SIZE), rng);
}

Tensor<float> observationOf(float a, float b, float c) {
    Tensor<float> observation({INPUT_SIZE, 1});
    observation.at({0, 0}) = a;
    observation.at({1, 0}) = b;
    observation.at({2, 0}) = c;
    return observation;
}

TrajectoryStep stepAt(std::size_t actionIndex, const Tensor<float>& observation) {
    TrajectoryStep step;
    step.observation = observation;
    step.actionIndex = actionIndex;
    return step;
}

/// Trajectoire de trois pas distincts, suffisante pour distinguer une moyenne d'une somme.
Trajectory threeSteps() {
    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(0, observationOf(1.0f, 0.0f, 0.0f)));
    trajectory.steps.push_back(stepAt(1, observationOf(0.0f, 1.0f, 0.0f)));
    trajectory.steps.push_back(stepAt(2, observationOf(0.0f, 0.0f, 1.0f)));
    return trajectory;
}

float lossValue(aisolver::nn::Network& policy, const Trajectory& trajectory,
                const std::vector<float>& weights) {
    return computeWeightedPolicyGradientLoss(policy, trajectory, weights)->value.data()[0];
}

}  // namespace

/**
 * @brief Des poids tous nuls annulent la perte : aucun pas ne pousse dans une direction.
 * \castest{<b>Poids nuls : perte nulle.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Trajectoire de trois pas, poids `{0, 0, 0}`.<br/>
 * \tattendu La perte vaut exactement zéro.}
 */
TEST(PolicyGradientLossTest, PoidsNulsAnnulentLaPerte) {
    const std::unique_ptr<aisolver::nn::Network> policy = tinyPolicy(11);
    EXPECT_FLOAT_EQ(lossValue(*policy, threeSteps(), {0.0f, 0.0f, 0.0f}), 0.0f);
}

/**
 * @brief La perte est **linéaire** en les poids : les doubler double la perte, les opposer
 * l'oppose.
 *
 * C'est ce qui rend la primitive réutilisable telle quelle par REINFORCE et par l'acteur-critique :
 * seule la *nature* du poids change (retour brut ou avantage), jamais la formule. Un avantage
 * négatif doit donc pousser exactement en sens inverse d'un avantage positif de même amplitude.
 * \castest{<b>La perte est linéaire en les poids.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Perte avec des poids donnés.<br/>2. Perte avec ces poids doublés.<br/>3. Perte avec
 * ces poids opposés.<br/>
 * \tattendu La deuxième vaut le double de la première, la troisième son opposée.}
 */
TEST(PolicyGradientLossTest, LaPerteEstLineaireEnLesPoids) {
    const std::unique_ptr<aisolver::nn::Network> policy = tinyPolicy(12);
    const Trajectory trajectory = threeSteps();

    const std::vector<float> weights{1.0f, -0.5f, 2.0f};
    const std::vector<float> doubled{2.0f, -1.0f, 4.0f};
    const std::vector<float> opposite{-1.0f, 0.5f, -2.0f};

    const float base = lossValue(*policy, trajectory, weights);
    ASSERT_NE(base, 0.0f) << "trajectoire degeneree : le test ne discriminerait rien";

    EXPECT_NEAR(lossValue(*policy, trajectory, doubled), 2.0f * base, std::abs(base) * 1e-4f);
    EXPECT_NEAR(lossValue(*policy, trajectory, opposite), -base, std::abs(base) * 1e-4f);
}

/**
 * @brief La perte est une **moyenne** par pas, pas une somme : allonger la trajectoire de pas
 * identiques ne change pas son amplitude.
 *
 * Sans cela, l'amplitude du gradient dépendrait de la longueur — variable — de l'épisode, et un
 * épisode long pèserait mécaniquement plus qu'un épisode court à qualité de politique égale.
 * \castest{<b>La perte est une moyenne par pas, pas une somme.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Perte d'une trajectoire d'UN pas, poids unitaire.<br/>2. Perte d'une trajectoire de
 * TROIS pas identiques au premier, poids unitaires.<br/>
 * \tattendu Les deux pertes coïncident.}
 */
TEST(PolicyGradientLossTest, MoyenneParPasEtNonSomme) {
    const std::unique_ptr<aisolver::nn::Network> policy = tinyPolicy(13);
    const Tensor<float> observation = observationOf(0.25f, -0.5f, 0.75f);

    Trajectory single;
    single.steps.push_back(stepAt(1, observation));

    Trajectory repeated;
    for (int repetition = 0; repetition < 3; ++repetition) {
        repeated.steps.push_back(stepAt(1, observation));
    }

    const float singleLoss = lossValue(*policy, single, {1.0f});
    const float repeatedLoss = lossValue(*policy, repeated, {1.0f, 1.0f, 1.0f});
    EXPECT_NEAR(repeatedLoss, singleLoss, std::abs(singleLoss) * 1e-4f);
}

/**
 * @brief La perte ignore `TrajectoryStep::logProbability` : elle rejoue le passage avant avec les
 * poids **actuels** du réseau.
 *
 * `logProbability` est une valeur détachée, relevée au moment de la collecte et sans historique de
 * graphe : la consommer donnerait un gradient nul (une constante) et gèlerait l'apprentissage dès
 * que la politique aurait changé depuis la collecte. Le contrat est écrit dans l'en-tête ; ce test
 * le rend exécutable.
 * \castest{<b>La log-probabilité détachée de la trajectoire n'est jamais consommée.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Perte d'une trajectoire dont les log-probabilités valent zéro.<br/>2. Même
 * trajectoire dont les log-probabilités et récompenses sont remplacées par des valeurs
 * absurdes.<br/>
 * \tattendu Les deux pertes sont identiques.}
 */
TEST(PolicyGradientLossTest, IgnoreLaLogProbabiliteDetachee) {
    const std::unique_ptr<aisolver::nn::Network> policy = tinyPolicy(14);
    const std::vector<float> weights{1.0f, 1.0f, 1.0f};

    const Trajectory pristine = threeSteps();
    Trajectory tampered = threeSteps();
    for (TrajectoryStep& step : tampered.steps) {
        step.logProbability = -999.0f;  // valeur absurde : aucune influence attendue
        step.reward = 42.0f;            // idem, les recompenses passent par `weights`
    }

    EXPECT_FLOAT_EQ(lossValue(*policy, tampered, weights), lossValue(*policy, pristine, weights));
}

/**
 * @brief Un poids strictement positif rend une perte strictement positive : l'opposé du logarithme
 * d'une probabilité strictement comprise entre 0 et 1 est positif.
 * \castest{<b>Poids positif : perte strictement positive.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Perte d'une trajectoire de trois pas, poids tous strictement positifs.<br/>
 * \tattendu La perte est strictement positive et finie.}
 */
TEST(PolicyGradientLossTest, PoidsPositifDonneUnePerteStrictementPositive) {
    const std::unique_ptr<aisolver::nn::Network> policy = tinyPolicy(15);
    const float loss = lossValue(*policy, threeSteps(), {1.0f, 2.0f, 0.5f});
    EXPECT_GT(loss, 0.0f);
    EXPECT_TRUE(std::isfinite(loss));
}
