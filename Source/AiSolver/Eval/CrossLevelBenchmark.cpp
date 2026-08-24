// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/CrossLevelBenchmark.h"

#include <cmath>
#include <fstream>
#include <system_error>

#include "AiSolver/Eval/BenchmarkRunner.h"
#include "AiSolver/Stats/CsvFormat.h"

namespace aisolver::eval {

std::vector<CrossLevelBenchmarkResult> runCrossLevelCampaign(
    const std::vector<CrossLevelPair>& pairs, const BenchmarkConfig& config) {
    std::vector<CrossLevelBenchmarkResult> results;
    results.reserve(pairs.size());
    for (const CrossLevelPair& pair : pairs) {
        BenchmarkResult result =
            BenchmarkRunner::run(pair.policy, pair.executedOnLevelPath, config);
        results.push_back(CrossLevelBenchmarkResult{pair.trainedOnLevel, pair.executedOnLevel,
                                                    std::move(result)});
    }
    return results;
}

void writeCrossLevelCsv(const std::vector<CrossLevelBenchmarkResult>& results,
                        const std::filesystem::path& path) {
    std::error_code error;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }
    std::ofstream csvFile(path, std::ios::binary | std::ios::trunc);

    csvFile << "trainedOnLevel,executedOnLevel,successRate,meanStepCount,stepCountStdDev\n";
    for (const CrossLevelBenchmarkResult& row : results) {
        csvFile << escapeCsvField(row.trainedOnLevel) << ',' << escapeCsvField(row.executedOnLevel)
                << ',' << row.result.successRate() << ',' << row.result.meanStepsAll() << ','
                << std::sqrt(row.result.stepVariance()) << '\n';
    }
}

}  // namespace aisolver::eval
