/**
 * @file test_autodiff_ops.cpp
 * @brief Tests unitaires des opérations différentiables de base `aisolver::autodiff`
 * (LOT-ANNEXE-02, TACHE-02) et complémentaires (LOT-ANNEXE-03, TACHE-05) : passe avant et
 * composition, hors correction du gradient (`test_autodiff_gradient_checking.cpp`).
 */

#include <cmath>
#include <limits>
#include <stdexcept>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/Activations.h"
#include "Core/Diagnostics/Assert.h"

using aisolver::Tensor;
using aisolver::autodiff::NodePtr;
using aisolver::autodiff::variable;

namespace {
constexpr float TOLERANCE = 1e-5f;
}  // namespace

/**
 * @brief `add()` calcule la somme élément par élément en passe avant.
 * \castest{<b>Ops : `add()` passe avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux feuilles `{1, 2}` et `{5, 6}`.<br/>2. Appeler `add()`.<br/>
 * \tattendu `value` du résultat vaut `{6, 8}`.}
 */
TEST(AutodiffOpsTest, AddPasseAvant) {
    Tensor<float> dataA({2});
    dataA.at({0}) = 1.0f;
    dataA.at({1}) = 2.0f;
    Tensor<float> dataB({2});
    dataB.at({0}) = 5.0f;
    dataB.at({1}) = 6.0f;

    const NodePtr result = aisolver::autodiff::add(variable(dataA), variable(dataB));

    EXPECT_NEAR(result->value.at({0}), 6.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1}), 8.0f, TOLERANCE);
}

/**
 * @brief `multiply()` calcule le produit élément par élément en passe avant.
 * \castest{<b>Ops : `multiply()` passe avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux feuilles `{2, 3}` et `{4, 5}`.<br/>2. Appeler `multiply()`.<br/>
 * \tattendu `value` du résultat vaut `{8, 15}`.}
 */
TEST(AutodiffOpsTest, MultiplyPasseAvant) {
    Tensor<float> dataA({2});
    dataA.at({0}) = 2.0f;
    dataA.at({1}) = 3.0f;
    Tensor<float> dataB({2});
    dataB.at({0}) = 4.0f;
    dataB.at({1}) = 5.0f;

    const NodePtr result = aisolver::autodiff::multiply(variable(dataA), variable(dataB));

    EXPECT_NEAR(result->value.at({0}), 8.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1}), 15.0f, TOLERANCE);
}

/**
 * @brief `matmul()` calcule le produit matriciel en passe avant, sur un cas fixé à la main.
 * \castest{<b>Ops : `matmul()` passe avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `A = [[1,2],[3,4]]`, `B = [[5,6],[7,8]]`.<br/>2. Appeler `matmul()`.<br/>
 * \tattendu `value` du résultat vaut `[[19,22],[43,50]]`.}
 */
TEST(AutodiffOpsTest, MatmulPasseAvant) {
    Tensor<float> dataA({2, 2});
    dataA.at({0, 0}) = 1.0f;
    dataA.at({0, 1}) = 2.0f;
    dataA.at({1, 0}) = 3.0f;
    dataA.at({1, 1}) = 4.0f;
    Tensor<float> dataB({2, 2});
    dataB.at({0, 0}) = 5.0f;
    dataB.at({0, 1}) = 6.0f;
    dataB.at({1, 0}) = 7.0f;
    dataB.at({1, 1}) = 8.0f;

    const NodePtr result = aisolver::autodiff::matmul(variable(dataA), variable(dataB));

    EXPECT_EQ(result->value.shape(), (std::vector<std::size_t>{2, 2}));
    EXPECT_NEAR(result->value.at({0, 0}), 19.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({0, 1}), 22.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1, 0}), 43.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1, 1}), 50.0f, TOLERANCE);
}

/**
 * @brief `relu()` ramène à zéro les éléments négatifs et nuls, laisse inchangés les éléments
 * strictement positifs.
 * \castest{<b>Ops : `relu()` zéro sous le seuil.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `{-2, 0, 3}`.<br/>2. Appeler `relu()`.<br/>
 * \tattendu `value` du résultat vaut `{0, 0, 3}`.}
 */
TEST(AutodiffOpsTest, ReluZeroSousLeSeuil) {
    Tensor<float> data({3});
    data.at({0}) = -2.0f;
    data.at({1}) = 0.0f;
    data.at({2}) = 3.0f;

    const NodePtr result = aisolver::autodiff::relu(variable(data));

    EXPECT_NEAR(result->value.at({0}), 0.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1}), 0.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({2}), 3.0f, TOLERANCE);
}

/**
 * @brief `tanhOp()` reste strictement bornée dans `]-1, 1[`, sans `NaN`/`inf`, même pour des
 * entrées de grande amplitude.
 * \castest{<b>Ops : `tanhOp()` bornée.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `{-6, 6}` (amplitude assez grande pour approcher la
 * saturation sans y arriver exactement en précision `float32` : au-delà, `1 - tanh(x)` devient
 * inférieur à l'epsilon `float` et arrondit à `1.0f` pile, ce qui n'est pas un défaut de
 * `tanhOp`).<br/>2. Appeler `tanhOp()`.<br/>
 * \tattendu Les deux sorties sont finies et strictement dans `]-1, 1[`.}
 */
TEST(AutodiffOpsTest, TanhOpBornee) {
    Tensor<float> data({2});
    data.at({0}) = -6.0f;
    data.at({1}) = 6.0f;

    const NodePtr result = aisolver::autodiff::tanhOp(variable(data));

    for (std::size_t i = 0; i < 2; ++i) {
        const float value = result->value.at({i});
        EXPECT_TRUE(std::isfinite(value));
        EXPECT_GT(value, -1.0f);
        EXPECT_LT(value, 1.0f);
    }
}

/**
 * @brief Une composition `matmul` puis `add` (biais) puis `relu` — motif d'une couche dense —
 * calcule la bonne valeur en avant, sans appel à `backward()`.
 * \castest{<b>Ops : composition `matmul` + `add` + `relu`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser `x = [[1,2]]` (1x2), `w = [[1,-1],[1,1]]` (2x2), `b = [[-3,0]]` (1x2).<br/>
 * 2. Calculer `relu(add(matmul(x, w), b))`.<br/>
 * \tattendu `matmul(x,w) = [[3,1]]`, `+b = [[0,1]]`, `relu = [[0,1]]`.}
 */
TEST(AutodiffOpsTest, CompositionMatmulAddRelu) {
    Tensor<float> x({1, 2});
    x.at({0, 0}) = 1.0f;
    x.at({0, 1}) = 2.0f;

    Tensor<float> w({2, 2});
    w.at({0, 0}) = 1.0f;
    w.at({0, 1}) = -1.0f;
    w.at({1, 0}) = 1.0f;
    w.at({1, 1}) = 1.0f;

    Tensor<float> b({1, 2});
    b.at({0, 0}) = -3.0f;
    b.at({0, 1}) = 0.0f;

    const NodePtr xNode = variable(x);
    const NodePtr wNode = variable(w);
    const NodePtr bNode = variable(b);

    const NodePtr product = aisolver::autodiff::matmul(xNode, wNode);
    const NodePtr biased = aisolver::autodiff::add(product, bNode);
    const NodePtr activated = aisolver::autodiff::relu(biased);

    // matmul(x, w) = [1*1+2*1, 1*(-1)+2*1] = [3, 1] ; + b = [0, 1] ; relu([0, 1]) = [0, 1].
    EXPECT_NEAR(activated->value.at({0, 0}), 0.0f, TOLERANCE);
    EXPECT_NEAR(activated->value.at({0, 1}), 1.0f, TOLERANCE);
}

// --- Operations complementaires (LOT-ANNEXE-03, TACHE-05) ---------------------------------------

/**
 * @brief `subtract()` calcule la différence élément par élément en passe avant.
 * \castest{<b>Ops : `subtract()` passe avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux feuilles `{5, 6}` et `{1, 2}`.<br/>2. Appeler `subtract()`.<br/>
 * \tattendu `value` du résultat vaut `{4, 4}`.}
 */
TEST(AutodiffOpsTest, SubtractPasseAvant) {
    Tensor<float> dataA({2});
    dataA.at({0}) = 5.0f;
    dataA.at({1}) = 6.0f;
    Tensor<float> dataB({2});
    dataB.at({0}) = 1.0f;
    dataB.at({1}) = 2.0f;

    const NodePtr result = aisolver::autodiff::subtract(variable(dataA), variable(dataB));

    EXPECT_NEAR(result->value.at({0}), 4.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1}), 4.0f, TOLERANCE);
}

/**
 * @brief `divide()` calcule le quotient élément par élément en passe avant.
 * \castest{<b>Ops : `divide()` passe avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux feuilles `{8, 9}` et `{2, 3}`.<br/>2. Appeler `divide()`.<br/>
 * \tattendu `value` du résultat vaut `{4, 3}`.}
 */
TEST(AutodiffOpsTest, DividePasseAvant) {
    Tensor<float> dataA({2});
    dataA.at({0}) = 8.0f;
    dataA.at({1}) = 9.0f;
    Tensor<float> dataB({2});
    dataB.at({0}) = 2.0f;
    dataB.at({1}) = 3.0f;

    const NodePtr result = aisolver::autodiff::divide(variable(dataA), variable(dataB));

    EXPECT_NEAR(result->value.at({0}), 4.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1}), 3.0f, TOLERANCE);
}

/**
 * @brief `divide` déclenche l'assertion sur un diviseur exactement nul.
 * \castest{<b>Ops : `divide()` refuse un diviseur nul.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire `a = {1}`, `b = {0}`.<br/>2. Appeler `divide(a, b)`.<br/>
 * \tattendu Le gestionnaire d'assertion est invoqué avant tout calcul.}
 */
TEST(AutodiffOpsTest, DivideRefuseDiviseurNul) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    Tensor<float> dataA({1});
    dataA.at({0}) = 1.0f;
    Tensor<float> dataB({1});
    dataB.at({0}) = 0.0f;

    core::setAssertionHandler(
        [](const char*, const char*, const char*, int) { throw std::runtime_error("precondition"); });

    EXPECT_THROW(
        { [[maybe_unused]] NodePtr result = aisolver::autodiff::divide(variable(dataA), variable(dataB)); },
        std::runtime_error);

    core::setAssertionHandler(nullptr);
#endif
}

/**
 * @brief `divide` reste fini (aucun `NaN`/`inf`) pour un diviseur petit mais non nul.
 * \castest{<b>Ops : `divide()` stable sur un diviseur petit.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire `a = {1}`, `b = {1e-6}`.<br/>2. Appeler `divide(a, b)`.<br/>
 * \tattendu Le résultat est fini.}
 */
TEST(AutodiffOpsTest, DivideStableSurDiviseurPetit) {
    Tensor<float> dataA({1});
    dataA.at({0}) = 1.0f;
    Tensor<float> dataB({1});
    dataB.at({0}) = 1e-6f;

    const NodePtr result = aisolver::autodiff::divide(variable(dataA), variable(dataB));

    EXPECT_TRUE(std::isfinite(result->value.at({0})));
}

/**
 * @brief `addScalar()`/`multiplyScalar()` appliquent le scalaire à chaque élément en passe avant.
 * \castest{<b>Ops : `addScalar()`/`multiplyScalar()` passe avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `{2, 3}`.<br/>2. Appeler `addScalar(..., 10)` puis
 * `multiplyScalar(..., 2)`.<br/>
 * \tattendu `addScalar` vaut `{12, 13}`, `multiplyScalar` vaut `{4, 6}`.}
 */
TEST(AutodiffOpsTest, AddScalarEtMultiplyScalarPasseAvant) {
    Tensor<float> data({2});
    data.at({0}) = 2.0f;
    data.at({1}) = 3.0f;
    const NodePtr leaf = variable(data);

    const NodePtr added = aisolver::autodiff::addScalar(leaf, 10.0f);
    const NodePtr multiplied = aisolver::autodiff::multiplyScalar(leaf, 2.0f);

    EXPECT_NEAR(added->value.at({0}), 12.0f, TOLERANCE);
    EXPECT_NEAR(added->value.at({1}), 13.0f, TOLERANCE);
    EXPECT_NEAR(multiplied->value.at({0}), 4.0f, TOLERANCE);
    EXPECT_NEAR(multiplied->value.at({1}), 6.0f, TOLERANCE);
}

/**
 * @brief `logOp(expOp(a))` reproduit `a` à `1e-5` près en avant (composition de fonctions
 * réciproques).
 * \castest{<b>Ops : `logOp`/`expOp` réciproques en avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `{-2, 0, 3}`.<br/>2. Appeler `logOp(expOp(a))`.<br/>
 * \tattendu Le résultat reproduit `{-2, 0, 3}` à `1e-5` près.}
 */
TEST(AutodiffOpsTest, LogExpReciproquesEnAvant) {
    Tensor<float> data({3});
    data.at({0}) = -2.0f;
    data.at({1}) = 0.0f;
    data.at({2}) = 3.0f;

    const NodePtr result = aisolver::autodiff::logOp(aisolver::autodiff::expOp(variable(data)));

    EXPECT_NEAR(result->value.at({0}), -2.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1}), 0.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({2}), 3.0f, TOLERANCE);
}

/**
 * @brief `logOp` déclenche l'assertion sur un élément d'entrée non strictement positif.
 * \castest{<b>Ops : `logOp` refuse une entrée non positive.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille contenant `0.0f`.<br/>2. Appeler `logOp()`.<br/>
 * \tattendu Le gestionnaire d'assertion est invoqué avant tout calcul.}
 */
TEST(AutodiffOpsTest, LogOpRefuseEntreeNonPositive) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    Tensor<float> data({1});
    data.at({0}) = 0.0f;

    core::setAssertionHandler(
        [](const char*, const char*, const char*, int) { throw std::runtime_error("precondition"); });

    EXPECT_THROW({ [[maybe_unused]] NodePtr result = aisolver::autodiff::logOp(variable(data)); }, std::runtime_error);

    core::setAssertionHandler(nullptr);
#endif
}

/**
 * @brief `selectIndex()` extrait l'élément demandé dans un nœud de forme `[1]`.
 * \castest{<b>Ops : `selectIndex()` passe avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `{10, 20, 30}`.<br/>2. Appeler `selectIndex(a, 1)`.<br/>
 * \tattendu `value` du résultat a pour forme `[1]` et vaut `20`.}
 */
TEST(AutodiffOpsTest, SelectIndexPasseAvant) {
    Tensor<float> data({3});
    data.at({0}) = 10.0f;
    data.at({1}) = 20.0f;
    data.at({2}) = 30.0f;

    const NodePtr result = aisolver::autodiff::selectIndex(variable(data), 1);

    EXPECT_EQ(result->value.shape(), (std::vector<std::size_t>{1}));
    EXPECT_NEAR(result->value.at({0}), 20.0f, TOLERANCE);
}

/**
 * @brief `backward()` sur `selectIndex(a, 2)` accumule le gradient de sortie exactement à
 * l'indice `2` de `a`, et `0.0f` partout ailleurs.
 * \castest{<b>Ops : `selectIndex()` gradient localisé.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une feuille `{10, 20, 30}`.<br/>2. Appeler `backward()` sur
 * `selectIndex(a, 2)`.<br/>
 * \tattendu `a->grad` vaut `{0, 0, 1}`.}
 */
TEST(AutodiffOpsTest, SelectIndexGradientLocalise) {
    Tensor<float> data({3});
    data.at({0}) = 10.0f;
    data.at({1}) = 20.0f;
    data.at({2}) = 30.0f;
    const NodePtr leaf = variable(data);

    aisolver::autodiff::backward(aisolver::autodiff::selectIndex(leaf, 2));

    EXPECT_NEAR(leaf->grad.at({0}), 0.0f, TOLERANCE);
    EXPECT_NEAR(leaf->grad.at({1}), 0.0f, TOLERANCE);
    EXPECT_NEAR(leaf->grad.at({2}), 1.0f, TOLERANCE);
}

/**
 * @brief `minimum()` sélectionne, élément par élément, la plus petite des deux valeurs.
 * \castest{<b>Ops : `minimum()` passe avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux feuilles `{1, 5}` et `{3, 2}`.<br/>2. Appeler `minimum()`.<br/>
 * \tattendu `value` du résultat vaut `{1, 2}`.}
 */
TEST(AutodiffOpsTest, MinimumPasseAvant) {
    Tensor<float> dataA({2});
    dataA.at({0}) = 1.0f;
    dataA.at({1}) = 5.0f;
    Tensor<float> dataB({2});
    dataB.at({0}) = 3.0f;
    dataB.at({1}) = 2.0f;

    const NodePtr result = aisolver::autodiff::minimum(variable(dataA), variable(dataB));

    EXPECT_NEAR(result->value.at({0}), 1.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1}), 2.0f, TOLERANCE);
}

/**
 * @brief `backward()` sur `minimum()` ne fait remonter le gradient qu'à l'opérande effectivement
 * minimal pour chaque élément.
 * \castest{<b>Ops : `minimum()` sélection du bon opérande au backward.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire deux feuilles `{1, 5}` et `{3, 2}` (a minimal au premier élément, b
 * minimal au second).<br/>2. Réduire `minimum()` à un scalaire (somme) et appeler
 * `backward()`.<br/>
 * \tattendu `a->grad = {1, 0}`, `b->grad = {0, 1}`.}
 */
TEST(AutodiffOpsTest, MinimumSelectionDuBonOperandeAuBackward) {
    Tensor<float> dataA({2});
    dataA.at({0}) = 1.0f;
    dataA.at({1}) = 5.0f;
    Tensor<float> dataB({2});
    dataB.at({0}) = 3.0f;
    dataB.at({1}) = 2.0f;
    const NodePtr leafA = variable(dataA);
    const NodePtr leafB = variable(dataB);

    const NodePtr result = aisolver::autodiff::minimum(leafA, leafB);
    // Reduit [2] a un scalaire par somme des deux elements (pas de sum() dedie dans autodiff).
    const NodePtr scalarLoss = aisolver::autodiff::add(
        aisolver::autodiff::selectIndex(result, 0), aisolver::autodiff::selectIndex(result, 1));

    aisolver::autodiff::backward(scalarLoss);

    EXPECT_NEAR(leafA->grad.at({0}), 1.0f, TOLERANCE);
    EXPECT_NEAR(leafA->grad.at({1}), 0.0f, TOLERANCE);
    EXPECT_NEAR(leafB->grad.at({0}), 0.0f, TOLERANCE);
    EXPECT_NEAR(leafB->grad.at({1}), 1.0f, TOLERANCE);
}

/**
 * @brief `clamp()` borne chaque élément dans `[low, high]` en passe avant.
 * \castest{<b>Ops : `clamp()` passe avant.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `{-5, 0.5, 5}`.<br/>2. Appeler `clamp(a, -1, 1)`.<br/>
 * \tattendu `value` du résultat vaut `{-1, 0.5, 1}`.}
 */
TEST(AutodiffOpsTest, ClampPasseAvant) {
    Tensor<float> data({3});
    data.at({0}) = -5.0f;
    data.at({1}) = 0.5f;
    data.at({2}) = 5.0f;

    const NodePtr result = aisolver::autodiff::clamp(variable(data), -1.0f, 1.0f);

    EXPECT_NEAR(result->value.at({0}), -1.0f, TOLERANCE);
    EXPECT_NEAR(result->value.at({1}), 0.5f, TOLERANCE);
    EXPECT_NEAR(result->value.at({2}), 1.0f, TOLERANCE);
}

/**
 * @brief `backward()` sur `clamp()` transmet le gradient uniquement aux éléments strictement à
 * l'intérieur des bornes ; les éléments rognés reçoivent un gradient nul.
 * \castest{<b>Ops : `clamp()` gradient nul hors bornes.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une feuille `{-5, 0.5, 5}`.<br/>2. Réduire `clamp(a, -1, 1)` à un
 * scalaire (somme via `selectIndex`+`add`) et appeler `backward()`.<br/>
 * \tattendu Gradient nul aux indices `0` et `2` (rognés), non nul à l'indice `1` (intérieur).}
 */
TEST(AutodiffOpsTest, ClampGradientNulHorsBornes) {
    Tensor<float> data({3});
    data.at({0}) = -5.0f;
    data.at({1}) = 0.5f;
    data.at({2}) = 5.0f;
    const NodePtr leaf = variable(data);

    const NodePtr clamped = aisolver::autodiff::clamp(leaf, -1.0f, 1.0f);
    const NodePtr scalarLoss = aisolver::autodiff::add(
        aisolver::autodiff::add(aisolver::autodiff::selectIndex(clamped, 0), aisolver::autodiff::selectIndex(clamped, 1)),
        aisolver::autodiff::selectIndex(clamped, 2));

    aisolver::autodiff::backward(scalarLoss);

    EXPECT_NEAR(leaf->grad.at({0}), 0.0f, TOLERANCE);
    EXPECT_NEAR(leaf->grad.at({1}), 1.0f, TOLERANCE);
    EXPECT_NEAR(leaf->grad.at({2}), 0.0f, TOLERANCE);
}

/**
 * @brief Chaîne représentative de la perte de *policy gradient* (`LOT-ANNEXE-12`) :
 * `multiplyScalar(logOp(selectIndex(softmax(sortie), a)), -G)` produit des gradients non nuls sur
 * les poids d'un réseau traversé.
 * \castest{<b>Ops : chaîne policy gradient bout-en-bout.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire `w` `[3,2]`, `x` `[2,1]` (poids d'un réseau miniature).<br/>2. Calculer
 * `multiplyScalar(logOp(selectIndex(softmax(matmul(w,x)), 1)), -2.5)`.<br/>3. Appeler
 * `backward()`.<br/>
 * \tattendu Au moins un élément de `w->grad` est non nul.}
 */
TEST(AutodiffOpsTest, ChainePolicyGradientBoutEnBout) {
    Tensor<float> wData({3, 2});
    wData.at({0, 0}) = 0.2f;
    wData.at({0, 1}) = -0.1f;
    wData.at({1, 0}) = 0.5f;
    wData.at({1, 1}) = 0.3f;
    wData.at({2, 0}) = -0.4f;
    wData.at({2, 1}) = 0.1f;
    Tensor<float> xData({2, 1});
    xData.at({0, 0}) = 1.0f;
    xData.at({1, 0}) = -0.5f;

    const NodePtr w = variable(wData);
    const NodePtr x = variable(xData);

    const NodePtr logits = aisolver::autodiff::matmul(w, x);
    const NodePtr probabilities = aisolver::nn::softmax(logits);
    const NodePtr chosenProbability = aisolver::autodiff::selectIndex(probabilities, 1);
    const NodePtr loss = aisolver::autodiff::multiplyScalar(aisolver::autodiff::logOp(chosenProbability), -2.5f);

    aisolver::autodiff::backward(loss);

    bool anyNonZero = false;
    for (std::size_t i = 0; i < w->grad.size(); ++i) {
        if (w->grad.data()[i] != 0.0f) {
            anyNonZero = true;
            break;
        }
    }
    EXPECT_TRUE(anyNonZero);
}
