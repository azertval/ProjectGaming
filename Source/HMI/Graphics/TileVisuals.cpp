#include "HMI/Graphics/TileVisuals.h"

#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

// Region d'atlas (couleur) associee a chaque type de tuile, pour un rendu distinct.
core::AtlasRegion regionForTile(core::TileType type, const TextureAtlas& atlas) {
    switch (type) {
        case core::TileType::Solid:
            return atlas.tile(0, 2);  // gris
        case core::TileType::Danger:
        case core::TileType::DangerUp:
        case core::TileType::DangerDown:
        case core::TileType::DangerLeft:
        case core::TileType::DangerRight:
        case core::TileType::DangerMover:
        case core::TileType::DangerSwitched:
        case core::TileType::DangerBlink:
            // Meme region que Danger classique (rouge) : la geometrie affichee (case pleine ou
            // bande directionnelle, core::dangerHitbox appliquee par l'appelant — LevelScene.cpp,
            // DraftRenderer.cpp) distingue deja les variantes directionnelles ; simplification
            // assumee pour Mobile/Commute/Clignotant (memes couleur/forme que Danger classique,
            // seul leur TYPE — visible dans la palette — les distingue a l'edition).
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
        case core::TileType::RoundedUpLeft:
        case core::TileType::SlopeDownRight:
        case core::TileType::SlopeDownLeft:
        case core::TileType::RoundedDownRight:
        case core::TileType::RoundedDownLeft:
        case core::TileType::ConcaveUpRight:
        case core::TileType::ConcaveUpLeft:
        case core::TileType::ConcaveDownRight:
        case core::TileType::ConcaveDownLeft: {
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
        case core::TileType::SlopeDownRight:
            return AtlasGridPosition{2, 3};
        case core::TileType::SlopeDownLeft:
            return AtlasGridPosition{3, 3};
        case core::TileType::RoundedDownRight:
            return AtlasGridPosition{0, 4};
        case core::TileType::RoundedDownLeft:
            return AtlasGridPosition{1, 4};
        case core::TileType::ConcaveUpRight:
            return AtlasGridPosition{4, 1};
        case core::TileType::ConcaveUpLeft:
            return AtlasGridPosition{4, 2};
        case core::TileType::ConcaveDownRight:
            return AtlasGridPosition{4, 3};
        case core::TileType::ConcaveDownLeft:
            // (4, 4) est réservée au damier de transparence (TextureAtlas::transparentTileIndex,
            // vérifiée AVANT le masque de forme) — l'utiliser ici afficherait le damier au lieu de
            // la silhouette concave. (2, 4) est la seule des « sept cases libres » de l'épic qui
            // était en réalité disponible (l'épic comptait par erreur la case réservée parmi les
            // sept, qui n'en laissait donc que six réellement libres).
            return AtlasGridPosition{2, 4};
        default:
            return std::nullopt;
    }
}

}  // namespace hmi
