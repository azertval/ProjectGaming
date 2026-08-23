// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>

#include "Core/Physics/PlayerInput.h"

/**
 * @file AiSolver/Env/ActionSpace.h
 * @brief Espace d'action discret et fini de l'agent (`LOT-ANNEXE-07`, `EX-IA-007`).
 */

namespace aisolver {

/// Direction de déplacement horizontal discrète (voir la décision de cadrage de l'épic :
/// `core::PlayerInput::moveX` n'est en pratique jamais utilisé en dehors de `{-1, 0, 1}`).
enum class Direction { Left, None, Right };

/**
 * @brief Une action discrète : produit cartésien direction × saut-appuyé × saut-maintenu × dash.
 *
 * `moveY` n'est volontairement pas représenté ici (toujours `0` une fois traduit en
 * `core::PlayerInput`, voir `toPlayerInput`) : décision de cadrage de l'épic, aucun niveau
 * n'exploite un dash orienté verticalement.
 */
struct Action {
    Direction direction = Direction::None;
    bool jumpPressed = false;
    bool jumpHeld = false;
    bool dashPressed = false;

    [[nodiscard]] friend bool operator==(const Action&, const Action&) = default;
};

/// @return Nombre total d'actions distinctes de l'espace d'action (`3 × 2 × 2 × 2 = 24`).
[[nodiscard]] constexpr std::size_t actionCount() noexcept {
    return 3 * 2 * 2 * 2;
}

/**
 * @brief Action correspondant à un indice donné (inverse de `indexOf`).
 *
 * L'ordre d'énumération (direction en poids fort, puis `jumpPressed`, `jumpHeld`, `dashPressed`)
 * est un **invariant stable** : une fois qu'un modèle a été entraîné sur cet ordre (génération 2/3),
 * le changer romprait la correspondance entre l'indice de sortie d'un réseau déjà entraîné et
 * l'action qu'il désigne réellement (voir TACHE-01, points d'attention).
 * @param index Indice dans `[0, actionCount())`.
 * @pre `index < actionCount()`.
 */
[[nodiscard]] Action actionAt(std::size_t index);

/// @return L'indice (dans `[0, actionCount())`) correspondant à une action (inverse de `actionAt`).
[[nodiscard]] std::size_t indexOf(const Action& action);

/**
 * @brief Traduit une action discrète en `core::PlayerInput`, consommé par
 *        `HeadlessLevelEnvironment::step` (`LOT-ANNEXE-05`).
 *
 * `moveX` vaut `-1`/`0`/`1` selon `Direction`, `moveY` est toujours `0` (décision de cadrage de
 * l'épic). Les champs d'interaction (`interactPressed`/`interactHeld`/`interactReleased`) ne font
 * pas partie de l'espace d'action de ce lot et restent à leur valeur par défaut (`false`) : aucun
 * niveau de la séquence `demo-*.json` ne conditionne sa résolution sur l'interaction manuelle plutôt
 * que le contact automatique des mécanismes.
 */
[[nodiscard]] core::PlayerInput toPlayerInput(const Action& action);

}  // namespace aisolver
