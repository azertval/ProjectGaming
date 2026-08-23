// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"

/**
 * @file AiSolver/Nn/WeightInit.h
 * @brief Schémas d'initialisation des poids d'une couche dense (LOT-ANNEXE-03).
 */

namespace aisolver::nn {

/**
 * @brief Schéma d'initialisation des poids d'une couche dense.
 *
 * Le choix est un paramètre explicite à la construction de chaque `Dense` (jamais déduit de
 * l'activation qui suivra, que la couche ne connaît pas à sa construction) : `Xavier` pour une
 * activation `tanh`/`sigmoid`, `He` pour une activation `relu` (recommandation standard).
 */
enum class WeightInitScheme {
    /// Tirage uniforme dans `[-bound, bound]`, `bound = sqrt(6 / (fanIn + fanOut))` (Glorot & Bengio, 2010).
    Xavier,
    /// Tirage gaussien de moyenne `0` et d'écart-type `sqrt(2 / fanIn)` (He, Zhang, Ren, Sun, 2015).
    He,
};

/**
 * @brief Initialise `weights` en place selon `scheme`, `fanIn`/`fanOut` déduits de sa forme
 * `[outputSize, inputSize]` (convention posée par `Dense`, `fanOut = outputSize`, `fanIn =
 * inputSize`).
 * @param weights Tenseur de rang 2 à initialiser en place.
 * @param scheme  Schéma d'initialisation à appliquer.
 * @param rng     Source d'aléatoire déterministe (`LOT-ANNEXE-01`) ; deux appels avec des `Rng` de
 *                même graine produisent des poids identiques.
 */
void initializeWeights(Tensor<float>& weights, WeightInitScheme scheme, Rng& rng);

}  // namespace aisolver::nn
