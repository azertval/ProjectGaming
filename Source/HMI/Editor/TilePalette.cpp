#include "HMI/Editor/TilePalette.h"

#include <iterator>

#include "HMI/Editor/EditorLayout.h"

namespace hmi {

namespace {

// Types éditables, dans leur ordre d'affichage (cf. en-tête : limité à ce que Core gère).
constexpr core::TileType PALETTE_TYPES[] = {
    core::TileType::Empty,           core::TileType::Solid,           core::TileType::Danger,
    core::TileType::Entry,           core::TileType::Exit,            core::TileType::Switch,
    core::TileType::PressurePlate,   core::TileType::Door,            core::TileType::Block,
    core::TileType::SlopeUpRight,    core::TileType::SlopeUpLeft,
    core::TileType::RoundedUpRight,  core::TileType::RoundedUpLeft,
    core::TileType::SlopeDownRight,  core::TileType::SlopeDownLeft,
    core::TileType::RoundedDownRight, core::TileType::RoundedDownLeft,
    core::TileType::BlockHalf,       core::TileType::BlockQuarter,
};
// Garde-fou : EditorLayout::PALETTE_TYPE_COUNT dimensionne le panneau (haut de la barre d'outils,
// EditorLayout.h) a partir de ce meme compte, sans pouvoir le deriver directement (constexpr
// inter-fichiers) — cette assertion empeche les deux de diverger silencieusement.
static_assert(std::size(PALETTE_TYPES) == PALETTE_TYPE_COUNT,
             "PALETTE_TYPE_COUNT (EditorLayout.h) doit suivre le nombre d'entrees de PALETTE_TYPES");

// Libellé court affiché sous chaque entrée (découvrabilité, EX-EDIT-015) — pas d'accent, cohérent
// avec le reste des libellés affichés en jeu par cet ecran (police bitmap, place limitée).
[[nodiscard]] std::string labelFor(core::TileType type) {
    switch (type) {
        case core::TileType::Empty:
            return "Vide";
        case core::TileType::Solid:
            return "Plein";
        case core::TileType::Danger:
            return "Danger";
        case core::TileType::Entry:
            return "Entree";
        case core::TileType::Exit:
            return "Sortie";
        case core::TileType::Switch:
            return "Interr.";
        case core::TileType::PressurePlate:
            return "Plaque";
        case core::TileType::Door:
            return "Porte";
        case core::TileType::Block:
            return "Bloc";
        case core::TileType::SlopeUpRight:
            return "Pente D";
        case core::TileType::SlopeUpLeft:
            return "Pente G";
        case core::TileType::RoundedUpRight:
            return "Arrondi D";
        case core::TileType::RoundedUpLeft:
            return "Arrondi G";
        case core::TileType::SlopeDownRight:
            return "Pente D Plaf";
        case core::TileType::SlopeDownLeft:
            return "Pente G Plaf";
        case core::TileType::RoundedDownRight:
            return "Arrondi D Plaf";
        case core::TileType::RoundedDownLeft:
            return "Arrondi G Plaf";
        case core::TileType::BlockHalf:
            return "Bloc 1/2";
        case core::TileType::BlockQuarter:
            return "Bloc 1/4";
    }
    return "";
}

}  // namespace

// Disposition en colonne, dans le panneau lateral (LOT-15) : une ligne par type, icone + libelle
// dessines cote a cote par EditorScreen (geometrie ici, rendu delegue comme documente en en-tete).
TilePalette::TilePalette() {
    float y = PALETTE_TOP;
    for (const core::TileType type : PALETTE_TYPES) {
        _entries.push_back(
            Entry{type, PANEL_MARGIN, y, PANEL_ICON_SIZE, PANEL_ICON_SIZE, labelFor(type)});
        y += PANEL_ROW_PITCH;
    }
}

bool TilePalette::handleClick(float x, float y) {
    for (const Entry& entry : _entries) {
        if (x >= entry.x && x < entry.x + entry.width && y >= entry.y &&
            y < entry.y + entry.height) {
            _selected = entry.type;
            return true;
        }
    }
    return false;
}

}  // namespace hmi
