// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Replay/ReplayFile.h"

#include <fstream>
#include <sstream>
#include <system_error>

#include <nlohmann/json.hpp>

namespace aisolver {

namespace {

constexpr const char* FIELD_FORMAT_VERSION = "formatVersion";
constexpr const char* FIELD_LEVEL_PATH = "levelPath";
constexpr const char* FIELD_LEVEL_FINGERPRINT = "levelFingerprint";
constexpr const char* FIELD_STEPS = "steps";
constexpr const char* FIELD_ALGORITHM_NAME = "algorithmName";
constexpr const char* FIELD_EXPORTED_AT = "exportedAtIso8601";
constexpr const char* FIELD_SEED = "seed";
constexpr const char* FIELD_FINAL_REWARD = "finalReward";
constexpr const char* FIELD_TOTAL_DURATION_SECONDS = "totalDurationSeconds";
constexpr const char* FIELD_ALGORITHM_ID = "algorithmId";

constexpr const char* FIELD_MOVE_X = "moveX";
constexpr const char* FIELD_JUMP_PRESSED = "jumpPressed";
constexpr const char* FIELD_JUMP_HELD = "jumpHeld";
constexpr const char* FIELD_MOVE_Y = "moveY";
constexpr const char* FIELD_DASH_PRESSED = "dashPressed";
constexpr const char* FIELD_INTERACT_PRESSED = "interactPressed";
constexpr const char* FIELD_INTERACT_HELD = "interactHeld";
constexpr const char* FIELD_INTERACT_RELEASED = "interactReleased";

nlohmann::ordered_json stepToJson(const core::PlayerInput& step) {
    nlohmann::ordered_json json;
    json[FIELD_MOVE_X] = step.moveX;
    json[FIELD_JUMP_PRESSED] = step.jumpPressed;
    json[FIELD_JUMP_HELD] = step.jumpHeld;
    json[FIELD_MOVE_Y] = step.moveY;
    json[FIELD_DASH_PRESSED] = step.dashPressed;
    json[FIELD_INTERACT_PRESSED] = step.interactPressed;
    json[FIELD_INTERACT_HELD] = step.interactHeld;
    json[FIELD_INTERACT_RELEASED] = step.interactReleased;
    return json;
}

/// @return `false` si un champ booleen/numerique attendu est present mais de mauvais type.
[[nodiscard]] bool stepFromJson(const nlohmann::json& json, core::PlayerInput& outStep) {
    if (!json.is_object()) {
        return false;
    }
    core::PlayerInput step;
    if (json.contains(FIELD_MOVE_X)) {
        if (!json[FIELD_MOVE_X].is_number()) {
            return false;
        }
        step.moveX = json[FIELD_MOVE_X].get<float>();
    }
    if (json.contains(FIELD_JUMP_PRESSED)) {
        if (!json[FIELD_JUMP_PRESSED].is_boolean()) {
            return false;
        }
        step.jumpPressed = json[FIELD_JUMP_PRESSED].get<bool>();
    }
    if (json.contains(FIELD_JUMP_HELD)) {
        if (!json[FIELD_JUMP_HELD].is_boolean()) {
            return false;
        }
        step.jumpHeld = json[FIELD_JUMP_HELD].get<bool>();
    }
    if (json.contains(FIELD_MOVE_Y)) {
        if (!json[FIELD_MOVE_Y].is_number()) {
            return false;
        }
        step.moveY = json[FIELD_MOVE_Y].get<float>();
    }
    if (json.contains(FIELD_DASH_PRESSED)) {
        if (!json[FIELD_DASH_PRESSED].is_boolean()) {
            return false;
        }
        step.dashPressed = json[FIELD_DASH_PRESSED].get<bool>();
    }
    if (json.contains(FIELD_INTERACT_PRESSED)) {
        if (!json[FIELD_INTERACT_PRESSED].is_boolean()) {
            return false;
        }
        step.interactPressed = json[FIELD_INTERACT_PRESSED].get<bool>();
    }
    if (json.contains(FIELD_INTERACT_HELD)) {
        if (!json[FIELD_INTERACT_HELD].is_boolean()) {
            return false;
        }
        step.interactHeld = json[FIELD_INTERACT_HELD].get<bool>();
    }
    if (json.contains(FIELD_INTERACT_RELEASED)) {
        if (!json[FIELD_INTERACT_RELEASED].is_boolean()) {
            return false;
        }
        step.interactReleased = json[FIELD_INTERACT_RELEASED].get<bool>();
    }
    outStep = step;
    return true;
}

}  // namespace

bool writeReplay(const std::filesystem::path& path, const ReplayFile& replay) {
    nlohmann::ordered_json root;
    root[FIELD_FORMAT_VERSION] = replay.formatVersion;
    root[FIELD_LEVEL_PATH] = replay.levelPath;
    root[FIELD_LEVEL_FINGERPRINT] = replay.levelFingerprint;
    root[FIELD_ALGORITHM_NAME] = replay.algorithmName;
    root[FIELD_EXPORTED_AT] = replay.exportedAtIso8601;
    root[FIELD_SEED] = replay.seed;
    root[FIELD_FINAL_REWARD] = replay.finalReward;
    root[FIELD_TOTAL_DURATION_SECONDS] = replay.totalDurationSeconds;
    root[FIELD_ALGORITHM_ID] = replay.algorithmId;

    nlohmann::ordered_json steps = nlohmann::ordered_json::array();
    for (const core::PlayerInput& step : replay.steps) {
        steps.push_back(stepToJson(step));
    }
    root[FIELD_STEPS] = std::move(steps);

    std::error_code error;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return false;
    }
    file << root.dump(2) << "\n";
    return file.good();
}

ReplayLoadResult readReplay(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return ReplayLoadResult{
            .replay = std::nullopt,
            .error = "Fichier de rejeu introuvable ou illisible : " + path.string()};
    }
    std::ostringstream contents;
    contents << file.rdbuf();

    // accept() puis parse(..., false) : nlohmann leve par defaut, et aucune exception ne doit
    // franchir cette frontiere (EX-NFR-040), meme patron que SkinCatalog::loadFromString.
    const std::string text = contents.str();
    if (!nlohmann::json::accept(text)) {
        return ReplayLoadResult{.replay = std::nullopt, .error = "JSON malforme."};
    }
    const nlohmann::json root = nlohmann::json::parse(text, nullptr, false);
    if (!root.is_object()) {
        return ReplayLoadResult{.replay = std::nullopt,
                                .error = "La racine du document n'est pas un objet."};
    }

    ReplayFile replay;

    // Absent = version initiale (0), meme principe que core::kLevelFormatVersion (EX-LVL-005).
    if (root.contains(FIELD_FORMAT_VERSION)) {
        if (!root[FIELD_FORMAT_VERSION].is_number_unsigned()) {
            return ReplayLoadResult{.replay = std::nullopt,
                                    .error = "Le champ « formatVersion » n'est pas un entier."};
        }
        replay.formatVersion = root[FIELD_FORMAT_VERSION].get<std::uint32_t>();
    } else {
        replay.formatVersion = 0;
    }

    if (root.contains(FIELD_LEVEL_PATH)) {
        if (!root[FIELD_LEVEL_PATH].is_string()) {
            return ReplayLoadResult{.replay = std::nullopt,
                                    .error = "Le champ « levelPath » n'est pas une chaine."};
        }
        replay.levelPath = root[FIELD_LEVEL_PATH].get<std::string>();
    }

    if (root.contains(FIELD_LEVEL_FINGERPRINT)) {
        if (!root[FIELD_LEVEL_FINGERPRINT].is_number_unsigned()) {
            return ReplayLoadResult{.replay = std::nullopt,
                                    .error = "Le champ « levelFingerprint » n'est pas un entier."};
        }
        replay.levelFingerprint = root[FIELD_LEVEL_FINGERPRINT].get<std::uint64_t>();
    }

    if (root.contains(FIELD_ALGORITHM_NAME)) {
        if (!root[FIELD_ALGORITHM_NAME].is_string()) {
            return ReplayLoadResult{.replay = std::nullopt,
                                    .error = "Le champ « algorithmName » n'est pas une chaine."};
        }
        replay.algorithmName = root[FIELD_ALGORITHM_NAME].get<std::string>();
    }

    if (root.contains(FIELD_EXPORTED_AT)) {
        if (!root[FIELD_EXPORTED_AT].is_string()) {
            return ReplayLoadResult{
                .replay = std::nullopt,
                .error = "Le champ « exportedAtIso8601 » n'est pas une chaine."};
        }
        replay.exportedAtIso8601 = root[FIELD_EXPORTED_AT].get<std::string>();
    }

    if (root.contains(FIELD_SEED)) {
        if (!root[FIELD_SEED].is_number_unsigned()) {
            return ReplayLoadResult{.replay = std::nullopt,
                                    .error = "Le champ « seed » n'est pas un entier."};
        }
        replay.seed = root[FIELD_SEED].get<std::uint64_t>();
    }

    if (root.contains(FIELD_FINAL_REWARD)) {
        if (!root[FIELD_FINAL_REWARD].is_number()) {
            return ReplayLoadResult{.replay = std::nullopt,
                                    .error = "Le champ « finalReward » n'est pas un nombre."};
        }
        replay.finalReward = root[FIELD_FINAL_REWARD].get<float>();
    }

    // Champs propres a la version 2 : absents d'un fichier en version 1, qui reste un rejeu
    // valide -- seulement moins renseigne. Valeur sentinelle plutot qu'echec de lecture.
    if (root.contains(FIELD_TOTAL_DURATION_SECONDS)) {
        if (!root[FIELD_TOTAL_DURATION_SECONDS].is_number()) {
            return ReplayLoadResult{
                .replay = std::nullopt,
                .error = "Le champ « totalDurationSeconds » n'est pas un nombre."};
        }
        replay.totalDurationSeconds = root[FIELD_TOTAL_DURATION_SECONDS].get<float>();
    }

    if (root.contains(FIELD_ALGORITHM_ID)) {
        if (!root[FIELD_ALGORITHM_ID].is_string()) {
            return ReplayLoadResult{.replay = std::nullopt,
                                    .error = "Le champ « algorithmId » n'est pas une chaine."};
        }
        replay.algorithmId = root[FIELD_ALGORITHM_ID].get<std::string>();
    }

    if (root.contains(FIELD_STEPS)) {
        if (!root[FIELD_STEPS].is_array()) {
            return ReplayLoadResult{.replay = std::nullopt,
                                    .error = "Le champ « steps » n'est pas un tableau."};
        }
        replay.steps.reserve(root[FIELD_STEPS].size());
        for (const nlohmann::json& stepJson : root[FIELD_STEPS]) {
            core::PlayerInput step;
            if (!stepFromJson(stepJson, step)) {
                return ReplayLoadResult{.replay = std::nullopt,
                                        .error = "Un element de « steps » est invalide."};
            }
            replay.steps.push_back(step);
        }
    }

    return ReplayLoadResult{.replay = std::move(replay), .error = {}};
}

}  // namespace aisolver
