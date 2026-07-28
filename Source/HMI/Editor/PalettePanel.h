#pragma once

#include <QWidget>

#include "Core/Levels/TileType.h"

/**
 * @file HMI/Editor/PalettePanel.h
 * @brief Panneau « Palette » : arbre de sélection du type de tuile à peindre (LOT-35).
 */

class QModelIndex;
class QStandardItemModel;
class QTreeView;

namespace hmi {

class Localization;

/**
 * @brief Palette de tuiles en **arbre** (`QTreeView`) : catégories → sous-groupes → tuiles.
 *
 * Alimentée par la taxonomie pure (`hmi::tileTaxonomy`), elle remplace l'accordéon « maison »
 * (retiré au `LOT-38`) par un contrôle Qt natif (`EX-EDIT-018`, `EX-IHM-010`). Sélectionner une
 * **feuille** met à jour le type de tuile actif (`selectedTile`) et émet `tileSelected` — consommé
 * par l'outil de peinture (LOT-35 TACHE-03). Les en-têtes (catégories, sous-groupes) ne sont pas
 * sélectionnables comme tuile.
 */
class PalettePanel : public QWidget {
    Q_OBJECT

public:
    explicit PalettePanel(QWidget* parent = nullptr);

    /// @return Le type de tuile actuellement sélectionné (`Solid` par défaut).
    [[nodiscard]] core::TileType selectedTile() const noexcept {
        return _selected;
    }

    /// Applique la langue active (reconstruit l'arbre avec les libellés traduits).
    void retranslateUi(const Localization& loc);

signals:
    /// Émis quand l'utilisateur sélectionne une tuile (feuille) dans l'arbre.
    void tileSelected(core::TileType type);

private:
    void buildModel();
    void onCurrentChanged(const QModelIndex& current);

    QTreeView* _tree;
    QStandardItemModel* _model;
    core::TileType _selected = core::TileType::Solid;
    const Localization* _loc = nullptr;  ///< Catalogue courant (nul avant première retraduction).
};

}  // namespace hmi
