#pragma once

#include <optional>
#include <string>

/**
 * @file HMI/Interface/OptionsModel.h
 * @brief Logique (testable) du menu d'options : sélection, survol et actions.
 */

namespace hmi {

class Localization;
class InputState;

/**
 * @brief Action produite par la confirmation d'une option du menu d'options.
 *
 * `OptionsModel` ne connaît ni `GraphicsDevice` ni `Localization` en écriture (seulement en
 * lecture, pour les libellés) : c'est `OptionsScreen` qui interprète cette action et mute l'état
 * réel (bascule V-Sync, retour au menu), sur le même principe que `MenuModel::actionFor` produit
 * une `ScreenTransition` interprétée par `ScreenManager`.
 */
enum class OptionsAction {
    ToggleVSync,
    Back,
};

/**
 * @brief Logique du menu d'options, indépendante du dessin.
 *
 * Deux entrées navigables — bascule **V-Sync** et **Retour** — au clavier/manette (flèches +
 * Entrée, fusionnées dans `InputState`, `EX-CTRL-002`) et à la souris (survol + clic), sur le
 * **même** modèle que `MenuModel` : mise en page à chasse fixe, réutilisant directement les
 * constantes publiques de `MenuModel` (`MARGIN_X`, `OPTIONS_TOP`, `OPTION_SPACING`,
 * `OPTION_SCALE`) plutôt que d'en dupliquer un second jeu. Les libellés proviennent du
 * catalogue de traduction par leur clé (`EX-REN-033`).
 */
class OptionsModel {
public:
    /// Nombre d'options du menu.
    static constexpr int OPTION_COUNT = 2;

    /**
     * @brief Construit le modèle d'options.
     * @param localization  Catalogue de traduction résolvant les libellés (référence conservée).
     * @param vsyncEnabled  État V-Sync courant, pour afficher le bon libellé dès la première
     *                      frame (snapshot : voir `setVSyncEnabled` pour la resynchronisation).
     */
    OptionsModel(const Localization& localization, bool vsyncEnabled);

    /**
     * @brief Met à jour la sélection selon les entrées et renvoie une éventuelle action.
     * @param input État des entrées de la frame.
     * @return L'action associée à l'option confirmée (Entrée, ou clic sur une option survolée),
     *         sinon vide.
     */
    [[nodiscard]] std::optional<OptionsAction> update(const InputState& input);

    /// @return L'indice de l'option sélectionnée (0 à OPTION_COUNT-1).
    [[nodiscard]] int selectedIndex() const noexcept {
        return _selected;
    }

    /// @return Le libellé de l'option @p index (résolu par le catalogue ; l'option V-Sync
    ///         inclut son état courant : « Activé »/« Désactivé »).
    [[nodiscard]] std::string optionLabel(int index) const;

    /// @return La largeur en pixels du libellé de l'option @p index (chasse fixe).
    [[nodiscard]] float optionWidth(int index) const;

    /**
     * @brief Resynchronise le libellé affiché après une bascule V-Sync effective.
     * @param enabled Nouvel état V-Sync (déjà appliqué par l'appelant à `GraphicsDevice`).
     */
    void setVSyncEnabled(bool enabled) noexcept {
        _vsyncEnabled = enabled;
    }

private:
    /// @return L'indice de l'option dont le rectangle contient (@p x, @p y), ou -1.
    [[nodiscard]] int optionAtPoint(int x, int y) const;

    /// @return L'action associée à l'option @p index.
    [[nodiscard]] static OptionsAction actionFor(int index) noexcept;

    const Localization& _localization;
    bool _vsyncEnabled;
    int _selected = 0;
};

}  // namespace hmi
