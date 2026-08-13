#include "HMI/Graphics/TileVisuals.h"

#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

// Region d'atlas (couleur) associee a chaque type de tuile, pour un rendu distinct.
core::AtlasRegion regionForTile(core::TileType type) {
    switch (type) {
        case core::TileType::Solid:
            return TextureAtlas::tile(0, 2);  // gris
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
            return TextureAtlas::tile(0, 0);  // rouge
        case core::TileType::Entry:
            return TextureAtlas::tile(1, 0);  // vert
        case core::TileType::Exit:
            return TextureAtlas::tile(2, 0);  // bleu
        case core::TileType::Switch:
            return TextureAtlas::tile(3, 0);  // jaune
        case core::TileType::PressurePlate:
            return TextureAtlas::tile(
                1, 1);  // cyan (libere par LOT-17 : ancien placeholder du personnage)
        case core::TileType::Door:
            return TextureAtlas::tile(2, 1);  // orange
        case core::TileType::Key:
            return TextureAtlas::tile(4, 0);  // or (EX-GP-023, LOT-63)
        case core::TileType::LockedDoor:
            return TextureAtlas::tile(3, 4);  // brun fonce (EX-GP-023, LOT-63)
        case core::TileType::MovingPlatform:
            return TextureAtlas::tile(5, 0);  // azur (EX-GP-026, LOT-63 : grille agrandie a 6x6)
        case core::TileType::Block:
            return TextureAtlas::tile(3, 1);  // violet
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
            return TextureAtlas::tile(position.column, position.row);
        }
        case core::TileType::BlockHalf:
            return TextureAtlas::tile(1,
                                      3);  // gris foncé (variante teintée du bloc plein, EX-GP-005)
        case core::TileType::BlockQuarter:
            return TextureAtlas::tile(
                0, 3);  // gris clair (plus le bloc est petit, plus la teinte s'éclaircit)
        case core::TileType::Empty:
            break;
    }
    return TextureAtlas::tile(0, 0);
}

// Position dans la grille de tuiles reservee a un type de tuile a profil suivable — voir la
// documentation de l'en-tete (seule source de verite pour ces coordonnees).
std::optional<AtlasGridPosition> slopeTileGridPosition(core::TileType type) {
    switch (type) {
        case core::TileType::SlopeUpRight:
            return AtlasGridPosition{.column = 1, .row = 2};
        case core::TileType::SlopeUpLeft:
            return AtlasGridPosition{.column = 2, .row = 2};
        case core::TileType::RoundedUpRight:
            return AtlasGridPosition{.column = 3, .row = 2};
        case core::TileType::RoundedUpLeft:
            return AtlasGridPosition{.column = 0, .row = 1};
        case core::TileType::SlopeDownRight:
            return AtlasGridPosition{.column = 2, .row = 3};
        case core::TileType::SlopeDownLeft:
            return AtlasGridPosition{.column = 3, .row = 3};
        case core::TileType::RoundedDownRight:
            return AtlasGridPosition{.column = 0, .row = 4};
        case core::TileType::RoundedDownLeft:
            return AtlasGridPosition{.column = 1, .row = 4};
        case core::TileType::ConcaveUpRight:
            return AtlasGridPosition{.column = 4, .row = 1};
        case core::TileType::ConcaveUpLeft:
            return AtlasGridPosition{.column = 4, .row = 2};
        case core::TileType::ConcaveDownRight:
            return AtlasGridPosition{.column = 4, .row = 3};
        case core::TileType::ConcaveDownLeft:
            // (4, 4) est réservée au damier de transparence (TextureAtlas::transparentTileIndex,
            // vérifiée AVANT le masque de forme) — l'utiliser ici afficherait le damier au lieu de
            // la silhouette concave. (2, 4) est la seule des « sept cases libres » de l'épic qui
            // était en réalité disponible (l'épic comptait par erreur la case réservée parmi les
            // sept, qui n'en laissait donc que six réellement libres).
            return AtlasGridPosition{.column = 2, .row = 4};
        default:
            return std::nullopt;
    }
}

// Cherche la surcharge de texture assignee a une case precise (EX-EDIT-043, LOT-45).
std::optional<std::string> textureOverrideAt(
    const std::vector<core::TileTextureOverride>& overrides, core::GridPosition position) {
    for (const core::TileTextureOverride& override : overrides) {
        if (override.position == position) {
            return override.assetName;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
