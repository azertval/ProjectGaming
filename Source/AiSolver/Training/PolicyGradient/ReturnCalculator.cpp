// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/PolicyGradient/ReturnCalculator.h"

#include <cmath>
#include <cstddef>

#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

std::vector<float> computeReturns(const Trajectory& trajectory, float gamma) {
    PROJECTGAMING_ASSERT(gamma >= 0.0f && gamma <= 1.0f,
                         "computeReturns : gamma doit etre dans [0, 1]");

    std::vector<float> returns(trajectory.steps.size(), 0.0f);
    float runningReturn = 0.0f;
    for (std::size_t index = trajectory.steps.size(); index-- > 0;) {
        runningReturn = trajectory.steps[index].reward + gamma * runningReturn;
        returns[index] = runningReturn;
    }
    return returns;
}

WeightStatistics weightStatistics(const std::vector<std::vector<float>>& batch) {
    double total = 0.0;
    std::size_t count = 0;
    for (const std::vector<float>& weights : batch) {
        for (const float weight : weights) {
            total += weight;
            ++count;
        }
    }
    if (count == 0) {
        return WeightStatistics{};
    }
    const double mean = total / static_cast<double>(count);

    double sumOfSquaredDeviations = 0.0;
    for (const std::vector<float>& weights : batch) {
        for (const float weight : weights) {
            const double deviation = static_cast<double>(weight) - mean;
            sumOfSquaredDeviations += deviation * deviation;
        }
    }
    return WeightStatistics{
        static_cast<float>(mean),
        static_cast<float>(std::sqrt(sumOfSquaredDeviations / static_cast<double>(count)))};
}

void normalizeWeights(std::vector<float>& weights, WeightStatistics statistics) {
    // Un lot degenere (tous les poids egaux) a un ecart-type nul : le centrage suffit alors, et
    // diviser par l'epsilon amplifierait du bruit d'arrondi en un gradient enorme.
    constexpr float MINIMUM_DEVIATION = 1e-6f;
    const float scale = statistics.standardDeviation > MINIMUM_DEVIATION
                            ? 1.0f / statistics.standardDeviation
                            : 1.0f;
    for (float& weight : weights) {
        weight = (weight - statistics.mean) * scale;
    }
}

}  // namespace aisolver::training
