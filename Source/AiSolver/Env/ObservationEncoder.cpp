// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/ObservationEncoder.h"

#include <cmath>

namespace aisolver {

namespace {

// Case de grille ou se trouve le centre d'une boite -- meme convention que
// test_observation_determinisme.cpp (LOT-ANNEXE-06, TACHE-04).
core::GridPosition centerCell(const core::Aabb& box) {
    const core::Vector2 center = (box.min + box.max) * 0.5f;
    return core::GridPosition{static_cast<int>(std::floor(center.x)),
                              static_cast<int>(std::floor(center.y))};
}

}  // namespace

ObservationEncoder::ObservationEncoder(int radius) : _radius(radius), _tileEncoder(radius) {}

std::size_t ObservationEncoder::inputSize() const noexcept {
    const std::size_t windowSize = static_cast<std::size_t>(_tileEncoder.windowSize());
    const std::size_t windowCells = windowSize * windowSize;
    const std::size_t channelCount = static_cast<std::size_t>(_tileEncoder.channelCount()) +
                                     static_cast<std::size_t>(_mechanismEncoder.channelCount());
    return channelCount * windowCells + static_cast<std::size_t>(PlayerStateEncoder::size());
}

Tensor<float> ObservationEncoder::encode(const HeadlessLevelEnvironment& environment,
                                         const core::Aabb& playerBox,
                                         const core::Player& playerState,
                                         const core::Velocity& playerVelocity) const {
    const core::GridPosition center = centerCell(playerBox);
    const Tensor<float> tiles = _tileEncoder.encode(environment.level().tileMap(), center);
    const Tensor<float> mechanisms = _mechanismEncoder.encode(
        environment.mechanisms(), environment.dangers(), environment.level(), center, _radius);
    const Tensor<float> player =
        _playerEncoder.encode(playerState, playerVelocity, environment.level());

    Tensor<float> flat({inputSize(), 1});
    float* out = flat.data();
    std::size_t offset = 0;
    // Tenseurs source fraichement alloues (Tensor(shape)) : tampon dense et contigu dans l'ordre
    // logique, une simple copie lineaire de data() est donc valide (pas de vue a strides non
    // canoniques a ce stade).
    for (std::size_t i = 0; i < tiles.size(); ++i) {
        out[offset++] = tiles.data()[i];
    }
    for (std::size_t i = 0; i < mechanisms.size(); ++i) {
        out[offset++] = mechanisms.data()[i];
    }
    for (std::size_t i = 0; i < player.size(); ++i) {
        out[offset++] = player.data()[i];
    }
    return flat;
}

}  // namespace aisolver
