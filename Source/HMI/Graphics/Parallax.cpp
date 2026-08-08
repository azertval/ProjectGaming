#include "HMI/Graphics/Parallax.h"

#include <cmath>

namespace hmi {

// Facteur de defilement de la couche d'un decor (EX-DEC-006, voir en-tete).
float parallaxFactor(core::DecorLayer layer) noexcept {
    switch (layer) {
        case core::DecorLayer::Background:
            return PARALLAX_FACTOR_BACKGROUND;
        case core::DecorLayer::Decor:
            return PARALLAX_FACTOR_DECOR;
        case core::DecorLayer::Foreground:
            return PARALLAX_FACTOR_FOREGROUND;
    }
    return PARALLAX_FACTOR_DECOR;
}

// Position de rendu d'un decor decalee par sa parallaxe, relative au centre de la salle (voir
// en-tete) : centre + (position - centre) * facteur -- inchangee a facteur 1.0.
core::Vector2 parallaxRenderPosition(core::Vector2 decorPosition, float factor,
                                     const core::Rect& cameraBounds) noexcept {
    const core::Vector2 center = cameraBounds.position + cameraBounds.size * 0.5f;
    return center + (decorPosition - center) * factor;
}

// Arrondit une position monde au pixel ecran le plus proche (voir en-tete).
core::Vector2 roundToScreenPixel(core::Vector2 worldPosition, float pixelsPerWorldUnit) noexcept {
    if (pixelsPerWorldUnit <= 0.0f) {
        return worldPosition;
    }
    return core::Vector2{std::round(worldPosition.x * pixelsPerWorldUnit) / pixelsPerWorldUnit,
                         std::round(worldPosition.y * pixelsPerWorldUnit) / pixelsPerWorldUnit};
}

}  // namespace hmi
