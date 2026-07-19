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

}  // namespace

TilePalette::TilePalette() {
    float x = MARGIN;
    for (const core::TileType type : PALETTE_TYPES) {
        _entries.push_back(Entry{type, x, MARGIN, ENTRY_SIZE, ENTRY_SIZE});
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
