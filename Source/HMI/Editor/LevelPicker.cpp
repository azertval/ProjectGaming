#include "HMI/Editor/LevelPicker.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <system_error>

#include "HMI/Editor/EditorLog.h"
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
        EDITOR_LOG_TRACE("Niveaux trouves dans " + levelsDirectory.string() + " : " +
                         std::to_string(choices.size() - 1));
    } else {
        EDITOR_LOG_TRACE("Dossier de niveaux absent (" + levelsDirectory.string() +
                         ") : liste reduite a 'Nouveau niveau'");
    }
    return LevelPicker(std::move(choices));
}

// Nombre de choix affichables sans defilement pour une hauteur de viewport donnee (au moins 1).
int LevelPicker::visibleCount(float viewportHeight) {
    const int rows = static_cast<int>((viewportHeight - OPTIONS_TOP) / OPTION_SPACING);
    return (std::max)(1, rows);
}

// Borne le defilement a l'intervalle valide, sans autre effet (voir la documentation de l'en-tete).
void LevelPicker::clampScrollRange(int visible) noexcept {
    const int count = static_cast<int>(_choices.size());
    const int maxOffset = (std::max)(0, count - visible);
    _scrollOffset = std::clamp(_scrollOffset, 0, maxOffset);
}

// Recale le defilement pour que la selection reste visible, puis le borne a l'intervalle valide.
void LevelPicker::followSelection(int visible) noexcept {
    if (_selected < _scrollOffset) {
        _scrollOffset = _selected;
    } else if (_selected >= _scrollOffset + visible) {
        _scrollOffset = _selected - visible + 1;
    }
    clampScrollRange(visible);
}

// Met a jour la selection selon les entrees et renvoie une eventuelle confirmation.
//
// Le clavier deplace la selection (fleches, avec bouclage) et fait defiler la fenetre pour la
// suivre ; le survol souris la place sur le choix pointe (meme principe que MenuModel) ; la
// molette defile la liste SANS changer la selection (EX-EDIT-001, liste plus longue que la
// fenetre visible) -- parcourir la liste a la molette ne doit jamais etre annule par un recalage
// sur la selection courante. La validation (Entree, ou clic gauche sur un choix survole) renvoie
// l'indice courant.
std::optional<int> LevelPicker::update(const InputState& input, float viewportHeight) {
    const int count = static_cast<int>(_choices.size());
    if (count == 0) {
        return std::nullopt;
    }
    const int visible = visibleCount(viewportHeight);
    bool movedByKeyboard = false;
    if (input.keyPressed(Key::Up)) {
        _selected = (_selected + count - 1) % count;
        movedByKeyboard = true;
    }
    if (input.keyPressed(Key::Down)) {
        _selected = (_selected + 1) % count;
        movedByKeyboard = true;
    }
    if (movedByKeyboard) {
        followSelection(visible);
    }

    const int wheel = input.wheelDelta();
    if (wheel != 0) {
        constexpr float WHEEL_NOTCH = 120.0f;  // WHEEL_DELTA Win32 : un cran de molette standard.
        _scrollOffset -= static_cast<int>(wheel / WHEEL_NOTCH);
    }
    // Toujours borne (meme sans molette ni clavier ce pas-ci) : protege contre un redimensionnement
    // de fenetre qui reduirait visibleCount() entre deux pas, sans forcer le retour a la selection.
    clampScrollRange(visible);

    const int hovered = optionAtPoint(input.mouseX(), input.mouseY(), visible);
    if (hovered >= 0) {
        _selected = hovered;
    }

    bool validated = input.keyPressed(Key::Enter);
    if (input.mouseButtonPressed(MouseButton::Left) && hovered >= 0) {
        validated = true;
    }
    return validated ? std::optional<int>(_selected) : std::nullopt;
}

// Indice du choix dont le rectangle AFFICHE (compte tenu du defilement courant) contient (x, y),
// ou -1 — seuls les choix de la fenetre visible [scrollOffset, scrollOffset + visibleCount) sont
// testes, les autres etant hors ecran (donc non cliquables).
//
// Le rectangle de chaque libellé visible est déduit de la mise en page à chasse fixe (même
// principe que MenuModel::optionAtPoint) : coin haut-gauche (MARGIN_X, OPTIONS_TOP + rang affiche
// * OPTION_SPACING), largeur du libellé, hauteur d'une ligne.
int LevelPicker::optionAtPoint(int x, int y, int visible) const {
    const float pointX = static_cast<float>(x);
    const float pointY = static_cast<float>(y);
    const float height = static_cast<float>(BitmapFont::CELL_HEIGHT) * OPTION_SCALE;
    const int lastVisible = (std::min)(static_cast<int>(_choices.size()), _scrollOffset + visible);
    for (int index = _scrollOffset; index < lastVisible; ++index) {
        const float width = static_cast<float>(countCodePoints(_choices[static_cast<std::size_t>(index)].label) *
                                               BitmapFont::CELL_WIDTH) *
                            OPTION_SCALE;
        const float left = MARGIN_X;
        const float right = MARGIN_X + width;
        const float top = OPTIONS_TOP + static_cast<float>(index - _scrollOffset) * OPTION_SPACING;
        const float bottom = top + height;
        if (pointX >= left && pointX < right && pointY >= top && pointY < bottom) {
            return index;
        }
    }
    return -1;
}

}  // namespace hmi
