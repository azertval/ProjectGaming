// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <unordered_map>
#include <vector>

#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Optim/IOptimizer.h"

/**
 * @file AiSolver/Optim/Sgd.h
 * @brief Descente de gradient stochastique, avec inertie (*momentum*) optionnelle (LOT-ANNEXE-04).
 */

namespace aisolver::optim {

/**
 * @brief SGD : `value -= learningRate * grad`, avec un terme d'inertie optionnel.
 *
 * Avec `momentum == 0.0f` (valeur par défaut), la mise à jour n'accumule aucun état : c'est le
 * chemin le plus simple, utilisé comme référence de calibration. Avec `momentum > 0.0f`, une
 * vitesse est maintenue par paramètre (indexée par l'adresse du `Node`, stable pour toute la durée
 * de vie d'un `nn::Dense` — pas par l'adresse de son tampon de valeur, qui change à chaque `step`
 * puisque `value` est réassignée à un nouveau `Tensor`) : `vitesse = momentum * vitesse -
 * learningRate * grad ; value += vitesse`.
 */
class Sgd final : public IOptimizer {
public:
    /**
     * @brief Construit l'optimiseur.
     * @param learningRate Taux d'apprentissage, appliqué à chaque paramètre.
     * @param momentum     Coefficient d'inertie ; `0.0f` désactive la table de vitesse.
     */
    explicit Sgd(float learningRate, float momentum = 0.0f);

    void step(const std::vector<autodiff::NodePtr>& parameters) override;
    void zeroGrad(const std::vector<autodiff::NodePtr>& parameters) override;

private:
    float _learningRate;
    float _momentum;

    /// Vitesse par paramètre, indexée par l'adresse du `Node` ; vide et inutilisée tant que
    /// `_momentum == 0.0f`.
    std::unordered_map<const void*, Tensor<float>> _velocity;
};

}  // namespace aisolver::optim
