/**
 * @file test_activations.cpp
 * @brief Tests unitaires des activations `aisolver::nn::sigmoid`/`softmax` (LOT-ANNEXE-03,
 * TACHE-02) : bornes, stabilité numérique, gradient checking.
 */

#include <cmath>

#include <gtest/gtest.h>

#include "../Math/GradientCheck.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/Activations.h"

using aisolver::checkGradient;
using aisolver::GradientCheckResult;
using aisolver::Rng;
using aisolver::Tensor;
using aisolver::autodiff::NodePtr;
using aisolver::autodiff::variable;

namespace {
constexpr float TOLERANCE = 1e-5f;
}  // namespace

/**
 * @brief `sigmoid` reste strictement dans `]0, 1[`, sans `NaN`/`inf`, pour des entrées de grande
 * amplitude positive et négative.
 * \castest{<b>Activations : `sigmoid` bornée.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `{-10, 10}` (amplitude assez grande pour approcher la
 * saturation sans y arriver exactement en précision `float32` : au-delà, `exp(-x)` devient
 * inférieur à l'epsilon `float` et `1 - exp(-x)` arrondit à `1.0f` pile, ce qui n'est pas un défaut
 * de `sigmoid`, même piège que `tanhOp`, `LOT-ANNEXE-02`).<br/>2. Appeler `sigmoid()`.<br/>
 * \tattendu Les deux sorties sont finies et strictement dans `]0, 1[`.}
 */
TEST(ActivationsTest, SigmoidBornee) {
    Tensor<float> data({2});
    data.at({0}) = -10.0f;
    data.at({1}) = 10.0f;

    const NodePtr result = aisolver::nn::sigmoid(variable(data));

    for (std::size_t i = 0; i < 2; ++i) {
        const float value = result->value.at({i});
        EXPECT_TRUE(std::isfinite(value));
        EXPECT_GT(value, 0.0f);
        EXPECT_LT(value, 1.0f);
    }
}

/**
 * @brief `softmax` produit une distribution : somme à `1`, chaque élément dans `[0, 1]`.
 * \castest{<b>Activations : `softmax` somme à 1.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `[3,1]` quelconque `{1, 2, 3}`.<br/>2. Appeler
 * `softmax()`.<br/>
 * \tattendu La somme des sorties vaut `1.0` à `1e-5` près, chaque élément dans `[0, 1]`.}
 */
TEST(ActivationsTest, SoftmaxSommeAUn) {
    Tensor<float> data({3, 1});
    data.at({0, 0}) = 1.0f;
    data.at({1, 0}) = 2.0f;
    data.at({2, 0}) = 3.0f;

    const NodePtr result = aisolver::nn::softmax(variable(data));

    float total = 0.0f;
    for (std::size_t i = 0; i < 3; ++i) {
        const float value = result->value.at({i, 0});
        EXPECT_GE(value, 0.0f);
        EXPECT_LE(value, 1.0f);
        total += value;
    }
    EXPECT_NEAR(total, 1.0f, TOLERANCE);
}

/**
 * @brief `softmax` reste stable (aucun `NaN`/`inf`) sur un vecteur contenant un très grand
 * *logit* — vérifie explicitement la soustraction du maximum.
 * \castest{<b>Activations : `softmax` stable sur de grands logits.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une feuille `[2,1]` `{1000, 1}`.<br/>2. Appeler `softmax()`.<br/>
 * \tattendu Les deux sorties sont finies, somme à `1.0` près.}
 */
TEST(ActivationsTest, SoftmaxStableSurGrandsLogits) {
    Tensor<float> data({2, 1});
    data.at({0, 0}) = 1000.0f;
    data.at({1, 0}) = 1.0f;

    const NodePtr result = aisolver::nn::softmax(variable(data));

    EXPECT_TRUE(std::isfinite(result->value.at({0, 0})));
    EXPECT_TRUE(std::isfinite(result->value.at({1, 0})));
    EXPECT_NEAR(result->value.at({0, 0}) + result->value.at({1, 0}), 1.0f, TOLERANCE);
}

/**
 * @brief `softmax` sur un vecteur constant produit une sortie uniforme (`1/n` par élément).
 * \castest{<b>Activations : `softmax` sur entrées égales est uniforme.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `[4,1]` constante `{5, 5, 5, 5}`.<br/>2. Appeler
 * `softmax()`.<br/>
 * \tattendu Chaque élément vaut `0.25` à `1e-5` près.}
 */
TEST(ActivationsTest, SoftmaxSurEntreesEgalesEstUniforme) {
    Tensor<float> data({4, 1});
    for (std::size_t i = 0; i < 4; ++i) {
        data.at({i, 0}) = 5.0f;
    }

    const NodePtr result = aisolver::nn::softmax(variable(data));

    for (std::size_t i = 0; i < 4; ++i) {
        EXPECT_NEAR(result->value.at({i, 0}), 0.25f, TOLERANCE);
    }
}

/**
 * @brief `checkGradient` valide le gradient analytique de `sigmoid` sur des entrées aléatoires.
 * \castest{<b>Activations : gradient checking de `sigmoid`.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer un tenseur `[3,1]` aléatoire (graine fixe).<br/>2. Appeler `checkGradient`
 * avec `buildGraph = sigmoid(input)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(ActivationsTest, GradientCheckingSigmoid) {
    Rng rng(4001);
    Tensor<float> input({3, 1});
    for (std::size_t i = 0; i < 3; ++i) {
        input.data()[i] = rng.nextFloat(-2.0f, 2.0f);
    }

    const GradientCheckResult result =
        checkGradient([](const std::vector<NodePtr>& nodes) { return aisolver::nn::sigmoid(nodes[0]); }, {input});

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `softmax` sur des entrées aléatoires.
 * \castest{<b>Activations : gradient checking de `softmax`.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer un tenseur `[3,1]` aléatoire (graine fixe).<br/>2. Appeler `checkGradient`
 * avec `buildGraph = softmax(input)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(ActivationsTest, GradientCheckingSoftmax) {
    Rng rng(4002);
    Tensor<float> input({3, 1});
    for (std::size_t i = 0; i < 3; ++i) {
        input.data()[i] = rng.nextFloat(-2.0f, 2.0f);
    }

    const GradientCheckResult result =
        checkGradient([](const std::vector<NodePtr>& nodes) { return aisolver::nn::softmax(nodes[0]); }, {input});

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}
