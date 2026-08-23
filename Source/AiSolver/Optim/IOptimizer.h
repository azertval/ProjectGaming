// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"

/**
 * @file AiSolver/Optim/IOptimizer.h
 * @brief Interface commune des optimiseurs de descente de gradient (LOT-ANNEXE-04).
 */

namespace aisolver::optim {

/**
 * @brief Règle de mise à jour de paramètres à partir de leur gradient accumulé.
 *
 * Ne connaît que des `autodiff::Node` porteurs de gradient (`nn::Network::parameters()`), jamais
 * le réseau ni l'algorithme d'apprentissage qui les a produits. `step` lit `Node::grad()` et écrit
 * directement dans `Node::value()`, en dehors de toute construction de graphe : un optimiseur
 * n'est jamais lui-même différentié.
 */
class IOptimizer {
public:
    virtual ~IOptimizer() = default;

    /**
     * @brief Applique une mise à jour à partir des gradients déjà accumulés sur chaque paramètre.
     * @param parameters Paramètres à mettre à jour (ex. `nn::Network::parameters()`).
     */
    virtual void step(const std::vector<autodiff::NodePtr>& parameters) = 0;

    /**
     * @brief Remet à zéro le gradient de chaque paramètre fourni.
     *
     * Jamais implicite dans `step()` : un appelant qui accumule des gradients sur plusieurs passes
     * avant une mise à jour reste libre de ne pas appeler `zeroGrad` entre deux `step`.
     * @param parameters Paramètres dont le gradient est remis à zéro.
     */
    virtual void zeroGrad(const std::vector<autodiff::NodePtr>& parameters) = 0;
};

}  // namespace aisolver::optim
