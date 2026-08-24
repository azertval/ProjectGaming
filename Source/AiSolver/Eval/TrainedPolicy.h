// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>

#include "AiSolver/Eval/ActionDecodingMode.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "Core/Physics/PlayerInput.h"

/**
 * @file AiSolver/Eval/TrainedPolicy.h
 * @brief Adaptateur fin uniforme vers un modèle déjà entraîné (`LOT-ANNEXE-15`, TACHE-01,
 * `EX-IA-016`).
 */

namespace aisolver::eval {

/**
 * @brief Interface minimale permettant à `BenchmarkRunner` de traiter n'importe quel modèle
 * entraîné (évolutionniste, REINFORCE, acteur-critique, algorithme avancé) de façon uniforme, sans
 * connaître les détails internes de l'algorithme qui l'a produit.
 *
 * Chargement/inférence strictement en lecture seule : aucune implémentation ne met à jour de
 * poids. `Source/AiSolver/Eval` dépend des types de `Source/AiSolver/Training` (tout module
 * d'entraînement), jamais l'inverse (décision de cadrage de l'épic).
 */
class TrainedPolicy {
public:
    virtual ~TrainedPolicy() = default;

    /**
     * @brief Sélectionne une action à partir d'une observation déjà encodée.
     * @param observation Vecteur d'observation encodé (`ObservationEncoder::encode`), non modifié.
     * @param mode         Mode de décodage demandé.
     * @param rng          Seule source d'aléatoire consommée (mode `Stochastic` uniquement).
     * @return L'entrée joueur correspondante, ou `std::nullopt` si @p mode n'est pas supporté par
     *         cette politique (`supportsMode`) — erreur récupérable signalée, pas de plantage
     *         (`EX-NFR-040`).
     */
    [[nodiscard]] virtual std::optional<core::PlayerInput> selectAction(
        const Tensor<float>& observation, ActionDecodingMode mode, Rng& rng) = 0;

    /// @return `true` si @p mode est valide pour cette politique (voir les adaptateurs concrets).
    [[nodiscard]] virtual bool supportsMode(ActionDecodingMode mode) const noexcept {
        (void)mode;
        return true;
    }
};

namespace detail {

/**
 * @brief Décode une distribution/valeurs `Q(s, ·)` déjà calculée en une entrée joueur, factorisant
 * la logique commune aux quatre adaptateurs de `TrainedPolicy` (partagée, pas dupliquée par
 * adaptateur).
 * @param distribution Distribution de probabilité (politique) ou valeurs `Q(s, ·)`, une composante
 *        par action de l'espace discret.
 * @param mode Mode de décodage demandé (`Argmax` ou `Stochastic`).
 * @param rng Seule source d'aléatoire consommée (mode `Stochastic` uniquement).
 * @param allowStochastic `false` si l'appelant ne supporte pas `ActionDecodingMode::Stochastic`
 *        (évolutionniste, algorithme avancé fondé sur `Q(s, a)`).
 */
[[nodiscard]] std::optional<core::PlayerInput> decodeFromDistribution(
    const Tensor<float>& distribution, ActionDecodingMode mode, Rng& rng, bool allowStochastic);

}  // namespace detail

}  // namespace aisolver::eval
