#include "Core/Levels/DangerGeometry.h"

namespace core {

Aabb dangerHitbox(TileType type, int col, int row) noexcept {
    const float left = static_cast<float>(col);
    const float top = static_cast<float>(row);
    const float right = left + 1.0f;
    const float bottom = top + 1.0f;

    switch (type) {
        case TileType::DangerUp:
            return Aabb{Vector2{left, top}, Vector2{right, top + kDangerEdgeThickness}};
        case TileType::DangerDown:
            return Aabb{Vector2{left, bottom - kDangerEdgeThickness}, Vector2{right, bottom}};
        case TileType::DangerLeft:
            return Aabb{Vector2{left, top}, Vector2{left + kDangerEdgeThickness, bottom}};
        case TileType::DangerRight:
            return Aabb{Vector2{right - kDangerEdgeThickness, top}, Vector2{right, bottom}};
        default:
            return Aabb{Vector2{left, top}, Vector2{right, bottom}};
    }
}

}  // namespace core
