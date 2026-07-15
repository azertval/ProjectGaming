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
 * rendu alpha). La génération est **déterministe**. La classe est conçue pour être
 * remplaçable plus tard par un chargement de fichier (l'interface — vue de texture,
 * dimensions, régions — reste stable). Ressources Direct3D en RAII (`ComPtr`).
 */
class TextureAtlas {
public:
    /// Côté d'une tuile, en pixels (`EX-ARCH-021`).
    static constexpr int TILE_SIZE = 16;
    /// Nombre de tuiles par ligne et par colonne dans l'atlas généré.
    static constexpr int TILES_PER_SIDE = 4;

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
        return TILE_SIZE * TILES_PER_SIDE;
    }

    /// @return Hauteur de l'atlas, en pixels.
    [[nodiscard]] int height() const {
        return TILE_SIZE * TILES_PER_SIDE;
    }

    /**
     * @brief Région (en pixels) de la tuile à une position de la grille.
     * @param column Colonne de la tuile (0 à TILES_PER_SIDE-1).
     * @param row    Ligne de la tuile (0 à TILES_PER_SIDE-1).
     * @return La région d'atlas correspondante, en pixels.
     */
    [[nodiscard]] core::AtlasRegion tile(int column, int row) const;

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _view;
};

}  // namespace hmi
