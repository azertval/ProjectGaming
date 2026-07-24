#include "HMI/Editor/TilePalette.h"

#include <algorithm>

#include "HMI/Editor/EditorLayout.h"

namespace hmi {

namespace {

// Prefixe d'en-tete (categorie/sous-groupe) indiquant l'etat de depliage courant : la meme
// convention textuelle que le reste de la palette (libelles courts, sans accent, police bitmap).
[[nodiscard]] std::string headerLabel(const std::string& name, bool expanded) {
    return (expanded ? "v " : "> ") + name;
}

// Place reservee sous la fenetre visible de la palette pour ToolBar (3 lignes) et une marge basse
// -- la barre d'outils reste toujours juste sous la fenetre, jamais hors ecran ni chevauchee.
constexpr float RESERVED_BELOW_PALETTE = PANEL_SECTION_GAP + 3.0f * PANEL_ROW_PITCH + 24.0f;

}  // namespace

// Disposition en colonne (LOT-15/LOT-27), dans le panneau lateral : une ligne par entree
// actuellement visible, icone + libelle dessines cote a cote par EditorScreen (geometrie ici,
// rendu delegue comme documente en en-tete). Categories/sous-groupes tous replies au depart.
TilePalette::TilePalette() { relayout(); }

int TilePalette::visibleRowCount(float viewportHeight) {
    const int rows =
        static_cast<int>((viewportHeight - PALETTE_TOP - RESERVED_BELOW_PALETTE) / PANEL_ROW_PITCH);
    return (std::max)(1, rows);
}

void TilePalette::setViewportHeight(float viewportHeight) {
    _viewportHeight = viewportHeight;
    relayout();
}

void TilePalette::scroll(int wheelDelta) {
    constexpr float WHEEL_NOTCH = 120.0f;  // WHEEL_DELTA Win32 : un cran de molette standard.
    _scrollOffset -= static_cast<int>(static_cast<float>(wheelDelta) / WHEEL_NOTCH);
    relayout();
}

void TilePalette::followRow(int absoluteIndex) {
    const int visible = visibleRowCount(_viewportHeight);
    if (absoluteIndex < _scrollOffset) {
        _scrollOffset = absoluteIndex;
    } else if (absoluteIndex >= _scrollOffset + visible) {
        _scrollOffset = absoluteIndex - visible + 1;
    }
    relayout();
}

// Reconstruit la liste complete (etat de depliage courant, positions absolues), puis decoupe sur
// la fenetre visible determinee par _scrollOffset/_viewportHeight -- seul point de passage qui
// pose les positions ecran finales de _entries/_rows (voir la doc de l'en-tete).
void TilePalette::relayout() {
    std::vector<Entry> fullEntries;
    std::vector<Row> fullRows;

    float y = PALETTE_TOP;

    // Ajoute une entree-feuille (selectionnable) a l'indentation donnee.
    const auto pushLeaf = [&](core::TileType type, const std::string& label, float indent) {
        fullEntries.push_back(Entry{type, PANEL_MARGIN + indent, y, PANEL_ICON_SIZE, PANEL_ICON_SIZE, label});
        fullRows.push_back(Row{RowAction::SelectType, type, Category::Tuile, Subgroup::Pente});
        y += PANEL_ROW_PITCH;
    };
    // Ajoute une entree autonome (Vide, Piege) : selectionnable directement, jamais repliable.
    const auto pushStandalone = [&](core::TileType type, const std::string& label) {
        pushLeaf(type, label, 0.0f);
    };
    // Ajoute un en-tete de categorie (niveau 1, jamais indente) : replie/deplie au clic.
    const auto pushCategoryHeader = [&](Category category, core::TileType icon,
                                        const std::string& name, bool expanded) {
        fullEntries.push_back(
            Entry{icon, PANEL_MARGIN, y, PANEL_ICON_SIZE, PANEL_ICON_SIZE, headerLabel(name, expanded)});
        fullRows.push_back(Row{RowAction::ToggleCategory, core::TileType::Empty, category, Subgroup::Pente});
        y += PANEL_ROW_PITCH;
    };
    // Ajoute un en-tete de sous-groupe (niveau 2, indente d'un cran sous sa categorie).
    const auto pushSubgroupHeader = [&](Subgroup subgroup, core::TileType icon,
                                        const std::string& name, bool expanded) {
        fullEntries.push_back(Entry{icon, PANEL_MARGIN + PALETTE_INDENT_STEP, y, PANEL_ICON_SIZE,
                                    PANEL_ICON_SIZE, headerLabel(name, expanded)});
        fullRows.push_back(Row{RowAction::ToggleSubgroup, core::TileType::Empty, Category::Tuile, subgroup});
        y += PANEL_ROW_PITCH;
    };

    const bool tuileOpen = _categoryExpanded[static_cast<std::size_t>(Category::Tuile)];
    const bool interactifOpen = _categoryExpanded[static_cast<std::size_t>(Category::Interactif)];
    const bool jalonOpen = _categoryExpanded[static_cast<std::size_t>(Category::Jalon)];
    const bool penteOpen = _subgroupExpanded[static_cast<std::size_t>(Subgroup::Pente)];
    const bool arrondiOpen = _subgroupExpanded[static_cast<std::size_t>(Subgroup::Arrondi)];
    const bool blocOpen = _subgroupExpanded[static_cast<std::size_t>(Subgroup::Bloc)];

    // ---- Vide (autonome) ----
    pushStandalone(core::TileType::Empty, "Vide");

    // ---- Tuile : Plein (feuille directe), Pente et Arrondi (sous-groupes) ----
    pushCategoryHeader(Category::Tuile, core::TileType::Solid, "Tuile", tuileOpen);
    if (tuileOpen) {
        pushLeaf(core::TileType::Solid, "Plein", PALETTE_INDENT_STEP);

        pushSubgroupHeader(Subgroup::Pente, core::TileType::SlopeUpRight, "Pente", penteOpen);
        if (penteOpen) {
            const float indent = PALETTE_INDENT_STEP * 2.0f;
            pushLeaf(core::TileType::SlopeUpRight, "Sol D", indent);
            pushLeaf(core::TileType::SlopeUpLeft, "Sol G", indent);
            pushLeaf(core::TileType::SlopeDownRight, "Plaf D", indent);
            pushLeaf(core::TileType::SlopeDownLeft, "Plaf G", indent);
        }

        pushSubgroupHeader(Subgroup::Arrondi, core::TileType::RoundedUpRight, "Arrondi", arrondiOpen);
        if (arrondiOpen) {
            const float indent = PALETTE_INDENT_STEP * 2.0f;
            pushLeaf(core::TileType::RoundedUpRight, "Sol D", indent);
            pushLeaf(core::TileType::RoundedUpLeft, "Sol G", indent);
            pushLeaf(core::TileType::RoundedDownRight, "Plaf D", indent);
            pushLeaf(core::TileType::RoundedDownLeft, "Plaf G", indent);
        }
    }

    // ---- Interactif : Porte/Plaque/Interrupteur (feuilles directes), Bloc poussable (sous-groupe) ----
    pushCategoryHeader(Category::Interactif, core::TileType::Door, "Interactif", interactifOpen);
    if (interactifOpen) {
        pushLeaf(core::TileType::Door, "Porte", PALETTE_INDENT_STEP);
        pushLeaf(core::TileType::PressurePlate, "Plaque", PALETTE_INDENT_STEP);
        pushLeaf(core::TileType::Switch, "Interr.", PALETTE_INDENT_STEP);

        pushSubgroupHeader(Subgroup::Bloc, core::TileType::Block, "Bloc poussable", blocOpen);
        if (blocOpen) {
            const float indent = PALETTE_INDENT_STEP * 2.0f;
            pushLeaf(core::TileType::Block, "Plein", indent);
            pushLeaf(core::TileType::BlockHalf, "1/2", indent);
            pushLeaf(core::TileType::BlockQuarter, "1/4", indent);
        }
    }

    // ---- Piege (autonome, ex-Danger, seule tuile de sa categorie) ----
    pushStandalone(core::TileType::Danger, "Piege");

    // ---- Jalon : Entree/Sortie (feuilles directes, pas de sous-groupe) ----
    pushCategoryHeader(Category::Jalon, core::TileType::Entry, "Jalon", jalonOpen);
    if (jalonOpen) {
        pushLeaf(core::TileType::Entry, "Entree", PALETTE_INDENT_STEP);
        pushLeaf(core::TileType::Exit, "Sortie", PALETTE_INDENT_STEP);
    }

    // ---- Decoupage sur la fenetre visible (defilement, EX-EDIT-018) ----
    _totalRows = static_cast<int>(fullEntries.size());
    const int visible = visibleRowCount(_viewportHeight);
    const int maxOffset = (std::max)(0, _totalRows - visible);
    _scrollOffset = std::clamp(_scrollOffset, 0, maxOffset);
    const int lastVisible = (std::min)(_totalRows, _scrollOffset + visible);

    _entries.clear();
    _rows.clear();
    for (int index = _scrollOffset; index < lastVisible; ++index) {
        Entry entry = fullEntries[static_cast<std::size_t>(index)];
        entry.y = PALETTE_TOP + static_cast<float>(index - _scrollOffset) * PANEL_ROW_PITCH;
        _entries.push_back(std::move(entry));
        _rows.push_back(fullRows[static_cast<std::size_t>(index)]);
    }
    _bottom = PALETTE_TOP + static_cast<float>(_entries.size()) * PANEL_ROW_PITCH;
}

bool TilePalette::handleClick(float x, float y) {
    for (std::size_t index = 0; index < _entries.size(); ++index) {
        const Entry& entry = _entries[index];
        if (x < entry.x || x >= entry.x + entry.width || y < entry.y || y >= entry.y + entry.height) {
            continue;
        }
        const Row& row = _rows[index];
        // Indice ABSOLU (independant du defilement) de la ligne cliquee : reste valide apres le
        // depliage/repliage ci-dessous (seul ce qui la SUIT dans la liste change, voir followRow),
        // donc calcule avant relayout(), qui reconstruit _entries/_rows sur une nouvelle fenetre.
        const int absoluteIndex = _scrollOffset + static_cast<int>(index);
        switch (row.action) {
            case RowAction::SelectType:
                _selected = row.type;
                break;
            case RowAction::ToggleCategory: {
                bool& expanded = _categoryExpanded[static_cast<std::size_t>(row.category)];
                expanded = !expanded;
                followRow(absoluteIndex);
                break;
            }
            case RowAction::ToggleSubgroup: {
                bool& expanded = _subgroupExpanded[static_cast<std::size_t>(row.subgroup)];
                expanded = !expanded;
                followRow(absoluteIndex);
                break;
            }
        }
        return true;
    }
    return false;
}

}  // namespace hmi
