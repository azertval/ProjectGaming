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
 * @brief Une action discrète : produit cartésien direction × saut-appuyé × saut-maintenu × dash ×
 * interagir.
 *
 * `moveY` n'est volontairement pas représenté ici (toujours `0` une fois traduit en
 * `core::PlayerInput`, voir `toPlayerInput`) : décision de cadrage de l'épic, aucun niveau
 * n'exploite un dash orienté verticalement.
 *
 * `interactPressed` fait partie de l'espace parce qu'une clé (`TileType::Key`) ne se ramasse qu'au
 * contact **et** à l'action « Interagir » (`core::MechanismController::update`, `EX-CTRL-022`) :
 * sans cette dimension, aucune politique ne pourrait ouvrir une porte verrouillée, quel que soit
 * son entraînement.
 *
 * Ce bit n'a aujourd'hui aucun inconvénient à être posé — il ne sert qu'aux clés, et son effet est
 * définitif : une porte ouverte ne se referme pas, rappuyer est sans effet. L'agent l'apprend donc
 * plutôt que de le recevoir câblé : c'est le choix de rester fidèle aux entrées du jeu réel, et de
 * ne pas dépendre du fait qu'interagir reste gratuit si un mécanisme futur lui donne un coût.
 * Le prix est connu : la couche de sortie de chaque réseau double.
 */
struct Action {
    Direction direction = Direction::None;
    bool jumpPressed = false;
    bool jumpHeld = false;
    bool dashPressed = false;
    bool interactPressed = false;

    [[nodiscard]] friend bool operator==(const Action&, const Action&) = default;
};

/// @return Nombre total d'actions distinctes de l'espace d'action (`3 × 2 × 2 × 2 × 2 = 48`).
[[nodiscard]] constexpr std::size_t actionCount() noexcept {
    return 3 * 2 * 2 * 2 * 2;
}

/**
 * @brief Action correspondant à un indice donné (inverse de `indexOf`).
 *
 * L'ordre d'énumération (direction en poids fort, puis `jumpPressed`, `jumpHeld`, `dashPressed`,
 * `interactPressed` en poids faible) est un **invariant** : l'indice de sortie d'un réseau
 * entraîné ne désigne l'action attendue que sous cet ordre. Le changer sans réentraîner rendrait
 * les modèles silencieusement faux ; changer le *nombre* d'actions, lui, les rend franchement
 * incompatibles — `nn::loadWeights` refuse la forme.
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
 * l'épic). `interactPressed` est reporté tel quel ; `interactHeld`/`interactReleased` restent à
 * `false` (aucun mécanisme du jeu ne les consulte, seul le front `interactPressed` compte pour le
 * ramassage d'une clé, `core::MechanismController::update`).
 */
[[nodiscard]] core::PlayerInput toPlayerInput(const Action& action);

}  // namespace aisolver
