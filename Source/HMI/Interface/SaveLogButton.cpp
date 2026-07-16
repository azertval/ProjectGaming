#include "HMI/Interface/SaveLogButton.h"

#include "HMI/Input/InputState.h"
#include "HMI/Interface/LanguageSelector.h"

namespace hmi {

// Rectangle du bouton, ancré à gauche du bouton de langue.
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

// Indique si le bouton a été cliqué cette frame.
// true si un clic gauche est survenu dans le rectangle du bouton.
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
