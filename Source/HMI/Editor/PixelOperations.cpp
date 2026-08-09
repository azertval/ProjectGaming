#include "HMI/Editor/PixelOperations.h"

#include <cstdlib>
#include <utility>
#include <vector>

namespace hmi {

namespace {

// Index a plat d'un pixel (x, y) dans un tampon de largeur width -- appelant deja borne-verifie.
std::size_t pixelIndex(int width, int x, int y) noexcept {
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
          static_cast<std::size_t>(x);
}

// Etend une region pour qu'elle couvre aussi (x, y).
PixelRegion expand(PixelRegion region, int x, int y) noexcept {
    if (region.empty()) {
        return PixelRegion{x, y, x, y};
    }
    return PixelRegion{
        region.minX < x ? region.minX : x,
        region.minY < y ? region.minY : y,
        region.maxX > x ? region.maxX : x,
        region.maxY > y ? region.maxY : y,
    };
}

}  // namespace

PixelRegion setPixel(DecodedImage& image, int x, int y, std::uint32_t color) {
    if (!pixelInBounds(image.width, image.height, x, y)) {
        return {};
    }
    image.pixels[pixelIndex(image.width, x, y)] = color;
    return PixelRegion{x, y, x, y};
}

PixelRegion erasePixel(DecodedImage& image, int x, int y) {
    if (!pixelInBounds(image.width, image.height, x, y)) {
        return {};
    }
    // Alpha nul, RVB conserve : re-colorer un pixel efface ne fait jamais resurgir une teinte
    // fantome, et effacer par-dessus une gomme reste sans effet visuel.
    image.pixels[pixelIndex(image.width, x, y)] &= 0x00FFFFFFu;
    return PixelRegion{x, y, x, y};
}

PixelRegion drawLine(DecodedImage& image, int x0, int y0, int x1, int y1, std::uint32_t color) {
    // Bresenham entier : aucune division, aucun pixel saute meme sur un segment tres oblique.
    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    PixelRegion region;
    int x = x0;
    int y = y0;
    for (;;) {
        if (!setPixel(image, x, y, color).empty()) {
            region = expand(region, x, y);
        }
        if (x == x1 && y == y1) {
            break;
        }
        const int doubledError = 2 * error;
        if (doubledError >= dy) {
            error += dy;
            x += sx;
        }
        if (doubledError <= dx) {
            error += dx;
            y += sy;
        }
    }
    return region;
}

PixelRegion floodFill(DecodedImage& image, int x, int y, std::uint32_t color) {
    if (!pixelInBounds(image.width, image.height, x, y)) {
        return {};
    }
    const std::size_t startIndex = pixelIndex(image.width, x, y);
    const std::uint32_t startColor = image.pixels[startIndex];
    if (startColor == color) {
        // Deja la couleur cible : aucun changement, et surtout aucune raison de parcourir
        // l'image -- une image entierement uniforme ne doit jamais boucler.
        return {};
    }

    PixelRegion region{x, y, x, y};
    image.pixels[startIndex] = color;

    // Pile explicite (jamais de recursion : une grande zone deborderait la pile d'appel).
    std::vector<std::pair<int, int>> pending;
    pending.emplace_back(x, y);
    while (!pending.empty()) {
        const auto [currentX, currentY] = pending.back();
        pending.pop_back();

        static constexpr int NEIGHBOR_OFFSETS[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (const auto& offset : NEIGHBOR_OFFSETS) {
            const int neighborX = currentX + offset[0];
            const int neighborY = currentY + offset[1];
            if (!pixelInBounds(image.width, image.height, neighborX, neighborY)) {
                continue;
            }
            const std::size_t neighborIndex = pixelIndex(image.width, neighborX, neighborY);
            if (image.pixels[neighborIndex] != startColor) {
                continue;  // deja repeint (marquage a la mise en pile) ou hors zone connexe.
            }
            image.pixels[neighborIndex] = color;
            region = expand(region, neighborX, neighborY);
            pending.emplace_back(neighborX, neighborY);
        }
    }
    return region;
}

std::optional<std::uint32_t> pickColor(const DecodedImage& image, int x, int y) noexcept {
    if (!pixelInBounds(image.width, image.height, x, y)) {
        return std::nullopt;
    }
    return image.pixels[pixelIndex(image.width, x, y)];
}

std::vector<std::uint32_t> readRegion(const DecodedImage& image, const PixelRegion& region) {
    if (region.empty()) {
        return {};
    }
    std::vector<std::uint32_t> data;
    data.reserve(static_cast<std::size_t>(region.width()) *
                static_cast<std::size_t>(region.height()));
    for (int yy = region.minY; yy <= region.maxY; ++yy) {
        for (int xx = region.minX; xx <= region.maxX; ++xx) {
            data.push_back(image.pixels[pixelIndex(image.width, xx, yy)]);
        }
    }
    return data;
}

void writeRegion(DecodedImage& image, const PixelRegion& region,
                 const std::vector<std::uint32_t>& data) {
    if (region.empty()) {
        return;
    }
    std::size_t index = 0;
    for (int yy = region.minY; yy <= region.maxY; ++yy) {
        for (int xx = region.minX; xx <= region.maxX; ++xx) {
            image.pixels[pixelIndex(image.width, xx, yy)] = data[index];
            ++index;
        }
    }
}

}  // namespace hmi
