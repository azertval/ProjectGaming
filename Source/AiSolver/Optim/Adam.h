// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <unordered_map>
#include <vector>

#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Optim/IOptimizer.h"

/**
 * @file AiSolver/Optim/Adam.h
 * @brief Optimiseur Adam (moments d'ordre 1/2, correction de biais) (LOT-ANNEXE-04).
 */

namespace aisolver::optim {

/**
 * @brief Adam : moyennes mobiles du gradient et de son carré, normalisation par leur amplitude
 * récente — plus robuste au choix du taux d'apprentissage que `Sgd`, au prix d'un état interne
 * plus riche par paramètre.
 *
 * L'état (moments `m`/`v`, compteur de pas) persiste d'un appel à `step` à l'autre : une même
 * instance ne doit servir qu'un seul réseau (le compteur de pas est partagé par tous les
 * paramètres passés à `step`, pas remis à zéro entre deux réseaux distincts).
 */
class Adam final : public IOptimizer {
public:
    /**
     * @brief Construit l'optimiseur.
     * @param learningRate Taux d'apprentissage.
     * @param beta1        Coefficient de la moyenne mobile du premier moment (gradient).
     * @param beta2        Coefficient de la moyenne mobile du second moment (carré du gradient).
     * @param epsilon      Terme additif au dénominateur, évite la division par une valeur nulle.
     */
    explicit Adam(float learningRate = 0.001f, float beta1 = 0.9f, float beta2 = 0.999f,
                  float epsilon = 1e-8f);

    void step(const std::vector<autodiff::NodePtr>& parameters) override;
    void zeroGrad(const std::vector<autodiff::NodePtr>& parameters) override;

private:
    struct Moments {
        Tensor<float> m;
        Tensor<float> v;
    };

    float _learningRate;
    float _beta1;
    float _beta2;
    float _epsilon;

    /// Compteur de pas, partagé par tous les paramètres ; incrémenté une fois par appel à `step`.
    int _stepCount = 0;

    /// Moments par paramètre, indexés par l'adresse du `Node` (même mécanisme que la vitesse de
    /// `Sgd` ; pas l'adresse du tampon de valeur, qui change à chaque `step`).
    std::unordered_map<const void*, Moments> _moments;
};

}  // namespace aisolver::optim
