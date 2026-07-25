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
 * la touche courante de @p bindings (remappable depuis Options → Touches de jeu), **et** — tant
 * qu'aucune autre action ne se l'est appropriée (`GameKeyBindings::isKeyClaimedByOtherAction`) —
 * sa touche par défaut (`GameKeyBindings::defaultKey`).
 *
 * Cette seconde vérification est un filet de sécurité pour la manette (`Window::pollGamepad`,
 * `EX-CTRL-002`) : elle n'écrit que dans les touches par défaut, câblées en dur, sans connaître les
 * bindings courants (un vrai remappage manette est hors périmètre). Sans ce filet, remapper une
 * action au clavier désactiverait le bouton manette équivalent. La condition « tant qu'aucune
 * autre action ne se l'est appropriée » évite qu'une touche par défaut réclamée par une **autre**
 * action (remappée dessus) ne déclenche les deux actions à la fois — ce qui, pour une paire
 * opposée comme Gauche/Droite, s'annulerait silencieusement (`moveX` figé à `0`).
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
