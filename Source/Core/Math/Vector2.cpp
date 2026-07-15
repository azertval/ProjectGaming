#include "Core/Math/Vector2.h"

#include <cmath>

#include "Core/Math/MathUtils.h"

namespace core {

/**
 * @brief Construit un vecteur à partir de ses composantes.
 * @param xComponent Composante horizontale.
 * @param yComponent Composante verticale.
 */
Vector2::Vector2(float xComponent, float yComponent) : x(xComponent), y(yComponent) {}

/**
 * @brief Ajoute un autre vecteur composante à composante.
 * @param other Vecteur à ajouter.
 * @return Une référence sur ce vecteur, modifié.
 */
Vector2& Vector2::operator+=(const Vector2& other) {
    x += other.x;
    y += other.y;
    return *this;
}

/**
 * @brief Soustrait un autre vecteur composante à composante.
 * @param other Vecteur à soustraire.
 * @return Une référence sur ce vecteur, modifié.
 */
Vector2& Vector2::operator-=(const Vector2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

/**
 * @brief Met le vecteur à l'échelle par un scalaire.
 * @param scalar Facteur d'échelle.
 * @return Une référence sur ce vecteur, modifié.
 */
Vector2& Vector2::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

/**
 * @brief Divise le vecteur par un scalaire.
 * @param scalar Diviseur (supposé non nul).
 * @return Une référence sur ce vecteur, modifié.
 */
Vector2& Vector2::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

/**
 * @brief Produit scalaire avec un autre vecteur.
 * @param other Autre opérande.
 * @return La somme des produits composante à composante.
 */
float Vector2::dot(const Vector2& other) const {
    return x * other.x + y * other.y;
}

/**
 * @brief Carré de la longueur euclidienne (évite une racine carrée).
 * @return `x*x + y*y`.
 */
float Vector2::lengthSquared() const {
    return x * x + y * y;
}

/**
 * @brief Longueur euclidienne du vecteur.
 * @return `sqrt(x*x + y*y)`.
 */
float Vector2::length() const {
    return std::sqrt(lengthSquared());
}

/**
 * @brief Renvoie le vecteur normalisé (longueur 1).
 * @return Un vecteur unitaire de même direction ; le vecteur nul si la longueur
 *         est négligeable (aucune direction définie).
 */
Vector2 Vector2::normalized() const {
    const float len = length();
    if (len <= kEpsilon) {
        // Le vecteur nul (ou quasi nul) n'a pas de direction : on renvoie zéro.
        return Vector2{};
    }
    return Vector2{x / len, y / len};
}

/**
 * @brief Somme de deux vecteurs.
 * @param lhs Premier opérande.
 * @param rhs Second opérande.
 * @return Le vecteur somme.
 */
Vector2 operator+(const Vector2& lhs, const Vector2& rhs) {
    return Vector2{lhs.x + rhs.x, lhs.y + rhs.y};
}

/**
 * @brief Différence de deux vecteurs.
 * @param lhs Premier opérande.
 * @param rhs Second opérande.
 * @return Le vecteur différence `lhs - rhs`.
 */
Vector2 operator-(const Vector2& lhs, const Vector2& rhs) {
    return Vector2{lhs.x - rhs.x, lhs.y - rhs.y};
}

/**
 * @brief Produit d'un vecteur par un scalaire.
 * @param vector Vecteur à mettre à l'échelle.
 * @param scalar Facteur d'échelle.
 * @return Le vecteur mis à l'échelle.
 */
Vector2 operator*(const Vector2& vector, float scalar) {
    return Vector2{vector.x * scalar, vector.y * scalar};
}

/**
 * @brief Produit d'un scalaire par un vecteur (commutatif).
 * @param scalar Facteur d'échelle.
 * @param vector Vecteur à mettre à l'échelle.
 * @return Le vecteur mis à l'échelle.
 */
Vector2 operator*(float scalar, const Vector2& vector) {
    return vector * scalar;
}

/**
 * @brief Division d'un vecteur par un scalaire.
 * @param vector Vecteur à diviser.
 * @param scalar Diviseur (supposé non nul).
 * @return Le vecteur divisé.
 */
Vector2 operator/(const Vector2& vector, float scalar) {
    return Vector2{vector.x / scalar, vector.y / scalar};
}

/**
 * @brief Opposé d'un vecteur.
 * @param vector Vecteur à inverser.
 * @return Le vecteur de composantes opposées.
 */
Vector2 operator-(const Vector2& vector) {
    return Vector2{-vector.x, -vector.y};
}

/**
 * @brief Égalité approchée entre deux vecteurs (comparaison par tolérance).
 * @param lhs Premier opérande.
 * @param rhs Second opérande.
 * @return `true` si les composantes coïncident à la tolérance près.
 */
bool operator==(const Vector2& lhs, const Vector2& rhs) {
    return approximatelyEqual(lhs.x, rhs.x) && approximatelyEqual(lhs.y, rhs.y);
}

/**
 * @brief Inégalité approchée entre deux vecteurs.
 * @param lhs Premier opérande.
 * @param rhs Second opérande.
 * @return `true` si les vecteurs diffèrent au-delà de la tolérance.
 */
bool operator!=(const Vector2& lhs, const Vector2& rhs) {
    return !(lhs == rhs);
}

}  // namespace core
