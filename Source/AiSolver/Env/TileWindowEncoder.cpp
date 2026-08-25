// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/TileWindowEncoder.h"

#include <cstddef>

#include "Core/Diagnostics/Assert.h"

namespace aisolver {

TileWindowEncoder::TileWindowEncoder(int radius) : _radius(radius) {
    PROJECTGAMING_ASSERT(radius >= 0, "TileWindowEncoder : radius doit etre >= 0");
}

Tensor<float> TileWindowEncoder::encode(const core::TileMap& tiles,
                                        core::GridPosition center) const {
    const int size = windowSize();
    Tensor<float> result({static_cast<std::size_t>(CHANNEL_COUNT), static_cast<std::size_t>(size),
                          static_cast<std::size_t>(size)});

    for (int dr = -_radius; dr <= _radius; ++dr) {
        for (int dc = -_radius; dc <= _radius; ++dc) {
            const int column = center.column + dc;
            const int row = center.row + dr;
            if (!tiles.inBounds(column, row)) {
                continue;  // vecteur nul par defaut (Tensor initialise a T{})
            }
            const auto channel = static_cast<std::size_t>(tiles.tile(column, row));
            // Somme calculee en int, puis convertie : convertir chaque operande d'abord
            // elargirait le calcul lui-meme, ce que `bugprone-misplaced-widening-cast` refuse.
            const int windowRowIndex = dr + _radius;
            const int windowColumnIndex = dc + _radius;
            const auto windowRow = static_cast<std::size_t>(windowRowIndex);
            const auto windowColumn = static_cast<std::size_t>(windowColumnIndex);
            result.at({channel, windowRow, windowColumn}) = 1.0f;
        }
    }
    return result;
}

}  // namespace aisolver
