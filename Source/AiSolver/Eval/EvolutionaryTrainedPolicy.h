// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Eval/TrainedPolicy.h"
#include "AiSolver/Nn/Network.h"

/**
 * @file AiSolver/Eval/EvolutionaryTrainedPolicy.h
 * @brief Adaptateur `TrainedPolicy` vers un individu évolutionniste (`LOT-ANNEXE-10`/`11`)
 * (`LOT-ANNEXE-15`, TACHE-01, `EX-IA-016`).
 */

namespace aisolver::eval {

/**
 * @brief N'accepte que `ActionDecodingMode::Argmax` : seul mode que la génération 2 (évolutionniste)
 * a jamais connu (décision de cadrage de `LOT-ANNEXE-10`, réaffirmée ici) — un appel en
 * `Stochastic` est un usage incorrect signalé (`selectAction` retourne `std::nullopt`), pas
 * silencieusement ignoré.
 */
class EvolutionaryTrainedPolicy : public TrainedPolicy {
public:
    /// @param network Réseau déjà entraîné (`evolutionary::Individual::network()`), lecture seule.
    explicit EvolutionaryTrainedPolicy(nn::Network& network) : _network(network) {}

    [[nodiscard]] std::optional<core::PlayerInput> selectAction(const Tensor<float>& observation,
                                                                 ActionDecodingMode mode,
                                                                 Rng& rng) override;

    [[nodiscard]] bool supportsMode(ActionDecodingMode mode) const noexcept override {
        return mode == ActionDecodingMode::Argmax;
    }

private:
    nn::Network& _network;
};

}  // namespace aisolver::eval
