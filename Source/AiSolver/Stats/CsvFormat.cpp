// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Stats/CsvFormat.h"

#include <sstream>

namespace aisolver {

namespace {

std::string escapeCsvField(std::string_view field) {
    const bool needsQuoting =
        field.find(',') != std::string_view::npos || field.find('"') != std::string_view::npos;
    if (!needsQuoting) {
        return std::string(field);
    }
    std::string escaped;
    escaped.reserve(field.size() + 2);
    escaped += '"';
    for (const char character : field) {
        if (character == '"') {
            escaped += '"';
        }
        escaped += character;
    }
    escaped += '"';
    return escaped;
}

}  // namespace

std::string csvHeader() {
    return "index,bestReward,meanReward,worstReward,rewardStdDev,bestStepCount,successRate,seed,"
           "levelName,timestampIso8601,movingAverageReward,rewardDelta";
}

std::string csvRow(const TrainingStatsRow& row, float movingAverageReward, float rewardDelta,
                   std::string_view timestampIso8601) {
    std::ostringstream out;
    out << row.index << ',' << row.bestReward << ',' << row.meanReward << ',' << row.worstReward
        << ',' << row.rewardStdDev << ',' << row.bestStepCount << ',' << row.successRate << ','
        << row.seed << ',' << escapeCsvField(row.levelName) << ',' << timestampIso8601 << ','
        << movingAverageReward << ',' << rewardDelta;
    return out.str();
}

}  // namespace aisolver
