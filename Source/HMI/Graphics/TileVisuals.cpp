#include "HMI/Graphics/TileVisuals.h"

#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

// Region d'atlas (couleur) associee a chaque type de tuile, pour un rendu distinct.
core::AtlasRegion regionForTile(core::TileType type, const TextureAtlas& atlas) {
    switch (type) {
        case core::TileType::Solid:
            return atlas.tile(0, 2);  // gris
        case core::TileType::Danger:
            return atlas.tile(0, 0);  // rouge
        case core::TileType::Entry:
            return atlas.tile(1, 0);  // vert
        case core::TileType::Exit:
            return atlas.tile(2, 0);  // bleu
        case core::TileType::Switch:
            return atlas.tile(3, 0);  // jaune
        case core::TileType::PressurePlate:
            return atlas.tile(1, 1);  // cyan (libere par LOT-17 : ancien placeholder du personnage)
        case core::TileType::Door:
            return atlas.tile(2, 1);  // orange
        case core::TileType::Block:
            return atlas.tile(3, 1);  // violet
        case core::TileType::Empty:
            break;
    }
    return atlas.tile(0, 0);
}

}  // namespace hmi
