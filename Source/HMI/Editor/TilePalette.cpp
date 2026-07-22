#include "HMI/Editor/TilePalette.h"

namespace hmi {

namespace {

constexpr float ENTRY_SIZE = 40.0f;
constexpr float ENTRY_GAP = 6.0f;
constexpr float MARGIN = 8.0f;

// Types éditables, dans leur ordre d'affichage (cf. en-tête : limité à ce que Core gère).
constexpr core::TileType PALETTE_TYPES[] = {
    core::TileType::Empty, core::TileType::Solid,  core::TileType::Danger, core::TileType::Entry,
    core::TileType::Exit,  core::TileType::Switch, core::TileType::Door,
};

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
        case core::TileType::Door:
            return "Porte";
    }
    return "";
}

}  // namespace

TilePalette::TilePalette() {
    float x = MARGIN;
    for (const core::TileType type : PALETTE_TYPES) {
        _entries.push_back(Entry{type, x, MARGIN, ENTRY_SIZE, ENTRY_SIZE, labelFor(type)});
        x += ENTRY_SIZE + ENTRY_GAP;
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
