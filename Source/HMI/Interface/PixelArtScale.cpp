#include "HMI/Interface/PixelArtScale.h"

#include <algorithm>

namespace hmi {

int pixelArtScale(int windowLogicalHeight) noexcept {
    // Division ENTIERE, jamais un arrondi : arrondir ferait passer une fenetre de 700 px a
    // l'echelle 2, pour laquelle il manque 20 px de hauteur -- la derniere entree du menu
    // disparaitrait sous le bord.
    const int fitted = windowLogicalHeight / PIXEL_ART_BASE_HEIGHT;
    return std::clamp(fitted, 1, PIXEL_ART_MAX_SCALE);
}

}  // namespace hmi
