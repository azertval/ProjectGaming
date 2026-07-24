#include "HMI/Editor/ToolBar.h"

#include "HMI/Editor/EditorLayout.h"

namespace hmi {

namespace {

// Outils proposes, dans leur ordre d'affichage (et l'ordre de defilement de Tab).
constexpr EditorTool TOOLS[] = {EditorTool::Paint, EditorTool::Rectangle, EditorTool::Selection};

}  // namespace

// Disposition en colonne dans le panneau lateral (LOT-15), sous la palette de tuiles. Position de
// depart provisoire (PALETTE_TOP) : EditorScreen appelle relayout() avant le premier rendu, la
// palette n'ayant plus de hauteur fixe depuis LOT-27.
ToolBar::ToolBar() { relayout(PALETTE_TOP); }

void ToolBar::relayout(float top) {
    _entries.clear();
    float y = top;
    for (const EditorTool tool : TOOLS) {
        _entries.push_back(Entry{tool, PANEL_MARGIN, y, PANEL_ICON_SIZE, PANEL_ICON_SIZE});
        y += PANEL_ROW_PITCH;
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
