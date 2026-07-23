#pragma once

/**
 * @file HMI/Editor/EditorLayout.h
 * @brief Disposition du panneau latéral de l'éditeur, partagée par ses widgets.
 */

namespace hmi {

/**
 * @brief Constantes de disposition du panneau latéral de l'éditeur (LOT-15, `EX-EDIT-015`).
 *
 * `TilePalette`, `ToolBar` et `EditorScreen` (bouton grille, décalage de la caméra) partagent ces
 * constantes pour rester cohérents : un **panneau vertical** fixe sur le bord gauche de l'écran,
 * plutôt que des bandes empilées en haut-gauche pouvant se superposer entre elles ou avec la
 * grille (LOT-14/LOT-15 TACHE-06 initiales).
 */
constexpr float PANEL_WIDTH = 168.0f;      ///< Largeur totale du panneau, en pixels écran.
constexpr float PANEL_MARGIN = 8.0f;       ///< Marge gauche des icônes dans le panneau.
constexpr float PANEL_ICON_SIZE = 26.0f;   ///< Côté d'une icône (palette, outil, bouton).
constexpr float PANEL_ROW_GAP = 6.0f;      ///< Espace vertical entre deux lignes.
constexpr float PANEL_ROW_PITCH = PANEL_ICON_SIZE + PANEL_ROW_GAP;  ///< Pas vertical d'une ligne.
constexpr float PANEL_SECTION_GAP = 14.0f;  ///< Espace supplémentaire entre deux sections.

constexpr float PALETTE_TOP = 8.0f;               ///< Haut de la première ligne (palette, 13 types).
/// Nombre de types de tuiles dans la palette (`TilePalette::PALETTE_TYPES`) — seule source de
/// vérité de ce compte, partagée ici pour dimensionner le panneau sans dupliquer la liste.
constexpr int PALETTE_TYPE_COUNT = 13;
/// Haut de la première ligne de la barre d'outils (sous les lignes de la palette).
constexpr float TOOLBAR_TOP =
    PALETTE_TOP + static_cast<float>(PALETTE_TYPE_COUNT) * PANEL_ROW_PITCH + PANEL_SECTION_GAP;

}  // namespace hmi
