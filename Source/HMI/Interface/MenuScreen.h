#pragma once

#include "HMI/Interface/IScreen.h"
#include "HMI/Interface/LanguageSelector.h"
#include "HMI/Interface/MenuModel.h"

namespace hmi {

class Localization;

/**
 * @file HMI/Interface/MenuScreen.h
 * @brief Écran du menu principal : dessine le menu et délègue sa logique au MenuModel.
 */

/**
 * @brief Écran d'accueil : titre et trois options (Charger niveau, Mode Edition, Quitter).
 *
 * Fin habillage de `MenuModel` : `update` délègue la logique de sélection/transition au modèle
 * (testable), et `render` dessine le titre et les options via la police bitmap, en mettant en
 * évidence l'option sélectionnée (`EX-REN-030`, `EX-REN-032`). Les libellés proviennent du
 * catalogue de traduction. Un **bouton de langue** en bas à droite (drapeau de la langue
 * courante) bascule le catalogue entre français et anglais (`EX-REN-033`).
 */
class MenuScreen : public IScreen {
public:
    /**
     * @brief Construit l'écran de menu.
     * @param localization Catalogue de traduction (référence **mutable** : le bouton de langue
     *                     le recharge lors d'une bascule).
     */
    explicit MenuScreen(Localization& localization);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    Localization& _localization;
    MenuModel _model;
    LanguageSelector _languageButton;
    int _viewportWidth = 0;   ///< Dernière largeur connue (pour le survol/clic du bouton langue).
    int _viewportHeight = 0;  ///< Dernière hauteur connue.
};

}  // namespace hmi
