#pragma once

#include <memory>

#include <QWidget>

#include "HMI/Editor/EditorTool.h"

/**
 * @file Editor/ToolPanel.h
 * @brief Panneau « Outils » : sélection de l'outil d'édition (LOT-35). Layout dans `ToolPanel.ui`.
 */

namespace Ui {
class ToolPanel;
}

namespace editor {

/**
 * @brief Sélecteur d'outil d'édition (`EX-EDIT-014`) : Pinceau, Rectangle, Sélection.
 *
 * Mise en page dans `ToolPanel.ui` (boutons radio, exclusifs entre frères). Changer d'outil émet
 * `toolSelected`, consommé par le viewport (`GameViewport::setTool`). Pinceau actif par défaut.
 */
class ToolPanel : public QWidget {
    Q_OBJECT

public:
    explicit ToolPanel(QWidget* parent = nullptr);
    ~ToolPanel() override;

signals:
    void toolSelected(hmi::EditorTool tool);

private:
    std::unique_ptr<Ui::ToolPanel> _ui;
};

}  // namespace editor
