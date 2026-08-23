// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/Aabb.h"

/**
 * @file AiSolver/Env/Reward.h
 * @brief Signal de récompense unique et partagé de tous les algorithmes d'apprentissage du
 * programme (`LOT-ANNEXE-08`, `EX-IA-009`).
 */

namespace aisolver {

/**
 * @brief Constantes de la fonction de récompense (`EX-IA-009`).
 *
 * Valeurs par défaut choisies pour que `completionBonus` domine significativement la somme
 * plausible des autres termes sur un épisode typique (quelques centaines de pas) : à
 * `timePenalty = 0.01` et `progressScale = 1.0`, un épisode de `1000` pas perd au plus `10` en
 * pénalité de temps et gagne au plus quelques dizaines en progression cumulée (distance typique
 * d'un niveau `demo-*.json`), largement dominé par le bonus de complétion de `100`.
 */
struct RewardConfig {
    /// Multiplicateur de la récompense de progression (diminution de distance à la sortie).
    float progressScale = 1.0f;
    /// Récompense fixe accordée uniquement à `core::LevelOutcome::Won`.
    float completionBonus = 100.0f;
    /// Récompense (négative) accordée uniquement à `core::LevelOutcome::Lost`.
    float deathPenalty = -10.0f;
    /// Pénalité (positive, soustraite) appliquée à chaque pas, quelle que soit l'issue.
    float timePenalty = 0.01f;
};

/**
 * @brief Calcule la récompense d'un pas unique, fonction pure sans état interne ni effet de bord.
 *
 * La distance à la sortie est calculée en ligne droite (norme euclidienne) entre le centre de la
 * boîte du personnage et le centre de la case de sortie, jamais par un plus court chemin sur la
 * grille (décision de cadrage de l'épic `LOT-ANNEXE-08`).
 * @param config Constantes de la fonction de récompense.
 * @param previousBox Boîte du personnage avant ce pas.
 * @param currentBox Boîte du personnage après ce pas.
 * @param exit Position de la case de sortie du niveau (`core::Level::exit()`).
 * @param outcome Issue du niveau à l'issue de ce pas.
 * @return La récompense scalaire de ce pas.
 */
[[nodiscard]] float computeReward(const RewardConfig& config, const core::Aabb& previousBox,
                                  const core::Aabb& currentBox, const core::GridPosition& exit,
                                  core::LevelOutcome outcome);

}  // namespace aisolver
