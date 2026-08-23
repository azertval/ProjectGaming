// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

/**
 * @file Core/Levels/DangerGeometry.h
 * @brief Géométrie de la zone mortelle d'une case de danger (`EX-GP-050`).
 */

namespace core {

/**
 * @brief Épaisseur (fraction d'une case) de la bande mortelle d'un danger **directionnel**
 *        (`DangerUp`/`DangerDown`/`DangerLeft`/`DangerRight`, `EX-GP-050`).
 *
 * Le reste de la case est traversable sans risque — des pics contre un bord plutôt qu'une case
 * pleine mortelle sur toute sa surface (`Danger` classique).
 */
inline constexpr float kDangerEdgeThickness = 0.25f;

/**
 * @brief Rectangle **mortel** d'une case de danger, en unités de grille (une case = 1×1, origine
 *        haut-gauche — mêmes unités que `core::Aabb` consommé par `core::evaluateOutcome`).
 *
 * Case pleine pour `Danger` (danger classique) et pour les variantes dont la mortalité est
 * **temporelle** plutôt que géométrique (`DangerMover`, `DangerSwitched`, `DangerBlink` — leur
 * position/activation courante est résolue ailleurs, cette fonction ne décrit que leur silhouette
 * **quand elles sont actives/présentes**, identique à `Danger`). Bande étroite
 * (`kDangerEdgeThickness`) alignée sur le bord désigné pour les quatre variantes directionnelles :
 * `DangerUp`/`DangerDown` sur le bord **haut** / **bas** de la case, `DangerLeft`/`DangerRight`
 * sur le bord **gauche** / **droit** (`EX-GP-050`). Seule source de vérité, partagée par la
 * résolution de fin de niveau (`core::evaluateOutcome`) et l'aperçu visuel de l'éditeur
 * (`hmi::TileVisuals`) — même garantie de non-divergence que `core::tileVisualScale` pour les blocs
 * réduits (`EX-GP-005`).
 * @param type Type de tuile (tout type non listé ci-dessus renvoie la case pleine).
 * @param col  Colonne de la case (unités de grille).
 * @param row  Ligne de la case (unités de grille).
 * @return Le rectangle mortel, en unités de grille.
 */
[[nodiscard]] Aabb dangerHitbox(TileType type, int col, int row) noexcept;

/**
 * @brief Vrai pour les huit types de danger (`Danger` classique et les sept variantes de
 *        `LOT-31`), pour lesquels `dangerHitbox` décrit une géométrie **significative**
 *        (case pleine ou bande directionnelle) — utilisé par le rendu (`core::buildLevelScene`,
 *        `hmi::DraftRenderer`) pour ne recalculer le rectangle affiché qu'à ces types précis,
 *        laissant les autres à leur case pleine par défaut (`Transform` non modifié).
 */
[[nodiscard]] bool isDangerTileType(TileType type) noexcept;

}  // namespace core
