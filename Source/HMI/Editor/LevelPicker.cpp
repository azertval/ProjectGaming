#include "HMI/Editor/LevelPicker.h"

#include <system_error>

#include "HMI/Input/InputState.h"

namespace hmi {

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
    if (input.keyPressed(Key::Enter)) {
        return _selected;
    }
    return std::nullopt;
}

}  // namespace hmi
