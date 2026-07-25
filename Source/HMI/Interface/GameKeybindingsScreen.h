#pragma once

#include <filesystem>

#include "HMI/Interface/GameKeybindingsModel.h"
#include "HMI/Interface/IScreen.h"

namespace hmi {

class Localization;

/**
 * @file HMI/Interface/GameKeybindingsScreen.h
 * @brief Écran du sous-menu « Touches de jeu » (`EX-CTRL-012`, `LOT-29`).
 */

/**
 * @brief Écran de remappage des touches de jeu : dessine le sous-menu et applique ses actions.
 *
 * Fin habillage de `GameKeybindingsModel` : `update` délègue la navigation/capture au modèle
 * (testable) et interprète l'action renvoyée (retour au menu Options) ; `render` dessine le
 * titre, les huit lignes (celle sélectionnée mise en évidence, celle en cours de capture affichant
 * l'invite « Appuyez sur une touche… »).
 */
class GameKeybindingsScreen : public IScreen {
public:
    /**
     * @brief Construit l'écran.
     * @param localization Catalogue de traduction (référence conservée).
     * @param bindings     Bindings de jeu affichés/modifiés (référence conservée, mutable).
     * @param savePath     Chemin de persistance (`keybindings.json`).
     */
    GameKeybindingsScreen(Localization& localization, GameKeyBindings& bindings,
                         std::filesystem::path savePath);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    Localization& _localization;
    GameKeybindingsModel _model;
};

}  // namespace hmi
