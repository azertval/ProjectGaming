#include "HMI/Editor/LevelPicker.h"

#include <string_view>
#include <system_error>

#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Input/InputState.h"

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

LevelPicker::LevelPicker(std::vector<Choice> choices) : _choices(std::move(choices)) {}

LevelPicker LevelPicker::forDirectory(const std::filesystem::path& levelsDirectory) {
    std::vector<Choice> choices;
    choices.push_back(Choice{"Nouveau niveau", std::nullopt});

    std::error_code errorCode;
    // Un dossier absent (premier lancement, aucun niveau encore enregistre) n'est pas une
    // erreur : la liste se reduit simplement a "Nouveau niveau".
    if (std::filesystem::is_directory(levelsDirectory, errorCode)) {
        for (const std::filesystem::directory_entry& entry :
             std::filesystem::directory_iterator(levelsDirectory, errorCode)) {
            if (entry.path().extension() == ".json") {
                choices.push_back(Choice{entry.path().stem().string(), entry.path()});
            }
        }
    }
    return LevelPicker(std::move(choices));
}

// Met a jour la selection selon les entrees et renvoie une eventuelle confirmation.
//
// Le clavier deplace la selection (fleches, avec bouclage) ; le survol souris la place sur le
// choix pointe (meme principe que MenuModel). La validation (Entree, ou clic gauche sur un choix
// survole) renvoie l'indice courant.
std::optional<int> LevelPicker::update(const InputState& input) {
    const int count = static_cast<int>(_choices.size());
    if (count == 0) {
        return std::nullopt;
    }
    if (input.keyPressed(Key::Up)) {
        _selected = (_selected + count - 1) % count;
    }
    if (input.keyPressed(Key::Down)) {
        _selected = (_selected + 1) % count;
    }

    const int hovered = optionAtPoint(input.mouseX(), input.mouseY());
    if (hovered >= 0) {
        _selected = hovered;
    }

    bool validated = input.keyPressed(Key::Enter);
    if (input.mouseButtonPressed(MouseButton::Left) && hovered >= 0) {
        validated = true;
    }
    return validated ? std::optional<int>(_selected) : std::nullopt;
}

// Indice du choix dont le rectangle contient (x, y), ou -1.
//
// Le rectangle de chaque libellé est déduit de la mise en page à chasse fixe (même principe que
// MenuModel::optionAtPoint) : coin haut-gauche (MARGIN_X, OPTIONS_TOP + index * OPTION_SPACING),
// largeur du libellé, hauteur d'une ligne.
int LevelPicker::optionAtPoint(int x, int y) const {
    const float pointX = static_cast<float>(x);
    const float pointY = static_cast<float>(y);
    const float height = static_cast<float>(BitmapFont::CELL_HEIGHT) * OPTION_SCALE;
    for (std::size_t index = 0; index < _choices.size(); ++index) {
        const float width = static_cast<float>(countCodePoints(_choices[index].label) *
                                               BitmapFont::CELL_WIDTH) *
                            OPTION_SCALE;
        const float left = MARGIN_X;
        const float right = MARGIN_X + width;
        const float top = OPTIONS_TOP + static_cast<float>(index) * OPTION_SPACING;
        const float bottom = top + height;
        if (pointX >= left && pointX < right && pointY >= top && pointY < bottom) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

}  // namespace hmi
