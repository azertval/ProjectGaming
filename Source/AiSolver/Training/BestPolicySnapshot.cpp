// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/BestPolicySnapshot.h"

#include <cstddef>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Math/Autodiff/Node.h"

/**
 * @file AiSolver/Training/BestPolicySnapshot.cpp
 * @brief Voir `BestPolicySnapshot.h`.
 */

namespace aisolver::training {

bool PolicyScore::betterThan(const PolicyScore& other) const noexcept {
    if (solved != other.solved) {
        return solved;
    }
    if (solved) {
        if (stepCount != other.stepCount) {
            return stepCount < other.stepCount;
        }
        return reward > other.reward;
    }
    return reward > other.reward;
}

bool BestPolicySnapshot::consider(const nn::Network& network,
                                  const DeterministicReplayResult& replay) {
    const PolicyScore candidate{.solved = replay.status == EpisodeStatus::Won,
                                .reward = replay.finalReward,
                                .stepCount = static_cast<int>(replay.steps.size())};
    if (hasSnapshot() && !candidate.betterThan(_bestScore)) {
        return false;
    }

    const std::vector<autodiff::NodePtr> parameters = network.parameters();
    _parameters.clear();
    _parameters.reserve(parameters.size());
    for (const autodiff::NodePtr& parameter : parameters) {
        _parameters.push_back(parameter->value.clone());
    }
    _bestScore = candidate;
    _replay = replay;
    return true;
}

bool BestPolicySnapshot::restore(nn::Network& network) const {
    if (!hasSnapshot()) {
        return false;
    }
    const std::vector<autodiff::NodePtr> parameters = network.parameters();
    if (parameters.size() != _parameters.size()) {
        return false;
    }
    for (std::size_t index = 0; index < parameters.size(); ++index) {
        if (parameters[index]->value.shape() != _parameters[index].shape()) {
            return false;
        }
    }
    for (std::size_t index = 0; index < parameters.size(); ++index) {
        parameters[index]->value = _parameters[index].clone();
    }
    return true;
}

}  // namespace aisolver::training
