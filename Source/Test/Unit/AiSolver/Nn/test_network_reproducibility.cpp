/**
 * @file test_network_reproducibility.cpp
 * @brief Reproductibilité de bout en bout d'un `aisolver::nn::Network` complet, plusieurs couches
 * et activations mêlées (LOT-ANNEXE-03, TACHE-06) : condition d'acceptation 3/4 de l'épic. Au-delà
 * du test unitaire local de TACHE-04 (une seule paire de couches), celui-ci exerce l'assemblage.
 */

#include <filesystem>
#include <memory>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/Activations.h"
#include "AiSolver/Nn/Dense.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Nn/Serialization.h"
#include "AiSolver/Nn/WeightInit.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::autodiff::NodePtr;
using aisolver::nn::Dense;
using aisolver::nn::Network;
using aisolver::nn::WeightInitScheme;

namespace {

class TempDirectory {
public:
    TempDirectory()
        : _path(std::filesystem::temp_directory_path() / "aisolver_nn_test_reproducibility") {
        std::filesystem::create_directories(_path);
    }
    ~TempDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }
    TempDirectory(const TempDirectory&) = delete;
    TempDirectory& operator=(const TempDirectory&) = delete;

    [[nodiscard]] std::filesystem::path filePath(const char* name) const {
        return _path / name;
    }

private:
    std::filesystem::path _path;
};

std::unique_ptr<Network> buildMixedNetwork(std::uint64_t seed) {
    Rng rng(seed);
    auto network = std::make_unique<Network>();
    network->addLayer(std::make_unique<Dense>(5, 6, WeightInitScheme::He, rng),
                      aisolver::autodiff::relu);
    network->addLayer(std::make_unique<Dense>(6, 4, WeightInitScheme::Xavier, rng),
                      aisolver::autodiff::tanhOp);
    network->addLayer(std::make_unique<Dense>(4, 3, WeightInitScheme::Xavier, rng),
                      aisolver::nn::sigmoid);
    network->addLayer(std::make_unique<Dense>(3, 3, WeightInitScheme::Xavier, rng),
                      aisolver::nn::softmax);
    return network;
}

NodePtr columnVariable(std::size_t size, float fillValue) {
    Tensor<float> data({size, 1});
    for (std::size_t i = 0; i < size; ++i) {
        data.data()[i] = fillValue;
    }
    return aisolver::autodiff::variable(data);
}

}  // namespace

/**
 * @brief Deux réseaux à quatre couches (`relu`/`tanhOp`/`sigmoid`/`softmax` mêlées), aux poids
 * identiques chargés depuis le même fichier sérialisé, produisent une sortie identique bit à bit
 * sur la même entrée.
 * \castest{<b>Network : reproductibilité stricte bit-à-bit.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un `Network` à quatre couches, le sauvegarder.<br/>2. Construire un
 * second `Network` de structure identique (poids différents), y charger le fichier.<br/>
 * 3. Appeler `forward()` sur les deux, même entrée.<br/>
 * \tattendu Les deux sorties sont identiques élément par élément (`==`, pas seulement
 * approché).}
 */
TEST(NetworkReproducibilityTest, ReproductibiliteStricteBitABit) {
    TempDirectory tempDir;
    const std::filesystem::path filePath = tempDir.filePath("mixed.ainn");

    std::unique_ptr<Network> source = buildMixedNetwork(9001);
    ASSERT_TRUE(aisolver::nn::saveWeights(*source, filePath));

    std::unique_ptr<Network> networkA = buildMixedNetwork(9002);
    std::unique_ptr<Network> networkB = buildMixedNetwork(9003);
    ASSERT_TRUE(aisolver::nn::loadWeights(*networkA, filePath));
    ASSERT_TRUE(aisolver::nn::loadWeights(*networkB, filePath));

    const NodePtr input = columnVariable(5, 0.3f);
    const NodePtr outputA = networkA->forward(input);
    const NodePtr outputB = networkB->forward(input);

    ASSERT_EQ(outputA->value.shape(), outputB->value.shape());
    for (std::size_t i = 0; i < outputA->value.size(); ++i) {
        EXPECT_EQ(outputA->value.data()[i], outputB->value.data()[i]);
    }
}

/**
 * @brief Un réseau à quatre couches mêlées, sauvegardé puis rechargé, produit une sortie
 * identique à l'original pour la même entrée.
 * \castest{<b>Network : sauvegarde/rechargement produit la même sortie (réseau mêlé).</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un `Network` à quatre couches, appeler `forward()`.<br/>2. Le
 * sauvegarder puis le recharger dans un `Network` de structure identique.<br/>3. Appeler
 * `forward()` sur le réseau rechargé, même entrée.<br/>
 * \tattendu Les deux sorties sont identiques.}
 */
TEST(NetworkReproducibilityTest, SauvegardeRechargementReseauMele) {
    TempDirectory tempDir;
    const std::filesystem::path filePath = tempDir.filePath("mixed_roundtrip.ainn");

    std::unique_ptr<Network> original = buildMixedNetwork(9004);
    const NodePtr input = columnVariable(5, -0.2f);
    const NodePtr originalOutput = original->forward(input);

    ASSERT_TRUE(aisolver::nn::saveWeights(*original, filePath));

    std::unique_ptr<Network> reloaded = buildMixedNetwork(9005);
    ASSERT_TRUE(aisolver::nn::loadWeights(*reloaded, filePath));
    const NodePtr reloadedOutput = reloaded->forward(input);

    for (std::size_t i = 0; i < originalOutput->value.size(); ++i) {
        EXPECT_EQ(originalOutput->value.data()[i], reloadedOutput->value.data()[i]);
    }
}
