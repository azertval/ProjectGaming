#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Core/Ecs/Components/Sprite.h"

/**
 * @file HMI/Graphics/TextureAtlas.h
 * @brief Atlas de textures (spritesheet) et ses régions de tuiles.
 */

namespace hmi {

/**
 * @brief Texture d'atlas et table de régions, avec un contenu **généré en code**.
 *
 * En l'absence d'asset graphique, l'atlas est produit en mémoire : une grille de tuiles
 * 16×16 de couleurs distinctes, dont une avec des zones **transparentes** (pour valider le
 * rendu alpha), complétée d'une **région dédiée au personnage** (silhouette humanoïde
 * 16×32, `EX-REN-011`) sous la grille. La génération est **déterministe**. La classe est
 * conçue pour être remplaçable plus tard par un chargement de fichier (l'interface — vue
 * de texture, dimensions, régions — reste stable). Ressources Direct3D en RAII (`ComPtr`).
 */
class TextureAtlas {
public:
    /// Côté d'une tuile, en pixels (`EX-ARCH-021`).
    static constexpr int TILE_SIZE = 16;
    /// Nombre de tuiles par ligne et par colonne dans la grille de tuiles générée.
    static constexpr int TILES_PER_SIDE = 4;
    /// Largeur de la région dédiée au personnage, en pixels.
    static constexpr int PLAYER_REGION_WIDTH = 16;
    /// Hauteur de la région dédiée au personnage, en pixels (ratio 1:2, cf. `core::playerSize`).
    static constexpr int PLAYER_REGION_HEIGHT = 32;

    /**
     * @brief Génère l'atlas procédural et crée la ressource Direct3D associée.
     * @param device Device Direct3D 11 (crée la texture et sa vue de ressource).
     */
    explicit TextureAtlas(ID3D11Device* device);

    /// @return La vue de ressource de la texture d'atlas (non possédée par l'appelant).
    [[nodiscard]] ID3D11ShaderResourceView* textureView() const {
        return _view.Get();
    }

    /// @return Largeur de l'atlas, en pixels.
    [[nodiscard]] int width() const {
        return _width;
    }

    /// @return Hauteur de l'atlas, en pixels.
    [[nodiscard]] int height() const {
        return _height;
    }

    /**
     * @brief Région (en pixels) de la tuile à une position de la grille.
     * @param column Colonne de la tuile (0 à TILES_PER_SIDE-1).
     * @param row    Ligne de la tuile (0 à TILES_PER_SIDE-1).
     * @return La région d'atlas correspondante, en pixels.
     */
    [[nodiscard]] core::AtlasRegion tile(int column, int row) const;

    /**
     * @brief Région (en pixels) de la silhouette du personnage, sous la grille de tuiles.
     * @return La région d'atlas de 16×32 pixels du personnage (`EX-REN-011`).
     */
    [[nodiscard]] core::AtlasRegion playerRegion() const;

private:
    int _width = 0;
    int _height = 0;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _view;
};

}  // namespace hmi
