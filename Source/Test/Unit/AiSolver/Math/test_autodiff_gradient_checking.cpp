/**
 * @file test_autodiff_gradient_checking.cpp
 * @brief Vérifie, par différences finies centrées (`GradientCheck.h`, LOT-ANNEXE-02, TACHE-04), la
 * correction du gradient analytique de chacune des cinq opérations de TACHE-02, et que l'outil
 * détecte effectivement une règle de dérivation volontairement fausse.
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Math/TensorOps.h"
#include "GradientCheck.h"

using aisolver::checkGradient;
using aisolver::GradientCheckResult;
using aisolver::Tensor;
using aisolver::autodiff::NodePtr;

namespace {

/// Tenseur de forme donnée, éléments tirés uniformément dans `[min, max]` via `Rng` (graine fixe,
/// reproductibilité du test).
Tensor<float> randomTensor(aisolver::Rng& rng, const std::vector<std::size_t>& shape, float min, float max) {
    Tensor<float> result(shape);
    float* data = result.data();
    for (std::size_t i = 0; i < result.size(); ++i) {
        data[i] = rng.nextFloat(min, max);
    }
    return result;
}

}  // namespace

/**
 * @brief `checkGradient` valide le gradient analytique d'`add` sur des tenseurs `2x2` aléatoires.
 * \castest{<b>Gradient checking : `add`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer deux tenseurs `2x2` aléatoires (graine fixe).<br/>2. Appeler `checkGradient`
 * avec `buildGraph = add(inputs[0], inputs[1])`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, Add) {
    aisolver::Rng rng(1001);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, -3.0f, 3.0f),
                                                randomTensor(rng, {2, 2}, -3.0f, 3.0f)};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::add(nodes[0], nodes[1]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `multiply` sur des tenseurs `2x2`
 * aléatoires.
 * \castest{<b>Gradient checking : `multiply`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer deux tenseurs `2x2` aléatoires (graine fixe).<br/>2. Appeler `checkGradient`
 * avec `buildGraph = multiply(inputs[0], inputs[1])`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, Multiply) {
    aisolver::Rng rng(1002);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, -3.0f, 3.0f),
                                                randomTensor(rng, {2, 2}, -3.0f, 3.0f)};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::multiply(nodes[0], nodes[1]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `matmul` sur des formes `[2,3]`/`[3,2]`.
 * \castest{<b>Gradient checking : `matmul`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer `a` `[2,3]` et `b` `[3,2]` aléatoires (graine fixe).<br/>2. Appeler
 * `checkGradient` avec `buildGraph = matmul(a, b)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, Matmul) {
    aisolver::Rng rng(1003);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 3}, -2.0f, 2.0f),
                                                randomTensor(rng, {3, 2}, -2.0f, 2.0f)};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::matmul(nodes[0], nodes[1]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `relu`, sur des entrées mêlant valeurs
 * strictement positives et strictement négatives, à distance du point non dérivable `x = 0` (la
 * différence finie n'y serait pas représentative — pas un défaut de `checkGradient`).
 * \castest{<b>Gradient checking : `relu`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer un tenseur `2x2` dans `[-3,-0.5] ∪ [0.5,3]` (graine fixe).<br/>2. Appeler
 * `checkGradient` avec `buildGraph = relu(input)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, Relu) {
    aisolver::Rng rng(1004);
    Tensor<float> input({2, 2});
    float* data = input.data();
    for (std::size_t i = 0; i < input.size(); ++i) {
        // Signe alterne, magnitude loin de 0 (>= 0.5) : evite le point non derivable de relu.
        const float magnitude = rng.nextFloat(0.5f, 3.0f);
        data[i] = (i % 2 == 0) ? magnitude : -magnitude;
    }
    const std::vector<Tensor<float>> inputs = {input};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::relu(nodes[0]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `tanhOp`, sur une plage d'entrées modérée
 * (loin de la saturation, où gradient analytique et numérique tendraient tous deux vers `0`,
 * rendant la comparaison peu discriminante).
 * \castest{<b>Gradient checking : `tanhOp`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer un tenseur `2x2` dans `[-1.5, 1.5]` (graine fixe).<br/>2. Appeler
 * `checkGradient` avec `buildGraph = tanhOp(input)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, TanhOp) {
    aisolver::Rng rng(1005);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, -1.5f, 1.5f)};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::tanhOp(nodes[0]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` détecte une règle de dérivation volontairement fausse : une addition dont
 * la règle renvoie `2×` le gradient de sortie au lieu de `1×` (câblée localement via `binaryOp`,
 * sans toucher `Ops.cpp`) échoue (`passed == false`).
 * \castest{<b>Gradient checking : détecte une régression volontaire.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer deux tenseurs `2x2` aléatoires (graine fixe).<br/>2. Appeler `checkGradient`
 * avec une addition dont la règle de dérivation vers le premier parent double le gradient de
 * sortie.<br/>
 * \tattendu `passed == false`.}
 */
TEST(GradientCheckingTest, DetecteUneReglederivationFausse) {
    aisolver::Rng rng(1006);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, -3.0f, 3.0f),
                                                randomTensor(rng, {2, 2}, -3.0f, 3.0f)};

    const auto wrongAdd = [](const NodePtr& a, const NodePtr& b) {
        return aisolver::autodiff::binaryOp(
            a, b,
            [](const Tensor<float>& valueA, const Tensor<float>& valueB) { return aisolver::add(valueA, valueB); },
            [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
               const Tensor<float>&) {
                // Faute volontaire : devrait propager outputGrad inchange (d(a+b)/da = 1), pas 2x.
                return aisolver::multiplyScalar(outputGrad, 2.0f);
            },
            [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
               const Tensor<float>&) { return outputGrad; });
    };

    const GradientCheckResult result =
        checkGradient([&wrongAdd](const std::vector<NodePtr>& nodes) { return wrongAdd(nodes[0], nodes[1]); }, inputs);

    EXPECT_FALSE(result.passed);
}
