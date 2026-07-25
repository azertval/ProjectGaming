#pragma once

#include <filesystem>

#include "HMI/Interface/GamepadBindingsModel.h"
#include "HMI/Interface/IScreen.h"

namespace hmi {

class Localization;

/**
 * @file HMI/Interface/GamepadBindingsScreen.h
 * @brief Écran du sous-menu « Touches de la manette » (`EX-CTRL-002`, `EX-CTRL-012`, `LOT-30`).
 */

/**
 * @brief Écran de remappage des touches manette : dessine le sous-menu et applique ses actions.
 *
 * Même patron que `GameKeybindingsScreen` (voir sa doc) — habillage de `GamepadBindingsModel`.
 * Affiche en plus une ligne d'état de connexion de la manette (`InputState::gamepadConnected()`,
 * capturée à `update()` puisque `render()` ne reçoit pas `InputState`, même principe
 * qu'`OptionsScreen`) : purement informatif, explique pourquoi une capture ne démarre pas si
 * aucune manette n'est connectée.
 */
class GamepadBindingsScreen : public IScreen {
public:
    /**
     * @brief Construit l'écran.
     * @param localization Catalogue de traduction (référence conservée).
     * @param bindings     Bindings manette affichés/modifiés (référence conservée, mutable).
     * @param savePath     Chemin de persistance (`keybindings.json`).
     */
    GamepadBindingsScreen(Localization& localization, GamepadBindings& bindings,
                         std::filesystem::path savePath);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    Localization& _localization;
    GamepadBindingsModel _model;
    bool _gamepadConnected = false;
};

}  // namespace hmi
