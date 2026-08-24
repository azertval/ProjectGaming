// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Nn/WeightInit.h"

/**
 * @file AiSolver/Training/Evolutionary/NetworkTopology.h
 * @brief Description déclarative d'un `nn::Network` et fabrique associée (`LOT-ANNEXE-10`).
 *
 * `nn::Network` (`LOT-ANNEXE-03`) n'est ni copiable ni introspectable après construction (aucun
 * accès à la forme/activation d'une couche une fois ajoutée) : un algorithme évolutionniste a
 * pourtant besoin de reconstruire plusieurs réseaux de topologie strictement identique (population
 * initiale, enfants de croisement, clone de l'élite). `NetworkTopology` porte cette description une
 * seule fois ; `buildNetwork` la matérialise à la demande.
 */

namespace aisolver::training::evolutionary {

/// Description d'une couche : dimensions, schéma d'initialisation des poids et activation.
struct LayerTopology {
    std::size_t inputSize;
    std::size_t outputSize;
    nn::WeightInitScheme initScheme;
    nn::Network::ActivationFn activation;
};

/// Description complète d'un réseau : une couche par élément, dans l'ordre d'empilement.
using NetworkTopology = std::vector<LayerTopology>;

/**
 * @brief Construit un `nn::Network` neuf conforme à @p topology.
 * @param topology Description des couches, dans l'ordre d'empilement.
 * @param rng      Source d'aléatoire déterministe consommée par l'initialisation de chaque couche,
 *                 dans l'ordre de @p topology (même graine + même topologie ⇒ mêmes poids).
 */
[[nodiscard]] std::unique_ptr<nn::Network> buildNetwork(const NetworkTopology& topology, Rng& rng);

/// Taille de la couche cachée par défaut de `policyTopology` : compromis capacité/coût de
/// propagation avant pour une politique évaluée des milliers de fois par entraînement.
inline constexpr std::size_t DEFAULT_HIDDEN_SIZE = 16;

/**
 * @brief Topologie standard « politique » du programme Lot-Annexe : une couche cachée `tanh`, une
 * couche de sortie `softmax` de taille `actionCount()` (`AiSolver/Env/ActionSpace.h`).
 * @param inputSize  Taille du vecteur d'observation (`ObservationEncoder::inputSize()`).
 * @param hiddenSize Taille de la couche cachée.
 */
[[nodiscard]] NetworkTopology policyTopology(std::size_t inputSize,
                                             std::size_t hiddenSize = DEFAULT_HIDDEN_SIZE);

}  // namespace aisolver::training::evolutionary
