// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/Episode.h"

namespace aisolver {

EpisodeStatus classifyEpisode(core::LevelOutcome outcome, int stepIndex, int stepsSinceProgress,
                              int hardStepBudget, int stuckThreshold) {
    if (outcome == core::LevelOutcome::Won) {
        return EpisodeStatus::Won;
    }
    if (outcome == core::LevelOutcome::Lost) {
        return EpisodeStatus::Lost;
    }
    if (stepIndex >= hardStepBudget) {
        return EpisodeStatus::TimedOut;
    }
    if (stepsSinceProgress >= stuckThreshold) {
        return EpisodeStatus::Stuck;
    }
    return EpisodeStatus::Ongoing;
}

}  // namespace aisolver
