// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Eval/TrainedPolicy.h"
#include "AiSolver/Nn/Network.h"

/**
 * @file AiSolver/Eval/ReinforceTrainedPolicy.h
 * @brief Adaptateur `TrainedPolicy` vers un réseau de politique REINFORCE (`LOT-ANNEXE-12`)
 * (`LOT-ANNEXE-15`, TACHE-01, `EX-IA-016`).
 */

namespace aisolver::eval {

/// Charge uniquement le réseau de politique ; les deux modes de décodage sont valides (même
/// distribution `softmax` qu'à l'entraînement).
class ReinforceTrainedPolicy : public TrainedPolicy {
public:
    /// @param policy Réseau de politique déjà entraîné, lecture seule.
    explicit ReinforceTrainedPolicy(nn::Network& policy) : _policy(policy) {}

    [[nodiscard]] std::optional<core::PlayerInput> selectAction(const Tensor<float>& observation,
                                                                ActionDecodingMode mode,
                                                                Rng& rng) override;

private:
    nn::Network& _policy;
};

}  // namespace aisolver::eval
