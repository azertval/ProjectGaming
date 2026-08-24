// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

/**
 * @file AiSolver/Training/PolicyGradient/ReinforceLoss.h
 * @brief Perte REINFORCE construite comme un graphe d'autodiff (`LOT-ANNEXE-12`, TACHE-03,
 * `EX-IA-013`) — la tâche centrale du lot.
 */

namespace aisolver::training {

/**
 * @brief Construit le graphe de la perte REINFORCE moyenne sur l'épisode :
 * `mean_t(-log(pi(a_t|s_t)) * G_t)`.
 *
 * Délègue à `computeWeightedPolicyGradientLoss` (`Training/PolicyGradientLoss.h`,
 * `LOT-ANNEXE-13` TACHE-02 : formule factorisée avec `computeActorCriticLoss`, poids = retour brut
 * ici plutôt que l'avantage) — comportement strictement inchangé depuis `LOT-ANNEXE-12`.
 *
 * Convention de signe : la perte est déjà le **négatif** de l'objectif de policy gradient (à
 * minimiser par un optimiseur de descente de gradient standard, `LOT-ANNEXE-04`).
 * @param policy     Réseau de politique, rejoué pas à pas (poids **non modifiés** par cet appel).
 * @param trajectory Trajectoire collectée (`TrajectoryCollector`).
 * @param returns    Retour par pas, même longueur que `trajectory.steps` (`computeReturns`).
 * @return Nœud scalaire (forme `[1]`) de la perte moyenne, prêt pour `autodiff::backward()`.
 * @pre `returns.size() == trajectory.steps.size()`, `!trajectory.steps.empty()`.
 */
[[nodiscard]] autodiff::NodePtr computeReinforceLoss(nn::Network& policy,
                                                     const Trajectory& trajectory,
                                                     const std::vector<float>& returns);

}  // namespace aisolver::training
