#pragma once

#include <QByteArray>
#include <QMainWindow>

/**
 * @file Editor/MainWindow.h
 * @brief Fenêtre principale de l'éditeur Qt : viewport central + panneaux dockables (LOT-35).
 */

class QDockWidget;

namespace editor {

class GameViewport;

/**
 * @brief Fenêtre principale de l'application Qt : **poste de travail d'éditeur** à panneaux
 *        dockables autour du viewport Direct3D 11.
 *
 * Le **widget central** est le viewport (`GameViewport`, LOT-34), non dockable. Autour, des
 * `QDockWidget` (Palette, Outils, Statut) **déplaçables, redimensionnables, détachables**
 * (`EX-IHM-010`). La **disposition** (géométrie + agencement des docks) est **persistée hors code**
 * via `QSettings` (`EX-IHM-011`) : restaurée au lancement, sauvegardée à la fermeture, et
 * réinitialisable à une disposition par défaut (menu « Affichage »).
 *
 * À cette tâche (LOT-35 TACHE-01), les panneaux sont des **coquilles** : leur contenu (arbre de
 * palette, barre d'outils, barre d'état) est rempli aux tâches suivantes.
 */
class MainWindow : public QMainWindow {
public:
    MainWindow();
    ~MainWindow() override;

protected:
    /// Sauvegarde la disposition avant fermeture.
    void closeEvent(QCloseEvent* event) override;

private:
    void createDockPanels();
    void createViewMenu();
    void restoreLayout();
    void saveLayout();

    GameViewport* _viewport;      ///< Surface de rendu D3D11 (possédée par le conteneur central).
    QDockWidget* _palettePanel;   ///< Panneau « Palette » (rempli au LOT-35 TACHE-02).
    QDockWidget* _toolPanel;      ///< Panneau « Outils » (rempli au LOT-35 TACHE-03).
    QDockWidget* _statusPanel;    ///< Panneau « Statut ».
    QByteArray _defaultState;     ///< Disposition par défaut (pour « Réinitialiser la disposition »).
};

}  // namespace editor
