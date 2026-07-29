#include "HMI/Graphics/SlopeMask.h"

#include <algorithm>
#include <optional>

#include "Core/Physics/SlopeGeometry.h"

namespace hmi {

namespace {

// Masque d'alpha d'un pixel R8G8B8A8_UNORM (octet de poids fort).
constexpr std::uint32_t ALPHA_MASK = 0xFF000000u;

}  // namespace

bool hasSilhouette(core::TileType type) noexcept {
    return std::find(std::begin(SILHOUETTE_TILE_TYPES), std::end(SILHOUETTE_TILE_TYPES), type) !=
           std::end(SILHOUETTE_TILE_TYPES);
}

bool isInsideSilhouette(core::TileType type, int localX, int localY, int size) noexcept {
    if (size <= 1) {
        return true;  // case degeneree : rien a decouper.
    }

    // Echantillonnage au BORD des pixels de coin (0 et size-1 atteignent exactement 0,0 et 1,0) et
    // non a leur centre. Voir la justification detaillee dans l'en-tete : sans cela, un arrondi
    // concave presente une encoche visible sur sa colonne de bord.
    const float lastIndex = static_cast<float>(size - 1);
    const float normalizedX = static_cast<float>(localX) / lastIndex;
    const float normalizedY = static_cast<float>(localY) / lastIndex;

    // Geometrie prise dans Core, jamais reimplementee ici : c'est la meme fonction que suit la
    // physique, donc l'affichage ne peut pas mentir sur la hitbox.
    const bool isCeiling = core::isCeilingSlope(type);
    const std::optional<float> surfaceHeight = isCeiling
                                                   ? core::ceilingSlopeHeight(type, normalizedX)
                                                   : core::slopeSurfaceHeight(type, normalizedX);
    if (!surfaceHeight) {
        return true;  // type sans silhouette : carre plein.
    }

    // Pente/arrondi de SOL : plein SOUS la surface suivie. De PLAFOND : plein AU-DESSUS de la
    // silhouette, miroir vertical de la variante de sol.
    return isCeiling ? normalizedY <= *surfaceHeight : normalizedY >= *surfaceHeight;
}

void applySilhouetteMask(core::TileType type, int width, int height,
                         std::vector<std::uint32_t>& pixels) {
    if (!hasSilhouette(type) || width != height || width <= 1) {
        return;
    }
    if (pixels.size() != static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {
        return;  // dimensions incoherentes : ne rien toucher plutot que d'ecrire hors bornes.
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (isInsideSilhouette(type, x, y, width)) {
                continue;
            }
            // Transparence franche : on efface l'alpha sans toucher aux composantes de couleur,
            // que le blend ignore une fois alpha nul (alpha NON premultiplie, cf. SpriteBatch).
            pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
                   static_cast<std::size_t>(x)] &= ~ALPHA_MASK;
        }
    }
}

}  // namespace hmi
