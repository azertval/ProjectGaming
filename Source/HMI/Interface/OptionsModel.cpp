#include "HMI/Interface/OptionsModel.h"

#include <algorithm>
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

// Nombre d'options affichables sans defilement pour une hauteur de viewport donnee (au moins 1) —
// meme calcul que LevelPicker::visibleCount, sur les constantes de MenuModel.
int OptionsModel::visibleOptionCount(float viewportHeight) {
    const int rows =
        static_cast<int>((viewportHeight - MenuModel::OPTIONS_TOP) / MenuModel::OPTION_SPACING);
    return (std::max)(1, rows);
}

// Borne le defilement a l'intervalle valide, sans autre effet.
void OptionsModel::clampScrollRange(int visible) noexcept {
    const int maxOffset = (std::max)(0, OPTION_COUNT - visible);
    _scrollOffset = std::clamp(_scrollOffset, 0, maxOffset);
}

// Recale le defilement pour que la selection reste visible, puis le borne.
void OptionsModel::followSelection(int visible) noexcept {
    if (_selected < _scrollOffset) {
        _scrollOffset = _selected;
    } else if (_selected >= _scrollOffset + visible) {
        _scrollOffset = _selected - visible + 1;
    }
    clampScrollRange(visible);
}

// Met à jour la sélection/défilement selon les entrées et renvoie une éventuelle action.
//
// Même principe que MenuModel::update pour la sélection (flèches, bouclage, survol souris, clic) ;
// le clavier fait suivre la selection dans la fenetre visible (followSelection), la molette
// defile SANS changer la selection (meme principe que LevelPicker::update).
std::optional<OptionsAction> OptionsModel::update(const InputState& input, float viewportHeight) {
    const int visible = visibleOptionCount(viewportHeight);
    bool movedByKeyboard = false;
    if (input.keyPressed(Key::Up)) {
        _selected = (_selected + OPTION_COUNT - 1) % OPTION_COUNT;
        movedByKeyboard = true;
    }
    if (input.keyPressed(Key::Down)) {
        _selected = (_selected + 1) % OPTION_COUNT;
        movedByKeyboard = true;
    }
    if (movedByKeyboard) {
        followSelection(visible);
    }

    const int wheel = input.wheelDelta();
    if (wheel != 0) {
        constexpr float WHEEL_NOTCH = 120.0f;  // WHEEL_DELTA Win32 : un cran de molette standard.
        _scrollOffset -= static_cast<int>(static_cast<float>(wheel) / WHEEL_NOTCH);
    }
    // Toujours borne (meme sans molette ni clavier ce pas-ci) : protege contre un redimensionnement
    // de fenetre qui reduirait visibleOptionCount() entre deux pas.
    clampScrollRange(visible);

    const int hovered = optionAtPoint(input.mouseX(), input.mouseY(), visible);
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
    switch (index) {
        case 0:
            return _localization.text(_vsyncEnabled ? "options.vsync_on" : "options.vsync_off");
        case 1:
            return _localization.text("keybindings.titre_jeu");
        case 2:
            return _localization.text("keybindings.titre_editeur");
        case 3:
            return _localization.text("keybindings.titre_manette");
        default:
            return _localization.text("options.retour");
    }
}

// La largeur en pixels du libellé de l'option index (chasse fixe).
float OptionsModel::optionWidth(int index) const {
    return static_cast<float>(countCodePoints(optionLabel(index)) * BitmapFont::CELL_WIDTH) *
          MenuModel::OPTION_SCALE;
}

// Indice de l'option dont le rectangle AFFICHE (compte tenu du defilement courant) contient
// (x, y), ou -1 — seules les options de la fenetre visible sont testees, meme principe que
// LevelPicker::optionAtPoint.
int OptionsModel::optionAtPoint(int x, int y, int visible) const {
    const float pointX = static_cast<float>(x);
    const float pointY = static_cast<float>(y);
    const int lastVisible = (std::min)(OPTION_COUNT, _scrollOffset + visible);
    for (int index = _scrollOffset; index < lastVisible; ++index) {
        const float left = MenuModel::MARGIN_X;
        const float right = MenuModel::MARGIN_X + optionWidth(index);
        const float top = MenuModel::OPTIONS_TOP +
                         static_cast<float>(index - _scrollOffset) * MenuModel::OPTION_SPACING;
        const float bottom = top + MenuModel::optionHeight();
        if (pointX >= left && pointX < right && pointY >= top && pointY < bottom) {
            return index;
        }
    }
    return -1;
}

// L'action associée à l'option index.
OptionsAction OptionsModel::actionFor(int index) noexcept {
    switch (index) {
        case 0:
            return OptionsAction::ToggleVSync;
        case 1:
            return OptionsAction::OpenGameKeybindings;
        case 2:
            return OptionsAction::OpenEditorKeybindings;
        case 3:
            return OptionsAction::OpenGamepadBindings;
        default:
            return OptionsAction::Back;
    }
}

}  // namespace hmi
