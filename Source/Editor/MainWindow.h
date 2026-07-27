#pragma once

#include <QByteArray>
#include <QMainWindow>

/**
 * @file Editor/MainWindow.h
 * @brief Fenêtre principale de l'éditeur Qt : viewport central + panneaux dockables (LOT-35).
 */

class QDockWidget;
class QStackedWidget;
class QWidget;

namespace editor {

class GameViewport;
class MainMenu;
class PalettePanel;
class ToolPanel;
class LevelBrowserPanel;

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
    void createMenus();
    void restoreLayout();
    void saveLayout();
    /// Ouvre la boîte de dialogue de redimensionnement du niveau (avec confirmation si destructeur).
    void openResizeDialog();

    /// Affiche le menu principal (docks et barre de menu masqués).
    void showMenu();
    /// Affiche l'éditeur (viewport + docks + barre de menu).
    void showEditor();
    /// Lance le jeu (séquence de niveaux démo) dans le viewport, docks masqués.
    void showGame();
    /// Ouvre la boîte de dialogue des options (V-Sync).
    void openOptionsDialog();
    /// Montre/masque tous les panneaux dockables.
    void setDocksVisible(bool visible);

    QStackedWidget* _stack;       ///< Central : empile menu principal et viewport (éditeur/jeu).
    MainMenu* _menu;              ///< Menu principal (page d'accueil).
    QWidget* _editorContainer;    ///< Conteneur natif du viewport (page éditeur/jeu).
    GameViewport* _viewport;      ///< Surface de rendu D3D11 (possédée par le conteneur central).
    QDockWidget* _palettePanel;   ///< Panneau « Palette » (dock hôte de `_palette`).
    PalettePanel* _palette;       ///< Arbre de sélection du type de tuile (LOT-35 TACHE-02).
    QDockWidget* _levelsPanel;    ///< Panneau « Niveaux » (dock hôte de `_levels`).
    LevelBrowserPanel* _levels;   ///< Liste/gestion des fichiers de niveaux (LOT-36).
    QDockWidget* _toolPanel;      ///< Panneau « Outils » (dock hôte de `_tools`).
    ToolPanel* _tools;            ///< Sélecteur d'outil d'édition (LOT-35).
    QDockWidget* _statusPanel;    ///< Panneau « Statut ».
    QByteArray _defaultState;     ///< Disposition par défaut (pour « Réinitialiser la disposition »).
};

}  // namespace editor
