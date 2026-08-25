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
