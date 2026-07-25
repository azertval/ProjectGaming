#include "HMI/Interface/GameKeybindingsModel.h"

#include <array>

#include "HMI/Input/InputState.h"
#include "HMI/Input/KeyName.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {
// Cles de traduction des six actions, dans l'ordre de GameAction (MoveLeft=0 .. Dash=5).
constexpr std::array<const char*, GAME_ACTION_COUNT> ACTION_LABEL_KEYS = {
    "keybindings.action.gauche", "keybindings.action.droite", "keybindings.action.haut",
    "keybindings.action.bas",    "keybindings.action.sauter", "keybindings.action.dash",
};
}  // namespace

GameKeybindingsModel::GameKeybindingsModel(const Localization& localization,
                                          GameKeyBindings& bindings,
                                          std::filesystem::path savePath)
    : _localization(localization), _bindings(bindings), _savePath(std::move(savePath)) {}

// Met a jour la selection/capture selon les entrees (voir en-tete pour le detail du protocole).
std::optional<GameKeybindingsAction> GameKeybindingsModel::update(const InputState& input) {
    if (_capturing) {
        if (input.keyPressed(Key::Escape)) {
            _capturing = false;  // annule la capture, binding inchange (convention TextInputField)
            return std::nullopt;
        }
        const std::optional<Key> captured = capturedKey(input);
        if (!captured) {
            return std::nullopt;  // rien d'assignable presse cette frame : capture reste ouverte
        }
        _bindings.setKey(actionForRow(_selected), *captured);
        _bindings.save(_savePath);
        _capturing = false;
        return GameKeybindingsAction::Rebound;
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
        return GameKeybindingsAction::Back;
    }
    if (_selected == RESET_ROW) {
        _bindings.resetToDefaults();
        _bindings.save(_savePath);
        return GameKeybindingsAction::Reset;
    }
    _capturing = true;
    return std::nullopt;
}

std::string GameKeybindingsModel::rowLabel(int index) const {
    if (index == RESET_ROW) {
        return _localization.text("keybindings.reinitialiser");
    }
    if (index == BACK_ROW) {
        return _localization.text("options.retour");
    }
    return _localization.text(ACTION_LABEL_KEYS[static_cast<std::size_t>(index)]);
}

std::string GameKeybindingsModel::rowValue(int index) const {
    if (index == RESET_ROW || index == BACK_ROW) {
        return "";
    }
    if (_capturing && index == _selected) {
        return _localization.text("keybindings.appuyez_touche");
    }
    return keyDisplayName(_bindings.key(actionForRow(index)));
}

// Indice de la ligne dont le rectangle contient (x, y), ou -1. Largeur cliquable fixe (pas une
// chasse mesuree) : chaque ligne combine un libelle et une valeur de largeur variable.
int GameKeybindingsModel::rowAtPoint(int x, int y) const {
    const float pointX = static_cast<float>(x);
    const float pointY = static_cast<float>(y);
    for (int index = 0; index < ROW_COUNT; ++index) {
        const float top = rowTop(index);
        const float bottom = top + rowHeight();
        if (pointX >= MenuModel::MARGIN_X && pointX < MenuModel::MARGIN_X + ROW_CLICK_WIDTH &&
            pointY >= top && pointY < bottom) {
            return index;
        }
    }
    return -1;
}

GameAction GameKeybindingsModel::actionForRow(int index) noexcept {
    return static_cast<GameAction>(index);
}

}  // namespace hmi
