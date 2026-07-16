#include "HMI/Graphics/SaveIcon.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

namespace {
/// Pixel blanc opaque (l'icône est colorée par la teinte au dessin).
constexpr std::uint32_t ON = 0xFFFFFFFFu;
/// Pixel transparent.
constexpr std::uint32_t OFF = 0x00000000u;

/**
 * @brief Indique si le pixel (@p x, @p y) appartient au dessin de l'icône.
 *
 * L'icône (16×16) figure une **flèche descendante** au-dessus d'un **support** : une hampe
 * verticale centrée, une pointe triangulaire vers le bas, et une barre horizontale (avec
 * montants) évoquant le plateau où « déposer » le fichier.
 */
[[nodiscard]] bool isDrawn(int x, int y) noexcept {
    // Hampe de la flèche.
    if ((x == 7 || x == 8) && y >= 2 && y <= 7) {
        return true;
    }
    // Pointe de la flèche (triangle inversé) sur les lignes 8 à 11.
    if (y >= 8 && y <= 11) {
        const int row = y - 8;
        const int left = 4 + row;
        const int right = 11 - row;
        if (x >= left && x <= right) {
            return true;
        }
    }
    // Support : barre horizontale du bas et ses deux montants.
    if (y == 14 && x >= 3 && x <= 12) {
        return true;
    }
    if ((x == 3 || x == 12) && y >= 12 && y <= 14) {
        return true;
    }
    return false;
}
}  // namespace

/**
 * @brief Génère la texture de l'icône et crée la ressource Direct3D associée.
 * @param device Device Direct3D 11 (crée la texture et sa vue de ressource).
 */
SaveIcon::SaveIcon(ID3D11Device* device) {
    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(SIZE) *
                                      static_cast<std::size_t>(SIZE));
    for (int y = 0; y < SIZE; ++y) {
        for (int x = 0; x < SIZE; ++x) {
            pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(SIZE) +
                   static_cast<std::size_t>(x)] = isDrawn(x, y) ? ON : OFF;
        }
    }

    D3D11_TEXTURE2D_DESC description{};
    description.Width = static_cast<UINT>(SIZE);
    description.Height = static_cast<UINT>(SIZE);
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = pixels.data();
    data.SysMemPitch = static_cast<UINT>(SIZE) * sizeof(std::uint32_t);

    if (FAILED(device->CreateTexture2D(&description, &data, &_texture))) {
        throw std::runtime_error("Echec de creation de la texture de l'icone d'enregistrement");
    }
    if (FAILED(device->CreateShaderResourceView(_texture.Get(), nullptr, &_view))) {
        throw std::runtime_error("Echec de creation de la vue de l'icone d'enregistrement");
    }
    GRAPHICS_LOG_TRACE("SaveIcon : icone d'enregistrement generee");
}

}  // namespace hmi
