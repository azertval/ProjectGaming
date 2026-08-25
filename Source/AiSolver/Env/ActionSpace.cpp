// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/ActionSpace.h"

#include "Core/Diagnostics/Assert.h"

namespace aisolver {

namespace {

/// Nombre de valeurs de `Direction`, dans son ordre d'énumération (`Left, None, Right`).
constexpr std::size_t kDirectionCount = 3;

[[nodiscard]] Direction directionAt(std::size_t index) {
    switch (index) {
        case 0:
            return Direction::Left;
        case 1:
            return Direction::None;
        default:
            return Direction::Right;
    }
}

[[nodiscard]] std::size_t indexOfDirection(Direction direction) {
    switch (direction) {
        case Direction::Left:
            return 0;
        case Direction::None:
            return 1;
        default:
            return 2;
    }
}

}  // namespace

Action actionAt(std::size_t index) {
    PROJECTGAMING_ASSERT(index < actionCount(), "actionAt() : indice hors de l'espace d'action");
    const bool interactPressed = (index % 2) != 0;
    index /= 2;
    const bool dashPressed = (index % 2) != 0;
    index /= 2;
    const bool jumpHeld = (index % 2) != 0;
    index /= 2;
    const bool jumpPressed = (index % 2) != 0;
    index /= 2;
    const Direction direction = directionAt(index % kDirectionCount);
    return Action{direction, jumpPressed, jumpHeld, dashPressed, interactPressed};
}

std::size_t indexOf(const Action& action) {
    std::size_t index = indexOfDirection(action.direction);
    index = index * 2 + (action.jumpPressed ? 1 : 0);
    index = index * 2 + (action.jumpHeld ? 1 : 0);
    index = index * 2 + (action.dashPressed ? 1 : 0);
    index = index * 2 + (action.interactPressed ? 1 : 0);
    return index;
}

core::PlayerInput toPlayerInput(const Action& action) {
    core::PlayerInput input;
    switch (action.direction) {
        case Direction::Left:
            input.moveX = -1.0f;
            break;
        case Direction::None:
            input.moveX = 0.0f;
            break;
        case Direction::Right:
            input.moveX = 1.0f;
            break;
    }
    input.jumpPressed = action.jumpPressed;
    input.jumpHeld = action.jumpHeld;
    input.dashPressed = action.dashPressed;
    input.interactPressed = action.interactPressed;
    return input;
}

}  // namespace aisolver
