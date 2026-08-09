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
 * @brief Panneau « Outils » : le **strict nécessaire** de l'outil Décor (`LOT-49` TACHE-04) —
 *        grille de vignettes (`hmi::AssetThumbnailView`, `LOT-43`) sur `Assets/Decors/` et un
 *        sélecteur de couche. La manipulation complète (déplacer, redimensionner, pivoter,
 *        réordonner) relève de `LOT-50`.
 *
 * Le choix de l'outil actif (`EX-EDIT-014`) se fait depuis la barre d'outils
 * (`hmi::EditorActions`, `LOT-56` TACHE-04), qui a remplacé les boutons radio empilés d'origine :
 * ce panneau n'en garde que le sélecteur de décor, affiché seulement quand l'outil Décor est
 * actif (`setActiveTool`).
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

    /// Applique la langue active aux libellés du sélecteur de décor.
    void retranslateUi(const Localization& loc);

    /**
     * @brief Affiche ou masque le sélecteur de décor selon l'outil actif.
     *
     * Appelé à chaque changement d'outil (barre d'outils ou touche dédiée de « Texture par
     * instance », via `GameViewport::toolChanged`) : seul l'outil Décor a besoin de ce sélecteur.
     */
    void setActiveTool(hmi::EditorTool tool);

signals:
    /// Émis quand l'utilisateur choisit un asset dans la grille de décors (`LOT-49`) : asset actif
    /// de l'outil Décor ; vide si aucun n'est sélectionné.
    void decorAssetSelected(const QString& fileName);
    /// Émis quand l'utilisateur choisit la couche du décor à poser (`LOT-49`, `EX-DEC-002`).
    void decorLayerSelected(core::DecorLayer layer);
    /// Émis quand l'utilisateur active/désactive l'aimantation sur la grille du geste de décors
    /// (`LOT-50` TACHE-02) — optionnelle et jamais imposée par défaut (`EX-DEC-001`).
    void decorSnapToGridChanged(bool enabled);

private:
    std::unique_ptr<Ui::ToolPanel> _ui;
    AssetThumbnailView* _decorView;  ///< Grille de vignettes de `Assets/Decors/` (`LOT-49`).
};

}  // namespace hmi
