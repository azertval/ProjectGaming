#include "HMI/Input/QtKeyMap.h"

#include <QtGlobal>
#include <qnamespace.h>

namespace hmi {

std::optional<hmi::Key> qtKeyToHmiKey(int qtKey) {
    switch (qtKey) {
        case Qt::Key_Escape:
            return hmi::Key::Escape;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            return hmi::Key::Tab;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            return hmi::Key::Enter;
        case Qt::Key_Backspace:
            return hmi::Key::Backspace;
        case Qt::Key_Shift:
            return hmi::Key::Shift;
        case Qt::Key_Control:
            return hmi::Key::Control;
        case Qt::Key_Space:
            return hmi::Key::Space;
        case Qt::Key_Left:
            return hmi::Key::Left;
        case Qt::Key_Up:
            return hmi::Key::Up;
        case Qt::Key_Right:
            return hmi::Key::Right;
        case Qt::Key_Down:
            return hmi::Key::Down;
        case Qt::Key_F1:
            return hmi::Key::F1;
        case Qt::Key_F2:
            return hmi::Key::F2;
        case Qt::Key_F10:
            return hmi::Key::F10;
        default:
            break;
    }
    // Lettres A–Z et chiffres 0–9 : Qt reprend les valeurs ASCII majuscules, identiques aux codes
    // virtuels Win32 — conversion directe.
    if ((qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9) || (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)) {
        return static_cast<hmi::Key>(qtKey);
    }
    return std::nullopt;
}

}  // namespace hmi
