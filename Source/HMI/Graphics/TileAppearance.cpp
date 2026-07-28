#include "HMI/Graphics/TileAppearance.h"

#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

// La geometrie composee doit etre identique dans les deux modes : le rendu bascule pour comparer
// l'habillage au physique, pas pour comparer deux tailles de tuiles. Cela n'est vrai que si le
// damier de repli occupe exactement une case.
static_assert(MISSING_TEXTURE_SIZE == TextureAtlas::TILE_SIZE,
              "Le damier de repli doit faire exactement une case, sinon basculer de mode change "
              "la taille des primitives composees.");

// Resout l'apparence d'une entite affichee selon le mode de rendu courant (point d'appel unique).
TileAppearance resolveTileAppearance(RenderMode mode,
                                     const core::AtlasRegion& physicalRegion) noexcept {
    if (mode == RenderMode::Physique) {
        // Mode de reference : la region deja resolue par hmi::regionForTile a la construction de
        // la scene. Rien n'est recalcule ici -- c'est ce qui garantit l'absence de regression.
        return TileAppearance{AppearanceSource::Atlas, physicalRegion};
    }
    // Mode Texture : le damier en entier, aucun skin n'existant avant LOT-42. C'est ici que la
    // priorite « surcharge par case > skin de tuile > damier » s'inserera.
    return TileAppearance{AppearanceSource::MissingTexture,
                          core::AtlasRegion{0, 0, MISSING_TEXTURE_SIZE, MISSING_TEXTURE_SIZE}};
}

}  // namespace hmi
