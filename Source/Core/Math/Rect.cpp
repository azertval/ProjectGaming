/**
 * @file Core/Math/Rect.cpp
 * @brief Implémentation du rectangle aligné sur les axes de `Core`.
 */

#include "Core/Math/Rect.h"

namespace core {

Rect::Rect(const Vector2& topLeft, const Vector2& dimensions)
    : position(topLeft), size(dimensions) {}

float Rect::left() const {
    return position.x;
}

float Rect::right() const {
    return position.x + size.x;
}

float Rect::top() const {
    return position.y;
}

float Rect::bottom() const {
    return position.y + size.y;
}

bool Rect::contains(const Vector2& point) const {
    // Inclusif haut/gauche, exclusif bas/droit : des rectangles jointifs pavent
    // le plan sans recouvrement ni trou.
    return point.x >= left() && point.x < right() && point.y >= top() && point.y < bottom();
}

bool Rect::intersects(const Rect& other) const {
    // Recouvrement strict : le contact par un bord ne compte pas comme intersection.
    return left() < other.right() && right() > other.left() && top() < other.bottom() &&
           bottom() > other.top();
}

}  // namespace core
