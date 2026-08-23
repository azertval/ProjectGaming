// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/AssetFileOperations.h"

#include <algorithm>
#include <system_error>
#include <utility>

#include "HMI/Editor/LevelNameValidation.h"

namespace hmi {

AssetFileOperations::AssetFileOperations(std::filesystem::path assetsDirectory)
    : _directory(std::move(assetsDirectory)) {}

std::filesystem::path AssetFileOperations::pathForStem(
    const std::string& stem, const std::filesystem::path& extension) const {
    return _directory / (stem + extension.string());
}

std::vector<std::filesystem::path> AssetFileOperations::list() const {
    std::vector<std::filesystem::path> assets;
    std::error_code error;
    if (!std::filesystem::is_directory(_directory, error)) {
        return assets;  // dossier absent : liste vide (meme robustesse que LevelFileOperations).
    }
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(_directory, error)) {
        if (entry.is_regular_file(error)) {
            assets.push_back(entry.path());
        }
    }
    std::ranges::sort(assets, [](const auto& lhs, const auto& rhs) {
        return lhs.filename().string() < rhs.filename().string();
    });
    return assets;
}

FileOpResult AssetFileOperations::import(const std::filesystem::path& sourceFile,
                                         AssetFamily family, int decodedWidth,
                                         int decodedHeight) const {
    std::error_code error;
    if (!std::filesystem::is_regular_file(sourceFile, error)) {
        return FileOpResult::failure("Fichier source introuvable.");
    }

    // Validation AVANT la copie, sur des dimensions deja decodees par l'appelant : un asset non
    // conforme doit etre refuse a l'import, pas decouvert plus tard au chargement par le
    // TextureCache.
    const AssetValidation validation =
        validateAsset(family, sourceFile.filename().string(), decodedWidth, decodedHeight);
    if (!validation.valid) {
        return FileOpResult::failure(validation.message);
    }

    const std::filesystem::path target = _directory / sourceFile.filename();
    if (std::filesystem::exists(target, error)) {
        return FileOpResult::failure("Un asset porte deja ce nom.");
    }
    std::filesystem::create_directories(_directory, error);
    // Copie, jamais deplacement : le fichier source appartient a l'utilisateur.
    if (!std::filesystem::copy_file(sourceFile, target, error) || error) {
        return FileOpResult::failure("Echec de la copie du fichier.");
    }
    return FileOpResult::success(target);
}

FileOpResult AssetFileOperations::rename(const std::filesystem::path& source,
                                         const std::string& newStem) const {
    std::error_code error;
    if (!std::filesystem::exists(source, error)) {
        return FileOpResult::failure("Asset introuvable.");
    }
    const std::string trimmed = hmi::trimLevelName(newStem);
    if (!hmi::isValidLevelName(newStem)) {
        return FileOpResult::failure("Nom d'asset invalide.");
    }
    const std::filesystem::path target = pathForStem(trimmed, source.extension());
    if (target != source && std::filesystem::exists(target, error)) {
        return FileOpResult::failure("Un asset porte deja ce nom.");
    }
    std::filesystem::rename(source, target, error);
    if (error) {
        return FileOpResult::failure("Echec du renommage.");
    }
    return FileOpResult::success(target);
}

FileOpResult AssetFileOperations::duplicate(const std::filesystem::path& source) const {
    std::error_code error;
    if (!std::filesystem::exists(source, error)) {
        return FileOpResult::failure("Asset introuvable.");
    }
    const std::string base = source.stem().string();
    // Cherche un nom de copie disponible : « base (copie) », « base (copie 2) », …
    std::string candidate = base + " (copie)";
    for (int index = 2; std::filesystem::exists(pathForStem(candidate, source.extension()), error);
         ++index) {
        candidate = base + " (copie " + std::to_string(index) + ")";
    }
    const std::filesystem::path target = pathForStem(candidate, source.extension());
    if (!std::filesystem::copy_file(source, target, error) || error) {
        return FileOpResult::failure("Echec de la duplication.");
    }
    return FileOpResult::success(target);
}

FileOpResult AssetFileOperations::remove(const std::filesystem::path& source) {
    std::error_code error;
    if (!std::filesystem::exists(source, error)) {
        return FileOpResult::failure("Asset introuvable.");
    }
    if (!std::filesystem::remove(source, error)) {
        return FileOpResult::failure("Echec de la suppression.");
    }
    return FileOpResult::success(source);
}

}  // namespace hmi
