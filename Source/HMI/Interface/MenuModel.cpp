#include "HMI/Interface/MenuModel.h"

#include <string_view>

#include "HMI/Input/InputState.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Une option du menu : sa clé de libellé et l'écran vers lequel elle mène (ou la fermeture).
struct Option {
    const char* key;
    ScreenId target;
    bool quit;
};

// Les quatre options, dans l'ordre d'affichage. Libellés résolus par clé (aucun texte en dur).
constexpr Option OPTIONS[MenuModel::OPTION_COUNT] = {
    {"menu.jouer", ScreenId::Game, false},
    {"menu.mode_edition", ScreenId::Editor, false},
    {"menu.options", ScreenId::Options, false},
    {"menu.quitter", ScreenId::Menu, true},
};

// Le nombre de **code points** UTF-8 de text (un octet de continuation n'en est pas un).
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

// Construit le modèle de menu.
MenuModel::MenuModel(const Localization& localization) : _localization(localization) {}

// Met à jour la sélection selon les entrées et renvoie une éventuelle transition.
//
// Le clavier déplace la sélection (flèches, avec bouclage aux extrémités) ; le survol souris
// la place sur l'option pointée. La validation (Entrée, ou clic gauche sur une option) produit
// la transition de l'option sélectionnée.
ScreenTransition MenuModel::update(const InputState& input) {
    if (input.keyPressed(Key::Up)) {
        _selected = (_selected + OPTION_COUNT - 1) % OPTION_COUNT;
    }
    if (input.keyPressed(Key::Down)) {
        _selected = (_selected + 1) % OPTION_COUNT;
    }

    // Survol : la souris pointant une option la sélectionne (cohérent avec les flèches).
    const int hovered = optionAtPoint(input.mouseX(), input.mouseY());
    if (hovered >= 0) {
        _selected = hovered;
    }

    // Validation : Entrée au clavier, ou clic gauche sur une option survolée.
    bool validated = input.keyPressed(Key::Enter);
    if (input.mouseButtonPressed(MouseButton::Left) && hovered >= 0) {
        validated = true;
    }

    return validated ? actionFor(_selected) : ScreenTransition::none();
}

// Le titre du menu (résolu par le catalogue).
std::string MenuModel::title() const {
    return _localization.text("menu.titre");
}

// Le libellé de l'option index (résolu par le catalogue).
std::string MenuModel::optionLabel(int index) const {
    return _localization.text(OPTIONS[index].key);
}

// La largeur en pixels du libellé de l'option index (chasse fixe).
float MenuModel::optionWidth(int index) const {
    return static_cast<float>(countCodePoints(optionLabel(index)) * BitmapFont::CELL_WIDTH) *
           OPTION_SCALE;
}

// Indice de l'option dont le rectangle contient (x, y), ou -1.
//
// Le rectangle de chaque libellé est déduit de la mise en page à chasse fixe : coin haut-gauche
// (MARGIN_X, optionTop(i)), largeur du libellé, hauteur d'une ligne.
int MenuModel::optionAtPoint(int x, int y) const {
    const float pointX = static_cast<float>(x);
    const float pointY = static_cast<float>(y);
    for (int index = 0; index < OPTION_COUNT; ++index) {
        const float left = MARGIN_X;
        const float right = MARGIN_X + optionWidth(index);
        const float top = optionTop(index);
        const float bottom = top + optionHeight();
        if (pointX >= left && pointX < right && pointY >= top && pointY < bottom) {
            return index;
        }
    }
    return -1;
}

// La transition associée à l'option index.
ScreenTransition MenuModel::actionFor(int index) noexcept {
    const Option& option = OPTIONS[index];
    return option.quit ? ScreenTransition::quit() : ScreenTransition::switchTo(option.target);
}

}  // namespace hmi
