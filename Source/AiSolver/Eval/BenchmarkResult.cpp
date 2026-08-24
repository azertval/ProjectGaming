// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/BenchmarkResult.h"

namespace aisolver::eval {

double BenchmarkResult::successRate() const {
    if (episodes.empty()) {
        return 0.0;
    }
    int wonCount = 0;
    for (const EpisodeOutcome& episode : episodes) {
        if (episode.outcome == core::LevelOutcome::Won) {
            ++wonCount;
        }
    }
    return static_cast<double>(wonCount) / static_cast<double>(episodes.size());
}

double BenchmarkResult::meanStepsAll() const {
    if (episodes.empty()) {
        return 0.0;
    }
    double total = 0.0;
    for (const EpisodeOutcome& episode : episodes) {
        total += static_cast<double>(episode.stepCount);
    }
    return total / static_cast<double>(episodes.size());
}

double BenchmarkResult::meanStepsOnSuccess() const {
    double total = 0.0;
    int wonCount = 0;
    for (const EpisodeOutcome& episode : episodes) {
        if (episode.outcome == core::LevelOutcome::Won) {
            total += static_cast<double>(episode.stepCount);
            ++wonCount;
        }
    }
    if (wonCount == 0) {
        return 0.0;
    }
    return total / static_cast<double>(wonCount);
}

double BenchmarkResult::stepVariance() const {
    if (episodes.empty()) {
        return 0.0;
    }
    const double mean = meanStepsAll();
    double sumSquaredDeviation = 0.0;
    for (const EpisodeOutcome& episode : episodes) {
        const double deviation = static_cast<double>(episode.stepCount) - mean;
        sumSquaredDeviation += deviation * deviation;
    }
    return sumSquaredDeviation / static_cast<double>(episodes.size());
}

}  // namespace aisolver::eval
