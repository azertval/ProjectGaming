#include "HMI/Interface/EditorKeybindingsModel.h"

#include <array>

#include "HMI/Input/InputState.h"
#include "HMI/Input/KeyName.h"
#include "HMI/Interface/MenuModel.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {
// Cles de traduction des neuf actions, dans l'ordre d'EditorAction (Save=0 .. Rename=8).
constexpr std::array<const char*, EDITOR_ACTION_COUNT> ACTION_LABEL_KEYS = {
    "keybindings.action.sauvegarder", "keybindings.action.annuler",
    "keybindings.action.refaire",     "keybindings.action.copier",
    "keybindings.action.coller",      "keybindings.action.test_rapide",
    "keybindings.action.grille",      "keybindings.action.aide",
    "keybindings.action.renommer",
};
}  // namespace

EditorKeybindingsModel::EditorKeybindingsModel(const Localization& localization,
                                              EditorKeyBindings& bindings,
                                              std::filesystem::path savePath)
    : _localization(localization), _bindings(bindings), _savePath(std::move(savePath)) {}

// Met a jour la selection/capture selon les entrees (meme protocole que GameKeybindingsModel).
std::optional<EditorKeybindingsAction> EditorKeybindingsModel::update(const InputState& input) {
    if (_capturing) {
        if (input.keyPressed(Key::Escape)) {
            _capturing = false;
            return std::nullopt;
        }
        const std::optional<Key> captured = capturedKey(input);
        if (!captured) {
            return std::nullopt;
        }
        _bindings.setKey(actionForRow(_selected), *captured);
        _bindings.save(_savePath);
        _capturing = false;
        return EditorKeybindingsAction::Rebound;
    }

    if (input.keyPressed(Key::Up)) {
        _selected = (_selected + ROW_COUNT - 1) % ROW_COUNT;
    }
    if (input.keyPressed(Key::Down)) {
        _selected = (_selected + 1) % ROW_COUNT;
    }

    const int hovered = rowAtPoint(input.mouseX(), input.mouseY());
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

    if (_selected == BACK_ROW) {
        return EditorKeybindingsAction::Back;
    }
    if (_selected == RESET_ROW) {
        _bindings.resetToDefaults();
        _bindings.save(_savePath);
        return EditorKeybindingsAction::Reset;
    }
    _capturing = true;
    return std::nullopt;
}

std::string EditorKeybindingsModel::rowLabel(int index) const {
    if (index == RESET_ROW) {
        return _localization.text("keybindings.reinitialiser");
    }
    if (index == BACK_ROW) {
        return _localization.text("options.retour");
    }
    return _localization.text(ACTION_LABEL_KEYS[static_cast<std::size_t>(index)]);
}

std::string EditorKeybindingsModel::rowValue(int index) const {
    if (index == RESET_ROW || index == BACK_ROW) {
        return "";
    }
    if (_capturing && index == _selected) {
        return _localization.text("keybindings.appuyez_touche");
    }
    return keyDisplayName(_bindings.key(actionForRow(index)));
}

int EditorKeybindingsModel::rowAtPoint(int x, int y) const {
    const float pointX = static_cast<float>(x);
    const float pointY = static_cast<float>(y);
    for (int index = 0; index < ROW_COUNT; ++index) {
        const float top = GameKeybindingsModel::rowTop(index);
        const float bottom = top + GameKeybindingsModel::rowHeight();
        if (pointX >= MenuModel::MARGIN_X &&
            pointX < MenuModel::MARGIN_X + GameKeybindingsModel::ROW_CLICK_WIDTH &&
            pointY >= top && pointY < bottom) {
            return index;
        }
    }
    return -1;
}

EditorAction EditorKeybindingsModel::actionForRow(int index) noexcept {
    return static_cast<EditorAction>(index);
}

}  // namespace hmi
