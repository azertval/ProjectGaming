// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sgd.cpp
 * @brief Tests unitaires de `aisolver::optim::Sgd` (LOT-ANNEXE-04, TACHE-01).
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Optim/Sgd.h"

using aisolver::Tensor;
using aisolver::autodiff::NodePtr;
using aisolver::autodiff::variable;
using aisolver::optim::Sgd;

namespace {
constexpr float TOLERANCE = 1e-5f;

NodePtr scalarVariable(float value) {
    Tensor<float> data({1});
    data.at({0}) = value;
    return variable(data);
}
}  // namespace

/**
 * @brief Sans inertie, `step` applique exactement `value -= learningRate * grad`.
 * \castest{<b>Sgd : mise à jour sans inertie.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un paramètre scalaire `value = 10`, `grad = 4`.<br/>2. Appliquer
 * `Sgd(0.5f).step({parameter})`.<br/>
 * \tattendu `value` vaut `10 - 0.5*4 = 8`.}
 */
TEST(SgdTest, MiseAJourSansInertie) {
    NodePtr parameter = scalarVariable(10.0f);
    parameter->grad.at({0}) = 4.0f;

    Sgd sgd(0.5f);
    sgd.step({parameter});

    EXPECT_NEAR(parameter->value.at({0}), 8.0f, TOLERANCE);
}

/**
 * @brief Avec inertie, la vitesse s'accumule sur deux `step` consécutifs à gradient constant,
 * selon `vitesse = momentum * vitesse - learningRate * grad`.
 * \castest{<b>Sgd : mise à jour avec inertie.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un paramètre scalaire `value = 0`, `grad = 1` constant.<br/>2. Appliquer
 * `Sgd(0.1f, 0.9f).step({parameter})` deux fois de suite (gradient reposé à `1` entre les
 * deux).<br/>
 * \tattendu Après le premier pas, `vitesse = -0.1`, `value = -0.1`. Après le second,
 * `vitesse = 0.9*(-0.1) - 0.1 = -0.19`, `value = -0.1 - 0.19 = -0.29`.}
 */
TEST(SgdTest, MiseAJourAvecInertie) {
    NodePtr parameter = scalarVariable(0.0f);
    Sgd sgd(0.1f, 0.9f);

    parameter->grad.at({0}) = 1.0f;
    sgd.step({parameter});
    EXPECT_NEAR(parameter->value.at({0}), -0.1f, TOLERANCE);

    parameter->grad.at({0}) = 1.0f;
    sgd.step({parameter});
    EXPECT_NEAR(parameter->value.at({0}), -0.29f, TOLERANCE);
}

/**
 * @brief `step`/`zeroGrad` appelés sur un sous-ensemble des paramètres d'un réseau ne modifient
 * pas les paramètres exclus de l'appel.
 * \castest{<b>Sgd : sans effet sur les paramètres non fournis.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux paramètres de gradient non nul.<br/>2. Appeler `step` et `zeroGrad`
 * sur un seul des deux.<br/>
 * \tattendu La valeur et le gradient du paramètre exclu restent inchangés.}
 */
TEST(SgdTest, SansEffetSurLesParametresNonFournis) {
    NodePtr included = scalarVariable(5.0f);
    NodePtr excluded = scalarVariable(5.0f);
    included->grad.at({0}) = 2.0f;
    excluded->grad.at({0}) = 2.0f;

    Sgd sgd(1.0f);
    sgd.step({included});
    sgd.zeroGrad({included});

    EXPECT_NEAR(excluded->value.at({0}), 5.0f, TOLERANCE);
    EXPECT_NEAR(excluded->grad.at({0}), 2.0f, TOLERANCE);
}
