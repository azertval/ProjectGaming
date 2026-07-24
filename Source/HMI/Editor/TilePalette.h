#pragma once

#include <array>
#include <string>
#include <vector>

#include "Core/Levels/TileType.h"

/**
 * @file HMI/Editor/TilePalette.h
 * @brief Sélection du type de tuile actif pour la peinture dans l'éditeur (`EX-EDIT-002`),
 *        organisée en catégories repliables (`LOT-27`, `EX-EDIT-018`).
 */

namespace hmi {

/**
 * @brief Bande de tuiles cliquables (palette) sélectionnant le type de tuile à peindre.
 *
 * Logique **pure** : ne connaît que la géométrie écran (rectangles en pixels) et l'état de
 * sélection/dépliage, sans dépendance de rendu — testable sans GPU (`EX-NFR-010`). Le dessin des
 * entrées (icônes d'atlas, surbrillance de la sélection) reste délégué à `EditorScreen`, qui lit
 * `entries()`/`selected()` exactement comme avant `LOT-27` : chaque `Entry` porte toujours un
 * `core::TileType` valide (le type sélectionnable pour une entrée-feuille, ou un type
 * **représentatif** pour une entrée d'en-tête, utilisé uniquement pour son icône) et un rectangle
 * écran — `EditorScreen` n'a donc rien à changer pour dessiner la nouvelle disposition.
 *
 * **Trois niveaux d'accordéon** (`EX-EDIT-018`), reflétés uniquement par la position/visibilité
 * des entrées (`entries()` n'expose jamais une entrée actuellement repliée) :
 * 1. Deux entrées **autonomes** toujours visibles (Vide, Piège — sélection directe, pas de
 *    dépliage) et trois **catégories** repliables (Tuile, Interactif, Jalon).
 * 2. Une catégorie dépliée expose ses tuiles **directes** (ex. Porte, Plaque, Interrupteur) et,
 *    pour Tuile/Interactif, des **sous-groupes** repliables (Pente, Arrondi, Concave, Bloc
 *    poussable — familles à plusieurs formes/tailles).
 * 3. Un sous-groupe déplié expose ses variantes (orientations, tailles).
 *
 * Cliquer une entrée d'en-tête (catégorie ou sous-groupe) **replie/déplie** ce niveau sans changer
 * la sélection courante ; cliquer une entrée-feuille **sélectionne** son type. Dans les deux cas,
 * `handleClick` renvoie `true` (clic consommé par la palette) — l'appelant (`EditorScreen`) n'a
 * pas à distinguer les deux cas.
 *
 * La hauteur totale de la palette variant désormais avec l'état de dépliage (plus de compte fixe
 * comme avant `LOT-27`), `bottom()` donne la position `y` courante juste sous la dernière entrée
 * visible — `EditorScreen` l'utilise pour repositionner dynamiquement `ToolBar` (`ToolBar::relayout`)
 * en dessous, à chaque frame.
 *
 * **Défilement** (`EX-EDIT-018`) : tout déplier en même temps (jusqu'à 26 lignes) peut dépasser la
 * hauteur de fenêtre disponible. `entries()` n'expose donc que la **fenêtre visible** courante
 * (au plus `visibleRowCount(viewportHeight)` lignes, jamais moins d'une) ; `scroll` la fait défiler
 * (molette, sans changer la sélection ni l'état de dépliage), sur le modèle de `LevelPicker`. Sans
 * cela, un long dépliage rendrait les dernières entrées définitivement inaccessibles derrière le
 * bas de l'écran, sans aucun moyen de les atteindre.
 */
class TilePalette {
public:
    /// Une entrée cliquable de la palette : son rectangle écran, un type de tuile (celui qu'elle
    /// sélectionne pour une feuille ; un type représentatif, non sélectionnable directement, pour
    /// l'icône d'une entrée d'en-tête) et son libellé — dessin délégué à `EditorScreen`. Le
    /// libellé d'un en-tête est préfixé de `> ` (replié) ou `v ` (déplié).
    struct Entry {
        core::TileType type;
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        std::string label;
    };

    /// Construit la palette : catégories/sous-groupes tous repliés, sélection initiale `Solid`.
    TilePalette();

    /// @return Les entrées **actuellement visibles** de la palette, dans leur ordre d'affichage
    ///         (une entrée sous une catégorie/un sous-groupe replié n'y figure pas).
    [[nodiscard]] const std::vector<Entry>& entries() const noexcept {
        return _entries;
    }

    /// @return Le type de tuile actuellement sélectionné (`Solid` par défaut).
    [[nodiscard]] core::TileType selected() const noexcept {
        return _selected;
    }

    /// @return La position `y`, en pixels écran, juste sous la dernière entrée **visible** de la
    ///         fenêtre courante — dépend de l'état de dépliage et de défilement (varie d'un clic
    ///         sur un en-tête, ou d'un défilement à la molette, à l'autre).
    [[nodiscard]] float bottom() const noexcept {
        return _bottom;
    }

    /// @return L'indice de la première ligne affichée (défilement) — les lignes d'indice inférieur
    ///         sont défilées au-dessus du haut de la palette, invisibles.
    [[nodiscard]] int scrollOffset() const noexcept {
        return _scrollOffset;
    }

    /// @return Le nombre total de lignes actuellement dépliées (feuilles + en-têtes visibles selon
    ///         l'état de dépliage), défilement mis à part — sert, comparé à `visibleRowCount`, à
    ///         savoir si une barre de défilement doit être dessinée.
    [[nodiscard]] int totalRowCount() const noexcept {
        return _totalRows;
    }

    /**
     * @brief Nombre de lignes affichables simultanément sans défilement, pour une hauteur de
     *        fenêtre donnée — réserve la place occupée par `ToolBar` (3 lignes) sous la palette.
     * @param viewportHeight Hauteur de la surface de rendu, en pixels.
     * @return Au moins 1 (une fenêtre trop petite pour une seule ligne reste franchissable).
     */
    [[nodiscard]] static int visibleRowCount(float viewportHeight);

    /// Synchronise la hauteur de fenêtre connue (fenêtrage du défilement) — à appeler chaque
    /// frame, avant tout clic ou défilement : un redimensionnement change le nombre de lignes
    /// visibles sans qu'aucun clic ni dépliage n'ait eu lieu.
    void setViewportHeight(float viewportHeight);

    /**
     * @brief Fait défiler la fenêtre visible (molette), sans changer la sélection ni l'état de
     *        dépliage — même esprit que `LevelPicker::update` (la molette parcourt la liste sans
     *        jamais la modifier). Utilise la dernière hauteur de fenêtre connue
     *        (`setViewportHeight`).
     * @param wheelDelta Cumul de molette de la frame (`InputState::wheelDelta`).
     */
    void scroll(int wheelDelta);

    /**
     * @brief Traite un clic en position écran : sélectionne l'entrée couverte, s'il y en a une —
     *        replie/déplie un en-tête, ou sélectionne le type d'une feuille. Seule la fenêtre
     *        visible courante (défilement compris) est testée.
     * @param x Abscisse écran du clic, en pixels.
     * @param y Ordonnée écran du clic, en pixels.
     * @return `true` si le clic a touché la palette (en-tête replié/déplié, ou sélection changée/
     *         confirmée) ; `false` s'il est en dehors — l'appelant doit alors le traiter comme un
     *         clic sur la grille.
     */
    bool handleClick(float x, float y);

private:
    /// Catégories repliables de premier niveau (l'ordre suit `CATEGORY_COUNT`).
    enum class Category { Tuile, Interactif, Jalon };
    static constexpr std::size_t CATEGORY_COUNT = 3;

    /// Sous-groupes repliables de second niveau (une famille à plusieurs formes/tailles).
    enum class Subgroup { Pente, Arrondi, Concave, Bloc };
    static constexpr std::size_t SUBGROUP_COUNT = 4;

    /// Action déclenchée par un clic sur une entrée ; parallèle à `_entries` (même index).
    enum class RowAction { SelectType, ToggleCategory, ToggleSubgroup };
    struct Row {
        RowAction action;
        core::TileType type = core::TileType::Empty;  ///< Valide seulement si `SelectType`.
        Category category = Category::Tuile;           ///< Valide seulement si `ToggleCategory`.
        Subgroup subgroup = Subgroup::Pente;            ///< Valide seulement si `ToggleSubgroup`.
    };

    /// Reconstruit la liste complète (état de dépliage courant), borne `_scrollOffset` à son
    /// intervalle valide pour `_viewportHeight`, puis découpe `_entries`/`_rows`/`_bottom`/
    /// `_totalRows` sur la fenêtre visible qui en résulte — seul point de passage qui pose les
    /// positions écran ; appelé à la construction et après toute bascule/défilement/redimensionnement.
    void relayout();

    /// Ajuste `_scrollOffset` pour que la ligne d'indice absolu @p absoluteIndex (dans la liste
    /// complète, indépendante du défilement) reste dans la fenêtre visible, puis redécoupe —
    /// appelé juste après avoir replié/déplié un en-tête, sinon un en-tête proche du bas de la
    /// liste pourrait se retrouver hors fenêtre (ou pousser un en-tête plus bas hors fenêtre) sans
    /// aucun indice visuel, la seule barre de défilement ne suffisant pas à le deviner.
    void followRow(int absoluteIndex);

    std::vector<Entry> _entries;  ///< Fenêtre visible seulement (défilement compris).
    std::vector<Row> _rows;  ///< Même index que `_entries` : ce que fait un clic sur chaque ligne.
    core::TileType _selected = core::TileType::Solid;
    float _bottom = 0.0f;
    int _totalRows = 0;        ///< Nombre de lignes dépliées, défilement mis à part.
    int _scrollOffset = 0;     ///< Indice de la première ligne affichée (défilement).
    float _viewportHeight = 720.0f;  ///< Dernière hauteur connue (`setViewportHeight`).

    std::array<bool, CATEGORY_COUNT> _categoryExpanded{};
    std::array<bool, SUBGROUP_COUNT> _subgroupExpanded{};
};

}  // namespace hmi
