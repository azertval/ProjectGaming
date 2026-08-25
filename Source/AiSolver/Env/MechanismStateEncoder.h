// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Math/Tensor.h"
#include "Core/Gameplay/DangerController.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"

/**
 * @file AiSolver/Env/MechanismStateEncoder.h
 * @brief État dynamique des mécanismes superposé à la fenêtre de tuiles (LOT-ANNEXE-06).
 */

namespace aisolver {

/**
 * @brief Encode l'état **dynamique** des mécanismes de la fenêtre (porte ouverte/fermée, danger
 *        actif), complémentaire au type **statique** encodé par `TileWindowEncoder`.
 *
 * Une porte fermée et une porte ouverte partagent le même `core::TileType::Door` dans `TileMap` :
 * seule la grille de collision recomposée à chaque pas par `core::MechanismController` en distingue
 * l'état. Ce canal rend cette distinction visible dans l'observation, sans quoi l'agent ne pourrait
 * jamais reconnaître une porte franchissable d'un mur (`EX-IA-006`).
 *
 * Produit un tenseur `(2, 2·radius + 1, 2·radius + 1)`, même fenêtre géométrique que
 * `TileWindowEncoder` (à concaténer par l'appelant, `radius` identique de part et d'autre) :
 *   - **Canal 0** (« porte ouverte ») : `1.0f` sur la case `doorPosition` de chaque
 *     `core::Mechanism` dont `mechanisms.isDoorOpen` est vrai, `0.0f` ailleurs (y compris sur une
 *     porte fermée).
 *   - **Canal 1** (« danger actif ») : `1.0f` sur toute case recouverte par un danger mobile
 *     (`dangers.moverBox`, rastérisé sur les cases qu'il chevauche), une case `DangerBlink` active
 *     (`dangers.isBlinkActive`) ou une case `DangerSwitched` active (`mechanisms.isDangerActive`).
 *     Un danger **statique** (`Danger`/`DangerUp`/…) n'active jamais ce canal : il est déjà
 *     identifiable sans ambiguïté par son canal catégoriel, invariant, dans `TileWindowEncoder`.
 */
class MechanismStateEncoder {
public:
    /**
     * @brief Encode l'état des mécanismes dans une fenêtre centrée sur @p center.
     * @param mechanisms Contrôleur de mécanismes du niveau chargé.
     * @param dangers    Contrôleur de dangers du niveau chargé.
     * @param level      Niveau chargé (dangers temporisés/commutés, positions).
     * @param center     Case de grille où se trouve le personnage.
     * @param radius     Rayon de fenêtre, **identique** à celui de `TileWindowEncoder` (à la charge
     *                   de l'appelant, pas garanti par le type).
     * @return Tenseur `(2, 2·radius + 1, 2·radius + 1)`.
     */
    [[nodiscard]] Tensor<float> encode(const core::MechanismController& mechanisms,
                                       const core::DangerController& dangers,
                                       const core::Level& level, core::GridPosition center,
                                       int radius) const;

    /// @return Le nombre de canaux produits (`2` : porte ouverte, danger actif).
    [[nodiscard]] int channelCount() const noexcept {
        return CHANNEL_COUNT;
    }

    static constexpr int CHANNEL_COUNT = 2;
};

}  // namespace aisolver
