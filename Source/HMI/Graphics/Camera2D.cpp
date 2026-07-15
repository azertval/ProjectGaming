#include "HMI/Graphics/Camera2D.h"

namespace hmi {

/**
 * @brief Construit une caméra pour une surface de rendu donnée.
 * @param viewportWidth  Largeur de la surface de rendu, en pixels.
 * @param viewportHeight Hauteur de la surface de rendu, en pixels.
 */
Camera2D::Camera2D(int viewportWidth, int viewportHeight)
    : _viewportWidth(viewportWidth), _viewportHeight(viewportHeight) {}

/**
 * @brief Met à jour les dimensions de la surface de rendu (redimensionnement).
 * @param viewportWidth  Nouvelle largeur, en pixels.
 * @param viewportHeight Nouvelle hauteur, en pixels.
 */
void Camera2D::setViewportSize(int viewportWidth, int viewportHeight) {
    _viewportWidth = viewportWidth;
    _viewportHeight = viewportHeight;
}

/**
 * @brief Place le centre de la caméra.
 * @param worldCenter Position visée, au centre de l'écran, en unités monde.
 */
void Camera2D::setCenter(const core::Vector2& worldCenter) {
    _center = worldCenter;
}

/**
 * @brief Règle le facteur de zoom.
 * @param zoom Multiplicateur d'échelle (> 0 ; entier recommandé).
 */
void Camera2D::setZoom(float zoom) {
    _zoom = zoom;
}

/// @return Le centre de la caméra, en unités monde.
core::Vector2 Camera2D::center() const {
    return _center;
}

/// @return Le facteur de zoom courant.
float Camera2D::zoom() const {
    return _zoom;
}

/// @return L'échelle effective, en pixels par unité monde (PIXELS_PER_UNIT × zoom).
float Camera2D::scale() const {
    return PIXELS_PER_UNIT * _zoom;
}

/**
 * @brief Matrice de projection monde → clip, pour le vertex shader.
 * @return La matrice (ligne-major DirectXMath) transformant une position monde en clip.
 */
DirectX::XMFLOAT4X4 Camera2D::projectionMatrix() const {
    // Échelle monde → clip sur chaque axe : la moitié d'écran (viewport/2 pixels) doit
    // couvrir 1 en NDC. L'axe Y est inversé (monde Y-bas, NDC Y-haut).
    const float scaleX = scale() * 2.0f / static_cast<float>(_viewportWidth);
    const float scaleY = scale() * 2.0f / static_cast<float>(_viewportHeight);
    const float translateX = -_center.x * scaleX;
    const float translateY = _center.y * scaleY;

    // Ligne-major, appliquée en `position * matrice` (convention DirectXMath) :
    // clip.x =  scaleX * (x - cx) ; clip.y = -scaleY * (y - cy) ; clip.z = 0 ; clip.w = 1.
    return DirectX::XMFLOAT4X4(scaleX, 0.0f, 0.0f, 0.0f,   //
                               0.0f, -scaleY, 0.0f, 0.0f,  //
                               0.0f, 0.0f, 1.0f, 0.0f,     //
                               translateX, translateY, 0.0f, 1.0f);
}

/**
 * @brief Convertit une position monde en pixels écran.
 * @param world Position en unités monde.
 * @return Position en pixels (origine haut-gauche, Y-bas).
 */
core::Vector2 Camera2D::worldToScreen(const core::Vector2& world) const {
    const float halfWidth = static_cast<float>(_viewportWidth) * 0.5f;
    const float halfHeight = static_cast<float>(_viewportHeight) * 0.5f;
    return core::Vector2{(world.x - _center.x) * scale() + halfWidth,
                         (world.y - _center.y) * scale() + halfHeight};
}

/**
 * @brief Convertit une position écran (pixels) en unités monde.
 * @param screen Position en pixels (origine haut-gauche, Y-bas).
 * @return Position en unités monde.
 */
core::Vector2 Camera2D::screenToWorld(const core::Vector2& screen) const {
    const float halfWidth = static_cast<float>(_viewportWidth) * 0.5f;
    const float halfHeight = static_cast<float>(_viewportHeight) * 0.5f;
    return core::Vector2{(screen.x - halfWidth) / scale() + _center.x,
                         (screen.y - halfHeight) / scale() + _center.y};
}

}  // namespace hmi
