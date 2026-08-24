// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ActorCritic/AdvantageCalculator.h"

#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

std::vector<float> computeAdvantages(const std::vector<float>& returns, CriticNetwork& critic,
                                     const Trajectory& trajectory) {
    PROJECTGAMING_ASSERT(returns.size() == trajectory.steps.size(),
                         "computeAdvantages : returns doit avoir la meme longueur que la "
                         "trajectoire");

    std::vector<float> advantages(returns.size());
    for (std::size_t index = 0; index < trajectory.steps.size(); ++index) {
        const float estimatedValue =
            critic.forward(trajectory.steps[index].observation)->value.data()[0];
        advantages[index] = returns[index] - estimatedValue;
    }
    return advantages;
}

}  // namespace aisolver::training
