// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_adam.cpp
 * @brief Tests unitaires de `aisolver::optim::Adam` (LOT-ANNEXE-04, TACHE-02).
 */

#include <cmath>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Optim/Adam.h"

using aisolver::Tensor;
using aisolver::autodiff::NodePtr;
using aisolver::autodiff::variable;
using aisolver::optim::Adam;

namespace {
constexpr float TOLERANCE = 1e-5f;

NodePtr scalarVariable(float value) {
    Tensor<float> data({1});
    data.at({0}) = value;
    return variable(data);
}
}  // namespace

/**
 * @brief Sur le tout premier `step` (compteur = 1), `mHat`/`vHat` correspondent à la formule de
 * correction de biais, pas aux moments bruts.
 * \castest{<b>Adam : correction de biais au premier pas.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un paramètre scalaire `value = 0`, `grad = 1`.<br/>2. Appliquer un seul
 * `Adam(0.1f, 0.9f, 0.999f, 1e-8f).step({parameter})`.<br/>
 * \tattendu `m = 0.1`, `v = 0.001`, `mHat = m/(1-0.9) = 1`, `vHat = v/(1-0.999) = 1`, donc
 * `value = 0 - 0.1 * 1/(sqrt(1)+1e-8) ≈ -0.1`, distinct de la formule non corrigée (qui donnerait
 * `-0.1*0.1/(sqrt(0.001)+1e-8) ≈ -0.316`).}
 */
TEST(AdamTest, CorrectionDeBiaisAuPremierPas) {
    NodePtr parameter = scalarVariable(0.0f);
    parameter->grad.at({0}) = 1.0f;

    Adam adam(0.1f, 0.9f, 0.999f, 1e-8f);
    adam.step({parameter});

    EXPECT_NEAR(parameter->value.at({0}), -0.1f, 1e-4f);
}

/**
 * @brief Deux paramètres distincts optimisés par la même instance d'`Adam` partagent le même
 * compteur de pas (incrémenté une fois par `step`, pas une fois par paramètre).
 * \castest{<b>Adam : compteur de pas partagé.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux paramètres scalaires de même valeur et même gradient.<br/>2. Les
 * optimiser ensemble par la même instance d'`Adam`, sur trois `step`.<br/>
 * \tattendu Les deux paramètres restent égaux après chaque pas (même trajectoire, même compteur).}
 */
TEST(AdamTest, CompteurDePasPartage) {
    NodePtr a = scalarVariable(0.0f);
    NodePtr b = scalarVariable(0.0f);
    Adam adam(0.1f);

    for (int i = 0; i < 3; ++i) {
        a->grad.at({0}) = 1.0f;
        b->grad.at({0}) = 1.0f;
        adam.step({a, b});
        EXPECT_NEAR(a->value.at({0}), b->value.at({0}), TOLERANCE);
    }
}

/**
 * @brief Sur un gradient nul persistant (paramètre déjà à l'optimum), `Adam` ne produit ni `NaN`
 * ni `inf`, grâce au terme `epsilon` au dénominateur.
 * \castest{<b>Adam : absence de NaN/inf sur gradient nul persistant.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un paramètre scalaire de gradient toujours nul.<br/>2. Appliquer 50
 * `step` consécutifs (gradient remis à zéro entre chaque, déjà nul).<br/>
 * \tattendu `value` reste fini après chaque pas.}
 */
TEST(AdamTest, AbsenceDeNanSurGradientNulPersistant) {
    NodePtr parameter = scalarVariable(3.0f);
    Adam adam;

    for (int i = 0; i < 50; ++i) {
        adam.step({parameter});
        EXPECT_TRUE(std::isfinite(parameter->value.at({0})));
    }
}

/**
 * @brief `Adam` fait converger un paramètre scalaire vers la cible d'une quadratique convexe
 * simple, en un nombre d'itérations borné.
 * \castest{<b>Adam : convergence sur une quadratique simple.</b><br/>
 * \tcat Unitaire · Optim<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un paramètre scalaire `x = 0`, cible `5`.<br/>2. Répéter (calcul du
 * gradient `2(x-cible)`, `step`, `zeroGrad`) jusqu'à 500 itérations.<br/>
 * \tattendu `x` est à moins de `1e-2` de la cible.}
 */
TEST(AdamTest, ConvergeSurQuadratiqueSimple) {
    constexpr float target = 5.0f;
    NodePtr x = scalarVariable(0.0f);
    Adam adam(0.1f);

    for (int i = 0; i < 500; ++i) {
        x->grad.at({0}) = 2.0f * (x->value.at({0}) - target);
        adam.step({x});
        adam.zeroGrad({x});
    }

    EXPECT_NEAR(x->value.at({0}), target, 1e-2f);
}
