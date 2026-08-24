// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/WeightInit.h"

/**
 * @file AiSolver/Nn/Dense.h
 * @brief Couche dense (transformation affine) du réseau de neurones maison (LOT-ANNEXE-03).
 */

namespace aisolver::nn {

/**
 * @brief Couche dense (entièrement connectée) : `y = W·x + b`, poids et biais différentiables.
 *
 * Non copiable : ses paramètres (`autodiff::NodePtr`) sont créés une seule fois à la construction
 * et vivent pour toute la durée de vie de la couche — un entraînement ajuste les **mêmes**
 * paramètres sur plusieurs passes, jamais des copies. Aucune activation appliquée : une
 * transformation affine pure, l'activation étant la responsabilité de l'appelant (`Network`,
 * `Activations.h`).
 */
class Dense {
public:
    Dense(const Dense&) = delete;
    Dense& operator=(const Dense&) = delete;

    /**
     * @brief Alloue et initialise les paramètres de la couche.
     * @param inputSize  Dimension de l'entrée attendue par `forward()`.
     * @param outputSize Dimension de la sortie produite par `forward()`.
     * @param scheme     Schéma d'initialisation des poids (`WeightInit.h`) ; le biais est
     * initialisé à zéro, indépendamment du schéma.
     * @param rng        Source d'aléatoire déterministe consommée par l'initialisation.
     */
    Dense(std::size_t inputSize, std::size_t outputSize, WeightInitScheme scheme, Rng& rng);

    /**
     * @brief Passe avant : `add(matmul(weights, input), bias)`.
     * @param input Nœud de forme `[inputSize, 1]`.
     * @return Nœud de forme `[outputSize, 1]`, relié au graphe d'autodiff (`LOT-ANNEXE-02`).
     */
    [[nodiscard]] autodiff::NodePtr forward(const autodiff::NodePtr& input);

    /// @return `{poids, biais}`, dans cet ordre — ordre stable, réutilisé par
    /// `Network::parameters()` et la sérialisation (`Serialization.h`).
    [[nodiscard]] std::vector<autodiff::NodePtr> parameters() const;

    /// @return Valeur courante des poids, forme `[outputSize, inputSize]`.
    [[nodiscard]] const Tensor<float>& weights() const;

    /// @return Valeur courante du biais, forme `[outputSize, 1]`.
    [[nodiscard]] const Tensor<float>& bias() const;

private:
    autodiff::NodePtr _weights;
    autodiff::NodePtr _bias;
};

}  // namespace aisolver::nn
