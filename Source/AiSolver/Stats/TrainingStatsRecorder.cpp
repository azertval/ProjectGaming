// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Stats/TrainingStatsRecorder.h"

#include <ctime>
#include <system_error>

#include "AiSolver/Stats/CsvFormat.h"

namespace aisolver {

namespace {

std::string currentIso8601() {
    const std::time_t now = std::time(nullptr);
    std::tm utcTime{};
#ifdef _WIN32
    gmtime_s(&utcTime, &now);
#else
    gmtime_r(&now, &utcTime);
#endif
    char buffer[32] = {};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
    return buffer;
}

}  // namespace

TrainingStatsRecorder::TrainingStatsRecorder(const std::filesystem::path& outputCsvPath,
                                             int movingAverageWindow, bool enabled)
    : movingAverage_(movingAverageWindow) {
    if (!enabled) {
        return;
    }
    std::error_code error;
    if (outputCsvPath.has_parent_path()) {
        std::filesystem::create_directories(outputCsvPath.parent_path(), error);
    }
    csvFile_.open(outputCsvPath, std::ios::binary | std::ios::trunc);
    csvFile_ << csvHeader() << "\n";
    csvFile_.flush();
}

void TrainingStatsRecorder::record(const TrainingStatsRow& row) {
    const float movingAverageReward = movingAverage_.push(row.bestReward);
    const float rewardDelta =
        hasPreviousMovingAverage_ ? (movingAverageReward - previousMovingAverage_) : 0.0f;
    previousMovingAverage_ = movingAverageReward;
    hasPreviousMovingAverage_ = true;

    if (csvFile_.is_open()) {
        csvFile_ << csvRow(row, movingAverageReward, rewardDelta, currentIso8601()) << "\n";
        csvFile_.flush();
    }

    if (onRecord_) {
        onRecord_(row);
    }
}

}  // namespace aisolver
