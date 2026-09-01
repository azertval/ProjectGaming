// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/ObjectiveEncoder.h"

#include <algorithm>
#include <array>
#include <cstddef>

namespace aisolver {

Tensor<float> ObjectiveEncoder::encode(const GridDistanceField& field,
                                       core::GridPosition center) const {
    Tensor<float> result({static_cast<std::size_t>(OBJECTIVE_STATE_SIZE)});
    float* out = result.data();

    const bool reachable = field.isReachable(center);
    out[0] = reachable ? 1.0f : 0.0f;

    // Normalisation par la sentinelle : borne superieure stricte de toute distance atteignable sur
    // la grille, donc la seule constante de normalisation qui ne depende pas du niveau charge.
    const auto sentinel = static_cast<float>(field.unreachableDistance());
    const int centerDistance = field.distance(center);
    out[1] = sentinel > 0.0f ? static_cast<float>(centerDistance) / sentinel : 1.0f;

    // Meme ordre de voisins que le BFS de `GridDistanceField` : haut, bas, gauche, droite.
    static constexpr std::array<int, 4> DELTA_COLUMN{0, 0, -1, 1};
    static constexpr std::array<int, 4> DELTA_ROW{-1, 1, 0, 0};
    for (std::size_t direction = 0; direction < DELTA_COLUMN.size(); ++direction) {
        const core::GridPosition neighbor{center.column + DELTA_COLUMN[direction],
                                          center.row + DELTA_ROW[direction]};
        float gradient = 0.0f;
        if (reachable && field.isReachable(neighbor)) {
            gradient = static_cast<float>(centerDistance - field.distance(neighbor));
        }
        out[2 + direction] = std::clamp(gradient, -1.0f, 1.0f);
    }
    return result;
}

}  // namespace aisolver
