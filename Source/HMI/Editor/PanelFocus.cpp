#include "HMI/Editor/PanelFocus.h"

namespace hmi {

const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT>& panelFocusCatalog() {
    // Outil Décor volontairement absent : ses contrôles d'auteur vivent dans le panneau Outils
    // (bloc démasqué par ToolPanel::setActiveTool), l'onglet Décors du panneau Textures n'étant
    // qu'un inspecteur d'un décor déjà posé, pas la surface de l'outil.
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
