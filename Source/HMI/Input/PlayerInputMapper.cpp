#include "HMI/Input/PlayerInputMapper.h"

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/InputState.h"

namespace hmi {

// Traduit l'etat clavier en intention de deplacement (voir en-tete).
core::PlayerInput toPlayerInput(const InputState& input) {
    // Actions logiques (dissociees des touches) : chaque sens accepte deux touches.
    const bool left = input.keyDown(Key::Left) || input.keyDown(Key::Q);
    const bool right = input.keyDown(Key::Right) || input.keyDown(Key::D);

    core::PlayerInput result;
    // Gauche et droite se neutralisent (-1 + 1 = 0) : comportement deterministe.
    result.moveX = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);
    return result;
}

}  // namespace hmi
