#include "HMI/Graphics/FlagIcons.h"

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

namespace {
/// Ordre des drapeaux dans la texture (indice de colonne).
constexpr int FLAG_FRANCE = 0;
constexpr int FLAG_UNITED_KINGDOM = 1;
constexpr int FLAG_COUNT = 2;

/// Assemble une couleur RVBA (octets) en pixel `R8G8B8A8_UNORM` (ordre mémoire R,G,B,A).
[[nodiscard]] std::uint32_t pack(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
                                 std::uint8_t alpha) noexcept {
    return static_cast<std::uint32_t>(red) | (static_cast<std::uint32_t>(green) << 8) |
           (static_cast<std::uint32_t>(blue) << 16) | (static_cast<std::uint32_t>(alpha) << 24);
}

// Couleurs des drapeaux.
const std::uint32_t FRENCH_BLUE = pack(0, 85, 164, 255);
const std::uint32_t FRENCH_RED = pack(239, 65, 53, 255);
const std::uint32_t WHITE = pack(255, 255, 255, 255);
const std::uint32_t UK_BLUE = pack(1, 33, 105, 255);
const std::uint32_t UK_RED = pack(200, 16, 46, 255);

/// @return La couleur du pixel (@p x, @p y) du drapeau **français** (trois bandes verticales).
[[nodiscard]] std::uint32_t franceColor(int x) noexcept {
    const int band = x / (FlagIcons::FLAG_WIDTH / 3);
    if (band <= 0) {
        return FRENCH_BLUE;
    }
    return band == 1 ? WHITE : FRENCH_RED;
}

/// Distance d'un point à une droite définie par a*x + b*y + c = 0.
[[nodiscard]] float lineDistance(float a, float b, float c, float x, float y) noexcept {
    return std::abs(a * x + b * y + c) / std::sqrt(a * a + b * b);
}

/**
 * @brief Couleur du pixel (@p x, @p y) d'un drapeau du Royaume-Uni **approché**.
 *
 * Construction simplifiée pour une icône : fond bleu, sautoir blanc puis rouge (les
 * diagonales), enfin la croix blanche puis rouge (verticale + horizontale centrées). Le
 * contre-écartèlement du sautoir n'est pas reproduit (approximation lisible à petite taille).
 */
[[nodiscard]] std::uint32_t unitedKingdomColor(int x, int y) noexcept {
    const float width = static_cast<float>(FlagIcons::FLAG_WIDTH);
    const float height = static_cast<float>(FlagIcons::FLAG_HEIGHT);
    const float pointX = static_cast<float>(x) + 0.5f;
    const float pointY = static_cast<float>(y) + 0.5f;
    const float centerX = width * 0.5f;
    const float centerY = height * 0.5f;

    // Diagonales : (0,0)-(W,H) et (W,0)-(0,H).
    const float distanceMain = lineDistance(height, -width, 0.0f, pointX, pointY);
    const float distanceAnti = lineDistance(height, width, -width * height, pointX, pointY);
    const float distanceSaltire = std::fmin(distanceMain, distanceAnti);

    std::uint32_t color = UK_BLUE;
    if (distanceSaltire < 2.2f) {
        color = WHITE;
    }
    if (distanceSaltire < 0.9f) {
        color = UK_RED;
    }

    // Croix (blanche puis rouge) par-dessus le sautoir.
    const float distanceCross = std::fmin(std::abs(pointX - centerX), std::abs(pointY - centerY));
    if (distanceCross < 3.0f) {
        color = WHITE;
    }
    if (distanceCross < 1.4f) {
        color = UK_RED;
    }
    return color;
}
}  // namespace

/**
 * @brief Génère la texture des drapeaux et crée la ressource Direct3D associée.
 * @param device Device Direct3D 11 (crée la texture et sa vue de ressource).
 */
FlagIcons::FlagIcons(ID3D11Device* device) {
    _width = FLAG_WIDTH * FLAG_COUNT;
    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(_width) *
                                      static_cast<std::size_t>(FLAG_HEIGHT));

    for (int y = 0; y < FLAG_HEIGHT; ++y) {
        for (int flag = 0; flag < FLAG_COUNT; ++flag) {
            for (int localX = 0; localX < FLAG_WIDTH; ++localX) {
                const std::uint32_t color = flag == FLAG_FRANCE
                                                ? franceColor(localX)
                                                : unitedKingdomColor(localX, y);
                const int x = flag * FLAG_WIDTH + localX;
                pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(_width) +
                       static_cast<std::size_t>(x)] = color;
            }
        }
    }

    D3D11_TEXTURE2D_DESC description{};
    description.Width = static_cast<UINT>(_width);
    description.Height = static_cast<UINT>(FLAG_HEIGHT);
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
        throw std::runtime_error("Echec de creation de la texture des drapeaux");
    }
    if (FAILED(device->CreateShaderResourceView(_texture.Get(), nullptr, &_view))) {
        throw std::runtime_error("Echec de creation de la vue de la texture des drapeaux");
    }
    GRAPHICS_LOG_TRACE("FlagIcons : drapeaux generes (France, Royaume-Uni)");
}

/// @brief Région du drapeau associé à une langue (drapeau français si langue inconnue).
core::AtlasRegion FlagIcons::region(std::string_view language) const {
    const int flag = language == "en" ? FLAG_UNITED_KINGDOM : FLAG_FRANCE;
    return core::AtlasRegion{flag * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT};
}

}  // namespace hmi
