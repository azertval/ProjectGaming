#pragma once

#include "HMI/Interface/IScreen.h"
#include "HMI/Interface/LanguageSelector.h"
#include "HMI/Interface/OptionsModel.h"

namespace hmi {

class Localization;
class GraphicsDevice;

/**
 * @file HMI/Interface/OptionsScreen.h
 * @brief Écran du menu d'options : dessine le menu et applique ses actions.
 */

/**
 * @brief Écran de réglages : bascule V-Sync, langue, état de la manette, retour au menu
 *        (`EX-REN-022`, `EX-CTRL-002`, LOT-20).
 *
 * Fin habillage d'`OptionsModel` : `update` délègue la sélection/confirmation au modèle
 * (testable), interprète l'action renvoyée (bascule V-Sync appliquée à `GraphicsDevice`, ou
 * retour au menu), et traite en plus le bouton de langue en coin (`LanguageSelector`, réutilisé
 * tel quel, même geste que `MenuScreen`). `render` dessine le titre, les deux options (celle
 * sélectionnée mise en évidence), une ligne d'état de connexion de la manette, et le bouton de
 * langue.
 */
class OptionsScreen : public IScreen {
public:
    /**
     * @brief Construit l'écran d'options.
     * @param localization Catalogue de traduction (référence **mutable** : le bouton de langue
     *                     le recharge lors d'une bascule).
     * @param graphics     Périphérique graphique dont le réglage V-Sync est lu/modifié.
     */
    OptionsScreen(Localization& localization, GraphicsDevice& graphics);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    Localization& _localization;
    GraphicsDevice& _graphics;
    OptionsModel _model;
    LanguageSelector _languageButton;
    bool _gamepadConnected = false;  ///< Capturé à update() : render() ne reçoit pas InputState.
    int _viewportWidth = 0;
    int _viewportHeight = 0;
};

}  // namespace hmi
