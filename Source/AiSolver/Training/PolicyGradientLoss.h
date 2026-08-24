// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

/**
 * @file AiSolver/Training/PolicyGradientLoss.h
 * @brief Formule de perte de policy gradient commune (`LOT-ANNEXE-13`, TACHE-02), partagée entre
 * `Training/PolicyGradient` (poids = retour brut, `LOT-ANNEXE-12`) et `Training/ActorCritic` (poids
 * = avantage, `LOT-ANNEXE-13`) sans faire dépendre l'un de l'autre.
 */

namespace aisolver::training {

/**
 * @brief Construit le graphe `mean_t(-log(pi(a_t|s_t)) * weights[t])` sur `trajectory`.
 *
 * Rejoue le passage avant de `policy` pas à pas (poids actuels) : jamais de dépendance à
 * `TrajectoryStep::logProbability`, valeur détachée sans historique de graphe. Utilisée telle
 * quelle par `computeReinforceLoss` (`weights` = retours) et `computeActorCriticLoss` (`weights` =
 * avantages) — seule la nature du poids par pas change, jamais la formule.
 * @param policy     Réseau rejoué pas à pas (poids **non modifiés** par cet appel).
 * @param trajectory Trajectoire collectée (`TrajectoryCollector`).
 * @param weights    Poids par pas (retour ou avantage), même longueur que `trajectory.steps`.
 * @return Nœud scalaire (forme `[1]`) de la perte moyenne, prêt pour `autodiff::backward()`.
 * @pre `weights.size() == trajectory.steps.size()`, `!trajectory.steps.empty()`.
 */
[[nodiscard]] autodiff::NodePtr computeWeightedPolicyGradientLoss(
    nn::Network& policy, const Trajectory& trajectory, const std::vector<float>& weights);

}  // namespace aisolver::training
