/**
 * @file test_autodiff_gradient_checking.cpp
 * @brief Vérifie, par différences finies centrées (`GradientCheck.h`, LOT-ANNEXE-02, TACHE-04), la
 * correction du gradient analytique de chacune des cinq opérations de TACHE-02, des neuf
 * opérations complémentaires de LOT-ANNEXE-03 TACHE-05, et que l'outil détecte effectivement une
 * règle de dérivation volontairement fausse.
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

// --- Operations complementaires (LOT-ANNEXE-03, TACHE-05) ---------------------------------------

/**
 * @brief `checkGradient` valide le gradient analytique de `subtract` sur des tenseurs `2x2`
 * aléatoires.
 * \castest{<b>Gradient checking : `subtract`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer deux tenseurs `2x2` aléatoires (graine fixe).<br/>2. Appeler `checkGradient`
 * avec `buildGraph = subtract(inputs[0], inputs[1])`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, Subtract) {
    aisolver::Rng rng(1101);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, -3.0f, 3.0f),
                                                randomTensor(rng, {2, 2}, -3.0f, 3.0f)};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::subtract(nodes[0], nodes[1]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `divide`, diviseur maintenu loin de
 * zéro.
 * \castest{<b>Gradient checking : `divide`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer `a` `2x2` dans `[-3,3]` et `b` `2x2` dans `[0.5,3]` (graine fixe).<br/>
 * 2. Appeler `checkGradient` avec `buildGraph = divide(inputs[0], inputs[1])`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, Divide) {
    aisolver::Rng rng(1102);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, -3.0f, 3.0f),
                                                randomTensor(rng, {2, 2}, 0.5f, 3.0f)};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::divide(nodes[0], nodes[1]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `addScalar`/`multiplyScalar`.
 * \castest{<b>Gradient checking : `addScalar`/`multiplyScalar`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer un tenseur `2x2` aléatoire (graine fixe).<br/>2. Appeler `checkGradient` avec
 * `buildGraph = addScalar(input, 3.0f)` puis `multiplyScalar(input, -2.0f)`.<br/>
 * \tattendu `passed == true` dans les deux cas.}
 */
TEST(GradientCheckingTest, AddScalarEtMultiplyScalar) {
    aisolver::Rng rng(1103);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, -3.0f, 3.0f)};

    const GradientCheckResult addResult = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::addScalar(nodes[0], 3.0f); }, inputs);
    const GradientCheckResult multiplyResult = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::multiplyScalar(nodes[0], -2.0f); }, inputs);

    EXPECT_TRUE(addResult.passed) << "Ecart maximal : " << addResult.maxAbsoluteError;
    EXPECT_TRUE(multiplyResult.passed) << "Ecart maximal : " << multiplyResult.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `logOp`, entrée strictement positive.
 * \castest{<b>Gradient checking : `logOp`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer un tenseur `2x2` dans `[0.5,3]` (graine fixe).<br/>2. Appeler
 * `checkGradient` avec `buildGraph = logOp(input)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, LogOp) {
    aisolver::Rng rng(1104);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, 0.5f, 3.0f)};

    const GradientCheckResult result =
        checkGradient([](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::logOp(nodes[0]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `expOp`, amplitude modérée (évite un
 * débordement lors des perturbations en différences finies).
 * \castest{<b>Gradient checking : `expOp`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer un tenseur `2x2` dans `[-2,2]` (graine fixe).<br/>2. Appeler `checkGradient`
 * avec `buildGraph = expOp(input)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, ExpOp) {
    aisolver::Rng rng(1105);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, -2.0f, 2.0f)};

    const GradientCheckResult result =
        checkGradient([](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::expOp(nodes[0]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `selectIndex`.
 * \castest{<b>Gradient checking : `selectIndex`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer un tenseur `[4]` aléatoire (graine fixe).<br/>2. Appeler `checkGradient`
 * avec `buildGraph = selectIndex(input, 2)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, SelectIndex) {
    aisolver::Rng rng(1106);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {4}, -3.0f, 3.0f)};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::selectIndex(nodes[0], 2); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `minimum`, opérandes maintenus à
 * distance stricte l'un de l'autre (point non dérivable à l'égalité, hors périmètre du contrôle).
 * \castest{<b>Gradient checking : `minimum`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Poser `a` et `b` `2x2` avec un écart d'au moins `1.0` élément par élément (graine
 * fixe).<br/>2. Appeler `checkGradient` avec `buildGraph = minimum(a, b)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, Minimum) {
    aisolver::Rng rng(1107);
    Tensor<float> a({2, 2});
    Tensor<float> b({2, 2});
    for (std::size_t i = 0; i < a.size(); ++i) {
        const float base = rng.nextFloat(-2.0f, 2.0f);
        a.data()[i] = base;
        b.data()[i] = base + 1.5f;  // ecart constant : jamais egaux, minimum toujours du cote de a.
    }
    const std::vector<Tensor<float>> inputs = {a, b};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::minimum(nodes[0], nodes[1]); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}

/**
 * @brief `checkGradient` valide le gradient analytique de `clamp`, entrée maintenue strictement
 * à l'intérieur des bornes (les points rognés ne sont pas dérivables, hors périmètre du contrôle).
 * \castest{<b>Gradient checking : `clamp`.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tirer un tenseur `2x2` dans `[-0.5,0.5]` (strictement à l'intérieur de `[-1,1]`,
 * graine fixe).<br/>2. Appeler `checkGradient` avec `buildGraph = clamp(input, -1, 1)`.<br/>
 * \tattendu `passed == true`.}
 */
TEST(GradientCheckingTest, Clamp) {
    aisolver::Rng rng(1108);
    const std::vector<Tensor<float>> inputs = {randomTensor(rng, {2, 2}, -0.5f, 0.5f)};

    const GradientCheckResult result = checkGradient(
        [](const std::vector<NodePtr>& nodes) { return aisolver::autodiff::clamp(nodes[0], -1.0f, 1.0f); }, inputs);

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}
