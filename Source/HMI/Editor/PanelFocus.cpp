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

const std::array<PixelPanelFocusEntry, PIXEL_PANEL_FOCUS_CATALOG_COUNT>& pixelPanelFocusCatalog() {
    // Les quatre outils du canevas mettent tous en avant le meme panneau (PixelCanvas) : des qu'un
    // outil de canevas est actif, on veut voir le canevas -- a la difference des outils de niveau,
    // aucun outil de canevas n'a de panneau "annexe" dedie.
    static const std::array<PixelPanelFocusEntry, PIXEL_PANEL_FOCUS_CATALOG_COUNT> catalog{{
        {PixelTool::Brush, PanelId::PixelCanvas},
        {PixelTool::Eraser, PanelId::PixelCanvas},
        {PixelTool::Fill, PanelId::PixelCanvas},
        {PixelTool::Eyedropper, PanelId::PixelCanvas},
    }};
    return catalog;
}

std::optional<PanelId> panelForPixelTool(PixelTool tool) {
    for (const PixelPanelFocusEntry& entry : pixelPanelFocusCatalog()) {
        if (entry.tool == tool) {
            return entry.panel;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
