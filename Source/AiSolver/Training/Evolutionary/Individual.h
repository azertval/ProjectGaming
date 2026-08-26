// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <limits>
#include <memory>
#include <utility>

#include "AiSolver/Nn/Network.h"

/**
 * @file AiSolver/Training/Evolutionary/Individual.h
 * @brief Un individu de population : un réseau de neurones et son dernier fitness connu
 * (`LOT-ANNEXE-10`, `EX-IA-011`).
 */

namespace aisolver::training::evolutionary {

/// Fitness d'un individu jamais encore évalué : distinct de toute valeur atteignable par
/// `Reward.h` (`LOT-ANNEXE-08`), qui ne produit jamais `-infini`.
inline constexpr float UNEVALUATED_FITNESS = -std::numeric_limits<float>::infinity();

/**
 * @brief Politique **pure** : poids propres (copie indépendante, jamais partagée) et dernier
 * fitness connu. Ne référence jamais un environnement ni un état d'exécution — l'évaluation
 * (`FitnessEvaluator.h`) est un aller simple (réseau + niveau → fitness), jamais l'inverse.
 */
class Individual {
public:
    explicit Individual(std::unique_ptr<nn::Network> network) : _network(std::move(network)) {}

    [[nodiscard]] nn::Network& network() {
        return *_network;
    }

    [[nodiscard]] const nn::Network& network() const {
        return *_network;
    }

    /// Récompense cumulée du dernier run complet, ou `UNEVALUATED_FITNESS` tant qu'aucun run n'a
    /// eu lieu (`FitnessEvaluator::evaluateFitness`).
    float fitness = UNEVALUATED_FITNESS;

private:
    std::unique_ptr<nn::Network> _network;
};

}  // namespace aisolver::training::evolutionary
