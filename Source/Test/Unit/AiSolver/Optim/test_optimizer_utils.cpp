// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optimizer_utils.cpp
 * @brief Tests unitaires de `aisolver::optim::zeroGrad` (LOT-ANNEXE-04, TACHE-01).
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Optim/OptimizerUtils.h"

using aisolver::Tensor;
using aisolver::autodiff::NodePtr;
using aisolver::autodiff::variable;

namespace {
constexpr float TOLERANCE = 1e-5f;

NodePtr scalarVariable(float value) {
    Tensor<float> data({1});
    data.at({0}) = value;
    return variable(data);
}
}  // namespace

/**
 * @brief `zeroGrad` remet le gradient de chaque paramètre fourni à zéro, sans affecter sa valeur.
 * \castest{<b>OptimizerUtils : `zeroGrad` remet les gradients à zéro.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux paramètres de gradient non nul.<br/>2. Appeler `zeroGrad` sur les
 * deux.<br/>
 * \tattendu Le gradient des deux paramètres vaut zéro ; leur valeur est inchangée.}
 */
TEST(OptimizerUtilsTest, ZeroGradRemetLesGradientsAZero) {
    NodePtr a = scalarVariable(3.0f);
    NodePtr b = scalarVariable(-2.0f);
    a->grad.at({0}) = 7.0f;
    b->grad.at({0}) = -4.0f;

    aisolver::optim::zeroGrad({a, b});

    EXPECT_NEAR(a->grad.at({0}), 0.0f, TOLERANCE);
    EXPECT_NEAR(b->grad.at({0}), 0.0f, TOLERANCE);
    EXPECT_NEAR(a->value.at({0}), 3.0f, TOLERANCE);
    EXPECT_NEAR(b->value.at({0}), -2.0f, TOLERANCE);
}

/**
 * @brief `zeroGrad` n'affecte pas les paramètres exclus de l'appel.
 * \castest{<b>OptimizerUtils : sans effet sur les paramètres non fournis.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux paramètres de gradient non nul.<br/>2. Appeler `zeroGrad` sur un
 * seul des deux.<br/>
 * \tattendu Le gradient du paramètre fourni est nul ; celui de l'autre reste inchangé.}
 */
TEST(OptimizerUtilsTest, SansEffetSurLesParametresNonFournis) {
    NodePtr a = scalarVariable(1.0f);
    NodePtr b = scalarVariable(1.0f);
    a->grad.at({0}) = 5.0f;
    b->grad.at({0}) = 9.0f;

    aisolver::optim::zeroGrad({a});

    EXPECT_NEAR(a->grad.at({0}), 0.0f, TOLERANCE);
    EXPECT_NEAR(b->grad.at({0}), 9.0f, TOLERANCE);
}

/**
 * @brief `clipGradientNorm` ramène la norme globale à la borne demandée **sans changer la
 *        direction** de la mise à jour.
 * \castest{<b>Écrêtage de gradient : norme bornée, direction préservée.</b><br/>
 * \tcat Unitaire · AiSolver Optim<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Donner à deux paramètres des gradients de norme globale connue.<br/>2. Écrêter
 * à une borne inférieure à cette norme.<br/>3. Écrêter à une borne supérieure.<br/>
 * \tattendu Au premier appel la norme finale vaut la borne et chaque composante a été
 * multipliée par le **même** facteur ; au second, aucun gradient n'est modifié.}
 */
TEST(OptimizerUtilsTest, EcretageBorneLaNormeSansChangerLaDirection) {
    Tensor<float> firstValue({2});
    Tensor<float> secondValue({2});
    const aisolver::autodiff::NodePtr first = aisolver::autodiff::variable(firstValue);
    const aisolver::autodiff::NodePtr second = aisolver::autodiff::variable(secondValue);
    // Norme globale = sqrt(3^2 + 4^2 + 0 + 0) = 5.
    first->grad.at({0}) = 3.0f;
    first->grad.at({1}) = 0.0f;
    second->grad.at({0}) = 4.0f;
    second->grad.at({1}) = 0.0f;

    const std::vector<aisolver::autodiff::NodePtr> parameters{first, second};
    const float normBefore = aisolver::optim::clipGradientNorm(parameters, 1.0f);
    EXPECT_FLOAT_EQ(normBefore, 5.0f);
    EXPECT_FLOAT_EQ(first->grad.at({0}), 3.0f / 5.0f);
    EXPECT_FLOAT_EQ(second->grad.at({0}), 4.0f / 5.0f);

    // Deuxieme appel, borne largement au-dessus de la norme courante (qui vaut 1) : rien ne bouge.
    const float untouched = aisolver::optim::clipGradientNorm(parameters, 100.0f);
    EXPECT_FLOAT_EQ(untouched, 1.0f);
    EXPECT_FLOAT_EQ(first->grad.at({0}), 3.0f / 5.0f);
    EXPECT_FLOAT_EQ(second->grad.at({0}), 4.0f / 5.0f);
}
