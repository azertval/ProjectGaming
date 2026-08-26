// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/ConvergenceComparator.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

#include "Core/Diagnostics/Assert.h"

namespace aisolver::eval {

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

/// @return Position de la colonne @p name dans @p header, absente si l'en-tete ne la porte pas.
std::optional<std::size_t> columnIndex(const std::vector<std::string>& header,
                                       const std::string& name) {
    for (std::size_t index = 0; index < header.size(); ++index) {
        if (header[index] == name) {
            return index;
        }
    }
    return std::nullopt;
}

/// @return Valeur flottante de @p field, absente si le champ n'est pas un nombre entier ou reel.
/// Extraction par flux plutot que `std::stof` : la conversion echoue par un drapeau d'etat, jamais
/// par une exception -- aucune ne doit franchir cette frontiere (`EX-NFR-040`).
std::optional<float> parseFloat(const std::string& field) {
    std::istringstream stream(field);
    float value = 0.0f;
    stream >> value;
    if (stream.fail()) {
        return std::nullopt;
    }
    return value;
}

/// @return Colonne `bestReward` du CSV @p csvPath, une valeur par episode, dans l'ordre du
/// fichier. Vide si le fichier est illisible ou si son en-tete ne declare pas cette colonne ; les
/// lignes tronquees ou non numeriques sont ignorees plutot que de fausser la serie.
std::vector<float> readBestRewardColumn(const std::filesystem::path& csvPath) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        return {};
    }

    std::string headerLine;
    std::getline(file, headerLine);
    const std::vector<std::string> header = splitCsvLine(headerLine);
    const std::optional<std::size_t> bestRewardColumn = columnIndex(header, "bestReward");
    if (!bestRewardColumn) {
        return {};
    }

    std::vector<float> rewards;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        const std::vector<std::string> fields = splitCsvLine(line);
        if (fields.size() <= *bestRewardColumn) {
            continue;
        }
        if (const std::optional<float> reward = parseFloat(fields[*bestRewardColumn])) {
            rewards.push_back(*reward);
        }
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

}  // namespace aisolver::eval
