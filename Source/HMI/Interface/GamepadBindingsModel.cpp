#include "HMI/Interface/GamepadBindingsModel.h"

#include <array>

#include "HMI/Input/GamepadButtonName.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/MenuModel.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {
// Cles de traduction des six actions, dans l'ordre de GameAction (MoveLeft=0 .. Dash=5) - memes
// cles que GameKeybindingsModel, les deux sous-menus nomment les memes actions.
constexpr std::array<const char*, GAME_ACTION_COUNT> ACTION_LABEL_KEYS = {
    "keybindings.action.gauche", "keybindings.action.droite", "keybindings.action.haut",
    "keybindings.action.bas",    "keybindings.action.sauter", "keybindings.action.dash",
};
}  // namespace

GamepadBindingsModel::GamepadBindingsModel(const Localization& localization,
                                          GamepadBindings& bindings,
                                          std::filesystem::path savePath)
    : _localization(localization), _bindings(bindings), _savePath(std::move(savePath)) {}

// Met a jour la selection/capture selon les entrees (meme protocole que GameKeybindingsModel) ;
// une capture ne peut demarrer que si une manette est connectee.
std::optional<GamepadBindingsAction> GamepadBindingsModel::update(const InputState& input) {
    if (_capturing) {
        if (input.keyPressed(Key::Escape)) {
            _capturing = false;
            return std::nullopt;
        }
        const std::optional<GamepadButton> captured = capturedGamepadButton(input);
        if (!captured) {
            return std::nullopt;
        }
        _bindings.setKey(actionForRow(_selected), *captured);
        _bindings.save(_savePath);
        _capturing = false;
        return GamepadBindingsAction::Rebound;
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
        return GamepadBindingsAction::Back;
    }
    if (_selected == RESET_ROW) {
        _bindings.resetToDefaults();
        _bindings.save(_savePath);
        return GamepadBindingsAction::Reset;
    }
    if (!input.gamepadConnected()) {
        return std::nullopt;  // pas de manette : capture impossible, ligne reste selectionnee.
    }
    _capturing = true;
    return std::nullopt;
}

std::string GamepadBindingsModel::rowLabel(int index) const {
    if (index == RESET_ROW) {
        return _localization.text("keybindings.reinitialiser");
    }
    if (index == BACK_ROW) {
        return _localization.text("options.retour");
    }
    return _localization.text(ACTION_LABEL_KEYS[static_cast<std::size_t>(index)]);
}

std::string GamepadBindingsModel::rowValue(int index) const {
    if (index == RESET_ROW || index == BACK_ROW) {
        return "";
    }
    if (_capturing && index == _selected) {
        return _localization.text("keybindings.appuyez_bouton");
    }
    return gamepadButtonDisplayName(_bindings.button(actionForRow(index)));
}

int GamepadBindingsModel::rowAtPoint(int x, int y) const {
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

GameAction GamepadBindingsModel::actionForRow(int index) noexcept {
    return static_cast<GameAction>(index);
}

}  // namespace hmi
