#include "HMI/Interface/SaveLogButton.h"

#include "HMI/Input/InputState.h"
#include "HMI/Interface/LanguageSelector.h"

namespace hmi {

/**
 * @brief Rectangle du bouton, ancré à gauche du bouton de langue.
 * @param viewportWidth  Largeur de la surface, en pixels.
 * @param viewportHeight Hauteur de la surface, en pixels.
 */
SaveLogButton::Rect SaveLogButton::rect(int viewportWidth, int viewportHeight) noexcept {
    Rect area;
    area.width = SIZE;
    area.height = SIZE;
    // À gauche du bouton de langue, aligné sur le même bord bas.
    area.x = static_cast<float>(viewportWidth) - LanguageSelector::MARGIN -
             LanguageSelector::BUTTON_WIDTH - GAP - SIZE;
    area.y = static_cast<float>(viewportHeight) - LanguageSelector::MARGIN - SIZE;
    return area;
}

/**
 * @brief Indique si le bouton a été cliqué cette frame.
 * @param input          État des entrées de la frame.
 * @param viewportWidth  Largeur de la surface de rendu, en pixels.
 * @param viewportHeight Hauteur de la surface de rendu, en pixels.
 * @return true si un clic gauche est survenu dans le rectangle du bouton.
 */
bool SaveLogButton::clicked(const InputState& input, int viewportWidth,
                            int viewportHeight) const {
    if (!input.mouseButtonPressed(MouseButton::Left)) {
        return false;
    }
    const Rect area = rect(viewportWidth, viewportHeight);
    const float pointX = static_cast<float>(input.mouseX());
    const float pointY = static_cast<float>(input.mouseY());
    return pointX >= area.x && pointX < area.x + area.width && pointY >= area.y &&
           pointY < area.y + area.height;
}

}  // namespace hmi
