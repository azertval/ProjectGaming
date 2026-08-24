// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Replay/ReplayValidation.h"

#include <fstream>
#include <sstream>

#include "AiSolver/Replay/LevelFingerprint.h"

namespace aisolver {

std::optional<ReplayValidationError> validateReplay(const ReplayFile& replay,
                                                      const std::filesystem::path& levelsDir) {
    const std::filesystem::path levelPath = levelsDir / replay.levelPath;

    std::ifstream file(levelPath, std::ios::binary);
    if (!file) {
        return ReplayValidationError::LevelFileMissing;
    }
    std::ostringstream contents;
    contents << file.rdbuf();

    const LevelFingerprint actualFingerprint = computeLevelFingerprint(contents.str());
    if (actualFingerprint != replay.levelFingerprint) {
        return ReplayValidationError::LevelFingerprintMismatch;
    }
    return std::nullopt;
}

}  // namespace aisolver
