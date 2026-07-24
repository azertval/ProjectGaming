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

constexpr float PALETTE_TOP = 8.0f;  ///< Haut de la première ligne (palette).

/// Décalage horizontal d'un niveau d'imbrication de la palette en accordéon (`LOT-27`,
/// `EX-EDIT-018`) : une entrée sous une catégorie ou un sous-groupe replié se décale d'un cran de
/// cette largeur, sans changer sa taille d'icône — seule sa position `x` en tient compte.
constexpr float PALETTE_INDENT_STEP = 14.0f;

/// La hauteur du panneau (palette en accordéon, `LOT-27`) varie désormais selon les catégories/
/// sous-groupes dépliés — plus de compte fixe ni de `TOOLBAR_TOP` constant : `TilePalette::bottom()`
/// donne la position courante, que `EditorScreen` répercute sur `ToolBar::relayout` chaque frame.

}  // namespace hmi
