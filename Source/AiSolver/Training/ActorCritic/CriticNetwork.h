// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/Network.h"

/**
 * @file AiSolver/Training/ActorCritic/CriticNetwork.h
 * @brief Réseau critique : estime la valeur d'un état (`LOT-ANNEXE-13`, TACHE-01, `EX-IA-014`).
 */

namespace aisolver::training {

/**
 * @brief Réseau distinct de la politique (`nn::Network`, `LOT-ANNEXE-03`), même bibliothèque de
 * couches, une seule sortie scalaire non bornée (aucune activation finale).
 *
 * Aucun poids partagé avec la politique (décision de cadrage de l'épic) : une instance de
 * `CriticNetwork` possède ses propres paramètres, entièrement indépendants de tout `nn::Network`
 * construit à côté sur la même observation.
 */
class CriticNetwork {
public:
    /// Taille de couche cachée par défaut : comparable à `evolutionary::DEFAULT_HIDDEN_SIZE`, pour
    /// que la comparaison de convergence (TACHE-04) isole l'effet de la réduction de variance, pas
    /// celui d'un critique disproportionnellement plus grand que la politique.
    static constexpr std::size_t kDefaultHiddenSize = 16;

    /**
     * @param inputSize  Taille du vecteur d'observation encodé (`ObservationEncoder::inputSize()`).
     * @param hiddenSize Taille de la couche cachée (configuration séparée de celle de la
     * politique).
     * @param rng        Source d'aléatoire déterministe consommée par l'initialisation des poids
     *                   (même schéma `Xavier` que la politique, `LOT-ANNEXE-03`).
     */
    CriticNetwork(std::size_t inputSize, std::size_t hiddenSize, Rng& rng);

    /**
     * @brief Passage avant unique : encode la valeur estimée de l'état correspondant à
     * @p observation.
     * @return Nœud de graphe d'autodiff, forme `[1]` (un unique scalaire).
     */
    [[nodiscard]] autodiff::NodePtr forward(const Tensor<float>& observation);

    /// @return Paramètres du réseau critique, dans l'ordre de `nn::Network::parameters()`.
    [[nodiscard]] std::vector<autodiff::NodePtr> parameters() const;

private:
    nn::Network _network;
};

}  // namespace aisolver::training
