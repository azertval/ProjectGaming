/**
 * @file test_dense.cpp
 * @brief Tests unitaires de la couche dense `aisolver::nn::Dense` (LOT-ANNEXE-03, TACHE-01) :
 * forme de sortie, paramètres exposés, intégration au graphe d'autodiff, initialisation non
 * constante.
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/Dense.h"
#include "AiSolver/Nn/WeightInit.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::autodiff::NodePtr;
using aisolver::nn::Dense;
using aisolver::nn::WeightInitScheme;

namespace {

NodePtr columnVariable(std::size_t size, float fillValue) {
    Tensor<float> data({size, 1});
    float* raw = data.data();
    for (std::size_t i = 0; i < size; ++i) {
        raw[i] = fillValue;
    }
    return aisolver::autodiff::variable(data);
}

}  // namespace

/**
 * @brief `forward()` produit une sortie de la forme `[outputSize, 1]` attendue.
 * \castest{<b>Dense : forme de sortie.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire `Dense(4, 3, Xavier, rng)`.<br/>2. Appeler `forward()` sur une entrée
 * `[4,1]`.<br/>
 * \tattendu La sortie a pour forme `[3, 1]`.}
 */
TEST(DenseTest, FormeDeSortie) {
    Rng rng(2001);
    Dense layer(4, 3, WeightInitScheme::Xavier, rng);

    const NodePtr output = layer.forward(columnVariable(4, 1.0f));

    EXPECT_EQ(output->value.shape(), (std::vector<std::size_t>{3, 1}));
}

/**
 * @brief `parameters()` renvoie exactement les poids puis le biais, aux formes attendues.
 * \castest{<b>Dense : paramètres exposés.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire `Dense(4, 3, Xavier, rng)`.<br/>2. Appeler `parameters()`.<br/>
 * \tattendu Deux éléments : poids de forme `[3,4]`, biais de forme `[3,1]`.}
 */
TEST(DenseTest, ParametresExposes) {
    Rng rng(2002);
    Dense layer(4, 3, WeightInitScheme::Xavier, rng);

    const std::vector<NodePtr> parameters = layer.parameters();

    ASSERT_EQ(parameters.size(), 2u);
    EXPECT_EQ(parameters[0]->value.shape(), (std::vector<std::size_t>{3, 4}));
    EXPECT_EQ(parameters[1]->value.shape(), (std::vector<std::size_t>{3, 1}));
    EXPECT_EQ(parameters[0]->value.shape(), layer.weights().shape());
    EXPECT_EQ(parameters[1]->value.shape(), layer.bias().shape());
}

/**
 * @brief `backward()` sur une perte scalaire construite sur la sortie de `forward()` produit des
 * gradients non nuls sur les poids et le biais.
 * \castest{<b>Dense : différentiable de bout en bout.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire `Dense(3, 2, He, rng)`.<br/>2. Sommer la sortie de `forward()` en un
 * scalaire (`add` répété).<br/>3. Appeler `autodiff::backward()`.<br/>
 * \tattendu Au moins un élément du gradient des poids et du biais est non nul.}
 */
TEST(DenseTest, DifferentiableDeBoutEnBout) {
    Rng rng(2003);
    Dense layer(3, 2, WeightInitScheme::He, rng);

    const NodePtr output = layer.forward(columnVariable(3, 0.5f));
    const NodePtr squared = aisolver::autodiff::multiply(output, output);
    // Reduit [2,1] a un scalaire [1,1] par matmul avec un vecteur ligne de 1 (pas de sum() dedie
    // dans autodiff a ce stade, LOT-ANNEXE-02) : somme des composantes de squared.
    Tensor<float> onesRow({1, 2});
    onesRow.data()[0] = 1.0f;
    onesRow.data()[1] = 1.0f;
    const NodePtr scalarLoss = aisolver::autodiff::matmul(aisolver::autodiff::variable(onesRow), squared);

    aisolver::autodiff::backward(scalarLoss);

    const std::vector<NodePtr> parameters = layer.parameters();
    bool weightsGradientNonZero = false;
    for (std::size_t i = 0; i < parameters[0]->grad.size(); ++i) {
        if (parameters[0]->grad.data()[i] != 0.0f) {
            weightsGradientNonZero = true;
            break;
        }
    }
    bool biasGradientNonZero = false;
    for (std::size_t i = 0; i < parameters[1]->grad.size(); ++i) {
        if (parameters[1]->grad.data()[i] != 0.0f) {
            biasGradientNonZero = true;
            break;
        }
    }

    EXPECT_TRUE(weightsGradientNonZero);
    EXPECT_TRUE(biasGradientNonZero);
}

/**
 * @brief Deux couches indépendantes, construites avec des `Rng` de graines différentes, ont des
 * poids différents (garde-fou contre une initialisation accidentellement constante).
 * \castest{<b>Dense : deux couches ont des poids différents.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux `Dense(4, 3, Xavier, ...)` avec deux `Rng` de graines différentes.<br/>
 * 2. Comparer leurs poids.<br/>
 * \tattendu Au moins un élément diffère entre les deux jeux de poids.}
 */
TEST(DenseTest, DeuxCouchesOntDesPoidsDifferents) {
    Rng rngA(3001);
    Rng rngB(3002);
    Dense layerA(4, 3, WeightInitScheme::Xavier, rngA);
    Dense layerB(4, 3, WeightInitScheme::Xavier, rngB);

    bool anyDifferent = false;
    for (std::size_t i = 0; i < layerA.weights().size(); ++i) {
        if (layerA.weights().data()[i] != layerB.weights().data()[i]) {
            anyDifferent = true;
            break;
        }
    }

    EXPECT_TRUE(anyDifferent);
}
