#include "HMI/Editor/PanelFocus.h"

namespace hmi {

const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT>& panelFocusCatalog() {
    // Outil Décor volontairement absent : ses contrôles (placement et inspection) vivent dans le
    // panneau Décors (`DecorsPanel`, `LOT-57`), qui n'est tabifié avec aucun autre panneau -- le
    // mettre en avant n'aurait aucun effet visible (rien à faire passer devant un onglet voisin).
    static const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT> catalog{{
        {EditorTool::Link, PanelId::Links},
        {EditorTool::TextureAssign, PanelId::Textures},
    }};
    return catalog;
}

std::optional<PanelId> panelForTool(EditorTool tool) {
    for (const PanelFocusEntry& entry : panelFocusCatalog()) {
        if (entry.tool == tool) {
            return entry.panel;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
