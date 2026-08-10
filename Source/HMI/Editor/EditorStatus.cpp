#include "HMI/Editor/EditorStatus.h"

#include <cmath>
#include <cstdio>

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

// Cle de traduction du libelle court d'un outil de canevas pixel art (LOT-54 TACHE-04) -- deja
// utilisees par EditorActions (memes cles que les actions de la barre d'outils du canevas).
const char* pixelToolLabelKey(PixelTool tool) {
    switch (tool) {
        case PixelTool::Brush:
            return "pixel_tool.brush";
        case PixelTool::Eraser:
            return "pixel_tool.eraser";
        case PixelTool::Fill:
            return "pixel_tool.fill";
        case PixelTool::Eyedropper:
            return "pixel_tool.eyedropper";
        case PixelTool::Selection:
            return "pixel_tool.selection";
    }
    return "pixel_tool.brush";
}

// Cle de traduction de l'aide contextuelle d'un outil de canevas pixel art (LOT-54 TACHE-04).
const char* pixelToolHelpKey(PixelTool tool) {
    switch (tool) {
        case PixelTool::Brush:
            return "status.help_pixel_brush";
        case PixelTool::Eraser:
            return "status.help_pixel_eraser";
        case PixelTool::Fill:
            return "status.help_pixel_fill";
        case PixelTool::Eyedropper:
            return "status.help_pixel_eyedropper";
        case PixelTool::Selection:
            return "status.help_pixel_selection";
    }
    return "status.help_pixel_brush";
}

// Couleur R8G8B8A8_UNORM (ordre memoire R,G,B,A, meme convention que decodeImageFile/
// encodeImageFile, LOT-54 TACHE-01) -> chaine hexadecimale "#rrggbbaa", alpha compris (une
// pipette peut prelever une couleur partiellement transparente).
std::string formatColorHex(std::uint32_t color) {
    char buffer[10] = {};
    std::snprintf(buffer, sizeof(buffer), "#%02x%02x%02x%02x", static_cast<unsigned>(color & 0xFFu),
                  static_cast<unsigned>((color >> 8) & 0xFFu),
                  static_cast<unsigned>((color >> 16) & 0xFFu),
                  static_cast<unsigned>((color >> 24) & 0xFFu));
    return std::string(buffer);
}

}  // namespace

// Contenu de la barre d'etat de l'editeur (voir en-tete) : zones permanentes puis aide.
EditorStatusLines editorStatusLines(const EditorStatusContext& context,
                                    const Localization& localization) {
    EditorStatusLines lines;
    // niveau/asset, modifie, outil, case/pixel survole, zoom, couleur courante (atelier seulement).
    lines.permanent.assign(6, std::string{});

    if (context.pixelEdit) {
        const PixelEditStatusInfo& pixel = *context.pixelEdit;
        if (!pixel.assetName.empty()) {
            lines.permanent[0] = formatOne(localization.text("status.zone.asset"), pixel.assetName);
        }
        if (pixel.dirty) {
            lines.permanent[1] = localization.text("status.zone.dirty");
        }
        lines.permanent[2] = localization.text(pixelToolLabelKey(pixel.tool));
        if (pixel.hoveredPixel) {
            lines.permanent[3] = formatTwo(localization.text("status.zone.hover"),
                                           pixel.hoveredPixel->first, pixel.hoveredPixel->second);
        }
        lines.permanent[4] = formatOne(localization.text("status.zone.zoom"), pixel.zoom * 100);
        // Mode contraint (LOT-54 TACHE-07) : integre a la meme zone couleur plutot qu'une septieme
        // zone -- l'information n'a de sens qu'accolee a la couleur qu'elle affecte.
        const char* const colorKey =
            pixel.paletteConstrained ? "status.zone.color_constrained" : "status.zone.color";
        lines.permanent[5] =
            formatOne(localization.text(colorKey), formatColorHex(pixel.currentColor));
        lines.help = localization.text(pixelToolHelpKey(pixel.tool));
        return lines;
    }

    if (!context.level) {
        return lines;  // aucun contexte actif : zones et aide vides, jamais de libelle de repli.
    }
    const LevelStatusInfo& level = *context.level;

    lines.permanent[0] = formatOne(localization.text("status.zone.level"), level.name);
    if (level.dirty) {
        lines.permanent[1] = localization.text("status.zone.dirty");
    }
    lines.permanent[2] = localization.text(toolLabelKey(level.tool));
    if (level.hoveredCell) {
        lines.permanent[3] = formatTwo(localization.text("status.zone.hover"),
                                       level.hoveredCell->column, level.hoveredCell->row);
    }
    const int zoomPercent = static_cast<int>(std::lround(level.zoom * 100.0f));
    lines.permanent[4] = formatOne(localization.text("status.zone.zoom"), zoomPercent);
    // permanent[5] (couleur) reste vide : sans objet hors atelier pixel art.

    lines.help = localization.text(toolHelpKey(level.tool));
    return lines;
}

}  // namespace hmi
