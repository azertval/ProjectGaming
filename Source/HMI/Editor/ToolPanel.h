#pragma once

#include <QWidget>
#include <memory>

#include "HMI/Editor/EditorTool.h"

/**
 * @file HMI/Editor/ToolPanel.h
 * @brief Panneau « Outils » : sélection de l'outil d'édition (LOT-35). Layout dans `ToolPanel.ui`.
 */

namespace Ui {
class ToolPanel;
}

namespace hmi {

class Localization;

/**
 * @brief Sélecteur d'outil d'édition (`EX-EDIT-014`) : Pinceau, Rectangle, Sélection, Lien.
 *
 * Mise en page dans `ToolPanel.ui` (boutons radio, exclusifs entre frères). Changer d'outil émet
 * `toolSelected`, consommé par le viewport (`GameViewport::setTool`). Pinceau actif par défaut.
 */
class ToolPanel : public QWidget {
    Q_OBJECT

public:
    explicit ToolPanel(QWidget* parent = nullptr);
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

private:
    std::unique_ptr<Ui::ToolPanel> _ui;
};

}  // namespace hmi
