// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Math/Tensor.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Levels/Level.h"

/**
 * @file AiSolver/Env/PlayerStateEncoder.h
 * @brief Vecteur d'état cinématique du personnage (LOT-ANNEXE-06).
 */

namespace aisolver {

/**
 * @brief Encode la cinématique propre du personnage (vitesse, contact, minuteries, budgets) en un
 *        vecteur de taille fixe, complémentaire à `TileWindowEncoder` (environnement immédiat).
 *
 * Composantes, dans cet ordre documenté (indices `0`..`size() - 1`) :
 *   0. `velocity.value.x` — brut, unités monde/s (pas de plafond arbitraire depuis `LOT-19`).
 *   1. `velocity.value.y` — idem.
 *   2. `player.grounded ? 1.0f : 0.0f`.
 *   3. `player.wallDirection` — déjà dans `{-1, 0, 1}`, utilisé tel quel.
 *   4. `player.coyoteTimer / NOMINAL_COYOTE_TIME`.
 *   5. `player.jumpBufferTimer / NOMINAL_JUMP_BUFFER_TIME`.
 *   6. `player.wallJumpLockTimer / NOMINAL_WALL_JUMP_LOCK_TIME`.
 *   7. `player.dashTimer / NOMINAL_DASH_DURATION`.
 *   8. Dash disponible (`dashChargesRemaining > 0 && dashTimer <= 0`) ? `1.0f` : `0.0f` — même
 *      condition de déclenchement que `core::CharacterPhysicsSystem` (dash en cours **ou** charges
 *      épuisées rendent tous deux un nouveau dash impossible).
 *   9. Budget de saut normalisé : `1.0f` si illimité (`jumpBudget() < 0`), `0.0f` si le tableau
 *      n'en autorise aucun (`jumpBudget() == 0` — sinon la division rendrait `NaN`, qui contamine
 *      toute la propagation avant), `jumpsRemaining / jumpBudget()` sinon.
 *   10. Budget de dash normalisé, même formule.
 *
 * L'épic (`Documentation/Lot-Annexe/LOT-ANNEXE-06-encodage-observation/epic.md`) annonce une taille
 * de `10` en toutes lettres, mais sa propre liste de composantes (reprise ci-dessus) en énumère
 * `11` — erreur d'arithmétique dans le texte, pas dans la formule : `size()` reflète la liste, pas
 * le nombre annoncé en prose.
 */
class PlayerStateEncoder {
public:
    /**
     * @brief Encode l'état cinématique courant du personnage.
     * @param player   Composant `core::Player` du personnage.
     * @param velocity Composant `core::Velocity` du personnage.
     * @param level    Niveau chargé, pour les budgets (`jumpBudget()`/`dashBudget()`).
     * @return Vecteur (tenseur de rang 1) de taille `size()`.
     */
    [[nodiscard]] Tensor<float> encode(const core::Player& player, const core::Velocity& velocity,
                                       const core::Level& level) const;

    /// @return La taille fixe du vecteur produit par `encode` (`11`, voir l'ordre documenté
    /// ci-dessus).
    [[nodiscard]] static constexpr int size() noexcept {
        return PLAYER_STATE_SIZE;
    }

    static constexpr int PLAYER_STATE_SIZE = 11;

    // Constantes de normalisation des minuteries de *game feel*, dupliquées depuis les valeurs par
    // défaut de core::PhysicsConfig (Core/Physics/PhysicsConfig.h) : CharacterPhysicsSystem garde
    // sa configuration privée (pas d'accesseur public), introduire un tel accesseur pour ce seul
    // besoin cosmétique toucherait Core (décision de cadrage, epic LOT-ANNEXE-06). Point de
    // recalibration explicite si PhysicsConfig change ses valeurs par défaut.
    static constexpr float NOMINAL_COYOTE_TIME = 0.08f;          // PhysicsConfig::coyoteTime
    static constexpr float NOMINAL_JUMP_BUFFER_TIME = 0.12f;     // PhysicsConfig::jumpBufferTime
    static constexpr float NOMINAL_WALL_JUMP_LOCK_TIME = 0.18f;  // PhysicsConfig::wallJumpLockTime
    static constexpr float NOMINAL_DASH_DURATION = 0.15f;        // PhysicsConfig::dashDuration
};

}  // namespace aisolver
