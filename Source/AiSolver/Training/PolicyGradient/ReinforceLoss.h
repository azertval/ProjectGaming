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
 * Pour chaque pas, **rejoue** le passage avant de `policy` sur l'observation enregistrée (poids
 * actuels du réseau) — ne réutilise jamais `TrajectoryStep::logProbability`, qui est une valeur
 * détachée sans historique de graphe. Le nœud renvoyé est un graphe d'autodiff normal :
 * `autodiff::backward()` dessus calcule, par rétropropagation, le gradient par rapport à tous les
 * paramètres de `policy` traversés — aucune formule de gradient écrite à la main ici.
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
