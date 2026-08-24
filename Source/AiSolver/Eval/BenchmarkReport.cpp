// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/BenchmarkReport.h"

#include <cmath>
#include <fstream>
#include <system_error>

#include "AiSolver/Stats/CsvFormat.h"

namespace aisolver::eval {

void BenchmarkReport::addResult(std::string algorithmName, std::string levelName,
                                const BenchmarkResult& result) {
    BenchmarkReportRow row;
    row.algorithmName = std::move(algorithmName);
    row.levelName = std::move(levelName);
    row.successRate = result.successRate();
    row.meanStepCount = result.meanStepsAll();
    row.stepCountStdDev = std::sqrt(result.stepVariance());
    _rows.push_back(std::move(row));
}

void BenchmarkReport::writeCsv(const std::filesystem::path& path) const {
    std::error_code error;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }
    std::ofstream csvFile(path, std::ios::binary | std::ios::trunc);

    csvFile << "algorithmName,levelName,successRate,meanStepCount,stepCountStdDev\n";
    for (const BenchmarkReportRow& row : _rows) {
        csvFile << escapeCsvField(row.algorithmName) << ',' << escapeCsvField(row.levelName) << ','
                << row.successRate << ',' << row.meanStepCount << ',' << row.stepCountStdDev << '\n';
    }
}

}  // namespace aisolver::eval
