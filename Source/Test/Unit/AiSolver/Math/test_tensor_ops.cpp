/**
 * @file test_tensor_ops.cpp
 * @brief Tests unitaires des opérations élémentaires et réductions `aisolver::TensorOps`
 * (LOT-ANNEXE-01, TACHE-03).
 */

#include <stdexcept>

#include <gtest/gtest.h>

#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Math/TensorOps.h"
#include "Core/Diagnostics/Assert.h"

namespace {
constexpr float TOLERANCE = 1e-5f;

aisolver::Tensor<float> makeMatrix2x2(float a, float b, float c, float d) {
    aisolver::Tensor<float> tensor({2, 2});
    tensor.at({0, 0}) = a;
    tensor.at({0, 1}) = b;
    tensor.at({1, 0}) = c;
    tensor.at({1, 1}) = d;
    return tensor;
}
}  // namespace

/**
 * @brief Addition, soustraction, multiplication et division élément par élément produisent le
 * résultat attendu, calculé à la main, sur deux matrices `2x2`.
 * \castest{<b>TensorOps : opérations élémentaires entre deux tenseurs.</b><br/>
 * \tcat Unitaire · Opérations tensorielles<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `a = [[1,2],[3,4]]` et `b = [[10,20],[30,40]]`.<br/>2. Calculer `add`,
 * `subtract`, `multiply`, `divide`.<br/>
 * \tattendu Chaque case du résultat vaut l'opération appliquée aux cases correspondantes de `a`
 * et `b`.}
 */
TEST(TensorOpsTest, OperationsElementParElement) {
    const aisolver::Tensor<float> a = makeMatrix2x2(1.0f, 2.0f, 3.0f, 4.0f);
    const aisolver::Tensor<float> b = makeMatrix2x2(10.0f, 20.0f, 30.0f, 40.0f);

    const aisolver::Tensor<float> sumResult = aisolver::add(a, b);
    EXPECT_NEAR(sumResult.at({0, 0}), 11.0f, TOLERANCE);
    EXPECT_NEAR(sumResult.at({0, 1}), 22.0f, TOLERANCE);
    EXPECT_NEAR(sumResult.at({1, 0}), 33.0f, TOLERANCE);
    EXPECT_NEAR(sumResult.at({1, 1}), 44.0f, TOLERANCE);

    const aisolver::Tensor<float> diff = aisolver::subtract(b, a);
    EXPECT_NEAR(diff.at({0, 0}), 9.0f, TOLERANCE);
    EXPECT_NEAR(diff.at({1, 1}), 36.0f, TOLERANCE);

    const aisolver::Tensor<float> product = aisolver::multiply(a, b);
    EXPECT_NEAR(product.at({0, 0}), 10.0f, TOLERANCE);
    EXPECT_NEAR(product.at({1, 1}), 160.0f, TOLERANCE);

    const aisolver::Tensor<float> quotient = aisolver::divide(b, a);
    EXPECT_NEAR(quotient.at({0, 0}), 10.0f, TOLERANCE);
    EXPECT_NEAR(quotient.at({1, 1}), 10.0f, TOLERANCE);

    // Les opérateurs surchargés délèguent aux mêmes fonctions.
    EXPECT_NEAR((a + b).at({0, 0}), 11.0f, TOLERANCE);
    EXPECT_NEAR((a - b).at({0, 0}), -9.0f, TOLERANCE);
    EXPECT_NEAR((a * b).at({0, 0}), 10.0f, TOLERANCE);
    EXPECT_NEAR((b / a).at({0, 0}), 10.0f, TOLERANCE);
}

/**
 * @brief La diffusion scalaire applique le même scalaire à chaque élément du tenseur.
 * \castest{<b>TensorOps : diffusion scalaire.</b><br/>
 * \tcat Unitaire · Opérations tensorielles<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser un tenseur `[[1,2],[3,4]]`.<br/>2. Calculer `tensor + 1`, `tensor * 2`,
 * `tensor - 1`, `tensor / 2`.<br/>
 * \tattendu Chaque élément du résultat est l'opération appliquée à l'élément d'origine et au
 * scalaire.}
 */
TEST(TensorOpsTest, DiffusionScalaire) {
    const aisolver::Tensor<float> a = makeMatrix2x2(1.0f, 2.0f, 3.0f, 4.0f);

    const aisolver::Tensor<float> plusOne = a + 1.0f;
    EXPECT_NEAR(plusOne.at({0, 0}), 2.0f, TOLERANCE);
    EXPECT_NEAR(plusOne.at({1, 1}), 5.0f, TOLERANCE);

    const aisolver::Tensor<float> timesTwo = a * 2.0f;
    EXPECT_NEAR(timesTwo.at({0, 0}), 2.0f, TOLERANCE);
    EXPECT_NEAR(timesTwo.at({1, 1}), 8.0f, TOLERANCE);

    const aisolver::Tensor<float> minusOne = a - 1.0f;
    EXPECT_NEAR(minusOne.at({0, 0}), 0.0f, TOLERANCE);

    const aisolver::Tensor<float> halved = a / 2.0f;
    EXPECT_NEAR(halved.at({1, 1}), 2.0f, TOLERANCE);
}

/**
 * @brief `sum`, `mean` et `max` sur un tenseur de valeurs connues renvoient les résultats
 * attendus (réduction globale, sur tous les éléments).
 * \castest{<b>TensorOps : réductions `sum`/`mean`/`max`.</b><br/>
 * \tcat Unitaire · Opérations tensorielles<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser un tenseur `{1, 2, 3, 4}` (forme `2x2`).<br/>2. Calculer `sum`, `mean`,
 * `max`.<br/>
 * \tattendu `sum == 10`, `mean == 2.5`, `max == 4`.}
 */
TEST(TensorOpsTest, ReductionsSumMeanMax) {
    const aisolver::Tensor<float> a = makeMatrix2x2(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_NEAR(aisolver::sum(a), 10.0f, TOLERANCE);
    EXPECT_NEAR(aisolver::mean(a), 2.5f, TOLERANCE);
    EXPECT_NEAR(aisolver::max(a), 4.0f, TOLERANCE);
}

/**
 * @brief `mean` et `max` sur un tenseur vide (dimension de taille `0`) sont des erreurs de
 * programmation (division par zéro, maximum non défini) : `PROJECTGAMING_ASSERT`, pas un cas géré
 * silencieusement.
 * \castest{<b>TensorOps : assertion sur `mean`/`max` d'un tenseur vide.</b><br/>
 * \tcat Unitaire · Opérations tensorielles<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Poser un tenseur de forme `{0}`.<br/>2. Appeler `mean` puis `max`.<br/>
 * \tattendu Le gestionnaire d'assertion est invoqué dans les deux cas.}
 */
TEST(TensorOpsTest, AssertionReductionSurTenseurVide) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    const aisolver::Tensor<float> empty({0});
    ASSERT_EQ(empty.size(), 0u);

    core::setAssertionHandler([](const char*, const char*, const char*, int) {
        throw std::runtime_error("precondition");
    });

    EXPECT_THROW({ [[maybe_unused]] float v = aisolver::mean(empty); }, std::runtime_error);
    EXPECT_THROW({ [[maybe_unused]] float v = aisolver::max(empty); }, std::runtime_error);

    core::setAssertionHandler(nullptr);
#endif
}

/**
 * @brief Une opération élémentaire entre deux tenseurs de formes différentes déclenche
 * `PROJECTGAMING_ASSERT` — erreur de programmation, pas une diffusion générale.
 * \castest{<b>TensorOps : assertion sur formes incompatibles.</b><br/>
 * \tcat Unitaire · Opérations tensorielles<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `a` de forme `{2, 2}` et `b` de forme `{3}`.<br/>2. Appeler `add(a, b)`.<br/>
 * \tattendu Le gestionnaire d'assertion est invoqué avant tout accès aux données.}
 */
TEST(TensorOpsTest, AssertionFormesIncompatibles) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    const aisolver::Tensor<float> a({2, 2});
    const aisolver::Tensor<float> b({3});

    core::setAssertionHandler([](const char*, const char*, const char*, int) {
        throw std::runtime_error("precondition");
    });

    EXPECT_THROW(
        { [[maybe_unused]] aisolver::Tensor<float> result = aisolver::add(a, b); },
        std::runtime_error);

    core::setAssertionHandler(nullptr);
#endif
}

/**
 * @brief Une opération élémentaire appliquée à une vue reshape produit le même résultat (aux
 * positions correspondantes) qu'appliquée au tenseur d'origine.
 * \castest{<b>TensorOps : cohérence avec `Tensor::view`.</b><br/>
 * \tcat Unitaire · Opérations tensorielles<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Poser `a` de forme `{2, 2}`, `view = a.view({4})`.<br/>2. Calculer `a * 2` et
 * `view * 2`.<br/>
 * \tattendu Les valeurs correspondent position par position (ordre row-major identique).}
 */
TEST(TensorOpsTest, CoherenceAvecView) {
    const aisolver::Tensor<float> a = makeMatrix2x2(1.0f, 2.0f, 3.0f, 4.0f);
    const aisolver::Tensor<float> view = a.view({4});

    const aisolver::Tensor<float> scaledFromMatrix = a * 2.0f;
    const aisolver::Tensor<float> scaledFromView = view * 2.0f;

    EXPECT_NEAR(scaledFromMatrix.at({0, 0}), scaledFromView.at({0}), TOLERANCE);
    EXPECT_NEAR(scaledFromMatrix.at({0, 1}), scaledFromView.at({1}), TOLERANCE);
    EXPECT_NEAR(scaledFromMatrix.at({1, 0}), scaledFromView.at({2}), TOLERANCE);
    EXPECT_NEAR(scaledFromMatrix.at({1, 1}), scaledFromView.at({3}), TOLERANCE);
}
