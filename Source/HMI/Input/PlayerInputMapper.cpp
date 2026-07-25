#include "HMI/Input/PlayerInputMapper.h"

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/InputState.h"

namespace hmi {

// Traduit l'etat clavier en intention de deplacement (voir en-tete).
core::PlayerInput toPlayerInput(const InputState& input, const GameKeyBindings& bindings) {
    // Actions logiques (dissociees des touches, EX-CTRL-012) : gauche/droite/sauter conservent en
    // plus un alias fixe non remappable (Q/D/W), le reste passe entierement par les bindings.
    const bool left = input.keyDown(bindings.key(GameAction::MoveLeft)) || input.keyDown(Key::Q);
    const bool right = input.keyDown(bindings.key(GameAction::MoveRight)) || input.keyDown(Key::D);

    core::PlayerInput result;
    // Gauche et droite se neutralisent (-1 + 1 = 0) : comportement deterministe.
    result.moveX = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);
    // Saut : touche liee ou W. jumpPressed = front (declenche/bufferise), jumpHeld = maintenu
    // (hauteur variable). keyPressed = front d'une frame, keyDown = maintenu.
    const Key jumpKey = bindings.key(GameAction::Jump);
    result.jumpPressed = input.keyPressed(jumpKey) || input.keyPressed(Key::W);
    result.jumpHeld = input.keyDown(jumpKey) || input.keyDown(Key::W);
    // Visee verticale du dash (y vers le bas) : Bas = +1, Haut = -1, sinon 0.
    const bool aimDown = input.keyDown(bindings.key(GameAction::AimDown));
    const bool aimUp = input.keyDown(bindings.key(GameAction::AimUp));
    result.moveY = (aimDown ? 1.0f : 0.0f) - (aimUp ? 1.0f : 0.0f);
    // Dash : touche liee, au front (`EX-CTRL-013`).
    result.dashPressed = input.keyPressed(bindings.key(GameAction::Dash));
    return result;
}

}  // namespace hmi
