// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Eval/TrainedPolicy.h"
#include "AiSolver/Training/Advanced/QNetwork.h"

/**
 * @file AiSolver/Eval/AdvancedAlgorithmTrainedPolicy.h
 * @brief Adaptateur `TrainedPolicy` vers l'algorithme avancé retenu par `LOT-ANNEXE-14` (DQN)
 * (`LOT-ANNEXE-15`, TACHE-01, `EX-IA-016`).
 */

namespace aisolver::eval {

/**
 * @brief Charge uniquement le réseau principal `Q(s, ·)` (jamais le réseau cible, qui ne sert qu'à
 * stabiliser l'entraînement, `LOT-ANNEXE-14`). N'accepte que `ActionDecodingMode::Argmax` :
 * `Q(s, a)` n'est pas une distribution de probabilité (valeurs non bornées, jamais normalisées),
 * l'action gloutonne (`argmax_a Q(s, a)`) est la seule sélection à laquelle DQN donne un sens hors
 * entraînement — l'exploration `epsilon`-greedy est une préoccupation d'entraînement, pas
 * d'évaluation.
 */
class AdvancedAlgorithmTrainedPolicy : public TrainedPolicy {
public:
    /// @param mainNetwork Réseau principal déjà entraîné (`DqnTrainer`), lecture seule.
    explicit AdvancedAlgorithmTrainedPolicy(training::QNetwork& mainNetwork)
        : _mainNetwork(mainNetwork) {}

    [[nodiscard]] std::optional<core::PlayerInput> selectAction(const Tensor<float>& observation,
                                                                 ActionDecodingMode mode,
                                                                 Rng& rng) override;

    [[nodiscard]] bool supportsMode(ActionDecodingMode mode) const noexcept override {
        return mode == ActionDecodingMode::Argmax;
    }

private:
    training::QNetwork& _mainNetwork;
};

}  // namespace aisolver::eval
