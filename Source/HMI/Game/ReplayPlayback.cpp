// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/ReplayPlayback.h"

#include "AiSolver/Replay/ReplayFile.h"
#include "AiSolver/Replay/ReplayValidation.h"

namespace hmi {

namespace {

std::string describeValidationError(aisolver::ReplayValidationError error) {
    switch (error) {
        case aisolver::ReplayValidationError::LevelFileMissing:
            return "Rejeu invalide : le niveau reference n'existe plus.";
        case aisolver::ReplayValidationError::LevelFingerprintMismatch:
            return "Rejeu invalide : le niveau reference a change depuis l'export.";
    }
    return "Rejeu invalide.";
}

}  // namespace

ReplayPlayback::ReplayPlayback(const std::filesystem::path& replayPath,
                               const std::filesystem::path& levelsDir) {
    const aisolver::ReplayLoadResult loaded = aisolver::readReplay(replayPath);
    if (!loaded.ok()) {
        _error = loaded.error;
        return;
    }
    if (const std::optional<aisolver::ReplayValidationError> validationError =
            aisolver::validateReplay(*loaded.replay, levelsDir)) {
        _error = describeValidationError(*validationError);
        return;
    }
    _levelPath = loaded.replay->levelPath;
    _steps = loaded.replay->steps;
    _valid = true;
}

std::optional<core::PlayerInput> ReplayPlayback::nextInput() {
    if (!_valid || _nextIndex >= _steps.size()) {
        return std::nullopt;
    }
    return _steps[_nextIndex++];
}

}  // namespace hmi
