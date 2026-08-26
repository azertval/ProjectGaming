// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_serialization.cpp
 * @brief Tests unitaires de `aisolver::nn::saveWeights`/`loadWeights` (LOT-ANNEXE-03, TACHE-04) :
 * sauvegarde/rechargement, rejet de fichier invalide/absent/incompatible.
 */

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
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

/// Repertoire temporaire du test courant, nettoye a la destruction (RAII), meme si le test echoue.
class TempDirectory {
public:
    TempDirectory()
        : _path(std::filesystem::temp_directory_path() / "aisolver_nn_test_serialization") {
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

std::unique_ptr<Network> buildNetwork(std::uint64_t seed) {
    Rng rng(seed);
    auto network = std::make_unique<Network>();
    network->addLayer(std::make_unique<Dense>(3, 4, WeightInitScheme::He, rng),
                      aisolver::autodiff::relu);
    network->addLayer(std::make_unique<Dense>(4, 2, WeightInitScheme::Xavier, rng), nullptr);
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
 * @brief Un réseau sauvegardé puis rechargé dans un réseau de structure identique produit une
 * sortie identique à l'original, pour la même entrée.
 * \castest{<b>Serialization : sauvegarde puis rechargement produit la même sortie.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un `Network`, appeler `forward()` sur une entrée fixe.<br/>2. Appeler
 * `saveWeights` puis, sur un second `Network` de structure identique (poids différents),
 * `loadWeights`.<br/>3. Appeler `forward()` sur le second réseau, même entrée.<br/>
 * \tattendu Les deux sorties sont identiques.}
 */
TEST(SerializationTest, SauvegardePuisRechargementProduitLaMemeSortie) {
    TempDirectory tempDir;
    const std::filesystem::path filePath = tempDir.filePath("network.ainn");

    std::unique_ptr<Network> original = buildNetwork(8001);
    const NodePtr input = columnVariable(3, 0.6f);
    const NodePtr originalOutput = original->forward(input);

    ASSERT_TRUE(aisolver::nn::saveWeights(*original, filePath));

    std::unique_ptr<Network> reloaded =
        buildNetwork(8002);  // graine differente : poids differents avant chargement.
    ASSERT_TRUE(aisolver::nn::loadWeights(*reloaded, filePath));
    const NodePtr reloadedOutput = reloaded->forward(input);

    ASSERT_EQ(originalOutput->value.shape(), reloadedOutput->value.shape());
    for (std::size_t i = 0; i < originalOutput->value.size(); ++i) {
        EXPECT_EQ(originalOutput->value.data()[i], reloadedOutput->value.data()[i]);
    }
}

/**
 * @brief `loadWeights` sur un fichier au magique incorrect renvoie `false` sans planter.
 * \castest{<b>Serialization : rejet d'un magique incorrect.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Écrire un fichier commençant par un magique arbitraire.<br/>2. Appeler
 * `loadWeights`.<br/>
 * \tattendu `loadWeights` renvoie `false`.}
 */
TEST(SerializationTest, RejetMagiqueIncorrect) {
    TempDirectory tempDir;
    const std::filesystem::path filePath = tempDir.filePath("bad_magic.ainn");
    {
        std::ofstream stream(filePath, std::ios::binary);
        const std::uint32_t wrongMagic = 0xDEADBEEFu;
        stream.write(reinterpret_cast<const char*>(&wrongMagic), sizeof(wrongMagic));
    }

    std::unique_ptr<Network> network = buildNetwork(8003);
    EXPECT_FALSE(aisolver::nn::loadWeights(*network, filePath));
}

/**
 * @brief `loadWeights` sur un fichier à la version inconnue renvoie `false` sans planter.
 * \castest{<b>Serialization : rejet d'une version inconnue.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Écrire un fichier au magique correct mais version `999`.<br/>2. Appeler
 * `loadWeights`.<br/>
 * \tattendu `loadWeights` renvoie `false`.}
 */
TEST(SerializationTest, RejetVersionInconnue) {
    TempDirectory tempDir;
    const std::filesystem::path filePath = tempDir.filePath("bad_version.ainn");
    {
        std::ofstream stream(filePath, std::ios::binary);
        stream.write(reinterpret_cast<const char*>(&aisolver::nn::WEIGHTS_FILE_MAGIC),
                     sizeof(std::uint32_t));
        const std::uint32_t wrongVersion = 999;
        stream.write(reinterpret_cast<const char*>(&wrongVersion), sizeof(wrongVersion));
    }

    std::unique_ptr<Network> network = buildNetwork(8004);
    EXPECT_FALSE(aisolver::nn::loadWeights(*network, filePath));
}

/**
 * @brief `loadWeights` sur un fichier dont la structure (nombre de couches) ne correspond pas au
 * `Network` cible renvoie `false`, sans modifier le réseau (poids inchangés).
 * \castest{<b>Serialization : rejet d'une structure incompatible, réseau inchangé.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Sauvegarder un réseau à deux couches.<br/>2. Appeler `loadWeights` sur un réseau à
 * une seule couche.<br/>
 * \tattendu `loadWeights` renvoie `false` ; le réseau cible garde ses poids d'origine.}
 */
TEST(SerializationTest, RejetStructureIncompatibleReseauInchange) {
    TempDirectory tempDir;
    const std::filesystem::path filePath = tempDir.filePath("two_layers.ainn");

    std::unique_ptr<Network> twoLayers = buildNetwork(8005);
    ASSERT_TRUE(aisolver::nn::saveWeights(*twoLayers, filePath));

    Rng rng(8006);
    Network oneLayer;
    oneLayer.addLayer(std::make_unique<Dense>(3, 4, WeightInitScheme::He, rng),
                      aisolver::autodiff::relu);
    const Tensor<float> weightsBefore = oneLayer.parameters()[0]->value.clone();

    EXPECT_FALSE(aisolver::nn::loadWeights(oneLayer, filePath));

    const Tensor<float>& weightsAfter = oneLayer.parameters()[0]->value;
    for (std::size_t i = 0; i < weightsBefore.size(); ++i) {
        EXPECT_EQ(weightsBefore.data()[i], weightsAfter.data()[i]);
    }
}

/**
 * @brief `loadWeights` sur un chemin inexistant renvoie `false` proprement.
 * \castest{<b>Serialization : rejet d'un fichier absent.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appeler `loadWeights` avec un chemin qui n'existe pas.<br/>
 * \tattendu `loadWeights` renvoie `false`.}
 */
TEST(SerializationTest, RejetFichierAbsent) {
    TempDirectory tempDir;
    std::unique_ptr<Network> network = buildNetwork(8007);

    EXPECT_FALSE(aisolver::nn::loadWeights(*network, tempDir.filePath("n_existe_pas.ainn")));
}

/**
 * @brief Un fichier annoncant des dimensions qu'il ne porte pas est refuse sans allouer.
 * \castest{<b>Serialization : rejet d'une forme surdimensionnee, sans allocation geante.</b><br/>
 * \tcat Unitaire · Nn<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Ecrire un fichier valide en entete, dont le premier tenseur annonce un rang
 * aberrant.<br/>2. Recommencer avec un rang correct mais une dimension enorme, sans les valeurs
 * correspondantes.<br/>3. `loadWeights` sur les deux.<br/>
 * \tattendu Les deux fichiers sont refuses. Rang et dimensions viennent du fichier : sans
 * validation prealable, une forme aberrante dimensionnerait l'allocation avant meme que la lecture
 * n'echoue.}
 */
TEST(SerializationTest, RejetFormeSurdimensionneeSansAllouer) {
    TempDirectory tempDir;

    const auto writeHeader = [](std::ofstream& stream, std::uint32_t layerCount) {
        stream.write(reinterpret_cast<const char*>(&aisolver::nn::WEIGHTS_FILE_MAGIC),
                     sizeof(std::uint32_t));
        stream.write(reinterpret_cast<const char*>(&aisolver::nn::WEIGHTS_FILE_VERSION),
                     sizeof(std::uint32_t));
        stream.write(reinterpret_cast<const char*>(&layerCount), sizeof(layerCount));
    };

    const std::filesystem::path rangAberrant = tempDir.filePath("rang_aberrant.ainn");
    {
        std::ofstream stream(rangAberrant, std::ios::binary);
        writeHeader(stream, 1);
        const std::uint64_t rank = 1ULL << 40;
        stream.write(reinterpret_cast<const char*>(&rank), sizeof(rank));
    }

    const std::filesystem::path dimensionEnorme = tempDir.filePath("dimension_enorme.ainn");
    {
        std::ofstream stream(dimensionEnorme, std::ios::binary);
        writeHeader(stream, 1);
        const std::uint64_t rank = 2;
        const std::uint64_t huge = 1ULL << 40;
        stream.write(reinterpret_cast<const char*>(&rank), sizeof(rank));
        stream.write(reinterpret_cast<const char*>(&huge), sizeof(huge));
        stream.write(reinterpret_cast<const char*>(&huge), sizeof(huge));
    }

    std::unique_ptr<Network> network = buildNetwork(8008);
    EXPECT_FALSE(aisolver::nn::loadWeights(*network, rangAberrant));
    EXPECT_FALSE(aisolver::nn::loadWeights(*network, dimensionEnorme));
}
