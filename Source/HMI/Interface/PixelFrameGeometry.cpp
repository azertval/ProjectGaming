#include "HMI/Interface/PixelFrameGeometry.h"

#include <algorithm>

namespace hmi {

std::vector<PixelFrameQuad> pixelFrameQuads(int width, int height, int scale) {
    if (width <= 0 || height <= 0) {
        return {};
    }
    // Epaisseur commune au contour et au biseau : un pixel de maquette. Bornee au quart de la plus
    // petite dimension -- au-dela, les deux biseaux opposes se croiseraient et la geometrie
    // sortirait du cadre au lieu de simplement s'y serrer.
    const int thickness = std::min({std::max(scale, 1), width / 4, height / 4});
    if (thickness <= 0) {
        // Cadre plus petit que ses propres bordures : un aplat, jamais un debordement.
        return {PixelFrameQuad{.role = PixelFrameRole::Fill,
                               .x = 0,
                               .y = 0,
                               .width = width,
                               .height = height}};
    }

    const int t = thickness;
    const int innerWidth = width - (2 * t);
    const int innerHeight = height - (2 * t);

    // Ordre de dessin : l'aplat d'abord, les biseaux par-dessus, le contour en dernier.
    return {
        {.role = PixelFrameRole::Fill, .x = t, .y = t, .width = innerWidth, .height = innerHeight},
        // Biseau clair : haut et gauche. La lumiere vient d'en haut a gauche, comme dans tout
        // l'habillage -- l'inverser retournerait les cadres en creux.
        {.role = PixelFrameRole::BevelLight, .x = t, .y = t, .width = innerWidth, .height = t},
        {.role = PixelFrameRole::BevelLight, .x = t, .y = t, .width = t, .height = innerHeight},
        // Biseau sombre : bas et droite.
        {.role = PixelFrameRole::BevelDark,
         .x = t,
         .y = height - (2 * t),
         .width = innerWidth,
         .height = t},
        {.role = PixelFrameRole::BevelDark,
         .x = width - (2 * t),
         .y = t,
         .width = t,
         .height = innerHeight},
        // Contour : quatre barres, JAMAIS un rectangle. Les quatre coins restent vides, et c'est
        // cette entaille qui fait lire le cadre comme du pixel art plutot que comme une bordure.
        {.role = PixelFrameRole::Outline, .x = t, .y = 0, .width = innerWidth, .height = t},
        {.role = PixelFrameRole::Outline,
         .x = t,
         .y = height - t,
         .width = innerWidth,
         .height = t},
        {.role = PixelFrameRole::Outline, .x = 0, .y = t, .width = t, .height = innerHeight},
        {.role = PixelFrameRole::Outline,
         .x = width - t,
         .y = t,
         .width = t,
         .height = innerHeight},
    };
}

std::vector<PixelFrameQuad> pixelCaretQuads(int size) {
    // Le dessin fait huit rangees de haut : le curseur est donc tracé par pas de size/8, et un
    // curseur plus petit que 8 px ne peut pas les porter sans en ecraser certaines.
    constexpr int ROWS = 8;
    const int step = size / ROWS;
    if (step <= 0) {
        return {};
    }
    // Largeur de chaque rangee, en pas : le triangle croit puis decroit symetriquement.
    constexpr int ROW_WIDTHS[ROWS] = {1, 2, 3, 4, 4, 3, 2, 1};

    std::vector<PixelFrameQuad> quads;
    quads.reserve(ROWS);
    for (int row = 0; row < ROWS; ++row) {
        quads.push_back(PixelFrameQuad{.role = PixelFrameRole::Fill,
                                       .x = 0,
                                       .y = row * step,
                                       .width = ROW_WIDTHS[row] * step,
                                       .height = step});
    }
    return quads;
}

}  // namespace hmi
