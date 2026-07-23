#pragma once

#include <string>
#include <string_view>

/**
 * @file HMI/Interface/LanguageSelector.h
 * @brief Logique (testable) du bouton de changement de langue en bas à droite.
 */

namespace hmi {

class InputState;

/**
 * @brief Logique du bouton de langue : un rectangle en bas à droite qui bascule FR ↔ EN.
 *
 * Le bouton occupe un rectangle fixe ancré au coin **bas-droit** de la surface de rendu. Un
 * clic gauche à l'intérieur demande de basculer vers l'**autre** langue (français ↔ anglais).
 * La logique est pure (aucune ressource GPU) : le dessin de l'icône (drapeau de la langue
 * courante) est assuré par l'écran. Testable en isolation (`EX-NFR-010`).
 */
class LanguageSelector {
public:
    /// Largeur du bouton, en pixels d'écran.
    static constexpr float BUTTON_WIDTH = 72.0f;
    /// Hauteur du bouton, en pixels d'écran.
    static constexpr float BUTTON_HEIGHT = 48.0f;
    /// Marge entre le bouton et les bords bas/droit, en pixels.
    static constexpr float MARGIN = 24.0f;

    /// Rectangle du bouton, en pixels d'écran.
    struct Rect {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    /// Résultat d'une mise à jour : une éventuelle demande de bascule de langue.
    struct Toggle {
        bool requested = false;
        std::string next;  ///< Langue vers laquelle basculer si `requested`.
    };

    /**
     * @brief Rectangle du bouton pour une surface de rendu donnée (ancré en bas à droite).
     * @param viewportWidth  Largeur de la surface, en pixels.
     * @param viewportHeight Hauteur de la surface, en pixels.
     */
    [[nodiscard]] static Rect rect(int viewportWidth, int viewportHeight) noexcept;

    /**
     * @brief L'autre langue (français ↔ anglais).
     * @param current Langue courante.
     * @return « fr » si @p current vaut « en », sinon « en ».
     */
    [[nodiscard]] static std::string other(std::string_view current);

    /**
     * @brief Traite les entrées et indique une éventuelle demande de bascule.
     * @param input          État des entrées de la frame.
     * @param current        Langue active courante.
     * @param viewportWidth  Largeur de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur de la surface de rendu, en pixels.
     * @return Une demande de bascule si le bouton a été cliqué cette frame, sinon aucune.
     */
    [[nodiscard]] Toggle update(const InputState& input, std::string_view current,
                                int viewportWidth, int viewportHeight) const;
};

}  // namespace hmi
