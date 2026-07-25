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
 * la touche courante de @p bindings (`LOT-29`, remappable depuis Options → Touches de jeu) **et**,
 * si elle reste libre, sa touche par défaut (`GameKeyBindings::defaultKey`) : la manette
 * (`Window::pollGamepad`, `EX-CTRL-002`) n'écrit que dans les touches par défaut, câblées en dur,
 * sans connaître les bindings courants — sans ce filet, remapper une action au clavier
 * désactiverait silencieusement le bouton manette équivalent (vrai remappage manette : hors
 * périmètre de `LOT-29`).
 *
 * **« Si elle reste libre »** : la touche par défaut d'une action n'est revérifiée que si aucune
 * **autre** action ne se l'est appropriée entre-temps (`GameKeyBindings::
 * isKeyClaimedByOtherAction`) — sinon elle appartient désormais exclusivement à cette autre
 * action. Sans cette garde, un alias fixe non remappable (`Q`/`D`/`W`, envisagé puis retiré en
 * cours de `LOT-29`) ou ce filet lui-même auraient pu faire déclencher deux actions à la fois dès
 * qu'une touche par défaut est réutilisée pour une autre action — silencieusement **annulé** pour
 * une paire opposée comme Gauche/Droite (bug constaté en usage réel : remapper « Aller à gauche »
 * sur `D` alors que `D` était l'alias fixe de « Aller à droite » neutralisait tout mouvement
 * horizontal).
 *
 * Gauche et droite simultanées se neutralisent (`moveX == 0`). Fonction **pure**, testable avec un
 * `InputState` injecté (`EX-NFR-010`) ; appelée une fois par frame en amont de la logique
 * (`EX-CTRL-020`, `EX-CTRL-021`).
 *
 * @param input        État clavier échantillonné de la frame.
 * @param bindings      Association action de jeu -> touche courante.
 * @return L'intention de déplacement correspondante.
 */
[[nodiscard]] core::PlayerInput toPlayerInput(const InputState& input,
                                              const GameKeyBindings& bindings);

}  // namespace hmi
