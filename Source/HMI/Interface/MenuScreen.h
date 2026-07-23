#pragma once

#include <functional>

#include "HMI/Interface/IScreen.h"
#include "HMI/Interface/LanguageSelector.h"
#include "HMI/Interface/MenuModel.h"
#include "HMI/Interface/SaveLogButton.h"

namespace hmi {

class Localization;

/**
 * @file HMI/Interface/MenuScreen.h
 * @brief Écran du menu principal : dessine le menu et délègue sa logique au MenuModel.
 */

/**
 * @brief Écran d'accueil : titre et quatre options (Jouer, Mode Edition, Options, Quitter).
 *
 * Fin habillage de `MenuModel` : `update` délègue la logique de sélection/transition au modèle
 * (testable), et `render` dessine le titre et les options via la police bitmap, en mettant en
 * évidence l'option sélectionnée (`EX-REN-030`, `EX-REN-032`). Les libellés proviennent du
 * catalogue de traduction. Deux boutons en bas à droite : un **bouton de langue** (drapeau de
 * la langue courante) qui bascule le catalogue français ↔ anglais (`EX-REN-033`), et un
 * **bouton d'enregistrement des logs** (à sa gauche) qui déclenche une action fournie.
 */
class MenuScreen : public IScreen {
public:
    /// Action déclenchée par le bouton d'enregistrement des logs (fournie par l'assemblage).
    using SaveLogAction = std::function<void()>;

    /**
     * @brief Construit l'écran de menu.
     * @param localization Catalogue de traduction (référence **mutable** : le bouton de langue
     *                     le recharge lors d'une bascule).
     * @param onSaveLog    Action exécutée à un clic sur le bouton d'enregistrement des logs
     *                     (peut être vide : le bouton est alors inactif).
     */
    MenuScreen(Localization& localization, SaveLogAction onSaveLog);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    Localization& _localization;
    MenuModel _model;
    LanguageSelector _languageButton;
    SaveLogButton _saveLogButton;
    SaveLogAction _onSaveLog;
    int _viewportWidth = 0;   ///< Dernière largeur connue (pour le survol/clic des boutons).
    int _viewportHeight = 0;  ///< Dernière hauteur connue.
};

}  // namespace hmi
