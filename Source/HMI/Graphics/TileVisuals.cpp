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
        case core::TileType::SlopeUpRight:
        case core::TileType::SlopeUpLeft:
        case core::TileType::RoundedUpRight:
        case core::TileType::RoundedUpLeft: {
            // Position partagée avec TextureAtlas (masque de forme triangulaire/courbe, rempli en
            // gris — même matériau qu'un `Solid` standard, cf. TextureAtlas::slopeShapePixel) :
            // voir slopeTileGridPosition, seule source de vérité pour ces coordonnées.
            const AtlasGridPosition position = *slopeTileGridPosition(type);
            return atlas.tile(position.column, position.row);
        }
        case core::TileType::BlockHalf:
            return atlas.tile(1, 3);  // gris foncé (variante teintée du bloc plein, EX-GP-005)
        case core::TileType::BlockQuarter:
            return atlas.tile(0, 3);  // gris clair (plus le bloc est petit, plus la teinte s'éclaircit)
        case core::TileType::Empty:
            break;
    }
    return atlas.tile(0, 0);
}

// Position dans la grille de tuiles reservee a un type de tuile a profil suivable — voir la
// documentation de l'en-tete (seule source de verite pour ces coordonnees).
std::optional<AtlasGridPosition> slopeTileGridPosition(core::TileType type) {
    switch (type) {
        case core::TileType::SlopeUpRight:
            return AtlasGridPosition{1, 2};
        case core::TileType::SlopeUpLeft:
            return AtlasGridPosition{2, 2};
        case core::TileType::RoundedUpRight:
            return AtlasGridPosition{3, 2};
        case core::TileType::RoundedUpLeft:
            return AtlasGridPosition{0, 1};
        default:
            return std::nullopt;
    }
}

}  // namespace hmi
