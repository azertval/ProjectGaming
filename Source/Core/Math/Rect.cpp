#include "Core/Math/Rect.h"

namespace core {

/**
 * @brief Construit un rectangle à partir de son coin haut-gauche et de sa taille.
 * @param topLeft    Coin haut-gauche.
 * @param dimensions Largeur et hauteur (>= 0).
 */
Rect::Rect(const Vector2& topLeft, const Vector2& dimensions)
    : position(topLeft), size(dimensions) {}

/**
 * @brief Abscisse du bord gauche.
 * @return La composante x de la position.
 */
float Rect::left() const {
    return position.x;
}

/**
 * @brief Abscisse du bord droit.
 * @return `left + largeur`.
 */
float Rect::right() const {
    return position.x + size.x;
}

/**
 * @brief Ordonnée du bord supérieur.
 * @return La composante y de la position.
 */
float Rect::top() const {
    return position.y;
}

/**
 * @brief Ordonnée du bord inférieur.
 * @return `top + hauteur`.
 */
float Rect::bottom() const {
    return position.y + size.y;
}

/**
 * @brief Teste si un point est contenu dans le rectangle.
 * @param point Point à tester, en coordonnées monde.
 * @return `true` si le point est à l'intérieur ou sur un bord inclus (haut/gauche).
 */
bool Rect::contains(const Vector2& point) const {
    // Inclusif haut/gauche, exclusif bas/droit : des rectangles jointifs pavent
    // le plan sans recouvrement ni trou.
    return point.x >= left() && point.x < right() && point.y >= top() && point.y < bottom();
}

/**
 * @brief Teste le recouvrement avec un autre rectangle.
 * @param other Autre rectangle.
 * @return `true` si les deux rectangles partagent une aire strictement positive.
 */
bool Rect::intersects(const Rect& other) const {
    // Recouvrement strict : le contact par un bord ne compte pas comme intersection.
    return left() < other.right() && right() > other.left() && top() < other.bottom() &&
           bottom() > other.top();
}

}  // namespace core
