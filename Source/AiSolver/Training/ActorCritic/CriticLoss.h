// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

/**
 * @file AiSolver/Training/ActorCritic/CriticLoss.h
 * @brief Perte d'entraînement du critique : erreur quadratique moyenne vis-à-vis du retour observé
 * (`LOT-ANNEXE-13`, TACHE-03, `EX-IA-014`).
 */

namespace aisolver::training {

/**
 * @brief Construit le graphe `mean_t((valeur_t − returns[t])^2)`.
 *
 * Rejoue `critic.forward` pas à pas (poids actuels) : graphe d'autodiff **indépendant** de celui de
 * la perte de politique (`ActorCriticLoss.h`) — jamais combinés dans un seul nœud scalaire final
 * (décision de cadrage de l'épic : pas de couplage de gradient entre acteur et critique). Utilise
 * le même retour Monte-Carlo complet (`computeReturns`, `LOT-ANNEXE-12`) que cible, pas une cible
 * bootstrapée.
 * @param critic     Réseau critique, rejoué pas à pas (poids **non modifiés** par cet appel).
 * @param trajectory Trajectoire collectée (`TrajectoryCollector`, `LOT-ANNEXE-12`).
 * @param returns    Retour par pas (`computeReturns`), même longueur que `trajectory.steps`.
 * @return Nœud scalaire (forme `[1]`) de la perte moyenne, prêt pour `autodiff::backward()`.
 * @pre `returns.size() == trajectory.steps.size()`, `!trajectory.steps.empty()`.
 */
[[nodiscard]] autodiff::NodePtr computeCriticLoss(CriticNetwork& critic,
                                                  const Trajectory& trajectory,
                                                  const std::vector<float>& returns);

}  // namespace aisolver::training
