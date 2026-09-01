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

/// Moyenne et écart-type d'un lot de poids de policy gradient (retours ou avantages).
struct WeightStatistics {
    float mean = 0.0f;
    float standardDeviation = 0.0f;
};

/**
 * @brief Moyenne et écart-type de **tous** les poids d'un lot d'épisodes.
 * @param batch Poids par épisode ; les épisodes vides sont ignorés.
 * @return Les statistiques du lot ; `{0, 0}` si le lot ne contient aucun poids.
 */
[[nodiscard]] WeightStatistics weightStatistics(const std::vector<std::vector<float>>& batch);

/**
 * @brief Centre et réduit @p weights d'après @p statistics, en place.
 *
 * Sans cette normalisation, un lot dont **tous** les retours sont négatifs — le cas ordinaire d'un
 * niveau que l'agent ne finit pas — ne produit que des mises à jour qui *découragent* les actions
 * tirées, jamais aucune qui en encourage une. La probabilité se déplace alors vers les actions que
 * le hasard n'a pas échantillonnées, l'entropie s'effondre en quelques dizaines d'épisodes, et la
 * politique se fige sur une trajectoire unique quel que soit le nombre d'épisodes restants.
 * Centrer les retours rétablit la moitié manquante du signal : ce qui compte est d'avoir fait
 * *mieux que la moyenne du lot*, pas d'avoir obtenu un retour positif.
 * @param weights    Poids d'un épisode, modifiés en place.
 * @param statistics Statistiques du lot (`weightStatistics`).
 */
void normalizeWeights(std::vector<float>& weights, WeightStatistics statistics);

}  // namespace aisolver::training
