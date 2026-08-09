#include "HMI/Editor/EditorStatus.h"

#include <cmath>

#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Remplace la premiere occurrence de "%1" (et "%2" pour les gabarits a deux valeurs) -- pas de
// dependance Qt, meme discipline que hmi::gameHudLines (LOT-52).
std::string replacePlaceholder(std::string text, const std::string& placeholder,
                               const std::string& value) {
    const std::size_t position = text.find(placeholder);
    if (position == std::string::npos) {
        return text;
    }
    text.replace(position, placeholder.size(), value);
    return text;
}

std::string formatOne(const std::string& templateText, const std::string& value) {
    return replacePlaceholder(templateText, "%1", value);
}

std::string formatOne(const std::string& templateText, int value) {
    return formatOne(templateText, std::to_string(value));
}

std::string formatTwo(const std::string& templateText, int first, int second) {
    return replacePlaceholder(formatOne(templateText, first), "%2", std::to_string(second));
}

// Cle de traduction du libelle court d'un outil (deja utilisees par EditorActions).
const char* toolLabelKey(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return "tool.brush";
        case EditorTool::Rectangle:
            return "tool.rectangle";
        case EditorTool::Selection:
            return "tool.selection";
        case EditorTool::Link:
            return "tool.link";
        case EditorTool::TextureAssign:
            return "tool.texture_assign";
        case EditorTool::Decor:
            return "tool.decor";
    }
    return "tool.brush";
}

// Cle de traduction de l'aide contextuelle d'un outil (LOT-57 TACHE-01, retire status.edit_help).
const char* toolHelpKey(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return "status.help_paint";
        case EditorTool::Rectangle:
            return "status.help_rectangle";
        case EditorTool::Selection:
            return "status.help_selection";
        case EditorTool::Link:
            return "status.help_link";
        case EditorTool::TextureAssign:
            return "status.help_texture_assign";
        case EditorTool::Decor:
            return "status.help_decor";
    }
    return "status.help_paint";
}

}  // namespace

// Contenu de la barre d'etat de l'editeur (voir en-tete) : zones permanentes puis aide.
EditorStatusLines editorStatusLines(const EditorStatusContext& context,
                                    const Localization& localization) {
    EditorStatusLines lines;
    lines.permanent.assign(5, std::string{});  // niveau, modifie, outil, case survolee, zoom.

    if (!context.level) {
        return lines;  // aucun niveau ouvert : zones et aide vides, jamais de libelle de repli.
    }
    const LevelStatusInfo& level = *context.level;

    lines.permanent[0] = formatOne(localization.text("status.zone.level"), level.name);
    if (level.dirty) {
        lines.permanent[1] = localization.text("status.zone.dirty");
    }
    lines.permanent[2] = localization.text(toolLabelKey(level.tool));
    if (level.hoveredCell) {
        lines.permanent[3] =
            formatTwo(localization.text("status.zone.hover"), level.hoveredCell->column,
                     level.hoveredCell->row);
    }
    const int zoomPercent = static_cast<int>(std::lround(level.zoom * 100.0f));
    lines.permanent[4] = formatOne(localization.text("status.zone.zoom"), zoomPercent);

    lines.help = localization.text(toolHelpKey(level.tool));
    return lines;
}

}  // namespace hmi
