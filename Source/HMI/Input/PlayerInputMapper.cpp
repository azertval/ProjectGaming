#include "HMI/Input/PlayerInputMapper.h"

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/InputState.h"

namespace hmi {

// Traduit l'etat clavier/manette en intention de deplacement (voir en-tete).
core::PlayerInput toPlayerInput(const InputState& input, const GameKeyBindings& gameKeyBindings,
                                const GamepadBindings& gamepadBindings) {
    // Chaque action : sa touche clavier liee OU son bouton manette lie (EX-CTRL-012).
    const bool left = input.keyDown(gameKeyBindings.key(GameAction::MoveLeft)) ||
                      input.gamepadButtonDown(gamepadBindings.button(GameAction::MoveLeft));
    const bool right = input.keyDown(gameKeyBindings.key(GameAction::MoveRight)) ||
                       input.gamepadButtonDown(gamepadBindings.button(GameAction::MoveRight));

    core::PlayerInput result;
    // Gauche et droite se neutralisent (-1 + 1 = 0) : comportement deterministe.
    result.moveX = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);
    // Saut : jumpPressed = front (declenche/bufferise), jumpHeld = maintenu (hauteur variable).
    const Key jumpKey = gameKeyBindings.key(GameAction::Jump);
    const GamepadButton jumpButton = gamepadBindings.button(GameAction::Jump);
    result.jumpPressed = input.keyPressed(jumpKey) || input.gamepadButtonPressed(jumpButton);
    result.jumpHeld = input.keyDown(jumpKey) || input.gamepadButtonDown(jumpButton);
    // Visee verticale du dash (y vers le bas) : Bas = +1, Haut = -1, sinon 0.
    const bool aimDown = input.keyDown(gameKeyBindings.key(GameAction::AimDown)) ||
                        input.gamepadButtonDown(gamepadBindings.button(GameAction::AimDown));
    const bool aimUp = input.keyDown(gameKeyBindings.key(GameAction::AimUp)) ||
                      input.gamepadButtonDown(gamepadBindings.button(GameAction::AimUp));
    result.moveY = (aimDown ? 1.0f : 0.0f) - (aimUp ? 1.0f : 0.0f);
    // Dash : au front (`EX-CTRL-013`).
    result.dashPressed = input.keyPressed(gameKeyBindings.key(GameAction::Dash)) ||
                        input.gamepadButtonPressed(gamepadBindings.button(GameAction::Dash));
    return result;
}

}  // namespace hmi
