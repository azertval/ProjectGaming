#include "HMI/Graphics/TextureAtlas.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

namespace {
/// Assemble une couleur RVBA (octets) en un pixel `R8G8B8A8_UNORM` (ordre mémoire R,G,B,A).
std::uint32_t pack(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) {
    return static_cast<std::uint32_t>(red) | (static_cast<std::uint32_t>(green) << 8) |
           (static_cast<std::uint32_t>(blue) << 16) | (static_cast<std::uint32_t>(alpha) << 24);
}

/// Couleur opaque de base d'une tuile, selon son index dans la grille (déterministe).
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
}  // namespace

/**
 * @brief Génère l'atlas procédural et crée la ressource Direct3D associée.
 * @param device Device Direct3D 11 (crée la texture et sa vue de ressource).
 */
TextureAtlas::TextureAtlas(ID3D11Device* device) {
    const int side = TILE_SIZE * TILES_PER_SIDE;
    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(side) *
                                      static_cast<std::size_t>(side));

    // Dernière tuile réservée au test de transparence : damier opaque / transparent.
    const int transparentTileIndex = TILES_PER_SIDE * TILES_PER_SIDE - 1;

    for (int y = 0; y < side; ++y) {
        for (int x = 0; x < side; ++x) {
            const int tileColumn = x / TILE_SIZE;
            const int tileRow = y / TILE_SIZE;
            const int tileIndex = tileRow * TILES_PER_SIDE + tileColumn;

            std::uint32_t color = tileColor(tileIndex);
            if (tileIndex == transparentTileIndex) {
                // Un damier 4×4 pixels : une case sur deux est entièrement transparente.
                const bool transparent = (((x / 4) + (y / 4)) % 2) == 0;
                color = transparent ? pack(0, 0, 0, 0) : pack(240, 240, 240, 255);
            }
            pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(side) +
                   static_cast<std::size_t>(x)] = color;
        }
    }

    D3D11_TEXTURE2D_DESC description{};
    description.Width = static_cast<UINT>(side);
    description.Height = static_cast<UINT>(side);
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = pixels.data();
    data.SysMemPitch = static_cast<UINT>(side) * sizeof(std::uint32_t);

    if (FAILED(device->CreateTexture2D(&description, &data, &_texture))) {
        throw std::runtime_error("Echec de creation de la texture d'atlas");
    }
    if (FAILED(device->CreateShaderResourceView(_texture.Get(), nullptr, &_view))) {
        throw std::runtime_error("Echec de creation de la vue de la texture d'atlas");
    }
    GRAPHICS_LOG_TRACE("TextureAtlas : atlas procedural genere");
}

/**
 * @brief Région (en pixels) de la tuile à une position de la grille.
 * @param column Colonne de la tuile (0 à TILES_PER_SIDE-1).
 * @param row    Ligne de la tuile (0 à TILES_PER_SIDE-1).
 * @return La région d'atlas correspondante, en pixels.
 */
core::AtlasRegion TextureAtlas::tile(int column, int row) const {
    return core::AtlasRegion{column * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
}

}  // namespace hmi
