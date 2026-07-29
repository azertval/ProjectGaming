#pragma once

#include <QPixmap>
#include <QWidget>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "Core/Levels/TileType.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"

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

    /**
     * @brief Désigne de quoi afficher des vignettes fidèles au canevas (`LOT-42`).
     * @param skinsDirectory Dossier des fichiers de skins, décodés **côté CPU** pour les vignettes.
     * @param catalog        Catalogue des skins, non possédé.
     */
    void setSkinSource(std::filesystem::path skinsDirectory, const SkinCatalog* catalog);

    /**
     * @brief Rafraîchit les vignettes de la palette (`EX-EDIT-027`).
     *
     * À appeler quand le mode de rendu bascule, quand le jeu de skins change, ou quand une
     * assignation est modifiée : les trois événements qui rendent les vignettes obsolètes.
     * @param mode    Mode de rendu courant.
     * @param setName Jeu de skins courant ; vide pour le jeu par défaut du catalogue.
     */
    void refreshThumbnails(RenderMode mode, const std::string& setName);

signals:
    /// Émis quand l'utilisateur sélectionne une tuile (feuille) dans l'arbre.
    void tileSelected(core::TileType type);

private:
    void buildModel();
    void onCurrentChanged(const QModelIndex& current);
    /// Vignette d'un type dans le mode courant, décodée au premier besoin puis mise en cache.
    [[nodiscard]] QPixmap thumbnailFor(core::TileType type);

    QTreeView* _tree;
    QStandardItemModel* _model;
    core::TileType _selected = core::TileType::Solid;
    const Localization* _loc = nullptr;  ///< Catalogue courant (nul avant première retraduction).

    std::filesystem::path _skinsDirectory;
    const SkinCatalog* _catalog = nullptr;  ///< Non possédé (propriété de `GameViewport`).
    RenderMode _mode = DEFAULT_RENDER_MODE;
    std::string _skinSet;
    /// Fichier de skin → image décodée. La palette affiche une trentaine d'entrées : décoder à
    /// chaque redessin, ou pendant le défilement, coûterait bien plus que de les garder.
    std::unordered_map<std::string, QPixmap> _decoded;
};

}  // namespace hmi
