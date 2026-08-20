// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/DangerGeometry.h"

namespace core {

Aabb dangerHitbox(TileType type, int col, int row) noexcept {
    const auto left = static_cast<float>(col);
    const auto top = static_cast<float>(row);
    const float right = left + 1.0F;
    const float bottom = top + 1.0F;

    switch (type) {
        case TileType::DangerUp:
            return Aabb{.min = Vector2{left, top},
                        .max = Vector2{right, top + kDangerEdgeThickness}};
        case TileType::DangerDown:
            return Aabb{.min = Vector2{left, bottom - kDangerEdgeThickness},
                        .max = Vector2{right, bottom}};
        case TileType::DangerLeft:
            return Aabb{.min = Vector2{left, top},
                        .max = Vector2{left + kDangerEdgeThickness, bottom}};
        case TileType::DangerRight:
            return Aabb{.min = Vector2{right - kDangerEdgeThickness, top},
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
