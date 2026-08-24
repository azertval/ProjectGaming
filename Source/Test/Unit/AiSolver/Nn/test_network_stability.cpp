/**
 * @file test_network_stability.cpp
 * @brief Stabilité numérique et gradient d'un `aisolver::nn::Network` complet, plusieurs couches
 * et activations mêlées (LOT-ANNEXE-03, TACHE-06) : conditions d'acceptation 1/5 de l'épic.
 */

#include <cmath>
#include <memory>

#include <gtest/gtest.h>

#include "../Math/GradientCheck.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/Activations.h"
#include "AiSolver/Nn/Dense.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Nn/WeightInit.h"

using aisolver::checkGradient;
using aisolver::GradientCheckResult;
using aisolver::Rng;
using aisolver::Tensor;
using aisolver::autodiff::NodePtr;
using aisolver::nn::Dense;
using aisolver::nn::Network;
using aisolver::nn::WeightInitScheme;

namespace {

NodePtr columnVariable(std::size_t size, float fillValue) {
    Tensor<float> data({size, 1});
    for (std::size_t i = 0; i < size; ++i) {
        data.data()[i] = fillValue;
    }
    return aisolver::autodiff::variable(data);
}

bool allFinite(const Tensor<float>& tensor) {
    for (std::size_t i = 0; i < tensor.size(); ++i) {
        if (!std::isfinite(tensor.data()[i])) {
            return false;
        }
    }
    return true;
}

}  // namespace

/**
 * @brief Sur des entrées d'amplitude extrême (très grandes, très petites, nulles), aucune couche
 * intermédiaire ni la sortie finale d'une composition `Dense`+`relu`/`tanhOp`/`sigmoid`/`softmax`
 * ne produit de valeur non finie.
 * \castest{<b>Network : absence de NaN/inf sur chaque couche, entrées extrêmes.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire quatre `Dense`+activation (`relu`, `tanhOp`, `sigmoid`, `softmax`).<br/>
 * 2. Pour des entrées `{1e6, -1e6, 1e-6, 0}` (remplissage constant), enchaîner les quatre passes
 * avant, en vérifiant la finitude après chacune.<br/>
 * \tattendu Toutes les valeurs intermédiaires et finales sont finies, pour les quatre
 * entrées.}
 */
TEST(NetworkStabilityTest, AucunNanInfSurChaqueCoucheEntreesExtremes) {
    const float extremeInputs[] = {1e6f, -1e6f, 1e-6f, 0.0f};

    for (float inputValue : extremeInputs) {
        Rng rng(9101);
        Dense layer1(4, 5, WeightInitScheme::He, rng);
        Dense layer2(5, 4, WeightInitScheme::Xavier, rng);
        Dense layer3(4, 3, WeightInitScheme::Xavier, rng);
        Dense layer4(3, 3, WeightInitScheme::Xavier, rng);

        const NodePtr input = columnVariable(4, inputValue);
        const NodePtr afterLayer1 = aisolver::autodiff::relu(layer1.forward(input));
        ASSERT_TRUE(allFinite(afterLayer1->value)) << "entree=" << inputValue << " apres relu";
        const NodePtr afterLayer2 = aisolver::autodiff::tanhOp(layer2.forward(afterLayer1));
        ASSERT_TRUE(allFinite(afterLayer2->value)) << "entree=" << inputValue << " apres tanhOp";
        const NodePtr afterLayer3 = aisolver::nn::sigmoid(layer3.forward(afterLayer2));
        ASSERT_TRUE(allFinite(afterLayer3->value)) << "entree=" << inputValue << " apres sigmoid";
        const NodePtr afterLayer4 = aisolver::nn::softmax(layer4.forward(afterLayer3));
        ASSERT_TRUE(allFinite(afterLayer4->value)) << "entree=" << inputValue << " apres softmax";
    }
}

/**
 * @brief `backward()` sur une perte scalaire construite à partir de `Network::forward()` (quatre
 * couches, activations mêlées) produit un gradient non nul sur chaque paramètre de
 * `Network::parameters()`, pour une entrée/perte non dégénérée.
 * \castest{<b>Network : gradient non nul sur tous les paramètres (réseau mêlé).</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un `Network` à quatre couches (`relu`, `tanhOp`, `sigmoid`,
 * `softmax`).<br/>2. Réduire `forward()` au squelette de perte de `LOT-ANNEXE-12`
 * (`multiplyScalar(logOp(selectIndex(sortie, a)), -1)`) et appeler `backward()`.<br/>
 * \tattendu Chaque paramètre a au moins un élément de gradient non nul.}
 */
TEST(NetworkStabilityTest, GradientNonNulSurTousLesParametresReseauMele) {
    Rng rng(9102);
    Network network;
    network.addLayer(std::make_unique<Dense>(4, 5, WeightInitScheme::He, rng),
                     aisolver::autodiff::relu);
    network.addLayer(std::make_unique<Dense>(5, 4, WeightInitScheme::Xavier, rng),
                     aisolver::autodiff::tanhOp);
    network.addLayer(std::make_unique<Dense>(4, 3, WeightInitScheme::Xavier, rng),
                     aisolver::nn::sigmoid);
    network.addLayer(std::make_unique<Dense>(3, 3, WeightInitScheme::Xavier, rng),
                     aisolver::nn::softmax);

    // Une perte sommant tous les elements d'une sortie softmax serait degeneree (somme == 1,
    // constante, gradient nul partout) : on isole une seule composante, comme le fait la perte de
    // policy gradient que ce squelette anticipe (LOT-ANNEXE-12).
    const NodePtr output = network.forward(columnVariable(4, 0.4f));
    const NodePtr scalarLoss = aisolver::autodiff::multiplyScalar(
        aisolver::autodiff::logOp(aisolver::autodiff::selectIndex(output, 0)), -1.0f);

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
 * @brief `sigmoid`/`softmax` passent le contrôle de gradient (`GradientCheck.h`, `LOT-ANNEXE-02`)
 * une fois composées dans un `Network` complet (poids fixes, gradient vérifié par rapport à
 * l'entrée), pas seulement isolées (déjà couvert par TACHE-02).
 * \castest{<b>Network : gradient checking de sigmoid/softmax en composition.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un `Network` à deux couches `Dense`+`sigmoid` puis `Dense`+`softmax`
 * (poids fixés par une graine).<br/>2. Appeler `checkGradient` sur l'entrée du réseau.<br/>
 * \tattendu `passed == true`.}
 */
TEST(NetworkStabilityTest, GradientCheckingSigmoidSoftmaxEnComposition) {
    Rng rng(9103);
    auto network = std::make_shared<Network>();
    network->addLayer(std::make_unique<Dense>(3, 4, WeightInitScheme::Xavier, rng),
                      aisolver::nn::sigmoid);
    network->addLayer(std::make_unique<Dense>(4, 3, WeightInitScheme::Xavier, rng),
                      aisolver::nn::softmax);

    Rng inputRng(9104);
    Tensor<float> input({3, 1});
    for (std::size_t i = 0; i < 3; ++i) {
        input.data()[i] = inputRng.nextFloat(-1.0f, 1.0f);
    }

    const GradientCheckResult result = checkGradient(
        [network](const std::vector<NodePtr>& nodes) { return network->forward(nodes[0]); },
        {input});

    EXPECT_TRUE(result.passed) << "Ecart maximal : " << result.maxAbsoluteError;
}
