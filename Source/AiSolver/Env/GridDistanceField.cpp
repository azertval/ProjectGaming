// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/GridDistanceField.h"

#include <array>
#include <queue>

namespace aisolver {

namespace {
constexpr int kUnreachable = -1;
}  // namespace

GridDistanceField::GridDistanceField(const core::TileMap& tileMap, const core::GridPosition& target)
    : _width(tileMap.width()),
      _height(tileMap.height()),
      _distances(static_cast<std::size_t>(_width) * static_cast<std::size_t>(_height),
                 kUnreachable) {
    if (!tileMap.inBounds(target.column, target.row) ||
        tileMap.isSolid(target.column, target.row)) {
        // Cible hors-grille ou solide (ne devrait pas arriver pour une sortie valide, garde
        // défensive) : le champ reste entièrement inatteignable, distance() renvoie la sentinelle
        // partout plutôt que de planter.
        return;
    }

    const auto index = [this](int column, int row) {
        return static_cast<std::size_t>(row) * static_cast<std::size_t>(_width) +
               static_cast<std::size_t>(column);
    };

    _distances[index(target.column, target.row)] = 0;
    std::queue<core::GridPosition> frontier;
    frontier.push(target);

    static constexpr std::array<int, 4> kDeltaColumn{0, 0, -1, 1};
    static constexpr std::array<int, 4> kDeltaRow{-1, 1, 0, 0};

    while (!frontier.empty()) {
        const core::GridPosition current = frontier.front();
        frontier.pop();
        const int currentDistance = _distances[index(current.column, current.row)];

        for (std::size_t direction = 0; direction < kDeltaColumn.size(); ++direction) {
            const int neighborColumn = current.column + kDeltaColumn[direction];
            const int neighborRow = current.row + kDeltaRow[direction];
            if (!tileMap.inBounds(neighborColumn, neighborRow) ||
                tileMap.isSolid(neighborColumn, neighborRow)) {
                continue;
            }
            const std::size_t neighborIndex = index(neighborColumn, neighborRow);
            if (_distances[neighborIndex] != kUnreachable) {
                continue;
            }
            _distances[neighborIndex] = currentDistance + 1;
            frontier.push(core::GridPosition{neighborColumn, neighborRow});
        }
    }
}

int GridDistanceField::distance(const core::GridPosition& position) const noexcept {
    const int sentinel = _width * _height;
    if (position.column < 0 || position.row < 0 || position.column >= _width ||
        position.row >= _height) {
        return sentinel;
    }
    const std::size_t linearIndex =
        static_cast<std::size_t>(position.row) * static_cast<std::size_t>(_width) +
        static_cast<std::size_t>(position.column);
    const int value = _distances[linearIndex];
    return value == kUnreachable ? sentinel : value;
}

}  // namespace aisolver
