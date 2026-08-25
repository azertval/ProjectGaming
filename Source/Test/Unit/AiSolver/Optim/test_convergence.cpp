// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_convergence.cpp
 * @brief Tests de convergence des optimiseurs sur des problèmes jouets (LOT-ANNEXE-04, TACHE-03).
 *
 * Suite BLOQUANTE avant tout usage des optimiseurs par la génération 3 (REINFORCE, acteur-critique,
 * algorithme avancé) : un optimiseur qui ne fait pas converger un problème connu et trivial ne doit
 * jamais être laissé entraîner un agent réel, où un échec de convergence serait indiscernable du
 * bruit inhérent à l'apprentissage par renforcement lui-même (même principe que le gradient
 * checking de LOT-ANNEXE-02, `GradientCheck.h`).
 */

#include <cmath>
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Optim/Adam.h"
#include "AiSolver/Optim/Sgd.h"
#include "ToyProblems.h"

using aisolver::PolynomialToyProblem;
using aisolver::QuadraticToyProblem;
using aisolver::Rng;
using aisolver::Tensor;
using aisolver::autodiff::backward;
using aisolver::autodiff::NodePtr;
using aisolver::autodiff::variable;
using aisolver::optim::Adam;
using aisolver::optim::Sgd;

namespace {

NodePtr scalarVariable(float value) {
    Tensor<float> data({1});
    data.at({0}) = value;
    return variable(data);
}

// Nombre d'itérations à partir de laquelle l'erreur reste sous `tolerance` jusqu'à la fin de la
// fenêtre simulée (pas la première itération où elle la franchit, potentiellement un croisement
// transitoire d'une oscillation amortie) ; `maxIterations` si elle ne s'y stabilise jamais.
std::size_t iterationsToConvergeOnQuadratic(float learningRate, float momentum, float tolerance,
                                            std::size_t maxIterations) {
    NodePtr x = scalarVariable(0.0f);
    Sgd sgd(learningRate, momentum);

    std::vector<float> absoluteErrors;
    absoluteErrors.reserve(maxIterations);
    for (std::size_t i = 0; i < maxIterations; ++i) {
        backward(QuadraticToyProblem::loss(x));
        sgd.step({x});
        sgd.zeroGrad({x});
        absoluteErrors.push_back(std::abs(x->value.at({0}) - QuadraticToyProblem::kTarget));
    }

    std::size_t lastAboveTolerance = maxIterations;
    for (std::size_t i = 0; i < absoluteErrors.size(); ++i) {
        if (absoluteErrors[i] >= tolerance) {
            lastAboveTolerance = i;
        }
    }
    return lastAboveTolerance == maxIterations ? maxIterations : lastAboveTolerance + 1;
}

}  // namespace

/**
 * @brief `Sgd` sans inertie fait converger la quadratique convexe jusqu'à une tolérance
 * documentée, en un nombre d'itérations borné.
 * \castest{<b>Convergence : `Sgd` sans inertie sur la quadratique convexe.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Minimiser `QuadraticToyProblem` par `Sgd(0.02f)` depuis `x = 0`.<br/>2. Chercher la
 * première itération à partir de laquelle l'erreur reste sous `1e-2` jusqu'à `1000`
 * itérations.<br/>
 * \tattendu La convergence est atteinte strictement avant la borne de `1000` itérations.}
 */
TEST(ConvergenceTest, SgdSansInertieConvergeSurQuadratique) {
    constexpr std::size_t maxIterations = 1000;
    const std::size_t iterations =
        iterationsToConvergeOnQuadratic(0.02f, 0.0f, 1e-2f, maxIterations);
    EXPECT_LT(iterations, maxIterations);
}

/**
 * @brief `Sgd` avec inertie converge en moins d'itérations que sans inertie sur le même problème,
 * mêmes conditions initiales et même taux d'apprentissage — démontre l'effet du terme de momentum.
 * \castest{<b>Convergence : `Sgd` avec inertie converge plus vite que sans inertie.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Minimiser `QuadraticToyProblem` par `Sgd(0.02f)` puis `Sgd(0.02f, 0.5f)`, mêmes
 * conditions initiales.<br/>2. Comparer le nombre d'itérations nécessaires pour rester sous la
 * tolérance `1e-2` jusqu'à `1000` itérations.<br/>
 * \tattendu Les deux convergent avant `1000` itérations ; la variante avec inertie converge en
 * strictement moins d'itérations.}
 */
TEST(ConvergenceTest, SgdAvecInertieConvergePlusVite) {
    constexpr std::size_t maxIterations = 1000;
    const std::size_t withoutMomentum =
        iterationsToConvergeOnQuadratic(0.02f, 0.0f, 1e-2f, maxIterations);
    const std::size_t withMomentum =
        iterationsToConvergeOnQuadratic(0.02f, 0.5f, 1e-2f, maxIterations);

    EXPECT_LT(withoutMomentum, maxIterations);
    EXPECT_LT(withMomentum, maxIterations);
    EXPECT_LT(withMomentum, withoutMomentum);
}

/**
 * @brief `Adam` fait converger la régression polynomiale à coefficients connus jusqu'à une
 * tolérance documentée sur la perte moyenne.
 * \castest{<b>Convergence : `Adam` sur la régression polynomiale.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Générer `PolynomialToyProblem::generateSamples` (graine `Rng` fixe).<br/>2.
 * Minimiser la perte MSE des poids `[1, 3]` par `Adam(0.1f)` sur `500` itérations.<br/>
 * \tattendu La perte finale est inférieure à `0.05`.}
 */
TEST(ConvergenceTest, AdamConvergeSurRegressionPolynomiale) {
    Rng rng(42);
    const std::vector<PolynomialToyProblem::Sample> samples =
        PolynomialToyProblem::generateSamples(rng);

    NodePtr weights = variable(Tensor<float>({1, 3}));
    Adam adam(0.1f);

    NodePtr lastLoss;
    for (int i = 0; i < 500; ++i) {
        lastLoss = PolynomialToyProblem::loss(weights, samples);
        backward(lastLoss);
        adam.step({weights});
        adam.zeroGrad({weights});
    }

    EXPECT_LT(lastLoss->value.at({0, 0}), 0.05f);
}

/**
 * @brief À taux d'apprentissage identique et choisi pour ce contraste, `Adam` converge sur la
 * régression polynomiale là où `Sgd` sans inertie diverge — démontre concrètement l'intérêt
 * pratique d'Adam, pas seulement sa formule.
 * \castest{<b>Convergence : `Adam` converge là où `Sgd` sans inertie diverge.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Générer les mêmes échantillons (graine `Rng` fixe).<br/>2. Minimiser la perte par
 * `Sgd(1.0f)` puis par `Adam(1.0f)` sur `200` itérations, même taux d'apprentissage `1.0`.<br/>
 * \tattendu La perte finale de `Sgd` n'est pas finie (divergence) ; celle d'`Adam` reste finie et
 * inférieure à `1.0`.}
 */
TEST(ConvergenceTest, AdamConvergeLaOuSgdDiverge) {
    constexpr float sharedLearningRate = 1.0f;

    Rng rngForSgd(7);
    const std::vector<PolynomialToyProblem::Sample> samplesForSgd =
        PolynomialToyProblem::generateSamples(rngForSgd);
    NodePtr sgdWeights = variable(Tensor<float>({1, 3}));
    Sgd sgd(sharedLearningRate);
    NodePtr sgdLoss;
    for (int i = 0; i < 200; ++i) {
        sgdLoss = PolynomialToyProblem::loss(sgdWeights, samplesForSgd);
        backward(sgdLoss);
        sgd.step({sgdWeights});
        sgd.zeroGrad({sgdWeights});
        if (!std::isfinite(sgdLoss->value.at({0, 0}))) {
            break;
        }
    }

    Rng rngForAdam(7);
    const std::vector<PolynomialToyProblem::Sample> samplesForAdam =
        PolynomialToyProblem::generateSamples(rngForAdam);
    NodePtr adamWeights = variable(Tensor<float>({1, 3}));
    Adam adam(sharedLearningRate);
    NodePtr adamLoss;
    for (int i = 0; i < 200; ++i) {
        adamLoss = PolynomialToyProblem::loss(adamWeights, samplesForAdam);
        backward(adamLoss);
        adam.step({adamWeights});
        adam.zeroGrad({adamWeights});
    }

    EXPECT_FALSE(std::isfinite(sgdLoss->value.at({0, 0})));
    EXPECT_TRUE(std::isfinite(adamLoss->value.at({0, 0})));
    EXPECT_LT(adamLoss->value.at({0, 0}), 1.0f);
}

/**
 * @brief Deux exécutions de la même suite d'entraînement, même graine `Rng`, produisent une
 * trajectoire de convergence identique.
 * \castest{<b>Convergence : reproductibilité à graine `Rng` fixée.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Exécuter deux fois la même boucle d'entraînement `Adam` sur
 * `PolynomialToyProblem`, même graine `Rng`.<br/>2. Comparer les poids finaux.<br/>
 * \tattendu Les poids finaux des deux exécutions sont strictement égaux.}
 */
TEST(ConvergenceTest, ReproductibiliteAGraineRngFixee) {
    auto run = []() {
        Rng rng(123);
        const std::vector<PolynomialToyProblem::Sample> samples =
            PolynomialToyProblem::generateSamples(rng);
        NodePtr weights = variable(Tensor<float>({1, 3}));
        Adam adam(0.1f);
        for (int i = 0; i < 50; ++i) {
            backward(PolynomialToyProblem::loss(weights, samples));
            adam.step({weights});
            adam.zeroGrad({weights});
        }
        return weights->value;
    };

    const Tensor<float> first = run();
    const Tensor<float> second = run();

    for (std::size_t i = 0; i < first.size(); ++i) {
        EXPECT_EQ(first.data()[i], second.data()[i]);
    }
}
