// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Training/Advanced/QNetwork.h"
#include "AiSolver/Training/Advanced/ReplayBuffer.h"

/**
 * @file AiSolver/Training/Advanced/DqnLoss.h
 * @brief Perte de Bellman (erreur quadratique) de DQN (`LOT-ANNEXE-14`, TACHE-01, `EX-IA-015`).
 */

namespace aisolver::training {

/**
 * @brief Erreur quadratique moyenne, sur un mini-lot, entre `Q(s_t, a_t)` (réseau principal, nœud
 * de graphe d'autodiff) et la cible de Bellman `reward_t + gamma * max_a Q_cible(s_{t+1}, a)`
 * (réseau cible, valeur détachée du graphe -- jamais rétropropagée, même convention que
 * `computeAdvantages`/`CriticLoss`, `LOT-ANNEXE-13`).
 *
 * Pour une transition terminale (`Transition::done == true`), la cible se réduit à `reward_t` (pas
 * de `max_a Q_cible` au-delà d'un état terminal).
 *
 * @param mainNetwork   Réseau principal, entraîné en place -- seul réseau dont le gradient est
 *                       accumulé par cette perte.
 * @param targetNetwork Réseau cible, poids gelés entre deux synchronisations
 *                       (`QNetwork::copyWeightsFrom`) -- son passage avant est calculé mais jamais
 *                       rétropropagé.
 * @param batch         Mini-lot échantillonné (`ReplayBuffer::sample`) ; `PROJECTGAMING_ASSERT` non
 *                       vide.
 * @param gamma         Facteur d'actualisation, dans `[0, 1]`.
 * @return Nœud scalaire (forme `[1]`), moyenné sur `batch.size()`.
 */
[[nodiscard]] autodiff::NodePtr computeDqnLoss(QNetwork& mainNetwork, QNetwork& targetNetwork,
                                               const std::vector<Transition>& batch, float gamma);

}  // namespace aisolver::training
