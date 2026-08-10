#pragma once

#include "HMI/Input/InputState.h"

/**
 * @file HMI/Input/GamepadPoller.h
 * @brief Sondage XInput de la manette, fusionné dans un `InputState` (réutilisable).
 */

namespace hmi {

/**
 * @brief Sonde la manette (XInput, joueur 0) et fusionne son état dans un `InputState`.
 *
 * Alimente deux pistes indépendantes du même relevé : la fusion clavier/manette sur `Key`
 * (`onGamepadKeyDown`/`onGamepadKeyUp`, fixe, navigation de menu — aucune touche clavier n'est
 * écrasée) et l'état brut par `GamepadButton` (remappable via `GamepadBindings`, `EX-CTRL-002`).
 *
 * Objet **à état** (dernier état de connexion + compteur d'anti-saccade), à appeler **une fois par
 * frame** avant que la logique ne consomme les entrées. Le sondage est *throttlé* tant que la
 * manette reste déconnectée (`XInputGetState` est coûteux sur un slot vide — micro-saccades chez un
 * joueur clavier, `LOT-33`).
 *
 * Extrait de l'ancienne fenêtre Win32 (`LOT-34`) pour être utilisé par le viewport Qt, sans
 * dépendre d'aucune fenêtre.
 */
class GamepadPoller {
public:
    /// Sonde la manette et met à jour @p input (piste `Key` + piste `GamepadButton` brute).
    void poll(InputState& input);

private:
    /// Nombre de frames entre deux sondages XInput d'un slot resté déconnecté (anti-saccade).
    static constexpr int DISCONNECTED_POLL_INTERVAL = 120;

    bool _wasConnected = false;  ///< Pour ne journaliser qu'un changement de connexion.
    int _pollCountdown = 0;      ///< Frames restantes avant de re-sonder une manette absente.
};

}  // namespace hmi
