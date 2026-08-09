#include "HMI/Interface/ActionCatalog.h"

#include <stdexcept>

namespace hmi {

const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& editorActionCatalog() {
    static const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT> catalog{{
        // Outils (ordre de la palette/du panneau Outils historique, EX-EDIT-014) : groupe
        // exclusif. Seul TextureAssign porte un raccourci -- la "touche dédiée" remappable
        // (EditorKeyBindings::TextureAssignTool), déjà gérée par hmi::GameViewport ; les cinq
        // autres n'ont pas de raccourci clavier aujourd'hui, et ce lot n'en introduit pas.
        {IconId::ToolPaint, "tool.brush", "", true, EditorActionGroup::LevelTools},
        {IconId::ToolRectangle, "tool.rectangle", "", true, EditorActionGroup::LevelTools},
        {IconId::ToolSelection, "tool.selection", "", true, EditorActionGroup::LevelTools},
        {IconId::ToolLink, "tool.link", "", true, EditorActionGroup::LevelTools},
        {IconId::ToolTextureAssign, "tool.texture_assign", "", true, EditorActionGroup::LevelTools},
        {IconId::ToolDecor, "tool.decor", "", true, EditorActionGroup::LevelTools},
        // Commandes principales : aucun groupe, aucune n'est cochable.
        {IconId::Save, "action.save", "Ctrl+S", false, EditorActionGroup::None},
        {IconId::Playtest, "action.playtest", "P", false, EditorActionGroup::None},
        {IconId::Undo, "action.undo", "Ctrl+Z", false, EditorActionGroup::None},
        {IconId::Redo, "action.redo", "Ctrl+Y", false, EditorActionGroup::None},
        {IconId::ToggleGrid, "action.toggle_grid", "F10", false, EditorActionGroup::None},
        {IconId::ResetCamera, "action.reset_camera", "0", false, EditorActionGroup::None},
        {IconId::ToggleRenderMode, "action.toggle_render_mode", "F8", false, EditorActionGroup::None},
    }};
    return catalog;
}

const EditorActionSpec& editorActionSpec(IconId id) {
    for (const EditorActionSpec& spec : editorActionCatalog()) {
        if (spec.id == id) {
            return spec;
        }
    }
    throw std::out_of_range("editorActionSpec: identifiant d'action inconnu");
}

std::optional<EditorTool> editorActionTool(IconId id) {
    switch (id) {
        case IconId::ToolPaint:
            return EditorTool::Paint;
        case IconId::ToolRectangle:
            return EditorTool::Rectangle;
        case IconId::ToolSelection:
            return EditorTool::Selection;
        case IconId::ToolLink:
            return EditorTool::Link;
        case IconId::ToolTextureAssign:
            return EditorTool::TextureAssign;
        case IconId::ToolDecor:
            return EditorTool::Decor;
        default:
            return std::nullopt;
    }
}

IconId editorActionForTool(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return IconId::ToolPaint;
        case EditorTool::Rectangle:
            return IconId::ToolRectangle;
        case EditorTool::Selection:
            return IconId::ToolSelection;
        case EditorTool::Link:
            return IconId::ToolLink;
        case EditorTool::TextureAssign:
            return IconId::ToolTextureAssign;
        case EditorTool::Decor:
            return IconId::ToolDecor;
    }
    return IconId::ToolPaint;
}

}  // namespace hmi
