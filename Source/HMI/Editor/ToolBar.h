#pragma once

#include <vector>

#include "HMI/Editor/EditorTool.h"

/**
 * @file HMI/Editor/ToolBar.h
 * @brief Sélection de l'outil actif de l'éditeur, à la souris (LOT-15, `EX-EDIT-015`).
 */

namespace hmi {

/**
 * @brief Bande d'icônes cliquables sélectionnant l'`EditorTool` actif.
 *
 * Logique **pure** : ne connaît que la géométrie écran (rectangles en pixels) et l'état de
 * sélection, sans dépendance de rendu — testable sans GPU (`EX-NFR-010`), même esprit que
 * `TilePalette`. Le dessin (icônes, libellés, surbrillance de la sélection) est délégué à
 * `EditorScreen`, qui lit `entries()`/`selected()`. `Tab` (dans `EditorScreen`) fait défiler les
 * outils via `selectNext()`, indépendamment d'un clic sur la barre.
 *
 * Depuis que `TilePalette` est un accordéon à hauteur variable (`LOT-27`), la barre ne peut plus
 * supposer une position de départ fixe sous la palette : `EditorScreen` appelle `relayout(top)`
 * chaque frame avec `TilePalette::bottom()` pour la repositionner juste en dessous.
 */
class ToolBar {
public:
    /// Une entrée cliquable de la barre : son rectangle écran et l'outil qu'elle sélectionne.
    struct Entry {
        EditorTool tool;
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    /// Construit la barre, disposée en bande horizontale sous la palette de tuiles (position de
    /// départ provisoire — `relayout` la corrige avant le premier rendu, voir en-tête).
    ToolBar();

    /// Recalcule les rectangles des entrées pour que la première commence à @p top (en pixels
    /// écran), sans changer l'outil sélectionné — appelé par `EditorScreen` chaque frame avec
    /// `TilePalette::bottom() + PANEL_SECTION_GAP`.
    void relayout(float top);

    /// @return Les entrées de la barre, dans leur ordre d'affichage.
    [[nodiscard]] const std::vector<Entry>& entries() const noexcept {
        return _entries;
    }

    /// @return L'outil actuellement sélectionné (`Paint` par défaut).
    [[nodiscard]] EditorTool selected() const noexcept {
        return _selected;
    }

    /**
     * @brief Traite un clic en position écran : sélectionne l'entrée couverte, s'il y en a une.
     * @param x Abscisse écran du clic, en pixels.
     * @param y Ordonnée écran du clic, en pixels.
     * @return `true` si le clic a touché la barre (sélection changée ou confirmée) ; `false`
     *         s'il est en dehors — l'appelant doit alors le traiter comme un clic sur la grille.
     */
    bool handleClick(float x, float y);

    /// Fait défiler l'outil actif dans l'ordre Pinceau → Rectangle → Sélection → Pinceau.
    void selectNext() noexcept;

private:
    std::vector<Entry> _entries;
    EditorTool _selected = EditorTool::Paint;
};

}  // namespace hmi
