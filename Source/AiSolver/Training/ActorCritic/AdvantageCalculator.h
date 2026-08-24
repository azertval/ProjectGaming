// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

/**
 * @file AiSolver/Training/ActorCritic/AdvantageCalculator.h
 * @brief Avantage (`retour − valeur estimée`) remplaçant le retour brut dans la perte de politique
 * (`LOT-ANNEXE-13`, TACHE-02, `EX-IA-014`).
 */

namespace aisolver::training {

/**
 * @brief Calcule `advantage_t = returns[t] − critic.forward(observation_t)`, pour chaque pas.
 *
 * La valeur estimée est **détachée du graphe du critique** (lue via `Node::value`, convertie en
 * `float`) : le gradient de la perte de politique qui consomme ce résultat ne remonte jamais dans
 * les poids du critique (cf. point d'attention de l'épic — pas de fuite de gradient entre les deux
 * réseaux, qui restent optimisés indépendamment).
 * @param returns   Retour par pas (`computeReturns`, `LOT-ANNEXE-12`).
 * @param critic    Réseau critique, rejoué en lecture seule (poids **non modifiés**).
 * @param trajectory Trajectoire collectée (`TrajectoryCollector`, `LOT-ANNEXE-12`).
 * @return Un avantage par pas, même longueur et même ordre que `trajectory.steps`.
 * @pre `returns.size() == trajectory.steps.size()`.
 */
[[nodiscard]] std::vector<float> computeAdvantages(const std::vector<float>& returns,
                                                   CriticNetwork& critic,
                                                   const Trajectory& trajectory);

}  // namespace aisolver::training
