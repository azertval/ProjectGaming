#include "HMI/Editor/ToolBar.h"

namespace hmi {

namespace {

constexpr float ENTRY_SIZE = 28.0f;
constexpr float ENTRY_GAP = 6.0f;
constexpr float MARGIN_X = 8.0f;
// Sous la palette de tuiles (TilePalette : y = 8, hauteur 40, donc bas a 48), avec un petit
// interstice.
constexpr float TOP_Y = 56.0f;

// Outils proposes, dans leur ordre d'affichage (et l'ordre de defilement de Tab).
constexpr EditorTool TOOLS[] = {EditorTool::Paint, EditorTool::Rectangle, EditorTool::Selection};

}  // namespace

ToolBar::ToolBar() {
    float x = MARGIN_X;
    for (const EditorTool tool : TOOLS) {
        _entries.push_back(Entry{tool, x, TOP_Y, ENTRY_SIZE, ENTRY_SIZE});
        x += ENTRY_SIZE + ENTRY_GAP;
    }
}

bool ToolBar::handleClick(float x, float y) {
    for (const Entry& entry : _entries) {
        if (x >= entry.x && x < entry.x + entry.width && y >= entry.y &&
            y < entry.y + entry.height) {
            _selected = entry.tool;
            return true;
        }
    }
    return false;
}

void ToolBar::selectNext() noexcept {
    switch (_selected) {
        case EditorTool::Paint:
            _selected = EditorTool::Rectangle;
            return;
        case EditorTool::Rectangle:
            _selected = EditorTool::Selection;
            return;
        case EditorTool::Selection:
            _selected = EditorTool::Paint;
            return;
    }
}

}  // namespace hmi
