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
 * @file AiSolver/Training/Advanced/QNetwork.h
 * @brief Réseau de valeur d'action `Q(s, ·)` (`LOT-ANNEXE-14`, TACHE-01, `EX-IA-015`).
 */

namespace aisolver::training {

/**
 * @brief Réseau distinct de la politique/du critique (`nn::Network`, `LOT-ANNEXE-03`), une sortie
 * par action de l'espace discret (`ActionSpace::actionCount()`, `LOT-ANNEXE-07`), aucune activation
 * finale (valeurs `Q(s, a)` non bornées, pas une distribution de probabilité).
 *
 * Même schéma d'initialisation (Xavier) que `CriticNetwork`/la politique évolutionniste
 * (`policyTopology`) : aucun schéma spécifique introduit pour DQN.
 */
class QNetwork {
public:
    /// Même taille de couche cachée par défaut que `CriticNetwork` (comparaison de génération 3 à
    /// capacité de réseau comparable, TACHE-03).
    static constexpr std::size_t kDefaultHiddenSize = 16;

    /**
     * @param inputSize  Taille du vecteur d'observation encodé (`ObservationEncoder::inputSize()`).
     * @param hiddenSize Taille de la couche cachée.
     * @param rng        Source d'aléatoire déterministe consommée par l'initialisation des poids.
     */
    QNetwork(std::size_t inputSize, std::size_t hiddenSize, Rng& rng);

    /**
     * @brief Passage avant unique : `Q(s, ·)` pour toutes les actions de l'espace discret.
     * @return Nœud de graphe d'autodiff, forme `[actionCount(), 1]`.
     */
    [[nodiscard]] autodiff::NodePtr forward(const Tensor<float>& observation);

    /// @return Paramètres du réseau, dans l'ordre de `nn::Network::parameters()`.
    [[nodiscard]] std::vector<autodiff::NodePtr> parameters() const;

    /**
     * @brief Copie complète des poids de @p source vers `*this` (synchronisation du réseau cible,
     * `LOT-ANNEXE-14` TACHE-01).
     *
     * Copie profonde (`Tensor::clone()`) : `*this` reste ensuite entièrement indépendant de
     * @p source (aucun tampon partagé), condition nécessaire pour que les mises à jour ultérieures
     * du réseau principal ne fassent pas dériver le réseau cible entre deux synchronisations.
     * @pre @p source a une topologie identique (même nombre de paramètres, mêmes formes).
     */
    void copyWeightsFrom(const QNetwork& source);

    /// @return Le réseau interne, pour la (dé)sérialisation des poids (`nn::saveWeights`/
    /// `loadWeights`, `LOT-ANNEXE-19`) — même patron d'accès que `evolutionary::Individual::network()`.
    [[nodiscard]] nn::Network& network() noexcept {
        return _network;
    }

    /// @copydoc network()
    [[nodiscard]] const nn::Network& network() const noexcept {
        return _network;
    }

private:
    nn::Network _network;
};

}  // namespace aisolver::training
