// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Math/Tensor.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileMap.h"

/**
 * @file AiSolver/Env/TileWindowEncoder.h
 * @brief Fenêtre de tuiles centrée sur le personnage, encodage catégoriel (LOT-ANNEXE-06).
 */

namespace aisolver {

/**
 * @brief Encode une fenêtre carrée de `core::TileMap`, centrée sur une case, en tenseur one-hot.
 *
 * Traduit l'état statique de la grille (`core::TileType`, sans notion de réseau de neurones) en un
 * tenseur `(channelCount(), windowSize(), windowSize())` : une case `(dc, dr)` relative au centre
 * vaut `1.0f` sur le canal de son `core::TileType`, `0.0f` ailleurs. Aucun ordre numérique n'est
 * suggéré entre catégories (one-hot plutôt qu'un indice scalaire, `EX-IA-006`).
 */
class TileWindowEncoder {
public:
    /**
     * @brief Construit un encodeur pour un rayon de fenêtre donné.
     * @param radius Rayon en cases (`radius >= 0`) ; une fenêtre de rayon `r` couvre
     *               `(2r + 1) × (2r + 1)` cases.
     */
    explicit TileWindowEncoder(int radius);

    /**
     * @brief Encode la fenêtre centrée sur @p center d'une `core::TileMap`.
     *
     * Une case hors limites (`!tiles.inBounds(...)`) est encodée en vecteur nul sur les
     * `channelCount()` canaux (aucune catégorie active) — distinguable de toute case réelle, sans
     * introduire de catégorie « hors limites » supplémentaire dans `Core`.
     * @param tiles  Grille de collision à lire.
     * @param center Case de grille où se trouve le personnage (calcul délégué à l'appelant, donnée
     *               pure).
     * @return Tenseur `(channelCount(), windowSize(), windowSize())`.
     */
    [[nodiscard]] Tensor<float> encode(const core::TileMap& tiles, core::GridPosition center) const;

    /// Nombre de canaux catégoriels = nombre de valeurs de `core::TileType`
    /// (`Core/Levels/TileType.h`). **Non dérivé automatiquement** (`Core` n'expose pas de compte de
    /// catégories) : à mettre à jour manuellement si `TileType` gagne une valeur — risque
    /// documenté, pas mitigé par ce lot (aucune modification de `Core`).
    [[nodiscard]] int channelCount() const noexcept {
        return kChannelCount;
    }

    /// @return La largeur/hauteur de la fenêtre, `2 * radius + 1`.
    [[nodiscard]] int windowSize() const noexcept {
        return 2 * _radius + 1;
    }

    /// Nombre de valeurs de `core::TileType` au moment de ce lot (33 : `Empty`…`MovingPlatform`).
    static constexpr int kChannelCount = 33;

private:
    int _radius;
};

}  // namespace aisolver
