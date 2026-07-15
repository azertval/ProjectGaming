/**
 * @file Core/Math/Vector2.cpp
 * @brief Implémentation du vecteur 2D de `Core`.
 */

#include "Core/Math/Vector2.h"

#include <cmath>

#include "Core/Math/MathUtils.h"

namespace core {

Vector2::Vector2(float xComponent, float yComponent) : x(xComponent), y(yComponent) {}

Vector2& Vector2::operator+=(const Vector2& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2& Vector2::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2& Vector2::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

float Vector2::dot(const Vector2& other) const {
    return x * other.x + y * other.y;
}

float Vector2::lengthSquared() const {
    return x * x + y * y;
}

float Vector2::length() const {
    return std::sqrt(lengthSquared());
}

Vector2 Vector2::normalized() const {
    const float len = length();
    if (len <= kEpsilon) {
        // Le vecteur nul (ou quasi nul) n'a pas de direction : on renvoie zéro.
        return Vector2{};
    }
    return Vector2{x / len, y / len};
}

Vector2 operator+(const Vector2& lhs, const Vector2& rhs) {
    return Vector2{lhs.x + rhs.x, lhs.y + rhs.y};
}

Vector2 operator-(const Vector2& lhs, const Vector2& rhs) {
    return Vector2{lhs.x - rhs.x, lhs.y - rhs.y};
}

Vector2 operator*(const Vector2& vector, float scalar) {
    return Vector2{vector.x * scalar, vector.y * scalar};
}

Vector2 operator*(float scalar, const Vector2& vector) {
    return vector * scalar;
}

Vector2 operator/(const Vector2& vector, float scalar) {
    return Vector2{vector.x / scalar, vector.y / scalar};
}

Vector2 operator-(const Vector2& vector) {
    return Vector2{-vector.x, -vector.y};
}

bool operator==(const Vector2& lhs, const Vector2& rhs) {
    return approximatelyEqual(lhs.x, rhs.x) && approximatelyEqual(lhs.y, rhs.y);
}

bool operator!=(const Vector2& lhs, const Vector2& rhs) {
    return !(lhs == rhs);
}

}  // namespace core
