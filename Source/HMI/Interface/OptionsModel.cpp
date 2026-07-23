#include "HMI/Interface/OptionsModel.h"

#include <string_view>

#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/MenuModel.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Le nombre de **code points** UTF-8 de text (un octet de continuation n'en est pas un) — voir
// MenuModel::countCodePoints, même besoin (largeur d'un libellé à chasse fixe).
[[nodiscard]] int countCodePoints(std::string_view text) noexcept {
    int count = 0;
    for (const char byte : text) {
        if ((static_cast<unsigned char>(byte) & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

}  // namespace

// Construit le modèle d'options.
OptionsModel::OptionsModel(const Localization& localization, bool vsyncEnabled)
    : _localization(localization), _vsyncEnabled(vsyncEnabled) {}

// Met à jour la sélection selon les entrées et renvoie une éventuelle action.
//
// Même principe que MenuModel::update : le clavier/la manette déplacent la sélection (flèches,
// bouclage), le survol souris la place sur l'option pointée, la validation (Entrée/A manette, ou
// clic gauche sur une option survolée) produit l'action de l'option sélectionnée.
std::optional<OptionsAction> OptionsModel::update(const InputState& input) {
    if (input.keyPressed(Key::Up)) {
        _selected = (_selected + OPTION_COUNT - 1) % OPTION_COUNT;
    }
    if (input.keyPressed(Key::Down)) {
        _selected = (_selected + 1) % OPTION_COUNT;
    }

    const int hovered = optionAtPoint(input.mouseX(), input.mouseY());
    if (hovered >= 0) {
        _selected = hovered;
    }

    bool validated = input.keyPressed(Key::Enter);
    if (input.mouseButtonPressed(MouseButton::Left) && hovered >= 0) {
        validated = true;
    }

    if (!validated) {
        return std::nullopt;
    }
    return actionFor(_selected);
}

// Le libellé de l'option index (résolu par le catalogue ; V-Sync inclut son état courant).
std::string OptionsModel::optionLabel(int index) const {
    if (index == 0) {
        return _localization.text(_vsyncEnabled ? "options.vsync_on" : "options.vsync_off");
    }
    return _localization.text("options.retour");
}

// La largeur en pixels du libellé de l'option index (chasse fixe).
float OptionsModel::optionWidth(int index) const {
    return static_cast<float>(countCodePoints(optionLabel(index)) * BitmapFont::CELL_WIDTH) *
          MenuModel::OPTION_SCALE;
}

// Indice de l'option dont le rectangle contient (x, y), ou -1. Mise en page à chasse fixe
// partagée avec MenuModel (mêmes constantes publiques, pas de duplication).
int OptionsModel::optionAtPoint(int x, int y) const {
    const float pointX = static_cast<float>(x);
    const float pointY = static_cast<float>(y);
    for (int index = 0; index < OPTION_COUNT; ++index) {
        const float left = MenuModel::MARGIN_X;
        const float right = MenuModel::MARGIN_X + optionWidth(index);
        const float top = MenuModel::optionTop(index);
        const float bottom = top + MenuModel::optionHeight();
        if (pointX >= left && pointX < right && pointY >= top && pointY < bottom) {
            return index;
        }
    }
    return -1;
}

// L'action associée à l'option index.
OptionsAction OptionsModel::actionFor(int index) noexcept {
    return index == 0 ? OptionsAction::ToggleVSync : OptionsAction::Back;
}

}  // namespace hmi
