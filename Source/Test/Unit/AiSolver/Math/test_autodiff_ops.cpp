/**
 * @file test_autodiff_ops.cpp
 * @brief Tests unitaires des opérations différentiables de base `aisolver::autodiff`
 * (LOT-ANNEXE-02, TACHE-02) : passe avant et composition, hors correction du gradient (TACHE-04).
 */

#include <cmath>
#include <limits>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Tensor.h"

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
