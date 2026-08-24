// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Env/GridDistanceField.h"
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
 * La distance à la sortie est une distance de plus court chemin sur la grille, respectant les murs
 * (`GridDistanceField`, amendement `LOT-ANNEXE-08` motivé par `EX-IA-023` : la distance euclidienne
 * en ligne droite, utilisée avant cet amendement, pénalisait les pas de détour pourtant nécessaires
 * autour d'un mur, un signal actif contre la bonne politique plutôt que simplement imparfait). Le
 * centre de la boîte du personnage est converti en case de grille (partie entière des coordonnées
 * monde, 1 case = 1 unité monde, même convention que `HMI::GameViewport::cellAt`).
 * @param config Constantes de la fonction de récompense.
 * @param distanceField Champ de distances vers la sortie, précalculé une fois par niveau
 *        (`GridDistanceField`, construit à partir de `core::Level::tileMap()`/`exit()`).
 * @param previousBox Boîte du personnage avant ce pas.
 * @param currentBox Boîte du personnage après ce pas.
 * @param outcome Issue du niveau à l'issue de ce pas.
 * @return La récompense scalaire de ce pas.
 */
[[nodiscard]] float computeReward(const RewardConfig& config,
                                  const GridDistanceField& distanceField,
                                  const core::Aabb& previousBox, const core::Aabb& currentBox,
                                  core::LevelOutcome outcome);

}  // namespace aisolver
