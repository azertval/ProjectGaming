/**
 * @file test_matmul.cpp
 * @brief Tests unitaires de `aisolver::matmul`/`aisolver::transpose` (LOT-ANNEXE-01, TACHE-04).
 */

#include <stdexcept>

#include <gtest/gtest.h>

#include "AiSolver/Math/Matmul.h"
#include "AiSolver/Math/Tensor.h"
#include "Core/Diagnostics/Assert.h"

namespace {
constexpr float TOLERANCE = 1e-5f;

aisolver::Tensor<float> identity(std::size_t n) {
    aisolver::Tensor<float> result({n, n});
    for (std::size_t i = 0; i < n; ++i) {
        result.at({i, i}) = 1.0f;
    }
    return result;
}
}  // namespace

/**
 * @brief Le produit d'une matrice quelconque par la matrice identité renvoie la matrice
 * d'origine, inchangée.
 * \castest{<b>Matmul : produit par l'identité.</b><br/>
 * \tcat Unitaire · Produit matriciel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `a` de forme `{2, 3}` de valeurs quelconques.<br/>2. Calculer
 * `matmul(a, identite(3))`.<br/>
 * \tattendu Le résultat est égal à `a`, position par position.}
 */
TEST(MatmulTest, ProduitParIdentite) {
    aisolver::Tensor<float> a({2, 3});
    a.at({0, 0}) = 1.0f;
    a.at({0, 1}) = 2.0f;
    a.at({0, 2}) = 3.0f;
    a.at({1, 0}) = 4.0f;
    a.at({1, 1}) = 5.0f;
    a.at({1, 2}) = 6.0f;

    const aisolver::Tensor<float> result = aisolver::matmul(a, identity(3));

    EXPECT_EQ(result.shape(), (std::vector<std::size_t>{2, 3}));
    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 3; ++j) {
            EXPECT_NEAR(result.at({i, j}), a.at({i, j}), TOLERANCE);
        }
    }
}

/**
 * @brief Le produit matriciel de deux matrices `2x2` fixées à la main correspond au calcul de
 * référence posé en commentaire.
 * \castest{<b>Matmul : produit croisé connu.</b><br/>
 * \tcat Unitaire · Produit matriciel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `A = [[1,2],[3,4]]`, `B = [[5,6],[7,8]]`.<br/>2. Calculer `matmul(A, B)`.<br/>
 * \tattendu `C = [[19,22],[43,50]]` (calcul détaillé en commentaire).}
 */
TEST(MatmulTest, ProduitCroiseConnu) {
    aisolver::Tensor<float> a({2, 2});
    a.at({0, 0}) = 1.0f;
    a.at({0, 1}) = 2.0f;
    a.at({1, 0}) = 3.0f;
    a.at({1, 1}) = 4.0f;

    aisolver::Tensor<float> b({2, 2});
    b.at({0, 0}) = 5.0f;
    b.at({0, 1}) = 6.0f;
    b.at({1, 0}) = 7.0f;
    b.at({1, 1}) = 8.0f;

    const aisolver::Tensor<float> c = aisolver::matmul(a, b);

    // C[0][0] = 1*5 + 2*7 = 19    C[0][1] = 1*6 + 2*8 = 22
    // C[1][0] = 3*5 + 4*7 = 43    C[1][1] = 3*6 + 4*8 = 50
    EXPECT_NEAR(c.at({0, 0}), 19.0f, TOLERANCE);
    EXPECT_NEAR(c.at({0, 1}), 22.0f, TOLERANCE);
    EXPECT_NEAR(c.at({1, 0}), 43.0f, TOLERANCE);
    EXPECT_NEAR(c.at({1, 1}), 50.0f, TOLERANCE);
}

/**
 * @brief `matmul` entre deux matrices dont la dimension interne diffère (`3 != 4`) déclenche
 * `PROJECTGAMING_ASSERT`.
 * \castest{<b>Matmul : assertion sur dimensions incompatibles.</b><br/>
 * \tcat Unitaire · Produit matriciel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `a` de forme `{2, 3}`, `b` de forme `{4, 5}`.<br/>2. Appeler
 * `matmul(a, b)`.<br/>
 * \tattendu Le gestionnaire d'assertion est invoqué avant tout calcul.}
 */
TEST(MatmulTest, AssertionDimensionsIncompatibles) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    const aisolver::Tensor<float> a({2, 3});
    const aisolver::Tensor<float> b({4, 5});

    core::setAssertionHandler(
        [](const char*, const char*, const char*, int) { throw std::runtime_error("precondition"); });

    EXPECT_THROW({ [[maybe_unused]] aisolver::Tensor<float> result = aisolver::matmul(a, b); }, std::runtime_error);

    core::setAssertionHandler(nullptr);
#endif
}

/**
 * @brief `matmul`/`transpose` appelés sur un tenseur de rang `1` ou `3` déclenchent
 * `PROJECTGAMING_ASSERT` (restreints aux matrices, rang 2).
 * \castest{<b>Matmul : assertion sur rang invalide.</b><br/>
 * \tcat Unitaire · Produit matriciel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser un vecteur (rang 1) et un tenseur 3D (rang 3).<br/>2. Appeler `matmul` et
 * `transpose` sur chacun.<br/>
 * \tattendu Le gestionnaire d'assertion est invoqué à chaque appel.}
 */
TEST(MatmulTest, AssertionRangInvalide) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    const aisolver::Tensor<float> vector({3});
    const aisolver::Tensor<float> cube({2, 2, 2});
    const aisolver::Tensor<float> matrix({2, 2});

    core::setAssertionHandler(
        [](const char*, const char*, const char*, int) { throw std::runtime_error("precondition"); });

    EXPECT_THROW({ [[maybe_unused]] auto r = aisolver::matmul(vector, matrix); }, std::runtime_error);
    EXPECT_THROW({ [[maybe_unused]] auto r = aisolver::matmul(matrix, cube); }, std::runtime_error);
    EXPECT_THROW({ [[maybe_unused]] auto r = aisolver::transpose(vector); }, std::runtime_error);
    EXPECT_THROW({ [[maybe_unused]] auto r = aisolver::transpose(cube); }, std::runtime_error);

    core::setAssertionHandler(nullptr);
#endif
}

/**
 * @brief Transposer deux fois une matrice renvoie une matrice égale à l'originale (mêmes valeurs,
 * même forme).
 * \castest{<b>Matmul : `transpose` est involutive.</b><br/>
 * \tcat Unitaire · Produit matriciel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `a` de forme `{2, 3}`.<br/>2. Calculer `transpose(transpose(a))`.<br/>
 * \tattendu Même forme et mêmes valeurs que `a`.}
 */
TEST(MatmulTest, TransposeEstInvolutive) {
    aisolver::Tensor<float> a({2, 3});
    a.at({0, 0}) = 1.0f;
    a.at({0, 1}) = 2.0f;
    a.at({0, 2}) = 3.0f;
    a.at({1, 0}) = 4.0f;
    a.at({1, 1}) = 5.0f;
    a.at({1, 2}) = 6.0f;

    const aisolver::Tensor<float> doubleTransposed = aisolver::transpose(aisolver::transpose(a));

    EXPECT_EQ(doubleTransposed.shape(), a.shape());
    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 3; ++j) {
            EXPECT_NEAR(doubleTransposed.at({i, j}), a.at({i, j}), TOLERANCE);
        }
    }
}

/**
 * @brief `matmul(a, transpose(a))` sur une matrice non carrée produit une matrice carrée
 * symétrique de la bonne forme, valeurs vérifiées sur un cas fixé à la main.
 * \castest{<b>Matmul : `transpose` puis `matmul`.</b><br/>
 * \tcat Unitaire · Produit matriciel<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Poser `a` de forme `{2, 3}`.<br/>2. Calculer `matmul(a, transpose(a))`.<br/>
 * \tattendu Résultat de forme `{2, 2}`, symétrique, valeurs correctes.}
 */
TEST(MatmulTest, TransposePuisMatmul) {
    aisolver::Tensor<float> a({2, 3});
    a.at({0, 0}) = 1.0f;
    a.at({0, 1}) = 2.0f;
    a.at({0, 2}) = 3.0f;
    a.at({1, 0}) = 4.0f;
    a.at({1, 1}) = 5.0f;
    a.at({1, 2}) = 6.0f;

    const aisolver::Tensor<float> result = aisolver::matmul(a, aisolver::transpose(a));

    EXPECT_EQ(result.shape(), (std::vector<std::size_t>{2, 2}));
    // ligne 0 . ligne 0 = 1+4+9 = 14   ligne 0 . ligne 1 = 4+10+18 = 32
    // ligne 1 . ligne 1 = 16+25+36 = 77
    EXPECT_NEAR(result.at({0, 0}), 14.0f, TOLERANCE);
    EXPECT_NEAR(result.at({0, 1}), 32.0f, TOLERANCE);
    EXPECT_NEAR(result.at({1, 0}), 32.0f, TOLERANCE);
    EXPECT_NEAR(result.at({1, 1}), 77.0f, TOLERANCE);
}
