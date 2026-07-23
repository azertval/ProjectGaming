#pragma once

#include <string>

#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Interface/IScreen.h"

namespace hmi {

class Localization;

/**
 * @file HMI/Interface/MenuModel.h
 * @brief Logique (testable) du menu principal : sélection, survol et transitions.
 */

/**
 * @brief Logique du menu principal, indépendante du dessin.
 *
 * Modélise les quatre options du menu (Charger niveau, Mode Edition, Options, Quitter), la **sélection**
 * courante et sa mise à jour au **clavier** (flèches + Entrée) et à la **souris** (survol +
 * clic), puis produit la **transition** correspondante. Les libellés proviennent du catalogue
 * de traduction par leur **clé** (`EX-REN-033`) : aucun texte en dur. La mise en page (chasse
 * fixe) est calculée à partir des constantes de la police, sans dépendre du GPU, ce qui rend
 * cette logique testable en isolation (`EX-NFR-010`). Le dessin est assuré par `MenuScreen`.
 */
class MenuModel {
public:
    /// Nombre d'options du menu.
    static constexpr int OPTION_COUNT = 4;

    // Mise en page en pixels (espace écran), partagée par la logique et le dessin.
    static constexpr float MARGIN_X = 80.0f;        ///< Marge gauche des textes.
    static constexpr float TITLE_Y = 90.0f;         ///< Ordonnée du titre.
    static constexpr float TITLE_SCALE = 8.0f;      ///< Échelle du titre.
    static constexpr float OPTIONS_TOP = 320.0f;    ///< Ordonnée de la première option.
    static constexpr float OPTION_SPACING = 96.0f;  ///< Écart vertical entre options.
    static constexpr float OPTION_SCALE = 6.0f;     ///< Échelle des options.

    /**
     * @brief Construit le modèle de menu.
     * @param localization Catalogue de traduction résolvant les libellés (référence conservée).
     */
    explicit MenuModel(const Localization& localization);

    /**
     * @brief Met à jour la sélection selon les entrées et renvoie une éventuelle transition.
     * @param input État des entrées de la frame.
     * @return La transition à appliquer (Entrée ou clic sur une option), sinon « rester ».
     */
    [[nodiscard]] ScreenTransition update(const InputState& input);

    /// @return L'indice de l'option sélectionnée (0 à OPTION_COUNT-1).
    [[nodiscard]] int selectedIndex() const noexcept {
        return _selected;
    }

    /// @return Le titre du menu (résolu par le catalogue).
    [[nodiscard]] std::string title() const;

    /// @return Le libellé de l'option @p index (résolu par le catalogue).
    [[nodiscard]] std::string optionLabel(int index) const;

    /// @return La largeur en pixels du libellé de l'option @p index (chasse fixe).
    [[nodiscard]] float optionWidth(int index) const;

    /// @return L'ordonnée (haut) de l'option @p index, en pixels.
    [[nodiscard]] static constexpr float optionTop(int index) noexcept {
        return OPTIONS_TOP + static_cast<float>(index) * OPTION_SPACING;
    }

    /// @return La hauteur d'une option, en pixels.
    [[nodiscard]] static constexpr float optionHeight() noexcept {
        return static_cast<float>(BitmapFont::CELL_HEIGHT) * OPTION_SCALE;
    }

private:
    /// @return L'indice de l'option dont le rectangle contient (@p x, @p y), ou -1.
    [[nodiscard]] int optionAtPoint(int x, int y) const;

    /// @return La transition associée à l'option @p index.
    [[nodiscard]] static ScreenTransition actionFor(int index) noexcept;

    const Localization& _localization;
    int _selected = 0;
};

}  // namespace hmi
