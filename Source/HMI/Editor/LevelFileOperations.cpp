#include "HMI/Editor/LevelFileOperations.h"

#include <algorithm>
#include <system_error>
#include <utility>

#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "HMI/Editor/LevelNameValidation.h"

namespace hmi {

namespace {

// Charge un niveau, en fait un brouillon renommé, valide, écrit à `target`. Facteur commun à
// rename/duplicate. Renvoie un FileOpResult (jamais d'exception).
[[nodiscard]] FileOpResult writeRenamed(const std::filesystem::path& source,
                                        const std::string& newName,
                                        const std::filesystem::path& target) {
    core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(source);
    if (!loaded.ok()) {
        return FileOpResult::failure("Niveau source illisible : " + loaded.error);
    }
    core::LevelDraft draft = core::LevelDraft::fromLevel(*loaded.level);
    draft.setName(newName);
    core::LevelLoadResult validated = draft.toLevel();
    if (!validated.ok()) {
        return FileOpResult::failure("Niveau invalide : " + validated.error);
    }
    if (!core::LevelWriter::saveToFile(*validated.level, target)) {
        return FileOpResult::failure("Échec de l'écriture du fichier.");
    }
    return FileOpResult::success(target);
}

}  // namespace

LevelFileOperations::LevelFileOperations(std::filesystem::path levelsDir)
    : _dir(std::move(levelsDir)) {}

std::filesystem::path LevelFileOperations::pathForName(const std::string& name) const {
    return _dir / (name + ".json");
}

std::vector<std::filesystem::path> LevelFileOperations::list() const {
    std::vector<std::filesystem::path> levels;
    std::error_code error;
    if (!std::filesystem::is_directory(_dir, error)) {
        return levels;  // dossier absent : liste vide (robustesse).
    }
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(_dir, error)) {
        if (entry.is_regular_file(error) && entry.path().extension() == ".json") {
            levels.push_back(entry.path());
        }
    }
    std::ranges::sort(levels, [](const auto& lhs, const auto& rhs) {
        return lhs.filename().string() < rhs.filename().string();
    });
    return levels;
}

FileOpResult LevelFileOperations::create(const std::string& name, int width, int height) const {
    const std::string trimmed = hmi::trimLevelName(name);
    if (!hmi::isValidLevelName(name)) {
        return FileOpResult::failure("Nom de niveau invalide.");
    }
    if (width < 2 || height < 1) {
        return FileOpResult::failure("Dimensions trop petites.");
    }
    const std::filesystem::path target = pathForName(trimmed);
    std::error_code error;
    if (std::filesystem::exists(target, error)) {
        return FileOpResult::failure("Un niveau porte déjà ce nom.");
    }
    // Niveau minimal valide : grille vide + une entrée et une sortie distinctes (coins bas).
    core::LevelDraft draft = core::LevelDraft::empty(trimmed, width, height);
    draft.setEntry(0, height - 1);
    draft.setExit(width - 1, height - 1);
    core::LevelLoadResult validated = draft.toLevel();
    if (!validated.ok()) {
        return FileOpResult::failure("Niveau invalide : " + validated.error);
    }
    if (!core::LevelWriter::saveToFile(*validated.level, target)) {
        return FileOpResult::failure("Échec de l'écriture du fichier.");
    }
    return FileOpResult::success(target);
}

FileOpResult LevelFileOperations::rename(const std::filesystem::path& source,
                                         const std::string& newName) const {
    std::error_code error;
    if (!std::filesystem::exists(source, error)) {
        return FileOpResult::failure("Niveau introuvable.");
    }
    const std::string trimmed = hmi::trimLevelName(newName);
    if (!hmi::isValidLevelName(newName)) {
        return FileOpResult::failure("Nom de niveau invalide.");
    }
    const std::filesystem::path target = pathForName(trimmed);
    if (target != source && std::filesystem::exists(target, error)) {
        return FileOpResult::failure("Un niveau porte déjà ce nom.");
    }
    const FileOpResult written = writeRenamed(source, trimmed, target);
    if (written.ok && target != source) {
        std::filesystem::remove(source, error);  // retire l'ancien fichier (échec non bloquant).
    }
    return written;
}

FileOpResult LevelFileOperations::duplicate(const std::filesystem::path& source) const {
    std::error_code error;
    if (!std::filesystem::exists(source, error)) {
        return FileOpResult::failure("Niveau introuvable.");
    }
    const std::string base = source.stem().string();
    // Cherche un nom de copie disponible : « base (copie) », « base (copie 2) », …
    std::string candidate = base + " (copie)";
    for (int index = 2; std::filesystem::exists(pathForName(candidate), error); ++index) {
        candidate = base + " (copie " + std::to_string(index) + ")";
    }
    return writeRenamed(source, candidate, pathForName(candidate));
}

FileOpResult LevelFileOperations::remove(const std::filesystem::path& source) {
    std::error_code error;
    if (!std::filesystem::exists(source, error)) {
        return FileOpResult::failure("Niveau introuvable.");
    }
    if (!std::filesystem::remove(source, error)) {
        return FileOpResult::failure("Échec de la suppression.");
    }
    return FileOpResult::success(source);
}

}  // namespace hmi
