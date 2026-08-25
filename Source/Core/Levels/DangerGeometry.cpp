// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/DangerGeometry.h"

namespace core {

Aabb dangerHitbox(TileType type, int column, int row) noexcept {
    const auto left = static_cast<float>(column);
    const auto top = static_cast<float>(row);
    const float right = left + 1.0F;
    const float bottom = top + 1.0F;

    switch (type) {
        case TileType::DangerUp:
            return Aabb{.min = Vector2{left, top},
                        .max = Vector2{right, top + DANGER_EDGE_THICKNESS}};
        case TileType::DangerDown:
            return Aabb{.min = Vector2{left, bottom - DANGER_EDGE_THICKNESS},
                        .max = Vector2{right, bottom}};
        case TileType::DangerLeft:
            return Aabb{.min = Vector2{left, top},
                        .max = Vector2{left + DANGER_EDGE_THICKNESS, bottom}};
        case TileType::DangerRight:
            return Aabb{.min = Vector2{right - DANGER_EDGE_THICKNESS, top},
                        .max = Vector2{right, bottom}};
        default:
            return Aabb{.min = Vector2{left, top}, .max = Vector2{right, bottom}};
    }
}

bool isDangerTileType(TileType type) noexcept {
    switch (type) {
        case TileType::Danger:
        case TileType::DangerUp:
        case TileType::DangerDown:
        case TileType::DangerLeft:
        case TileType::DangerRight:
        case TileType::DangerMover:
        case TileType::DangerSwitched:
        case TileType::DangerBlink:
            return true;
        default:
            return false;
    }
}

}  // namespace core
