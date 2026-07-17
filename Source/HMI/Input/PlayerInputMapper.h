#pragma once

#include "Core/Physics/PlayerInput.h"

/**
 * @file HMI/Input/PlayerInputMapper.h
 * @brief Traduction des entrées clavier en intention de déplacement (action logique).
 */

namespace hmi {

class InputState;

/**
 * @brief Traduit l'état clavier en ::core::PlayerInput, dissocié des touches physiques
 *        (`EX-CTRL-010`).
 *
 * Le reste du moteur ne connaît que l'**intention** (`moveX`), pas les touches : un futur
 * remappage ou une manette (`EX-CTRL-012`) ne toucherait que cette fonction. Correspondance :
 * gauche = `←` ou `Q` ; droite = `→` ou `D`. Gauche et droite simultanées se neutralisent
 * (`moveX == 0`). Fonction **pure**, testable avec un `InputState` injecté (`EX-NFR-010`) ;
 * appelée une fois par frame en amont de la logique (`EX-CTRL-020`, `EX-CTRL-021`).
 *
 * @param input État clavier échantillonné de la frame.
 * @return L'intention de déplacement correspondante.
 */
[[nodiscard]] core::PlayerInput toPlayerInput(const InputState& input);

}  // namespace hmi
