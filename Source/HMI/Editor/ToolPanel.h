#pragma once

#include <QWidget>
#include <filesystem>
#include <memory>

#include "Core/Levels/Decor.h"
#include "HMI/Editor/EditorTool.h"

/**
 * @file HMI/Editor/ToolPanel.h
 * @brief Panneau « Outils » : sélection de l'outil d'édition (LOT-35). Layout dans `ToolPanel.ui`.
 */

namespace Ui {
class ToolPanel;
}

namespace hmi {

class AssetThumbnailView;
class Localization;

/**
 * @brief Sélecteur d'outil d'édition (`EX-EDIT-014`) : Pinceau, Rectangle, Sélection, Lien,
 *        Texture par instance, Décor.
 *
 * Mise en page dans `ToolPanel.ui` (boutons radio, exclusifs entre frères). Changer d'outil émet
 * `toolSelected`, consommé par le viewport (`GameViewport::setTool`). Pinceau actif par défaut.
 *
 * Porte aussi le **strict nécessaire** de l'outil Décor (`LOT-49` TACHE-04) : une grille de
 * vignettes (`hmi::AssetThumbnailView`, `LOT-43`) sur `Assets/Decors/` et un sélecteur de couche —
 * la manipulation complète (déplacer, redimensionner, pivoter, réordonner) relève de `LOT-50`.
 */
class ToolPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construit le panneau.
     * @param decorsDirectory Dossier balayé pour peupler la grille de décors (outil Décor,
     *                        `LOT-49`).
     * @param parent          Widget parent.
     */
    explicit ToolPanel(std::filesystem::path decorsDirectory, QWidget* parent = nullptr);
    ~ToolPanel() override;

    /// Applique la langue active aux libellés des outils.
    void retranslateUi(const Localization& loc);

    /**
     * @brief Coche le bouton radio de @p tool sans émettre `toolSelected` (`LOT-45`).
     *
     * Resynchronise le panneau quand l'outil change par un autre moyen que ses propres boutons —
     * le raccourci clavier de « Texture par instance ». Émettre `toolSelected` ici rebouclerait
     * sur `GameViewport::setTool`, déjà à jour.
     */
    void setActiveTool(hmi::EditorTool tool);

signals:
    void toolSelected(hmi::EditorTool tool);
    /// Émis quand l'utilisateur choisit un asset dans la grille de décors (`LOT-49`) : asset actif
    /// de l'outil Décor ; vide si aucun n'est sélectionné.
    void decorAssetSelected(const QString& fileName);
    /// Émis quand l'utilisateur choisit la couche du décor à poser (`LOT-49`, `EX-DEC-002`).
    void decorLayerSelected(core::DecorLayer layer);
    /// Émis quand l'utilisateur active/désactive l'aimantation sur la grille du geste de décors
    /// (`LOT-50` TACHE-02) — optionnelle et jamais imposée par défaut (`EX-DEC-001`).
    void decorSnapToGridChanged(bool enabled);

private:
    void updateDecorPickerVisibility();

    std::unique_ptr<Ui::ToolPanel> _ui;
    AssetThumbnailView* _decorView;  ///< Grille de vignettes de `Assets/Decors/` (`LOT-49`).
};

}  // namespace hmi
