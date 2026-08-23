// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Advanced/GenerationComparator.h"

namespace aisolver::training {

std::vector<GenerationComparisonResult> compareGenerations(const std::vector<NamedSeries>& series,
                                                            float rewardThreshold,
                                                            int finalWindowSize) {
    std::vector<GenerationComparisonResult> results;
    results.reserve(series.size());
    for (const NamedSeries& oneSeries : series) {
        GenerationComparisonResult result;
        result.name = oneSeries.name;
        if (!oneSeries.csvPaths.empty()) {
            result.report = compareConvergence(oneSeries.csvPaths, rewardThreshold, finalWindowSize);
        }
        results.push_back(std::move(result));
    }
    return results;
}

std::size_t evolutionaryEpisodeBudget(std::size_t generationCount, std::size_t populationSize) {
    return generationCount * populationSize;
}

}  // namespace aisolver::training
