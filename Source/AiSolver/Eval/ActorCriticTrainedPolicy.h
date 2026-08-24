// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Eval/TrainedPolicy.h"
#include "AiSolver/Nn/Network.h"

/**
 * @file AiSolver/Eval/ActorCriticTrainedPolicy.h
 * @brief Adaptateur `TrainedPolicy` vers l'acteur d'un modèle acteur-critique (`LOT-ANNEXE-13`)
 * (`LOT-ANNEXE-15`, TACHE-01, `EX-IA-016`).
 */

namespace aisolver::eval {

/**
 * @brief Charge uniquement l'acteur ; le critique n'est **jamais** chargé ni utilisé en évaluation
 * (il ne sert qu'à l'entraînement, décision de cadrage de `LOT-ANNEXE-13` réaffirmée ici). Les deux
 * modes de décodage sont valides.
 */
class ActorCriticTrainedPolicy : public TrainedPolicy {
public:
    /// @param actor Réseau de politique (acteur) déjà entraîné, lecture seule.
    explicit ActorCriticTrainedPolicy(nn::Network& actor) : _actor(actor) {}

    [[nodiscard]] std::optional<core::PlayerInput> selectAction(const Tensor<float>& observation,
                                                                 ActionDecodingMode mode,
                                                                 Rng& rng) override;

private:
    nn::Network& _actor;
};

}  // namespace aisolver::eval
