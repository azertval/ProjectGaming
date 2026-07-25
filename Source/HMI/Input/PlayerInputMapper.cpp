#include "HMI/Input/PlayerInputMapper.h"

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/InputState.h"

namespace hmi {

// Traduit l'etat clavier en intention de deplacement (voir en-tete).
//
// Chaque action verifie SA touche liee (remappable) ET sa touche par defaut (GameKeyBindings::
// defaultKey) : la manette (Window::pollGamepad) ecrit exclusivement dans les touches par defaut,
// cablees en dur (A -> Espace, D-pad/stick -> flèches, RB -> Maj), sans aucune connaissance des
// bindings courants (EX-CTRL-002) - sans ce second OR, remapper une action au clavier
// desactiverait silencieusement le bouton manette equivalent. Vrai remappage manette : hors
// perimetre de ce lot (LOT-29), un lot dedie devra d'abord retirer ce cablage en dur.
core::PlayerInput toPlayerInput(const InputState& input, const GameKeyBindings& bindings) {
    // Actions logiques (dissociees des touches, EX-CTRL-012) : gauche/droite/sauter conservent en
    // plus un alias fixe non remappable (Q/D/W), le reste passe par les bindings + leur defaut.
    const bool left = input.keyDown(bindings.key(GameAction::MoveLeft)) ||
                      input.keyDown(Key::Q) ||
                      input.keyDown(GameKeyBindings::defaultKey(GameAction::MoveLeft));
    const bool right = input.keyDown(bindings.key(GameAction::MoveRight)) ||
                       input.keyDown(Key::D) ||
                       input.keyDown(GameKeyBindings::defaultKey(GameAction::MoveRight));

    core::PlayerInput result;
    // Gauche et droite se neutralisent (-1 + 1 = 0) : comportement deterministe.
    result.moveX = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);
    // Saut : touche liee, W, ou defaut (Espace, alimente par le bouton A manette). jumpPressed =
    // front (declenche/bufferise), jumpHeld = maintenu (hauteur variable).
    const Key jumpKey = bindings.key(GameAction::Jump);
    const Key jumpDefault = GameKeyBindings::defaultKey(GameAction::Jump);
    result.jumpPressed =
        input.keyPressed(jumpKey) || input.keyPressed(Key::W) || input.keyPressed(jumpDefault);
    result.jumpHeld = input.keyDown(jumpKey) || input.keyDown(Key::W) || input.keyDown(jumpDefault);
    // Visee verticale du dash (y vers le bas) : Bas = +1, Haut = -1, sinon 0.
    const bool aimDown = input.keyDown(bindings.key(GameAction::AimDown)) ||
                        input.keyDown(GameKeyBindings::defaultKey(GameAction::AimDown));
    const bool aimUp = input.keyDown(bindings.key(GameAction::AimUp)) ||
                      input.keyDown(GameKeyBindings::defaultKey(GameAction::AimUp));
    result.moveY = (aimDown ? 1.0f : 0.0f) - (aimUp ? 1.0f : 0.0f);
    // Dash : touche liee ou defaut (Maj, alimentee par l'epaule droite manette), au front
    // (`EX-CTRL-013`).
    result.dashPressed = input.keyPressed(bindings.key(GameAction::Dash)) ||
                        input.keyPressed(GameKeyBindings::defaultKey(GameAction::Dash));
    return result;
}

}  // namespace hmi
