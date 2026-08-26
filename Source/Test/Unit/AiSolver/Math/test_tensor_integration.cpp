// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tensor_integration.cpp
 * @brief Cas transverses combinant plusieurs briques de `AiSolver::Math` (LOT-ANNEXE-01,
 * TACHE-05) : non-régression sur des identités mathématiques connues, et cas limites qui ne
 * trouvent naturellement leur place dans aucun fichier isolé (`test_rng.cpp`, `test_tensor.cpp`,
 * `test_tensor_ops.cpp`, `test_matmul.cpp`).
 */

#include <cmath>
#include <limits>

#include <gtest/gtest.h>

#include "AiSolver/Math/Matmul.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Math/TensorOps.h"

namespace {
constexpr float TOLERANCE = 1e-5f;
}

/**
 * @brief Le produit d'une matrice `2x2` inversible par son inverse connu (calculé à la main)
 * reproduit l'identité, à `1e-5` près.
 * \castest{<b>Intégration tenseurs : identité mathématique connue (matrice inverse).</b><br/>
 * \tcat Unitaire · Intégration tensorielle<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `A = [[4,7],[2,6]]` (déterminant `10`) et son inverse connu
 * `Ainv = [[0.6,-0.7],[-0.2,0.4]]`.<br/>2. Calculer `matmul(A, Ainv)`.<br/>
 * \tattendu Le résultat est la matrice identité `2x2`, à `1e-5` près.}
 */
TEST(TensorIntegrationTest, MatmulParInverseConnuReproduitIdentite) {
    aisolver::Tensor<float> a({2, 2});
    a.at({0, 0}) = 4.0f;
    a.at({0, 1}) = 7.0f;
    a.at({1, 0}) = 2.0f;
    a.at({1, 1}) = 6.0f;

    // Inverse calculé à la main : det(A) = 4*6 - 7*2 = 10 ;
    // Ainv = (1/10) * [[6,-7],[-2,4]] = [[0.6,-0.7],[-0.2,0.4]].
    aisolver::Tensor<float> aInverse({2, 2});
    aInverse.at({0, 0}) = 0.6f;
    aInverse.at({0, 1}) = -0.7f;
    aInverse.at({1, 0}) = -0.2f;
    aInverse.at({1, 1}) = 0.4f;

    const aisolver::Tensor<float> result = aisolver::matmul(a, aInverse);

    EXPECT_NEAR(result.at({0, 0}), 1.0f, TOLERANCE);
    EXPECT_NEAR(result.at({0, 1}), 0.0f, TOLERANCE);
    EXPECT_NEAR(result.at({1, 0}), 0.0f, TOLERANCE);
    EXPECT_NEAR(result.at({1, 1}), 1.0f, TOLERANCE);
}

/**
 * @brief `matmul` est associatif : `matmul(matmul(a, b), c) == matmul(a, matmul(b, c))` sur trois
 * petites matrices compatibles fixées à la main.
 * \castest{<b>Intégration tenseurs : produits croisés en chaîne (associativité).</b><br/>
 * \tcat Unitaire · Intégration tensorielle<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `a` (2x3), `b` (3x2), `c` (2x2) de valeurs fixées.<br/>2. Calculer
 * `matmul(matmul(a,b),c)` et `matmul(a,matmul(b,c))`.<br/>
 * \tattendu Les deux résultats sont égaux (mêmes valeurs, même forme).}
 */
TEST(TensorIntegrationTest, MatmulEstAssociatif) {
    aisolver::Tensor<float> a({2, 3});
    a.at({0, 0}) = 1.0f;
    a.at({0, 1}) = 2.0f;
    a.at({0, 2}) = 3.0f;
    a.at({1, 0}) = 4.0f;
    a.at({1, 1}) = 5.0f;
    a.at({1, 2}) = 6.0f;

    aisolver::Tensor<float> b({3, 2});
    b.at({0, 0}) = 1.0f;
    b.at({0, 1}) = 0.0f;
    b.at({1, 0}) = 0.0f;
    b.at({1, 1}) = 1.0f;
    b.at({2, 0}) = 1.0f;
    b.at({2, 1}) = 1.0f;

    aisolver::Tensor<float> c({2, 2});
    c.at({0, 0}) = 2.0f;
    c.at({0, 1}) = 0.0f;
    c.at({1, 0}) = 1.0f;
    c.at({1, 1}) = 3.0f;

    const aisolver::Tensor<float> left = aisolver::matmul(aisolver::matmul(a, b), c);
    const aisolver::Tensor<float> right = aisolver::matmul(a, aisolver::matmul(b, c));

    ASSERT_EQ(left.shape(), right.shape());
    for (std::size_t i = 0; i < left.shape()[0]; ++i) {
        for (std::size_t j = 0; j < left.shape()[1]; ++j) {
            EXPECT_NEAR(left.at({i, j}), right.at({i, j}), TOLERANCE);
        }
    }
}

/**
 * @brief Une vue (`view`), une opération élémentaire (`TensorOps`) puis une réduction (`sum`)
 * restent cohérentes utilisées ensemble, pas seulement isolément.
 * \castest{<b>Intégration tenseurs : vue + opération élémentaire + réduction.</b><br/>
 * \tcat Unitaire · Intégration tensorielle<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un tenseur `2x3` de valeurs `1..6`.<br/>2. Prendre une vue `{6}`.<br/>
 * 3. Multiplier la vue par `2`.<br/>4. Réduire par `sum`.<br/>
 * \tattendu `sum == 2 * (1+2+3+4+5+6) == 42`.}
 */
TEST(TensorIntegrationTest, VuePuisOperationPuisReduction) {
    aisolver::Tensor<float> tensor({2, 3});
    tensor.at({0, 0}) = 1.0f;
    tensor.at({0, 1}) = 2.0f;
    tensor.at({0, 2}) = 3.0f;
    tensor.at({1, 0}) = 4.0f;
    tensor.at({1, 1}) = 5.0f;
    tensor.at({1, 2}) = 6.0f;

    const aisolver::Tensor<float> flat = tensor.view({6});
    const aisolver::Tensor<float> doubled = aisolver::multiplyScalar(flat, 2.0f);

    EXPECT_NEAR(aisolver::sum(doubled), 42.0f, TOLERANCE);
}

/**
 * @brief Un tenseur à un seul élément (forme `{1}`) se construit, s'indexe et se réduit
 * correctement — cas limite le plus petit possible pour une forme non vide.
 * \castest{<b>Intégration tenseurs : cas limite, tenseur à un seul élément.</b><br/>
 * \tcat Unitaire · Intégration tensorielle<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire `Tensor<float>({1})`.<br/>2. Écrire une valeur, lire `sum`/`mean`/
 * `max`.<br/>
 * \tattendu `sum == mean == max == valeur écrite`.}
 */
TEST(TensorIntegrationTest, TenseurAUnSeulElement) {
    aisolver::Tensor<float> tensor({1});
    tensor.at({0}) = 5.0f;

    EXPECT_NEAR(aisolver::sum(tensor), 5.0f, TOLERANCE);
    EXPECT_NEAR(aisolver::mean(tensor), 5.0f, TOLERANCE);
    EXPECT_NEAR(aisolver::max(tensor), 5.0f, TOLERANCE);
}

/**
 * @brief Une dimension de taille `0` est une forme valide (aucune assertion à la construction) :
 * le tenseur résultant a une taille nulle et zéro élément accessible.
 * \castest{<b>Intégration tenseurs : cas limite, dimension de taille zéro.</b><br/>
 * \tcat Unitaire · Intégration tensorielle<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire `Tensor<float>({2, 0})`.<br/>2. Lire `rank()`, `shape()`,
 * `size()`.<br/>
 * \tattendu `rank() == 2`, `shape() == {2, 0}`, `size() == 0`, aucun crash.}
 */
TEST(TensorIntegrationTest, DimensionDeTailleZeroEstUneFormeValide) {
    const aisolver::Tensor<float> tensor({2, 0});

    EXPECT_EQ(tensor.rank(), 2u);
    EXPECT_EQ(tensor.shape(), (std::vector<std::size_t>{2, 0}));
    EXPECT_EQ(tensor.size(), 0u);
}

/**
 * @brief Les opérations élémentaires et réductions restent finies (ni `NaN` ni `inf`
 * inattendus) sur des valeurs proches des bornes représentables d'un `float`.
 * \castest{<b>Intégration tenseurs : cas limite, valeurs flottantes extrêmes.</b><br/>
 * \tcat Unitaire · Intégration tensorielle<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Poser un tenseur contenant `std::numeric_limits<float>::max()` et un autre
 * contenant `std::numeric_limits<float>::min()`.<br/>2. Appliquer une diffusion scalaire modérée
 * et `sum`/`max`.<br/>
 * \tattendu Tous les résultats restent finis (`std::isfinite`), sans `NaN` inattendu.}
 */
TEST(TensorIntegrationTest, ValeursFlottantesExtremesRestentFinies) {
    aisolver::Tensor<float> nearMax({2});
    nearMax.at({0}) = std::numeric_limits<float>::max();
    nearMax.at({1}) = std::numeric_limits<float>::max() / 2.0f;

    const aisolver::Tensor<float> halved = aisolver::multiplyScalar(nearMax, 0.5f);
    EXPECT_TRUE(std::isfinite(halved.at({0})));
    EXPECT_TRUE(std::isfinite(halved.at({1})));
    EXPECT_TRUE(std::isfinite(aisolver::max(nearMax)));

    aisolver::Tensor<float> nearMin({2});
    nearMin.at({0}) = std::numeric_limits<float>::min();
    nearMin.at({1}) = std::numeric_limits<float>::min() * 2.0f;

    const aisolver::Tensor<float> shifted = aisolver::addScalar(nearMin, 1.0f);
    EXPECT_TRUE(std::isfinite(shifted.at({0})));
    EXPECT_TRUE(std::isfinite(shifted.at({1})));
    EXPECT_TRUE(std::isfinite(aisolver::sum(nearMin)));
    EXPECT_FALSE(std::isnan(aisolver::sum(nearMin)));
}
