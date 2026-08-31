// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"
#include "AiSolver/Training/PolicyGradientLoss.h"

/**
 * @file AiSolver/Training/ActorCritic/ActorCriticLoss.h
 * @brief Perte de politique acteur-critique : `computeReinforceLoss` (`LOT-ANNEXE-12`) où le poids
 * par pas est l'avantage plutôt que le retour brut (`LOT-ANNEXE-13`, TACHE-02, `EX-IA-014`).
 */

namespace aisolver::training {

/**
 * @brief Construit le graphe `mean_t(-log(pi(a_t|s_t)) * advantage_t)`.
 *
 * Délègue à `computeWeightedPolicyGradientLoss` (`Training/PolicyGradientLoss.h`) — même graphe
 * que `computeReinforceLoss`, seul le vecteur de poids par pas change (avantage au lieu du retour
 * brut). Réutilise la politique (`nn::Network`, `LOT-ANNEXE-03`) sans modification.
 * @param policy     Réseau de politique, rejoué pas à pas (poids **non modifiés** par cet appel).
 * @param trajectory Trajectoire collectée (`TrajectoryCollector`, `LOT-ANNEXE-12`).
 * @param advantages Avantage par pas (`computeAdvantages`), même longueur que `trajectory.steps`,
 *        typiquement centré-réduit sur le lot (`normalizeWeights`).
 * @param entropyCoefficient Poids du terme d'entropie, transmis tel quel
 *        (`computeWeightedPolicyGradientLoss`).
 * @return Nœud scalaire (forme `[1]`) de la perte moyenne, prêt pour `autodiff::backward()`.
 * @pre `advantages.size() == trajectory.steps.size()`, `!trajectory.steps.empty()`.
 */
[[nodiscard]] autodiff::NodePtr computeActorCriticLoss(
    nn::Network& policy, const Trajectory& trajectory, const std::vector<float>& advantages,
    float entropyCoefficient = DEFAULT_ENTROPY_COEFFICIENT);

}  // namespace aisolver::training
