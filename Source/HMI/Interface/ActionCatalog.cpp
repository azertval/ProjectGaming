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
        // Outils du canevas pixel art (LOT-54 TACHE-04) : groupe exclusif SEPARE des outils de
        // niveau ci-dessus -- les deux groupes ne s'excluent jamais entre eux. Aucun raccourci
        // clavier dedie aujourd'hui (choix par la barre d'outils du canevas uniquement).
        {IconId::PixelBrush, "pixel_tool.brush", "", true, EditorActionGroup::PixelTools},
        {IconId::PixelEraser, "pixel_tool.eraser", "", true, EditorActionGroup::PixelTools},
        {IconId::PixelFill, "pixel_tool.fill", "", true, EditorActionGroup::PixelTools},
        {IconId::PixelEyedropper, "pixel_tool.eyedropper", "", true, EditorActionGroup::PixelTools},
        // Commandes principales : aucun groupe, aucune n'est cochable.
        {IconId::Save, "action.save", "Ctrl+S", false, EditorActionGroup::None},
        {IconId::Playtest, "action.playtest", "P", false, EditorActionGroup::None},
        {IconId::Undo, "action.undo", "Ctrl+Z", false, EditorActionGroup::None},
        {IconId::Redo, "action.redo", "Ctrl+Y", false, EditorActionGroup::None},
        {IconId::ToggleGrid, "action.toggle_grid", "F10", false, EditorActionGroup::None},
        {IconId::ResetCamera, "action.reset_camera", "0", false, EditorActionGroup::None},
        {IconId::ToggleRenderMode, "action.toggle_render_mode", "F8", false, EditorActionGroup::None},
        // Déduplication des commandes (LOT-57 TACHE-04) : branchées sur les actions d'éditeur
        // remappables jusqu'ici définies et jamais lues (`EditorKeyBindings`). Valeurs par défaut
        // alignées sur `EditorKeyBindings::defaultKey` ; la valeur effective vient toujours
        // d'`EditorActions::applyShortcuts`, jamais de ce littéral seul (cf. son commentaire).
        {IconId::Copy, "action.copy", "Ctrl+C", false, EditorActionGroup::None},
        {IconId::Paste, "action.paste", "Ctrl+V", false, EditorActionGroup::None},
        {IconId::Rename, "action.rename", "F2", false, EditorActionGroup::None},
        {IconId::ShortcutsOverview, "action.shortcuts_overview", "F1", false,
         EditorActionGroup::None},
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

std::optional<PixelTool> editorActionPixelTool(IconId id) {
    switch (id) {
        case IconId::PixelBrush:
            return PixelTool::Brush;
        case IconId::PixelEraser:
            return PixelTool::Eraser;
        case IconId::PixelFill:
            return PixelTool::Fill;
        case IconId::PixelEyedropper:
            return PixelTool::Eyedropper;
        default:
            return std::nullopt;
    }
}

IconId editorActionForPixelTool(PixelTool tool) {
    switch (tool) {
        case PixelTool::Brush:
            return IconId::PixelBrush;
        case PixelTool::Eraser:
            return IconId::PixelEraser;
        case PixelTool::Fill:
            return IconId::PixelFill;
        case PixelTool::Eyedropper:
            return IconId::PixelEyedropper;
    }
    return IconId::PixelBrush;
}

const std::array<KeyBindingIconEntry, KEY_BINDING_ICON_COUNT>& keyBindingIconCatalog() {
    static const std::array<KeyBindingIconEntry, KEY_BINDING_ICON_COUNT> catalog{{
        {EditorAction::Save, IconId::Save},
        {EditorAction::Undo, IconId::Undo},
        {EditorAction::Redo, IconId::Redo},
        {EditorAction::Copy, IconId::Copy},
        {EditorAction::Paste, IconId::Paste},
        {EditorAction::Playtest, IconId::Playtest},
        {EditorAction::ToggleGrid, IconId::ToggleGrid},
        {EditorAction::ToggleHelp, IconId::ShortcutsOverview},
        {EditorAction::Rename, IconId::Rename},
    }};
    return catalog;
}

IconId iconForKeyBindingAction(EditorAction action) {
    for (const KeyBindingIconEntry& entry : keyBindingIconCatalog()) {
        if (entry.action == action) {
            return entry.id;
        }
    }
    return IconId::Save;  // inatteignable pour les 9 actions couvertes ; TextureAssignTool exclue.
}

std::optional<EditorAction> keyBindingActionForIcon(IconId id) {
    for (const KeyBindingIconEntry& entry : keyBindingIconCatalog()) {
        if (entry.id == id) {
            return entry.action;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
