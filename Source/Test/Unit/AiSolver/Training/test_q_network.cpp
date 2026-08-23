// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_q_network.cpp
 * @brief Tests unitaires de `QNetwork` (LOT-ANNEXE-14, TACHE-01).
 */

#include <gtest/gtest.h>

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Training/Advanced/QNetwork.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::actionCount;
using aisolver::training::QNetwork;

namespace {

constexpr std::size_t kInputSize = 3;
constexpr std::size_t kHiddenSize = 4;

Tensor<float> observationOf(float a, float b, float c) {
    Tensor<float> observation({kInputSize, 1});
    observation.at({0, 0}) = a;
    observation.at({1, 0}) = b;
    observation.at({2, 0}) = c;
    return observation;
}

}  // namespace

/**
 * @brief `forward` produit exactement une valeur par action de l'espace discret.
 * \castest{<b>QNetwork : forme de sortie.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `QNetwork` sur une observation quelconque.<br/>2. `forward`.<br/>
 * \tattendu Taille de sortie == `actionCount()`.}
 */
TEST(QNetworkTest, FormeDeSortie) {
    Rng rng(1);
    QNetwork network(kInputSize, kHiddenSize, rng);
    const auto output = network.forward(observationOf(0.1f, -0.2f, 0.3f));
    EXPECT_EQ(output->value.size(), actionCount());
}

/**
 * @brief Après `copyWeightsFrom`, modifier les poids du réseau source ne modifie pas ceux de la
 * cible (copie profonde, aucun tampon partagé).
 * \castest{<b>QNetwork : copie profonde independante.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Synchroniser une cible depuis une source.<br/>2. Muter un poids de la source.<br/>
 * \tattendu Le poids correspondant de la cible reste inchangé.}
 */
TEST(QNetworkTest, CopieProfondeIndependante) {
    Rng sourceRng(2);
    QNetwork source(kInputSize, kHiddenSize, sourceRng);
    Rng targetRng(3);
    QNetwork target(kInputSize, kHiddenSize, targetRng);

    target.copyWeightsFrom(source);

    const auto sourceParameters = source.parameters();
    const auto targetParameters = target.parameters();
    ASSERT_EQ(sourceParameters.size(), targetParameters.size());
    for (std::size_t i = 0; i < sourceParameters.size(); ++i) {
        ASSERT_EQ(sourceParameters[i]->value.size(), targetParameters[i]->value.size());
        for (std::size_t j = 0; j < sourceParameters[i]->value.size(); ++j) {
            EXPECT_FLOAT_EQ(sourceParameters[i]->value.data()[j],
                            targetParameters[i]->value.data()[j]);
        }
    }

    // Mutation de la source apres synchronisation : la cible ne doit pas suivre.
    const float originalTargetValue = targetParameters.front()->value.data()[0];
    sourceParameters.front()->value.data()[0] += 100.0f;
    EXPECT_FLOAT_EQ(targetParameters.front()->value.data()[0], originalTargetValue);
}
