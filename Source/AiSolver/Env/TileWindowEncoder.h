// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Math/Tensor.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

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
    /// (`Core/Levels/TileType.h`). **Dérivé** de `core::TILE_TYPE_COUNT` depuis le `LOT-74`
    /// TACHE-02 : ajouter un type de tuile met cette valeur à jour toute seule, là où elle était
    /// auparavant un littéral à corriger à la main.
    ///
    /// ⚠️ Une valeur qui change modifie la **forme** du tenseur d'observation, donc rend
    /// inutilisables les modèles entraînés avant ce changement (voir `AiSolver` pour le refus
    /// explicite au chargement). La **signification** des canaux existants, elle, est préservée
    /// tant que les nouveaux types sont ajoutés en **fin** d'énumération — c'est précisément la
    /// raison pour laquelle le `LOT-74` a ajouté ses trois blocs volatils après `MovingPlatform`
    /// plutôt que de les insérer.
    [[nodiscard]] int channelCount() const noexcept {
        return CHANNEL_COUNT;
    }

    /// @return La largeur/hauteur de la fenêtre, `2 * radius + 1`.
    [[nodiscard]] int windowSize() const noexcept {
        return 2 * _radius + 1;
    }

    /// Nombre de valeurs de `core::TileType`, dérivé de l'énumération elle-même (36 depuis le
    /// `LOT-74` : `Empty`…`VanishingBlock`).
    static constexpr int CHANNEL_COUNT = core::TILE_TYPE_COUNT;

private:
    int _radius;
};

}  // namespace aisolver
