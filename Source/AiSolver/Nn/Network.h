// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Nn/Dense.h"

/**
 * @file AiSolver/Nn/Network.h
 * @brief Composition de couches `Dense` en un réseau séquentiel (LOT-ANNEXE-03).
 */

namespace aisolver::nn {

/**
 * @brief Séquence de couches `Dense`, chacune suivie d'une activation optionnelle.
 *
 * Non copiable : possède ses couches (RAII, pas de gestion manuelle de durée de vie côté
 * appelant). Ne connaît aucune fonction de perte : `forward()` est sa seule responsabilité, la
 * perte et sa rétropropagation restent à la charge de l'appelant (génération 2/3).
 */
class Network {
public:
    /// Une fonction d'activation quelconque (`autodiff::relu`, `autodiff::tanhOp`, `nn::sigmoid`,
    /// `nn::softmax`, ...), ou `nullptr` pour « aucune activation » (couche de sortie linéaire).
    using ActivationFn = std::function<autodiff::NodePtr(const autodiff::NodePtr&)>;

    Network() = default;
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    /**
     * @brief Ajoute une couche en fin de séquence.
     * @param layer      Couche possédée par `Network` à partir de cet appel.
     * @param activation Appliquée à la sortie de `layer` dans `forward()` ; `nullptr` pour aucune
     *                    activation (sortie affine pure).
     */
    void addLayer(std::unique_ptr<Dense> layer, ActivationFn activation = nullptr);

    /**
     * @brief Passe avant bout-en-bout : enchaîne `couche->forward()` puis son activation (si non
     * nulle) pour chaque couche, dans l'ordre d'ajout ; la sortie d'une couche devient l'entrée de
     * la suivante.
     */
    [[nodiscard]] autodiff::NodePtr forward(const autodiff::NodePtr& input);

    /// @return Concaténation de `parameters()` (`Dense`) de toutes les couches, dans l'ordre
    /// d'ajout — c'est cette liste que consomme l'optimiseur (`LOT-ANNEXE-04`).
    [[nodiscard]] std::vector<autodiff::NodePtr> parameters() const;

    /// @return Nombre de couches, utilisé par la sérialisation (`Serialization.h`, TACHE-04) pour
    /// écrire/valider l'en-tête du fichier ; `parameters()` a alors `2 * layerCount()` éléments
    /// (poids puis biais par couche, `Dense::parameters()`, dans l'ordre d'ajout).
    [[nodiscard]] std::size_t layerCount() const;

private:
    struct Layer {
        std::unique_ptr<Dense> dense;
        ActivationFn activation;
    };

    std::vector<Layer> _layers;
};

}  // namespace aisolver::nn
