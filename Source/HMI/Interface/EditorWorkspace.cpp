#include "HMI/Interface/EditorWorkspace.h"

namespace hmi {

WorkspaceDressing dressingForWorkspace(EditorWorkspace workspace) noexcept {
    switch (workspace) {
        case EditorWorkspace::Level:
            return WorkspaceDressing{.levelToolBarVisible = true,
                                     .pixelToolBarVisible = false,
                                     .workshopMenuVisible = false};
        case EditorWorkspace::PixelArt:
            return WorkspaceDressing{.levelToolBarVisible = false,
                                     .pixelToolBarVisible = true,
                                     .workshopMenuVisible = true};
    }
    return WorkspaceDressing{};
}

EditorWorkspace workspaceForPanel(PanelId panel) noexcept {
    switch (panel) {
        case PanelId::Palette:
        case PanelId::Decors:
        case PanelId::Levels:
        case PanelId::Links:
        case PanelId::Properties:
        case PanelId::Textures:
            return EditorWorkspace::Level;
        case PanelId::PixelCanvas:
        case PanelId::PixelHistory:
        case PanelId::PixelPalette:
            return EditorWorkspace::PixelArt;
    }
    // Inatteignable : le switch couvre l'enumeration, et un test verifie qu'il la couvre encore
    // apres ajout d'un panneau. Le repli choisit l'espace d'edition, celui ou l'auteur travaille.
    return EditorWorkspace::Level;
}

EditorWorkspace workspaceForTool(EditorTool /*tool*/) noexcept {
    // Tous les outils de EditorTool sont des outils de NIVEAU : le groupe est disjoint de PixelTool
    // par construction (deux QActionGroup distincts, ActionCatalog.h). La fonction existe pour que
    // MainWindow suive une table plutot qu'une condition ecrite en dur -- exactement le defaut qui
    // avait laisse l'outil « Parcours » debranche au LOT-67.
    return EditorWorkspace::Level;
}

EditorWorkspace workspaceForPixelTool(PixelTool /*tool*/) noexcept {
    return EditorWorkspace::PixelArt;
}

}  // namespace hmi
