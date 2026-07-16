#include "HMI/Interface/LanguageSelector.h"

#include "HMI/Input/InputState.h"

namespace hmi {

/**
 * @brief Rectangle du bouton pour une surface de rendu donnée (ancré en bas à droite).
 * @param viewportWidth  Largeur de la surface, en pixels.
 * @param viewportHeight Hauteur de la surface, en pixels.
 */
LanguageSelector::Rect LanguageSelector::rect(int viewportWidth, int viewportHeight) noexcept {
    Rect area;
    area.width = BUTTON_WIDTH;
    area.height = BUTTON_HEIGHT;
    area.x = static_cast<float>(viewportWidth) - MARGIN - BUTTON_WIDTH;
    area.y = static_cast<float>(viewportHeight) - MARGIN - BUTTON_HEIGHT;
    return area;
}

/**
 * @brief L'autre langue (français ↔ anglais).
 * @param current Langue courante.
 * @return « fr » si @p current vaut « en », sinon « en ».
 */
std::string LanguageSelector::other(std::string_view current) {
    return current == "en" ? std::string("fr") : std::string("en");
}

/**
 * @brief Traite les entrées et indique une éventuelle demande de bascule.
 * @param input          État des entrées de la frame.
 * @param current        Langue active courante.
 * @param viewportWidth  Largeur de la surface de rendu, en pixels.
 * @param viewportHeight Hauteur de la surface de rendu, en pixels.
 * @return Une demande de bascule si le bouton a été cliqué cette frame, sinon aucune.
 */
LanguageSelector::Toggle LanguageSelector::update(const InputState& input, std::string_view current,
                                                  int viewportWidth, int viewportHeight) const {
    if (!input.mouseButtonPressed(MouseButton::Left)) {
        return {};
    }

    const Rect area = rect(viewportWidth, viewportHeight);
    const float pointX = static_cast<float>(input.mouseX());
    const float pointY = static_cast<float>(input.mouseY());
    const bool inside = pointX >= area.x && pointX < area.x + area.width && pointY >= area.y &&
                        pointY < area.y + area.height;
    if (!inside) {
        return {};
    }

    return {true, other(current)};
}

}  // namespace hmi
