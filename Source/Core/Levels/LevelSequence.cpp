// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/LevelSequence.h"

#include <fstream>
#include <sstream>
#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Levels/LevelsLog.h"

namespace core {

namespace {

// Construit un résultat d'échec avec un message et un code catégorisé -- même patron que
// LevelLoader.cpp::failure (point unique de journalisation).
[[nodiscard]] LevelSequenceLoadResult failure(std::string message, LevelSequenceError code) {
    LEVELS_LOG_WARNING("Echec du chargement de la sequence : " + message);
    return LevelSequenceLoadResult{
        .sequence = std::nullopt, .error = std::move(message), .errorCode = code};
}

}  // namespace

LevelSequenceLoadResult LevelSequenceLoader::loadFromString(std::string_view json) {
    try {
        const nlohmann::json root = nlohmann::json::parse(json);

        if (!root.contains("levels")) {
            return failure("Champ obligatoire manquant ('levels')", LevelSequenceError::ParseError);
        }
        if (!root.at("levels").is_array()) {
            return failure("Le champ 'levels' doit etre une liste", LevelSequenceError::ParseError);
        }

        // Version du format (EX-LVL-005) : absente = version initiale (0), même
        // rétrocompatibilité que LevelLoader.
        const int version = root.value("version", 0);
        if (version > LEVEL_SEQUENCE_FORMAT_VERSION) {
            return failure("Version de format non geree : " + std::to_string(version) +
                               " (maximum gere : " + std::to_string(LEVEL_SEQUENCE_FORMAT_VERSION) +
                               ")",
                           LevelSequenceError::UnsupportedFormatVersion);
        }

        LevelSequence result;
        result.titleKey = root.value("titleKey", std::string{});
        for (const nlohmann::json& entry : root.at("levels")) {
            result.levels.push_back(entry.get<std::string>());
        }
        if (result.levels.empty()) {
            return failure("Sequence sans aucun niveau", LevelSequenceError::EmptySequence);
        }

        return LevelSequenceLoadResult{.sequence = std::move(result), .error = {}};
    } catch (const nlohmann::json::exception& error) {
        return failure(std::string("JSON invalide : ") + error.what(),
                       LevelSequenceError::ParseError);
    }
}

LevelSequenceLoadResult LevelSequenceLoader::loadFromFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return failure("Fichier de sequence introuvable : " + path.string(),
                       LevelSequenceError::FileNotFound);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    LevelSequenceLoadResult result = loadFromString(buffer.str());
    if (!result.ok()) {
        return result;
    }

    // Chaque niveau référencé doit exister, à côté du fichier de séquence lui-même (c'est là que
    // vivent les niveaux, Source/Elements/Levels) -- une entrée fautive est nommée, pas un
    // plantage différé au premier chargement de niveau (EX-NFR-040).
    const std::filesystem::path directory = path.parent_path();
    for (const std::string& levelName : result.sequence->levels) {
        if (!std::filesystem::exists(directory / levelName)) {
            return failure("Niveau reference introuvable : " + levelName,
                           LevelSequenceError::MissingLevelFile);
        }
    }
    return result;
}

}  // namespace core
