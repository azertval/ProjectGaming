#pragma once

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/GameKeyBindings.h"

/**
 * @file HMI/Input/PlayerInputMapper.h
 * @brief Traduction des entrées clavier en intention de déplacement (action logique).
 */

namespace hmi {

class InputState;

/**
 * @brief Traduit l'état clavier en ::core::PlayerInput, dissocié des touches physiques
 *        (`EX-CTRL-010`, `EX-CTRL-012`).
 *
 * Le reste du moteur ne connaît que l'**intention** (`moveX`), pas les touches. Chaque action lit
 * la touche courante de @p bindings (`LOT-29`, remappable depuis Options → Touches de jeu) ;
 * gauche/droite/sauter conservent en plus un alias fixe non remappable (`Q`/`D`/`W`), pour ne rien
 * casser du confort ZQSD/WASD existant — dash et visée haut/bas n'en ont pas. Gauche et droite
 * simultanées se neutralisent (`moveX == 0`). Fonction **pure**, testable avec un `InputState`
 * injecté (`EX-NFR-010`) ; appelée une fois par frame en amont de la logique (`EX-CTRL-020`,
 * `EX-CTRL-021`).
 *
 * @param input        État clavier échantillonné de la frame.
 * @param bindings      Association action de jeu -> touche courante.
 * @return L'intention de déplacement correspondante.
 */
[[nodiscard]] core::PlayerInput toPlayerInput(const InputState& input,
                                              const GameKeyBindings& bindings);

}  // namespace hmi
