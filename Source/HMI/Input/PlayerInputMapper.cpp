#include "HMI/Input/PlayerInputMapper.h"

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/InputState.h"

namespace hmi {

namespace {

// Touche par defaut de action, seulement si aucune AUTRE action ne se l'est appropriee entre-temps
// (sinon elle appartient desormais exclusivement a cette autre action - la revalider ferait
// declencher les deux a la fois, silencieusement annule pour une paire opposee comme
// Gauche/Droite : c'est exactement le bug reproduit et corrige en cours de LOT-29). Filet de
// securite manette (EX-CTRL-002) : Window::pollGamepad n'ecrit que dans les touches par defaut.
[[nodiscard]] bool safeDefaultDown(const InputState& input, const GameKeyBindings& bindings,
                                   GameAction action) {
    const Key def = GameKeyBindings::defaultKey(action);
    return !bindings.isKeyClaimedByOtherAction(action, def) && input.keyDown(def);
}

[[nodiscard]] bool safeDefaultPressed(const InputState& input, const GameKeyBindings& bindings,
                                      GameAction action) {
    const Key def = GameKeyBindings::defaultKey(action);
    return !bindings.isKeyClaimedByOtherAction(action, def) && input.keyPressed(def);
}

}  // namespace

// Traduit l'etat clavier en intention de deplacement (voir en-tete).
core::PlayerInput toPlayerInput(const InputState& input, const GameKeyBindings& bindings) {
    // Actions logiques (dissociees des touches, EX-CTRL-012) : chaque action verifie sa touche
    // liee ET, si elle reste libre (voir safeDefault* ci-dessus), sa touche par defaut.
    const bool left = input.keyDown(bindings.key(GameAction::MoveLeft)) ||
                      safeDefaultDown(input, bindings, GameAction::MoveLeft);
    const bool right = input.keyDown(bindings.key(GameAction::MoveRight)) ||
                       safeDefaultDown(input, bindings, GameAction::MoveRight);

    core::PlayerInput result;
    // Gauche et droite se neutralisent (-1 + 1 = 0) : comportement deterministe.
    result.moveX = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);
    // Saut : touche liee, ou defaut si libre. jumpPressed = front (declenche/bufferise), jumpHeld
    // = maintenu (hauteur variable).
    const Key jumpKey = bindings.key(GameAction::Jump);
    result.jumpPressed =
        input.keyPressed(jumpKey) || safeDefaultPressed(input, bindings, GameAction::Jump);
    result.jumpHeld = input.keyDown(jumpKey) || safeDefaultDown(input, bindings, GameAction::Jump);
    // Visee verticale du dash (y vers le bas) : Bas = +1, Haut = -1, sinon 0.
    const bool aimDown = input.keyDown(bindings.key(GameAction::AimDown)) ||
                        safeDefaultDown(input, bindings, GameAction::AimDown);
    const bool aimUp = input.keyDown(bindings.key(GameAction::AimUp)) ||
                      safeDefaultDown(input, bindings, GameAction::AimUp);
    result.moveY = (aimDown ? 1.0f : 0.0f) - (aimUp ? 1.0f : 0.0f);
    // Dash : touche liee, ou defaut si libre, au front (`EX-CTRL-013`).
    result.dashPressed = input.keyPressed(bindings.key(GameAction::Dash)) ||
                        safeDefaultPressed(input, bindings, GameAction::Dash);
    return result;
}

}  // namespace hmi
