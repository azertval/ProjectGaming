// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Training/PolicyGradient/Trajectory.h"

/**
 * @file AiSolver/Training/PolicyGradient/ReturnCalculator.h
 * @brief Calcul du retour actualisé par pas d'une trajectoire (`LOT-ANNEXE-12`, TACHE-02,
 * `EX-IA-013`).
 */

namespace aisolver::training {

/**
 * @brief Retour actualisé `G_t = reward_t + gamma * G_{t+1}` (`G_T = reward_T` au dernier pas), par
 * parcours arrière linéaire de `trajectory` — fonction pure, ne dépend d'aucun état d'environnement
 * ni de réseau.
 * @param trajectory Trajectoire déjà collectée (`TrajectoryCollector`).
 * @param gamma      Facteur d'actualisation, dans `[0, 1]` (`PROJECTGAMING_ASSERT` sinon).
 * @return Un retour par pas, même longueur et même ordre chronologique que `trajectory.steps`.
 */
[[nodiscard]] std::vector<float> computeReturns(const Trajectory& trajectory, float gamma);

}  // namespace aisolver::training
