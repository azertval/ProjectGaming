#pragma once

#include <QMainWindow>

/**
 * @file Editor/MainWindow.h
 * @brief Fenêtre principale de l'éditeur Qt : héberge le viewport Direct3D 11 (LOT-34).
 */

namespace editor {

class GameViewport;

/**
 * @brief Fenêtre principale de l'application Qt.
 *
 * À cette tâche (LOT-34 TACHE-02), son **widget central** est le viewport Direct3D 11
 * (`GameViewport`), inséré via `QWidget::createWindowContainer`. Les panneaux dockables de l'éditeur
 * (palette, outils, niveaux…) seront ajoutés autour au LOT-35.
 */
class MainWindow : public QMainWindow {
public:
    MainWindow();
    ~MainWindow() override;

private:
    GameViewport* _viewport;  ///< Surface de rendu D3D11 (possédée par le conteneur central).
};

}  // namespace editor
