#include "HMI/Graphics/TextureAtlas.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

namespace {
// Assemble une couleur RVBA (octets) en un pixel `R8G8B8A8_UNORM` (ordre mémoire R,G,B,A).
std::uint32_t pack(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) {
    return static_cast<std::uint32_t>(red) | (static_cast<std::uint32_t>(green) << 8) |
           (static_cast<std::uint32_t>(blue) << 16) | (static_cast<std::uint32_t>(alpha) << 24);
}

// Couleur opaque de base d'une tuile, selon son index dans la grille (déterministe).
std::uint32_t tileColor(int tileIndex) {
    static const std::uint32_t palette[] = {
        pack(200, 60, 60, 255),   pack(60, 200, 60, 255),  pack(60, 60, 200, 255),
        pack(200, 200, 60, 255),  pack(200, 60, 200, 255), pack(60, 200, 200, 255),
        pack(230, 140, 40, 255),  pack(140, 40, 230, 255), pack(120, 120, 120, 255),
        pack(80, 160, 120, 255),  pack(160, 80, 120, 255), pack(120, 80, 160, 255),
        pack(200, 200, 200, 255), pack(90, 90, 90, 255),   pack(40, 120, 200, 255),
        pack(200, 120, 40, 255),
    };
    const int count = static_cast<int>(std::size(palette));
    return palette[((tileIndex % count) + count) % count];
}

// Vrai si value est dans l'intervalle ferme [low, high] (bornes des zones du personnage).
bool inRange(int value, int low, int high) {
    return value >= low && value <= high;
}

// Couleur du pixel (x, y) de la silhouette du personnage, (0,0) = coin haut-gauche de la
// region 16x32 (EX-REN-011). Silhouette humanoide simple, blocs rectangulaires : cheveux,
// peau, chemise/manches, mains, pantalon, chaussures. Transparent hors silhouette.
std::uint32_t playerPixel(int x, int y) {
    const std::uint32_t hair = pack(90, 60, 40, 255);
    const std::uint32_t skin = pack(230, 190, 150, 255);
    const std::uint32_t shirt = pack(50, 110, 200, 255);
    const std::uint32_t pants = pack(60, 60, 70, 255);
    const std::uint32_t shoes = pack(30, 30, 35, 255);
    const std::uint32_t transparent = pack(0, 0, 0, 0);

    // Tete (lignes 0-7) : cheveux, puis peau avec cheveux sur les cotes, puis nuque.
    if (inRange(y, 0, 1)) {
        return inRange(x, 5, 10) ? hair : transparent;
    }
    if (inRange(y, 2, 4)) {
        if (x == 5 || x == 10) {
            return hair;
        }
        return inRange(x, 6, 9) ? skin : transparent;
    }
    if (inRange(y, 5, 6)) {
        return inRange(x, 5, 10) ? skin : transparent;
    }
    if (y == 7) {
        return inRange(x, 6, 9) ? skin : transparent;
    }
    // Torse (lignes 8-19) : epaules, puis bras+torse, mains aux extremites des bras, epaules.
    if (y == 8 || inRange(y, 17, 19)) {
        return inRange(x, 4, 11) ? shirt : transparent;
    }
    if (inRange(y, 9, 14)) {
        return inRange(x, 2, 13) ? shirt : transparent;
    }
    if (inRange(y, 15, 16)) {
        if (inRange(x, 2, 3) || inRange(x, 12, 13)) {
            return skin;
        }
        return inRange(x, 4, 11) ? shirt : transparent;
    }
    // Jambes (lignes 20-31) : pantalon puis chaussures, separees par un espace transparent.
    if (inRange(y, 20, 27)) {
        return (inRange(x, 5, 7) || inRange(x, 9, 11)) ? pants : transparent;
    }
    if (inRange(y, 28, 31)) {
        return (inRange(x, 5, 7) || inRange(x, 9, 11)) ? shoes : transparent;
    }
    return transparent;
}
}  // namespace

// Génère l'atlas procédural et crée la ressource Direct3D associée.
TextureAtlas::TextureAtlas(ID3D11Device* device) {
    const int gridSide = TILE_SIZE * TILES_PER_SIDE;
    // La region du personnage est ajoutee sous la grille de tuiles, dans la meme texture
    // (le rendu ne dessine qu'une seule texture par passe, cf. SpriteRenderer).
    const int playerRegionTop = gridSide;
    _width = gridSide;
    _height = gridSide + PLAYER_REGION_HEIGHT;
    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(_width) *
                                      static_cast<std::size_t>(_height));

    // Dernière tuile réservée au test de transparence : damier opaque / transparent.
    const int transparentTileIndex = TILES_PER_SIDE * TILES_PER_SIDE - 1;

    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            std::uint32_t color = pack(0, 0, 0, 0);
            if (y < gridSide) {
                const int tileColumn = x / TILE_SIZE;
                const int tileRow = y / TILE_SIZE;
                const int tileIndex = tileRow * TILES_PER_SIDE + tileColumn;

                color = tileColor(tileIndex);
                if (tileIndex == transparentTileIndex) {
                    // Un damier 4×4 pixels : une case sur deux est entièrement transparente.
                    const bool transparent = (((x / 4) + (y / 4)) % 2) == 0;
                    color = transparent ? pack(0, 0, 0, 0) : pack(240, 240, 240, 255);
                }
            } else if (x < PLAYER_REGION_WIDTH) {
                color = playerPixel(x, y - playerRegionTop);
            }
            pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(_width) +
                   static_cast<std::size_t>(x)] = color;
        }
    }

    D3D11_TEXTURE2D_DESC description{};
    description.Width = static_cast<UINT>(_width);
    description.Height = static_cast<UINT>(_height);
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = pixels.data();
    data.SysMemPitch = static_cast<UINT>(_width) * sizeof(std::uint32_t);

    if (FAILED(device->CreateTexture2D(&description, &data, &_texture))) {
        throw std::runtime_error("Echec de creation de la texture d'atlas");
    }
    if (FAILED(device->CreateShaderResourceView(_texture.Get(), nullptr, &_view))) {
        throw std::runtime_error("Echec de creation de la vue de la texture d'atlas");
    }
    GRAPHICS_LOG_TRACE("TextureAtlas : atlas procedural genere");
}

// Région (en pixels) de la tuile à une position de la grille.
// La région d'atlas correspondante, en pixels.
core::AtlasRegion TextureAtlas::tile(int column, int row) const {
    return core::AtlasRegion{column * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
}

// Région (en pixels) de la silhouette du personnage, sous la grille de tuiles.
core::AtlasRegion TextureAtlas::playerRegion() const {
    return core::AtlasRegion{0, TILE_SIZE * TILES_PER_SIDE, PLAYER_REGION_WIDTH,
                             PLAYER_REGION_HEIGHT};
}

}  // namespace hmi
