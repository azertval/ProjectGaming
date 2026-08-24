// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Stats/TrainingRunPath.h"

#include <ctime>
#include <system_error>

namespace aisolver {

namespace {

std::string currentIso8601Compact() {
    const std::time_t now = std::time(nullptr);
    std::tm utcTime{};
#ifdef _WIN32
    gmtime_s(&utcTime, &now);
#else
    gmtime_r(&now, &utcTime);
#endif
    char buffer[16] = {};
    std::strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", &utcTime);
    return buffer;
}

}  // namespace

std::filesystem::path makeTrainingRunPath(const std::filesystem::path& trainingRunsRoot,
                                          std::string_view levelName, std::string_view runId) {
    const std::filesystem::path runDirectory =
        trainingRunsRoot / std::filesystem::path(levelName) / std::filesystem::path(runId);
    std::error_code error;
    std::filesystem::create_directories(runDirectory, error);
    return runDirectory / "stats.csv";
}

std::string generateRunId() {
    // Compteur de repli en cas d'appels multiples dans la meme seconde (etat local a la fonction,
    // un seul processus d'entrainement appelle generateRunId a la fois).
    static std::string lastTimestamp;
    static int collisionCounter = 0;

    const std::string timestamp = currentIso8601Compact();
    if (timestamp == lastTimestamp) {
        ++collisionCounter;
        return timestamp + "-" + std::to_string(collisionCounter);
    }
    lastTimestamp = timestamp;
    collisionCounter = 0;
    return timestamp;
}

}  // namespace aisolver
