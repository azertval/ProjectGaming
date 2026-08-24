/**
 * @file test_network.cpp
 * @brief Tests unitaires de `aisolver::nn::Network` (LOT-ANNEXE-03, TACHE-03) : composition,
 * accès aux paramètres, rétropropagation de bout en bout, ordre des couches.
 */

#include <memory>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/Activations.h"
#include "AiSolver/Nn/Dense.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Nn/WeightInit.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::autodiff::NodePtr;
using aisolver::nn::Dense;
using aisolver::nn::Network;
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

/// Reduit un noeud de forme [n,1] a un scalaire [1,1] par matmul avec un vecteur ligne de 1
/// (pas de sum() dedie dans autodiff a ce stade, LOT-ANNEXE-02).
NodePtr sumToScalar(const NodePtr& node) {
    const std::size_t size = node->value.shape()[0];
    Tensor<float> onesRow({1, size});
    for (std::size_t i = 0; i < size; ++i) {
        onesRow.data()[i] = 1.0f;
    }
    return aisolver::autodiff::matmul(aisolver::autodiff::variable(onesRow), node);
}

}  // namespace

/**
 * @brief Un réseau à trois couches (`relu`, `sigmoid`, sans activation) transforme une entrée
 * `[4,1]` en une sortie de la forme attendue par la dernière couche.
 * \castest{<b>Network : forward bout-en-bout.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `Network` de trois couches `Dense(4,5)+relu`, `Dense(5,3)+sigmoid`,
 * `Dense(3,2)` sans activation.<br/>2. Appeler `forward()` sur une entrée `[4,1]`.<br/>
 * \tattendu La sortie a pour forme `[2,1]`.}
 */
TEST(NetworkTest, ForwardBoutEnBout) {
    Rng rng(5001);
    Network network;
    network.addLayer(std::make_unique<Dense>(4, 5, WeightInitScheme::He, rng),
                     aisolver::autodiff::relu);
    network.addLayer(std::make_unique<Dense>(5, 3, WeightInitScheme::Xavier, rng),
                     aisolver::nn::sigmoid);
    network.addLayer(std::make_unique<Dense>(3, 2, WeightInitScheme::Xavier, rng), nullptr);

    const NodePtr output = network.forward(columnVariable(4, 0.5f));

    EXPECT_EQ(output->value.shape(), (std::vector<std::size_t>{2, 1}));
}

/**
 * @brief `parameters()` renvoie `2 * layerCount()` `NodePtr`, sans doublon ni omission.
 * \castest{<b>Network : `parameters()` couvre toutes les couches.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `Network` de trois couches.<br/>2. Appeler `parameters()` et
 * `layerCount()`.<br/>
 * \tattendu `parameters().size() == 2 * layerCount()`.}
 */
TEST(NetworkTest, ParametersCouvreToutesLesCouches) {
    Rng rng(5002);
    Network network;
    network.addLayer(std::make_unique<Dense>(4, 5, WeightInitScheme::He, rng),
                     aisolver::autodiff::relu);
    network.addLayer(std::make_unique<Dense>(5, 3, WeightInitScheme::Xavier, rng),
                     aisolver::nn::sigmoid);
    network.addLayer(std::make_unique<Dense>(3, 2, WeightInitScheme::Xavier, rng), nullptr);

    EXPECT_EQ(network.layerCount(), 3u);
    EXPECT_EQ(network.parameters().size(), 6u);
}

/**
 * @brief `backward()` sur une perte scalaire construite à partir de `forward()` produit des
 * gradients non nuls sur chaque paramètre de chaque couche, y compris la première.
 * \castest{<b>Network : `backward()` bout-en-bout sur toutes les couches.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un `Network` de trois couches.<br/>2. Réduire `forward()` à un scalaire
 * et appeler `autodiff::backward()`.<br/>
 * \tattendu Chaque paramètre de `parameters()` a au moins un élément de gradient non nul.}
 */
TEST(NetworkTest, BackwardBoutEnBoutSurToutesLesCouches) {
    Rng rng(5003);
    Network network;
    network.addLayer(std::make_unique<Dense>(4, 5, WeightInitScheme::He, rng),
                     aisolver::autodiff::relu);
    network.addLayer(std::make_unique<Dense>(5, 3, WeightInitScheme::Xavier, rng),
                     aisolver::nn::sigmoid);
    network.addLayer(std::make_unique<Dense>(3, 2, WeightInitScheme::Xavier, rng), nullptr);

    const NodePtr output = network.forward(columnVariable(4, 0.5f));
    const NodePtr squared = aisolver::autodiff::multiply(output, output);
    const NodePtr scalarLoss = sumToScalar(squared);

    aisolver::autodiff::backward(scalarLoss);

    for (const NodePtr& parameter : network.parameters()) {
        bool anyNonZero = false;
        for (std::size_t i = 0; i < parameter->grad.size(); ++i) {
            if (parameter->grad.data()[i] != 0.0f) {
                anyNonZero = true;
                break;
            }
        }
        EXPECT_TRUE(anyNonZero);
    }
}

/**
 * @brief Une couche ajoutée sans activation (`nullptr`) ne subit aucune transformation après
 * `Dense::forward()`.
 * \castest{<b>Network : activation `nullptr` laisse la sortie linéaire.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `Network` d'une seule couche `Dense(2,2)` sans activation.<br/>
 * 2. Comparer `network.forward(input)` à `layer.forward(input)` appelé directement (même
 * couche, mêmes poids).<br/>
 * \tattendu Les deux sorties sont identiques.}
 */
TEST(NetworkTest, ActivationNulleLaisseLaSortieLineaire) {
    Rng rngNetwork(5004);
    Rng rngDirect(5004);
    auto layerForNetwork = std::make_unique<Dense>(2, 2, WeightInitScheme::Xavier, rngNetwork);
    Dense layerDirect(2, 2, WeightInitScheme::Xavier, rngDirect);

    Network network;
    network.addLayer(std::move(layerForNetwork), nullptr);

    const NodePtr input = columnVariable(2, 0.75f);
    const NodePtr networkOutput = network.forward(input);
    const NodePtr directOutput = layerDirect.forward(input);

    EXPECT_NEAR(networkOutput->value.at({0, 0}), directOutput->value.at({0, 0}), 1e-6f);
    EXPECT_NEAR(networkOutput->value.at({1, 0}), directOutput->value.at({1, 0}), 1e-6f);
}

/**
 * @brief Deux réseaux construits avec les mêmes couches mais ajoutées dans un ordre différent
 * produisent des sorties différentes sur la même entrée (garde-fou contre un stockage qui
 * ignorerait l'ordre d'ajout).
 * \castest{<b>Network : l'ordre des couches est respecté.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux `Network` de deux couches `Dense(3,3)` chacune (mêmes graines),
 * ajoutées dans un ordre inverse l'une de l'autre.<br/>2. Appeler `forward()` sur la même
 * entrée.<br/>
 * \tattendu Les deux sorties diffèrent.}
 */
TEST(NetworkTest, OrdreDesCouchesRespecte) {
    // Sans relu (qui pourrait ecraser des valeurs a zero et masquer une difference reelle par
    // coincidence) : tanhOp est non degeneree partout, deux compositions dans un ordre different
    // de matrices distinctes n'ont pratiquement aucune chance de coincider exactement.
    Rng rngA1(6001);
    Rng rngA2(6002);
    Network networkForward;
    networkForward.addLayer(std::make_unique<Dense>(3, 3, WeightInitScheme::Xavier, rngA1),
                            aisolver::autodiff::tanhOp);
    networkForward.addLayer(std::make_unique<Dense>(3, 3, WeightInitScheme::Xavier, rngA2),
                            aisolver::autodiff::tanhOp);

    Rng rngB1(6001);
    Rng rngB2(6002);
    auto layer1 = std::make_unique<Dense>(3, 3, WeightInitScheme::Xavier, rngB1);
    auto layer2 = std::make_unique<Dense>(3, 3, WeightInitScheme::Xavier, rngB2);
    Network networkReversed;
    networkReversed.addLayer(std::move(layer2), aisolver::autodiff::tanhOp);
    networkReversed.addLayer(std::move(layer1), aisolver::autodiff::tanhOp);

    const NodePtr input = columnVariable(3, 0.4f);
    const NodePtr outputForward = networkForward.forward(input);
    const NodePtr outputReversed = networkReversed.forward(input);

    bool anyDifferent = false;
    for (std::size_t i = 0; i < 3; ++i) {
        if (outputForward->value.at({i, 0}) != outputReversed->value.at({i, 0})) {
            anyDifferent = true;
            break;
        }
    }
    EXPECT_TRUE(anyDifferent);
}
