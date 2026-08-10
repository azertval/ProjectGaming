#pragma once

#include <QByteArray>
#include <QMainWindow>
#include <array>
#include <filesystem>
#include <memory>

#include "HMI/Editor/EditContextTarget.h"
#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PanelFocus.h"
#include "HMI/Editor/PixelPalette.h"
#include "HMI/Editor/PixelTool.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"
#include "HMI/Localization/Localization.h"

/**
 * @file HMI/Interface/MainWindow.h
 * @brief Fenêtre principale de l'éditeur Qt : viewport central + panneaux dockables (LOT-35).
 */

class QAction;
class QLabel;
class QMenu;
class QStackedWidget;
class QTimer;
class QToolBar;
class QWidget;

namespace Ui {
class EditorMainWindow;
}

namespace core {
class MemoryLogSink;
}

namespace hmi {

class DecorsPanel;
class EditorActions;
class GameViewport;
class MainMenu;
class OptionsPage;
class PalettePanel;
class LevelBrowserPanel;
class LinkPanel;
class TexturePanel;
class PixelCanvas;
class PixelHistoryPanel;
class PixelPalettePanel;

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
    /// Recalcule et réaffiche la barre d'état (zones permanentes + aide contextuelle) depuis l'état
    /// courant du viewport (`LOT-57` TACHE-01, `hmi::editorStatusLines`) — seule voie de mise à
    /// jour, appelée par tout changement pertinent (outil, survol, zoom, brouillon) ainsi qu'à
    /// l'expiration d'un message transitoire, pour la restaurer.
    void refreshStatusHelp();
    /// Affiche @p message pour @p timeoutMs dans la barre d'état, puis restaure l'aide contextuelle
    /// (`refreshStatusHelp`) — remplace `QStatusBar::showMessage`'s minuteur interne, qui laissait
    /// la barre vide à l'expiration au lieu de reprendre la main.
    void showTransientStatusMessage(const QString& message, int timeoutMs);
    /// Met en avant le panneau associé à @p tool (`hmi::panelForTool`), si le réglage est actif et
    /// que l'utilisateur n'a rien imposé lui-même (`LOT-57` TACHE-02) — jamais un masquage, une
    /// simple suggestion de premier plan parmi les onglets regroupés.
    void applyPanelFocus(hmi::EditorTool tool);
    /// Équivalent pour les outils du canevas pixel art (`hmi::panelForPixelTool`, `LOT-54`
    /// TACHE-04) — même garde, même règle de non-masquage.
    void applyPixelPanelFocus(hmi::PixelTool tool);
    /// Met en avant @p panel (résolution `PanelId` -> `QDockWidget*`), sans jamais voler le focus
    /// clavier — factorisé entre `applyPanelFocus` et `applyPixelPanelFocus`.
    void raisePanel(hmi::PanelId panel);
    /// Réassigne le contexte d'édition actif (`_editContext`) et le contexte de la barre d'état
    /// selon le widget qui vient de recevoir le focus clavier (`LOT-54` TACHE-04, `EX-IHM-062`) :
    /// le canevas pixel art si le focus y entre, le niveau sinon.
    void updateActiveEditContext(QWidget* focused);

    // Atelier pixel art : ouvrir/créer/enregistrer (LOT-54 TACHE-05).
    /// @return `true` si l'on peut poursuivre (rien à perdre, ou perte confirmée) — même patron que
    ///         le garde-fou d'ouverture de niveau (`EX-EDIT-021`).
    [[nodiscard]] bool confirmDiscardPixelChanges();
    /// Ouvre un asset existant choisi par l'utilisateur (bibliothèque `Assets/`), après le
    /// garde-fou de perte de travail.
    void openPixelAssetOpenDialog();
    /// Crée un nouvel asset à une taille choisie parmi celles admises par le contrat de sa famille
    /// (`hmi::validAssetSizes`), après le garde-fou de perte de travail.
    void openPixelAssetCreateDialog();
    /// Enregistre l'asset ouvert. @p saveAs force le choix d'un nouveau chemin (copie), même sans
    /// chemin existant ; sans @p saveAs, réutilise le chemin d'ouverture s'il y en a un. Un
    /// écrasement d'asset référencé demande confirmation, nommant les références (`LOT-43`).
    void savePixelAsset(bool saveAs);

    // Palette de projet (LOT-54 TACHE-07).
    /// Recopie les couleurs de `_pixelPalette` vers `_pixelCanvas` (mode contraint) — à appeler
    /// après toute mutation qui change les couleurs ou leur ordre (l'ordre affecte le départage à
    /// distance égale, `hmi::nearestPaletteColor`).
    void syncPaletteToCanvas();
    /// Enregistre `_pixelPalette` dans `Assets/palettes.json`, journalise un échec éventuel.
    void savePixelPalette();
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
    /// Contexte d'édition actif, cible d'Annuler/Refaire/Copier/Coller (`LOT-57` TACHE-04) : `
    /// _viewport` (niveau) ou `_pixelCanvas` (atelier pixel art, `LOT-54` TACHE-04), selon le widget
    /// qui a le focus clavier (`updateActiveEditContext`) — le dispatch lui-même ne change jamais.
    EditContextTarget* _editContext = nullptr;
    PalettePanel* _palette;     ///< Arbre de sélection du type de tuile (contenu du dock Palette).
    LevelBrowserPanel*
        _levels;               ///< Liste/gestion des fichiers de niveaux (contenu du dock Niveaux).
    /// Placement/inspection de décors (dock Décors, `LOT-57` amendement) — contenait déjà tout ce
    /// qui concerne les décors (`ToolPanel`, `LOT-56` TACHE-04) avant d'y accueillir aussi
    /// l'inspecteur déplacé du panneau Textures. La barre d'outils reste hors de ce panneau.
    DecorsPanel* _decors;
    LinkPanel* _links;         ///< Liste/gestion des liaisons de mécanismes (dock Liens, LOT-37).
    TexturePanel* _textures;   ///< Habillage : jeu de skins et assignations (dock Textures, LOT-42).
    /// Canevas de l'atelier pixel art (dock Atelier, LOT-54 TACHE-04) : seconde implémentation de
    /// `EditContextTarget`, cible d'Annuler/Refaire/Copier/Coller quand elle a le focus clavier.
    PixelCanvas* _pixelCanvas;
    PixelHistoryPanel* _pixelHistoryPanel;  ///< Historique visuel de l'atelier (dock, LOT-54 TACHE-04).
    PixelPalettePanel* _pixelPalettePanel;  ///< Édition de la palette de projet (dock, LOT-54 TACHE-07).
    PixelPalette _pixelPalette;  ///< Palette de projet, chargée/enregistrée dans Assets/palettes.json.
    /// Chemin complet du fichier de l'asset ouvert dans l'atelier, vide si aucun ou pas encore
    /// enregistré (LOT-54 TACHE-05) — `PixelCanvas::assetName()` n'en garde que le nom de fichier,
    /// pour l'affichage ; ce chemin sert à `savePixelAsset` pour retrouver le dossier.
    std::filesystem::path _pixelAssetPath;
    EditorActions* _actions;   ///< Outils et commandes principales, barre d'outils (LOT-56 TACHE-04).
    QToolBar* _toolBar;        ///< Barre d'outils de l'éditeur, alimentée par `_actions`.
    QToolBar* _pixelToolBar;   ///< Barre d'outils du canevas pixel art (LOT-54 TACHE-04).
    QMenu* _pixelMenu = nullptr;  ///< Menu « Atelier » : ouvrir/créer/enregistrer (LOT-54 TACHE-05).
    QMenu* _themeMenu;         ///< Sous-menu Affichage > Thème (LOT-56 TACHE-06).
    QAction* _themeSystemAction;
    QAction* _themeLightAction;
    QAction* _themeDarkAction;
    QByteArray _defaultState;  ///< Disposition par défaut (pour « Réinitialiser la disposition »).

    // Regroupement des panneaux de droite en onglets, suivant l'outil actif (LOT-57 TACHE-02).
    QAction* _actFollowActiveTool = nullptr;  ///< Réglage persisté (menu Affichage).
    /// `true` dès que l'utilisateur a choisi un onglet ou déplacé un panneau lui-même : la mise en
    /// avant automatique cesse alors pour la session (jamais persisté, cf. `hmi::panelForTool`).
    bool _userPickedTab = false;

    // Mode d'inspection par calque, déplacé du panneau Textures vers le menu Affichage
    // (LOT-57 TACHE-03) : une entrée cochable par calque, dans l'ordre de dessin, plus « tout
    // afficher ».
    std::array<QAction*, 7> _layerVisibilityActions{};
    QAction* _actShowAllLayers = nullptr;
    /// `true` pendant un changement de visibilité **provoqué par le code** (mise en avant
    /// automatique, bascule de mode, restauration de disposition) : évite qu'un tel changement soit
    /// pris pour un choix explicite de l'utilisateur (`QDockWidget::visibilityChanged`).
    bool _suppressPanelFocusTracking = false;

    // Barre d'état structurée (LOT-57 TACHE-01) : zones permanentes (widgets ajoutés via
    // addPermanentWidget, jamais recouvertes par un message transitoire), dans l'ordre décidé par
    // hmi::editorStatusLines.
    QLabel* _statusLevel = nullptr;
    QLabel* _statusDirty = nullptr;
    QLabel* _statusTool = nullptr;
    QLabel* _statusHover = nullptr;
    QLabel* _statusZoom = nullptr;
    /// Couleur courante de l'atelier pixel art (`LOT-54` TACHE-04) ; vide hors contexte d'atelier.
    QLabel* _statusColor = nullptr;
    /// Restaure l'aide contextuelle à l'expiration d'un message transitoire (`showTransientStatusMessage`).
    QTimer* _statusMessageTimer = nullptr;

    Localization _loc;  ///< Catalogue de traduction (i18n), source de tous les textes.
    core::MemoryLogSink* _sessionLog;  ///< Sink mémoire des logs (nul en Release).

    // Navigation manette des menus (hors jeu) : sondage périodique -> événements clavier Qt.
    GamepadPoller _menuPad;
    InputState _menuPadInput;
    QTimer* _menuNavTimer = nullptr;
};

}  // namespace hmi
