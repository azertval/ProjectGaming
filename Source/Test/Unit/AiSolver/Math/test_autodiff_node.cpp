// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_autodiff_node.cpp
 * @brief Tests unitaires du graphe de calcul dynamique `aisolver::autodiff::Node`
 * (LOT-ANNEXE-02, TACHE-01).
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Math/TensorOps.h"

using aisolver::Tensor;
using aisolver::autodiff::binaryOp;
using aisolver::autodiff::NodePtr;
using aisolver::autodiff::unaryOp;
using aisolver::autodiff::variable;

/**
 * @brief `variable()` construit une feuille portant exactement la valeur fournie, avec un
 * gradient initial nul de même forme.
 * \castest{<b>Node : `variable()` crée une feuille.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un tenseur `{2}` de valeurs connues.<br/>2. Appeler `variable()`.<br/>
 * 3. Lire `value` et `grad` du nœud obtenu.<br/>
 * \tattendu `value` égale le tenseur fourni, `grad` est de même forme et nul partout.}
 */
TEST(NodeTest, VariableCreeUneFeuille) {
    Tensor<float> data({2});
    data.at({0}) = 3.0f;
    data.at({1}) = 5.0f;

    const NodePtr leaf = variable(data);

    EXPECT_FLOAT_EQ(leaf->value.at({0}), 3.0f);
    EXPECT_FLOAT_EQ(leaf->value.at({1}), 5.0f);

    EXPECT_EQ(leaf->grad.shape(), leaf->value.shape());
    EXPECT_FLOAT_EQ(leaf->grad.at({0}), 0.0f);
    EXPECT_FLOAT_EQ(leaf->grad.at({1}), 0.0f);
}

/**
 * @brief `zeroGrad()` remet `grad` à zéro (même forme), après qu'une valeur non nulle y a été
 * écrite manuellement.
 * \castest{<b>Node : `zeroGrad()` remet le gradient à zéro.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille de forme `{2, 2}`.<br/>2. Écrire des valeurs non nulles dans
 * `grad`.<br/>3. Appeler `zeroGrad()`.<br/>
 * \tattendu `grad` est nul partout, forme inchangée.}
 */
TEST(NodeTest, ZeroGradRemetAZero) {
    const NodePtr node = variable(Tensor<float>({2, 2}));

    node->grad.at({0, 0}) = 3.0f;
    node->grad.at({1, 1}) = -2.0f;

    node->zeroGrad();

    EXPECT_EQ(node->grad.shape(), (std::vector<std::size_t>{2, 2}));
    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 2; ++j) {
            EXPECT_FLOAT_EQ(node->grad.at({i, j}), 0.0f);
        }
    }
}

/**
 * @brief `unaryOp()` calcule `value` immédiatement (passe avant *eager*) : le résultat de
 * `forward` est visible dès la construction, sans appel supplémentaire.
 * \castest{<b>Node : `unaryOp()` calcule `value` immédiatement.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une feuille `{3, 5}`.<br/>2. Appeler `unaryOp()` avec un `forward` qui
 * double chaque élément.<br/>3. Lire `value` du nœud résultat, sans appeler `backward()` (qui
 * n'existe pas encore, TACHE-03).<br/>
 * \tattendu `value` du résultat vaut `{6, 10}` ; `grad` est nul, même forme.}
 */
TEST(NodeTest, UnaryOpCalculeValueImmediatement) {
    Tensor<float> data({2});
    data.at({0}) = 3.0f;
    data.at({1}) = 5.0f;
    const NodePtr input = variable(data);

    const NodePtr output = unaryOp(
        input, [](const Tensor<float>& value) { return aisolver::multiplyScalar(value, 2.0f); },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&) {
            return outputGrad;
        });

    EXPECT_FLOAT_EQ(output->value.at({0}), 6.0f);
    EXPECT_FLOAT_EQ(output->value.at({1}), 10.0f);

    EXPECT_EQ(output->grad.shape(), output->value.shape());
    EXPECT_FLOAT_EQ(output->grad.at({0}), 0.0f);
    EXPECT_FLOAT_EQ(output->grad.at({1}), 0.0f);
}

/**
 * @brief `binaryOp()` calcule `value` immédiatement, à partir des valeurs des deux parents.
 * \castest{<b>Node : `binaryOp()` calcule `value` immédiatement.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux feuilles `{1, 2}` et `{5, 6}`.<br/>2. Appeler `binaryOp()` avec un
 * `forward` d'addition élément par élément.<br/>3. Lire `value` du nœud résultat.<br/>
 * \tattendu `value` du résultat vaut `{6, 8}`.}
 */
TEST(NodeTest, BinaryOpCalculeValueImmediatement) {
    Tensor<float> dataA({2});
    dataA.at({0}) = 1.0f;
    dataA.at({1}) = 2.0f;
    Tensor<float> dataB({2});
    dataB.at({0}) = 5.0f;
    dataB.at({1}) = 6.0f;

    const NodePtr a = variable(dataA);
    const NodePtr b = variable(dataB);

    const NodePtr output = binaryOp(
        a, b,
        [](const Tensor<float>& valueA, const Tensor<float>& valueB) {
            return aisolver::add(valueA, valueB);
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>&) { return outputGrad; },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>&) { return outputGrad; });

    EXPECT_FLOAT_EQ(output->value.at({0}), 6.0f);
    EXPECT_FLOAT_EQ(output->value.at({1}), 8.0f);

    EXPECT_EQ(output->grad.shape(), output->value.shape());
    EXPECT_FLOAT_EQ(output->grad.at({0}), 0.0f);
    EXPECT_FLOAT_EQ(output->grad.at({1}), 0.0f);
}

/**
 * @brief `unaryOp()`/`binaryOp()` ne lèvent ni n'assertent sur un cas simple (aucune vérification
 * du contenu de `_parents`, laissée à TACHE-03 : ce test vérifie uniquement l'absence d'échec de
 * construction).
 * \castest{<b>Node : `unaryOp()`/`binaryOp()` construisent sans échec.</b><br/>
 * \tcat Unitaire · Autodiff<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire une ou deux feuilles.<br/>2. Appeler `unaryOp()` avec un `forward`
 * identité, puis `binaryOp()` avec un `forward` qui ignore le second parent.<br/>
 * \tattendu Aucune des deux constructions ne lève ni n'assert.}
 */
TEST(NodeTest, UnaryOpEtBinaryOpConstruisentSansEchec) {
    const NodePtr input = variable(Tensor<float>({2}));

    EXPECT_NO_THROW({
        const NodePtr output = unaryOp(
            input, [](const Tensor<float>& value) { return value; },
            [](const Tensor<float>&, const Tensor<float>&, const Tensor<float>& inputValue) {
                return Tensor<float>(inputValue.shape());
            });
        (void)output;
    });

    const NodePtr a = variable(Tensor<float>({2}));
    const NodePtr b = variable(Tensor<float>({2}));

    EXPECT_NO_THROW({
        const NodePtr output = binaryOp(
            a, b, [](const Tensor<float>& valueA, const Tensor<float>&) { return valueA; },
            [](const Tensor<float>&, const Tensor<float>&, const Tensor<float>& valueA,
               const Tensor<float>&) { return Tensor<float>(valueA.shape()); },
            [](const Tensor<float>&, const Tensor<float>&, const Tensor<float>&,
               const Tensor<float>& valueB) { return Tensor<float>(valueB.shape()); });
        (void)output;
    });
}
