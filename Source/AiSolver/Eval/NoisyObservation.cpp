// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/NoisyObservation.h"

namespace aisolver::eval {

Tensor<float> NoisyObservationWrapper::encode(const HeadlessLevelEnvironment& environment,
                                              const core::Aabb& playerBox,
                                              const core::Player& playerState,
                                              const core::Velocity& playerVelocity,
                                              Rng& rng) const {
    const Tensor<float> clean = _encoder.encode(environment, playerBox, playerState, playerVelocity);
    if (_noiseAmplitude <= 0.0f) {
        return clean;
    }
    Tensor<float> noisy = clean.clone();
    float* data = noisy.data();
    const std::size_t count = noisy.size();
    for (std::size_t index = 0; index < count; ++index) {
        data[index] += rng.nextGaussian(0.0f, _noiseAmplitude);
    }
    return noisy;
}

}  // namespace aisolver::eval
