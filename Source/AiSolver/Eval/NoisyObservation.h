// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Math/Rng.h"

/**
 * @file AiSolver/Eval/NoisyObservation.h
 * @brief Décorateur d'encodeur d'observation appliquant un bruit gaussien léger, pour le test de
 * robustesse (`LOT-ANNEXE-15`, TACHE-03, `EX-IA-016`).
 */

namespace aisolver::eval {

/**
 * @brief Décore un `ObservationEncoder` (`LOT-ANNEXE-06`) : encode normalement puis ajoute un bruit
 * gaussien indépendant (moyenne `0`, écart-type `noiseAmplitude`) à chaque composante du vecteur
 * d'observation, **avant** transmission à la politique.
 *
 * Ne touche jamais `HeadlessLevelEnvironment` ni `Source/AiSolver/Env` : décorateur pur qui
 * s'interpose entre l'encodeur existant et la politique (critère d'acceptation 5 de l'épic).
 * `noiseAmplitude <= 0.0f` retourne l'observation inchangée, sans consommer `rng` — condition
 * nécessaire pour que `runWithNoise(..., 0.0f, ...)` soit bit-à-bit identique à `run()`
 * (`BenchmarkRunner`).
 */
class NoisyObservationWrapper {
public:
    /// @param encoder Encodeur décoré, non possédé (doit survivre à ce wrapper).
    /// @param noiseAmplitude Écart-type du bruit ajouté à chaque composante ; `<= 0.0f` désactive
    ///        le bruit.
    NoisyObservationWrapper(const ObservationEncoder& encoder, float noiseAmplitude)
        : _encoder(encoder), _noiseAmplitude(noiseAmplitude) {}

    /// @brief Même contrat que `ObservationEncoder::encode`, plus @p rng (seule source d'aléatoire
    /// du bruit, dérivée par l'appelant de la même graine que le reste de la répétition).
    [[nodiscard]] Tensor<float> encode(const HeadlessLevelEnvironment& environment,
                                       const core::Aabb& playerBox, const core::Player& playerState,
                                       const core::Velocity& playerVelocity, Rng& rng) const;

    /// @return L'amplitude de bruit configurée.
    [[nodiscard]] float noiseAmplitude() const noexcept {
        return _noiseAmplitude;
    }

private:
    const ObservationEncoder& _encoder;
    float _noiseAmplitude;
};

}  // namespace aisolver::eval
