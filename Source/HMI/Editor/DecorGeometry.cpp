#include "HMI/Editor/DecorGeometry.h"

namespace hmi {

// Rectangle englobant d'un decor, en unites monde (voir en-tete).
core::Rect decorWorldBounds(const core::Decor& decor, core::Vector2 pixelSize) noexcept {
    return core::Rect{decor.position, core::Vector2{pixelSize.x * decor.scale.x,
                                                     pixelSize.y * decor.scale.y}};
}

namespace {
// Carre de cote 2*halfSize centre sur `center`.
core::Rect squareCenteredAt(core::Vector2 center, float halfSize) noexcept {
    return core::Rect{core::Vector2{center.x - halfSize, center.y - halfSize},
                      core::Vector2{halfSize * 2.0f, halfSize * 2.0f}};
}
}  // namespace

// Calcule les rectangles des poignees d'un decor selectionne (voir en-tete).
DecorHandleLayout decorHandleLayout(const core::Rect& bounds,
                                    float worldUnitsPerScreenPixel) noexcept {
    const float half = DECOR_HANDLE_SCREEN_SIZE * 0.5f * worldUnitsPerScreenPixel;
    const core::Vector2 topLeft = bounds.position;
    const core::Vector2 topRight{bounds.position.x + bounds.size.x, bounds.position.y};
    const core::Vector2 bottomLeft{bounds.position.x, bounds.position.y + bounds.size.y};
    const core::Vector2 bottomRight{bounds.position.x + bounds.size.x,
                                    bounds.position.y + bounds.size.y};
    const float rotationOffset = DECOR_ROTATION_HANDLE_SCREEN_OFFSET * worldUnitsPerScreenPixel;
    const core::Vector2 rotationCenter{bounds.position.x + bounds.size.x * 0.5f,
                                       bounds.position.y - rotationOffset};

    DecorHandleLayout layout;
    layout.topLeft = squareCenteredAt(topLeft, half);
    layout.topRight = squareCenteredAt(topRight, half);
    layout.bottomLeft = squareCenteredAt(bottomLeft, half);
    layout.bottomRight = squareCenteredAt(bottomRight, half);
    layout.rotation = squareCenteredAt(rotationCenter, half);
    return layout;
}

// Poignee de layout sous point, s'il y en a une (voir en-tete) : rotation en priorite, puis coins.
DecorHandle hitTestDecorHandles(core::Vector2 point, const DecorHandleLayout& layout) noexcept {
    if (layout.rotation.contains(point)) {
        return DecorHandle::Rotation;
    }
    if (layout.topLeft.contains(point)) {
        return DecorHandle::TopLeft;
    }
    if (layout.topRight.contains(point)) {
        return DecorHandle::TopRight;
    }
    if (layout.bottomLeft.contains(point)) {
        return DecorHandle::BottomLeft;
    }
    if (layout.bottomRight.contains(point)) {
        return DecorHandle::BottomRight;
    }
    return DecorHandle::None;
}

}  // namespace hmi
