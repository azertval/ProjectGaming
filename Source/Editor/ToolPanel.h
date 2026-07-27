#pragma once

#include <QWidget>

#include "HMI/Editor/EditorTool.h"

/**
 * @file Editor/ToolPanel.h
 * @brief Panneau « Outils » : sélection de l'outil d'édition (pinceau/rectangle/sélection) (LOT-35).
 */

class QButtonGroup;

namespace editor {

/**
 * @brief Sélecteur d'outil d'édition (`EX-EDIT-014`) : Pinceau, Rectangle, Sélection.
 *
 * Boutons **exclusifs** (`QButtonGroup`) ; changer d'outil émet `toolSelected`, consommé par le
 * viewport (`GameViewport::setTool`). Pinceau est actif par défaut.
 */
class ToolPanel : public QWidget {
    Q_OBJECT

public:
    explicit ToolPanel(QWidget* parent = nullptr);

signals:
    void toolSelected(hmi::EditorTool tool);

private:
    QButtonGroup* _group;
};

}  // namespace editor
