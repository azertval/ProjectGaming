#pragma once

#include <DirectXMath.h>

#include "Core/Math/Vector2.h"

/**
 * @file HMI/Graphics/Camera2D.h
 * @brief Caméra 2D : conversion entre unités monde et pixels écran.
 */

namespace hmi {

/**
 * @brief Caméra 2D orthographique : projette le monde vers l'écran pour le rendu.
 *
 * La caméra est centrée sur une position **en unités monde** et applique l'échelle
 * **16 pixels/unité** (`EX-ARCH-021`), avec l'origine écran en haut-gauche et l'axe Y vers
 * le bas (convention du projet). Un facteur de **zoom** (de préférence entier pour la
 * netteté du pixel art, `EX-ARCH-022`) multiplie cette échelle. La caméra est un objet de
 * **présentation** : elle lit des positions monde mais ne modifie jamais l'ECS.
 */
class Camera2D {
public:
    /// Nombre de pixels par unité monde (`EX-ARCH-021`).
    static constexpr float PIXELS_PER_UNIT = 16.0f;

    /**
     * @brief Construit une caméra pour une surface de rendu donnée.
     * @param viewportWidth  Largeur de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur de la surface de rendu, en pixels.
     */
    Camera2D(int viewportWidth, int viewportHeight);

    /**
     * @brief Met à jour les dimensions de la surface de rendu (redimensionnement).
     * @param viewportWidth  Nouvelle largeur, en pixels.
     * @param viewportHeight Nouvelle hauteur, en pixels.
     */
    void setViewportSize(int viewportWidth, int viewportHeight);

    /**
     * @brief Place le centre de la caméra.
     * @param worldCenter Position visée, au centre de l'écran, en unités monde.
     */
    void setCenter(const core::Vector2& worldCenter);

    /**
     * @brief Règle le facteur de zoom.
     * @param zoom Multiplicateur d'échelle (> 0 ; entier recommandé).
     */
    void setZoom(float zoom);

    /**
     * @brief Matrice de projection monde → clip, pour le vertex shader.
     * @return La matrice (ligne-major DirectXMath) transformant une position monde en clip.
     */
    [[nodiscard]] DirectX::XMFLOAT4X4 projectionMatrix() const;

    /**
     * @brief Convertit une position monde en pixels écran.
     * @param world Position en unités monde.
     * @return Position en pixels (origine haut-gauche, Y-bas).
     */
    [[nodiscard]] core::Vector2 worldToScreen(const core::Vector2& world) const;

    /**
     * @brief Convertit une position écran (pixels) en unités monde.
     * @param screen Position en pixels (origine haut-gauche, Y-bas).
     * @return Position en unités monde.
     */
    [[nodiscard]] core::Vector2 screenToWorld(const core::Vector2& screen) const;

private:
    /// @return L'échelle effective, en pixels par unité monde (PIXELS_PER_UNIT × zoom).
    [[nodiscard]] float scale() const;

    int _viewportWidth;
    int _viewportHeight;
    core::Vector2 _center{};
    float _zoom = 1.0f;
};

}  // namespace hmi
