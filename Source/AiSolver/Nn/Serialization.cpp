// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Nn/Serialization.h"

#include <cstdint>
#include <fstream>
#include <utility>
#include <vector>

namespace aisolver::nn {

namespace {

void writeUint32(std::ofstream& stream, std::uint32_t value) {
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void writeUint64(std::ofstream& stream, std::uint64_t value) {
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void writeTensor(std::ofstream& stream, const Tensor<float>& tensor) {
    writeUint64(stream, static_cast<std::uint64_t>(tensor.rank()));
    for (std::size_t dimension : tensor.shape()) {
        writeUint64(stream, static_cast<std::uint64_t>(dimension));
    }
    stream.write(reinterpret_cast<const char*>(tensor.data()),
                 static_cast<std::streamsize>(tensor.size() * sizeof(float)));
}

bool readUint32(std::ifstream& stream, std::uint32_t& value) {
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return static_cast<bool>(stream);
}

bool readUint64(std::ifstream& stream, std::uint64_t& value) {
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return static_cast<bool>(stream);
}

/// Rang maximal accepté à la lecture. Le format n'écrit que des tenseurs de rang 1 (biais) ou 2
/// (poids) ; la marge couvre une évolution du format sans jamais laisser un fichier corrompu
/// dimensionner une allocation.
constexpr std::uint64_t MAX_TENSOR_RANK = 4;

/// @return Nombre d'octets restant à lire dans @p stream, position courante rétablie.
std::streamoff remainingBytes(std::ifstream& stream) {
    const std::streampos current = stream.tellg();
    stream.seekg(0, std::ios::end);
    const std::streampos end = stream.tellg();
    stream.seekg(current);
    return end - current;
}

/// Lit une forme (rang puis dimensions) et les valeurs brutes qui suivent ; échoue proprement
/// (retourne `false`, `tensor` non modifié) sur flux tronqué, sans jamais lancer.
///
/// Rang et dimensions viennent du fichier : un fichier corrompu annonçant un rang énorme, ou des
/// dimensions dont le produit dépasse la mémoire, provoquerait une allocation gigantesque **avant**
/// que la lecture n'échoue. Les deux sont donc validés d'abord -- le rang contre une borne, la
/// taille annoncée contre ce que le fichier contient réellement.
bool readTensor(std::ifstream& stream, Tensor<float>& tensor) {
    std::uint64_t rank = 0;
    if (!readUint64(stream, rank)) {
        return false;
    }
    if (rank == 0 || rank > MAX_TENSOR_RANK) {
        return false;
    }
    std::vector<std::size_t> shape(static_cast<std::size_t>(rank));
    std::uint64_t elementCount = 1;
    for (std::uint64_t axis = 0; axis < rank; ++axis) {
        std::uint64_t dimension = 0;
        if (!readUint64(stream, dimension)) {
            return false;
        }
        if (dimension == 0) {
            return false;
        }
        // Produit borné avant multiplication : sinon un debordement rendrait la verification de
        // taille ci-dessous inoperante.
        if (dimension > UINT64_MAX / elementCount) {
            return false;
        }
        elementCount *= dimension;
        shape[static_cast<std::size_t>(axis)] = static_cast<std::size_t>(dimension);
    }

    const std::streamoff available = remainingBytes(stream);
    if (available < 0 || elementCount > static_cast<std::uint64_t>(available) / sizeof(float)) {
        return false;  // le fichier ne porte pas les valeurs qu'il annonce
    }

    Tensor<float> loaded(shape);
    stream.read(reinterpret_cast<char*>(loaded.data()),
                static_cast<std::streamsize>(loaded.size() * sizeof(float)));
    if (!stream) {
        return false;
    }
    tensor = std::move(loaded);
    return true;
}

}  // namespace

bool saveWeights(const Network& network, const std::filesystem::path& path) {
    std::ofstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return false;
    }

    writeUint32(stream, WEIGHTS_FILE_MAGIC);
    writeUint32(stream, WEIGHTS_FILE_VERSION);
    writeUint64(stream, static_cast<std::uint64_t>(network.layerCount()));

    const std::vector<autodiff::NodePtr> parameters = network.parameters();
    for (std::size_t layer = 0; layer < network.layerCount(); ++layer) {
        writeTensor(stream, parameters[2 * layer]->value);
        writeTensor(stream, parameters[2 * layer + 1]->value);
    }

    return static_cast<bool>(stream);
}

bool loadWeights(Network& network, const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return false;
    }

    std::uint32_t magic = 0;
    std::uint32_t version = 0;
    if (!readUint32(stream, magic) || magic != WEIGHTS_FILE_MAGIC) {
        return false;
    }
    if (!readUint32(stream, version) || version != WEIGHTS_FILE_VERSION) {
        return false;
    }
    std::uint64_t layerCount = 0;
    if (!readUint64(stream, layerCount) ||
        layerCount != static_cast<std::uint64_t>(network.layerCount())) {
        return false;
    }

    const std::vector<autodiff::NodePtr> parameters = network.parameters();
    std::vector<Tensor<float>> loadedValues;
    loadedValues.reserve(parameters.size());
    for (std::size_t layer = 0; layer < network.layerCount(); ++layer) {
        Tensor<float> weights({0});
        Tensor<float> bias({0});
        if (!readTensor(stream, weights) ||
            weights.shape() != parameters[2 * layer]->value.shape()) {
            return false;
        }
        if (!readTensor(stream, bias) || bias.shape() != parameters[2 * layer + 1]->value.shape()) {
            return false;
        }
        loadedValues.push_back(std::move(weights));
        loadedValues.push_back(std::move(bias));
    }

    // Toutes les formes sont validees : applique maintenant, network ne peut plus echouer a
    // mi-chemin (cf. points d'attention de la tache).
    for (std::size_t i = 0; i < parameters.size(); ++i) {
        parameters[i]->value = std::move(loadedValues[i]);
    }
    return true;
}

}  // namespace aisolver::nn
