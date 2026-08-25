// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_weight_init.cpp
 * @brief Tests statistiques de `aisolver::nn::initializeWeights` (LOT-ANNEXE-03, TACHE-04) :
 * bornes Xavier, plausibilité He, reproductibilité par graine. Couverture fonctionnelle de base
 * (forme, non-constance) déjà faite par `test_dense.cpp` (TACHE-01), qui livre l'implémentation.
 */

#include <cmath>

#include <gtest/gtest.h>

#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/WeightInit.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::nn::initializeWeights;
using aisolver::nn::WeightInitScheme;

/**
 * @brief Tous les éléments initialisés par `Xavier` restent dans `[-bound, bound]`, `bound =
 * sqrt(6 / (fanIn + fanOut))`.
 * \castest{<b>WeightInit : `Xavier` reste dans les bornes.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Initialiser un tenseur `[50, 30]` (fanOut=50, fanIn=30) avec `Xavier`.<br/>
 * 2. Comparer chaque élément à `±bound`.<br/>
 * \tattendu Tous les éléments sont dans `[-bound, bound]`.}
 */
TEST(WeightInitTest, XavierResteDansLesBornes) {
    Rng rng(7001);
    Tensor<float> weights({50, 30});
    initializeWeights(weights, WeightInitScheme::Xavier, rng);

    const float bound = std::sqrt(6.0f / static_cast<float>(30 + 50));
    for (std::size_t i = 0; i < weights.size(); ++i) {
        const float value = weights.data()[i];
        EXPECT_GE(value, -bound);
        EXPECT_LE(value, bound);
    }
}

/**
 * @brief `He` produit une moyenne empirique proche de `0` et un écart-type proche de
 * `sqrt(2/fanIn)` (tolérance large, même esprit que le test statistique faible de `Rng`,
 * `LOT-ANNEXE-01` TACHE-01).
 * \castest{<b>WeightInit : `He` plausible.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Initialiser un tenseur `[200, 100]` (fanIn=100) avec `He`.<br/>2. Calculer moyenne
 * et écart-type empiriques.<br/>
 * \tattendu Moyenne proche de `0` (±0.05), écart-type proche de `sqrt(2/100)` (±20% relatif).}
 */
TEST(WeightInitTest, HePlausible) {
    Rng rng(7002);
    Tensor<float> weights({200, 100});
    initializeWeights(weights, WeightInitScheme::He, rng);

    double sum = 0.0;
    for (std::size_t i = 0; i < weights.size(); ++i) {
        sum += weights.data()[i];
    }
    const double mean = sum / static_cast<double>(weights.size());

    double sumSquaredDeviation = 0.0;
    for (std::size_t i = 0; i < weights.size(); ++i) {
        const double deviation = weights.data()[i] - mean;
        sumSquaredDeviation += deviation * deviation;
    }
    const double stddev = std::sqrt(sumSquaredDeviation / static_cast<double>(weights.size()));
    const double expectedStddev = std::sqrt(2.0 / 100.0);

    EXPECT_NEAR(mean, 0.0, 0.05);
    EXPECT_NEAR(stddev, expectedStddev, expectedStddev * 0.2);
}

/**
 * @brief Deux tenseurs initialisés avec des `Rng` de même graine produisent des poids identiques.
 * \castest{<b>WeightInit : reproductibilité par graine.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Initialiser deux tenseurs `[4,3]` identiques avec deux `Rng` de même graine
 * (`Xavier`).<br/>2. Comparer élément par élément.<br/>
 * \tattendu Les deux tenseurs sont identiques bit à bit.}
 */
TEST(WeightInitTest, ReproductibiliteParGraine) {
    Rng rngA(7003);
    Rng rngB(7003);
    Tensor<float> weightsA({4, 3});
    Tensor<float> weightsB({4, 3});
    initializeWeights(weightsA, WeightInitScheme::Xavier, rngA);
    initializeWeights(weightsB, WeightInitScheme::Xavier, rngB);

    for (std::size_t i = 0; i < weightsA.size(); ++i) {
        EXPECT_EQ(weightsA.data()[i], weightsB.data()[i]);
    }
}
