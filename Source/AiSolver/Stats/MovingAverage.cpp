// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Stats/MovingAverage.h"

namespace aisolver {

MovingAverageTracker::MovingAverageTracker(int windowSize) : windowSize_(windowSize) {
}

float MovingAverageTracker::push(float value) {
    window_.push_back(value);
    sum_ += value;
    if (static_cast<int>(window_.size()) > windowSize_) {
        sum_ -= window_.front();
        window_.pop_front();
    }
    return sum_ / static_cast<float>(window_.size());
}

}  // namespace aisolver
