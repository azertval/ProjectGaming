#include "HMI/Editor/DecorGeometry.h"

#include <cmath>

#include "HMI/Graphics/Camera2D.h"

namespace hmi {

// Rectangle englobant d'un decor, en unites monde (voir en-tete). pixelSize est en PIXELS de
// l'asset : la conversion vers les unites monde (16 px/unite, EX-ARCH-021) doit se faire ici,
// meme formule que la taille du quad reellement rendu (hmi::composeWorldSprites, LOT-49) --
// l'oublier fait un rectangle 16x trop grand, une poignee bien plus grande que le decor lui-meme.
core::Rect decorWorldBounds(const core::Decor& decor, core::Vector2 pixelSize) noexcept {
    return core::Rect{decor.position,
                      core::Vector2{pixelSize.x / Camera2D::PIXELS_PER_UNIT * decor.scale.x,
                                    pixelSize.y / Camera2D::PIXELS_PER_UNIT * decor.scale.y}};
}

// Point tourne autour du centre de bounds (voir en-tete) -- meme formule que
// hmi::SpriteBatch::draw(const SpriteQuad&) pour un decor reellement rendu pivote (LOT-50), afin
// que le cadre de selection tourne visiblement avec lui plutot que de rester droit.
core::Vector2 decorRotatedPoint(const core::Rect& bounds, core::Vector2 localOffset,
                                float rotation) noexcept {
    const core::Vector2 center{bounds.position.x + bounds.size.x * 0.5f,
                               bounds.position.y + bounds.size.y * 0.5f};
    const float cosR = std::cos(rotation);
    const float sinR = std::sin(rotation);
    return core::Vector2{center.x + localOffset.x * cosR - localOffset.y * sinR,
                         center.y + localOffset.x * sinR + localOffset.y * cosR};
}

namespace {
// Carre de cote 2*halfSize centre sur `center`.
core::Rect squareCenteredAt(core::Vector2 center, float halfSize) noexcept {
    return core::Rect{core::Vector2{center.x - halfSize, center.y - halfSize},
                      core::Vector2{halfSize * 2.0f, halfSize * 2.0f}};
}
}  // namespace

// Calcule les rectangles des poignees d'un decor selectionne (voir en-tete).
DecorHandleLayout decorHandleLayout(const core::Rect& bounds, float worldUnitsPerScreenPixel,
                                    float rotation) noexcept {
    const float half = DECOR_HANDLE_SCREEN_SIZE * 0.5f * worldUnitsPerScreenPixel;
    const float halfWidth = bounds.size.x * 0.5f;
    const float halfHeight = bounds.size.y * 0.5f;
    const float rotationOffset = DECOR_ROTATION_HANDLE_SCREEN_OFFSET * worldUnitsPerScreenPixel;

    DecorHandleLayout layout;
    layout.topLeft = squareCenteredAt(
        decorRotatedPoint(bounds, core::Vector2{-halfWidth, -halfHeight}, rotation), half);
    layout.topRight = squareCenteredAt(
        decorRotatedPoint(bounds, core::Vector2{halfWidth, -halfHeight}, rotation), half);
    layout.bottomLeft = squareCenteredAt(
        decorRotatedPoint(bounds, core::Vector2{-halfWidth, halfHeight}, rotation), half);
    layout.bottomRight = squareCenteredAt(
        decorRotatedPoint(bounds, core::Vector2{halfWidth, halfHeight}, rotation), half);
    layout.rotation = squareCenteredAt(
        decorRotatedPoint(bounds, core::Vector2{0.0f, -halfHeight - rotationOffset}, rotation),
        half);
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
