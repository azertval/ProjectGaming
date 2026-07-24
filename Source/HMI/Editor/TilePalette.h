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
 *    pour Tuile/Interactif, des **sous-groupes** repliables (Pente, Arrondi, Bloc poussable —
 *    familles à plusieurs formes/tailles).
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

    /// @return La position `y`, en pixels écran, juste sous la dernière entrée visible — dépend de
    ///         l'état de dépliage courant (varie d'un clic sur un en-tête à l'autre).
    [[nodiscard]] float bottom() const noexcept {
        return _bottom;
    }

    /**
     * @brief Traite un clic en position écran : sélectionne l'entrée couverte, s'il y en a une —
     *        replie/déplie un en-tête, ou sélectionne le type d'une feuille.
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
    enum class Subgroup { Pente, Arrondi, Bloc };
    static constexpr std::size_t SUBGROUP_COUNT = 3;

    /// Action déclenchée par un clic sur une entrée ; parallèle à `_entries` (même index).
    enum class RowAction { SelectType, ToggleCategory, ToggleSubgroup };
    struct Row {
        RowAction action;
        core::TileType type = core::TileType::Empty;  ///< Valide seulement si `SelectType`.
        Category category = Category::Tuile;           ///< Valide seulement si `ToggleCategory`.
        Subgroup subgroup = Subgroup::Pente;            ///< Valide seulement si `ToggleSubgroup`.
    };

    /// Reconstruit `_entries`/`_rows`/`_bottom` d'après l'état de dépliage courant — seul point de
    /// passage qui pose les positions écran ; appelé à la construction et après chaque bascule.
    void relayout();

    std::vector<Entry> _entries;
    std::vector<Row> _rows;  ///< Même index que `_entries` : ce que fait un clic sur chaque ligne.
    core::TileType _selected = core::TileType::Solid;
    float _bottom = 0.0f;

    std::array<bool, CATEGORY_COUNT> _categoryExpanded{};
    std::array<bool, SUBGROUP_COUNT> _subgroupExpanded{};
};

}  // namespace hmi
