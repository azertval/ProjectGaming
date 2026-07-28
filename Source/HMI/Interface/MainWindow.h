#pragma once

#include <QByteArray>
#include <QMainWindow>
#include <memory>

#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"
#include "HMI/Localization/Localization.h"

/**
 * @file HMI/Interface/MainWindow.h
 * @brief Fenêtre principale de l'éditeur Qt : viewport central + panneaux dockables (LOT-35).
 */

class QStackedWidget;
class QTimer;
class QWidget;

namespace Ui {
class EditorMainWindow;
}

namespace core {
class MemoryLogSink;
}

namespace hmi {

class GameViewport;
class MainMenu;
class OptionsPage;
class PalettePanel;
class ToolPanel;
class LevelBrowserPanel;
class LinkPanel;

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
    /// @param sessionLog Sink mémoire des logs (build développement) pour « Enregistrer les
    ///                   journaux » ; `nullptr` en Release (bouton indisponible).
    explicit MainWindow(core::MemoryLogSink* sessionLog = nullptr);
    ~MainWindow() override;

protected:
    /// Sauvegarde la disposition avant fermeture.
    void closeEvent(QCloseEvent* event) override;

private:
    /// Crée les panneaux (contenu des docks du `.ui`) et branche les actions de la barre de menus.
    void buildUi();
    void restoreLayout();
    void saveLayout();
    /// Ouvre la boîte de dialogue de redimensionnement du niveau (avec confirmation si
    /// destructeur).
    void openResizeDialog();

    /// Applique la langue active à tous les textes de l'IHM (fenêtre, menus, docks, panneaux).
    void retranslateUi();
    /// Change la langue active (recharge le catalogue, persiste, retraduit tout).
    void changeLanguage(const QString& code);
    /// Enregistre les journaux de session accumulés dans un fichier horodaté.
    void saveSessionLogs();
    /// Raccourci : texte localisé d'une clé, en `QString`.
    [[nodiscard]] QString text(const char* key) const;

    /// Affiche le menu principal (docks et barre de menu masqués).
    void showMenu();
    /// Affiche l'éditeur (viewport + docks + barre de menu).
    void showEditor();
    /// Lance le jeu (séquence de niveaux démo) dans le viewport, docks masqués.
    void showGame();
    /// Affiche la page Options (onglets) dans la fenêtre.
    void showOptions();
    /// Montre/masque tous les panneaux dockables.
    void setDocksVisible(bool visible);

    /// Traduit la manette en navigation de focus Qt (menus/options) : appelé par `_menuNavTimer`.
    void pollMenuGamepad();
    /// Active/désactive la navigation manette des menus (inactive en jeu/édition).
    void setMenuGamepadActive(bool active);

    std::unique_ptr<Ui::EditorMainWindow> _ui;  ///< Mise en page (MainWindow.ui : menubar + docks).
    QStackedWidget* _stack;     ///< Central : empile menu principal, options et viewport.
    MainMenu* _menu;            ///< Menu principal (page d'accueil).
    OptionsPage* _options;      ///< Page Options à onglets.
    QWidget* _editorContainer;  ///< Conteneur natif du viewport (page éditeur/jeu).
    GameViewport* _viewport;    ///< Surface de rendu D3D11 (possédée par le conteneur central).
    PalettePanel* _palette;     ///< Arbre de sélection du type de tuile (contenu du dock Palette).
    LevelBrowserPanel*
        _levels;               ///< Liste/gestion des fichiers de niveaux (contenu du dock Niveaux).
    ToolPanel* _tools;         ///< Sélecteur d'outil d'édition (contenu du dock Outils).
    LinkPanel* _links;         ///< Liste/gestion des liaisons de mécanismes (dock Liens, LOT-37).
    QByteArray _defaultState;  ///< Disposition par défaut (pour « Réinitialiser la disposition »).

    Localization _loc;  ///< Catalogue de traduction (i18n), source de tous les textes.
    core::MemoryLogSink* _sessionLog;  ///< Sink mémoire des logs (nul en Release).

    // Navigation manette des menus (hors jeu) : sondage périodique -> événements clavier Qt.
    GamepadPoller _menuPad;
    InputState _menuPadInput;
    QTimer* _menuNavTimer = nullptr;
};

}  // namespace hmi
