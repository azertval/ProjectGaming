// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ActorCritic/ConvergenceComparator.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

namespace {

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

std::size_t columnIndex(const std::vector<std::string>& header, const std::string& name) {
    for (std::size_t index = 0; index < header.size(); ++index) {
        if (header[index] == name) {
            return index;
        }
    }
    PROJECTGAMING_ASSERT(false, "ConvergenceComparator : colonne introuvable dans l'en-tete du CSV");
    return 0;
}

std::vector<float> readBestRewardColumn(const std::filesystem::path& csvPath) {
    std::ifstream file(csvPath);
    PROJECTGAMING_ASSERT(file.is_open(), "ConvergenceComparator : CSV illisible");

    std::string headerLine;
    std::getline(file, headerLine);
    const std::vector<std::string> header = splitCsvLine(headerLine);
    const std::size_t bestRewardColumn = columnIndex(header, "bestReward");

    std::vector<float> rewards;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        const std::vector<std::string> fields = splitCsvLine(line);
        rewards.push_back(std::stof(fields[bestRewardColumn]));
    }
    return rewards;
}

}  // namespace

RunConvergenceMetrics analyzeRun(const std::filesystem::path& csvPath, float rewardThreshold,
                                 int finalWindowSize) {
    const std::vector<float> rewards = readBestRewardColumn(csvPath);

    RunConvergenceMetrics metrics;
    for (std::size_t index = 0; index < rewards.size(); ++index) {
        if (rewards[index] >= rewardThreshold) {
            metrics.episodesToThreshold = static_cast<int>(index);
            break;
        }
    }

    const std::size_t windowSize = static_cast<std::size_t>(std::max(finalWindowSize, 1));
    const std::size_t windowStart = rewards.size() > windowSize ? rewards.size() - windowSize : 0;
    float sum = 0.0f;
    std::size_t count = 0;
    for (std::size_t index = windowStart; index < rewards.size(); ++index) {
        sum += rewards[index];
        ++count;
    }
    metrics.finalWindowMeanReward = count > 0 ? sum / static_cast<float>(count) : 0.0f;
    return metrics;
}

ConvergenceReport compareConvergence(const std::vector<std::filesystem::path>& csvPaths,
                                     float rewardThreshold, int finalWindowSize) {
    PROJECTGAMING_ASSERT(!csvPaths.empty(), "compareConvergence : au moins un essai est requis");

    ConvergenceReport report;
    report.totalTrials = csvPaths.size();

    std::vector<float> episodesReaching;
    std::vector<float> finalWindowMeans;
    for (const std::filesystem::path& csvPath : csvPaths) {
        const RunConvergenceMetrics metrics = analyzeRun(csvPath, rewardThreshold, finalWindowSize);
        finalWindowMeans.push_back(metrics.finalWindowMeanReward);
        if (metrics.episodesToThreshold) {
            episodesReaching.push_back(static_cast<float>(*metrics.episodesToThreshold));
            ++report.trialsReachingThreshold;
        }
    }

    if (!episodesReaching.empty()) {
        float sum = 0.0f;
        for (const float value : episodesReaching) {
            sum += value;
        }
        report.meanEpisodesToThreshold = sum / static_cast<float>(episodesReaching.size());
    }

    float mean = 0.0f;
    for (const float value : finalWindowMeans) {
        mean += value;
    }
    mean /= static_cast<float>(finalWindowMeans.size());

    float variance = 0.0f;
    for (const float value : finalWindowMeans) {
        const float delta = value - mean;
        variance += delta * delta;
    }
    variance /= static_cast<float>(finalWindowMeans.size());
    report.finalRewardStdDev = std::sqrt(variance);

    return report;
}

}  // namespace aisolver::training
