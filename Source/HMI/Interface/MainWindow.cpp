#include "HMI/Interface/MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFontMetrics>
#include <QFormLayout>
#include <QGuiApplication>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QString>
#include <QStyleHints>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <array>
#include <ctime>
#include <filesystem>
#include <vector>

#include "Core/Diagnostics/MemoryLogSink.h"
#include "HMI/Diagnostics/SessionLog.h"
#include "HMI/Editor/EditorStatus.h"
#include "HMI/Editor/LevelBrowserPanel.h"
#include "HMI/Editor/LinkPanel.h"
#include "HMI/Editor/DecorsPanel.h"
#include "HMI/Editor/PanelFocus.h"
#include "HMI/Editor/TexturePanel.h"
#include "HMI/Editor/PalettePanel.h"
#include "HMI/Game/GameViewport.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/GamepadButton.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/EditorActions.h"
#include "HMI/Interface/MainMenu.h"
#include "HMI/Interface/OptionsPage.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "ui_MainWindow.h"

namespace hmi {

namespace {

// Version de la disposition sérialisée : à incrémenter si l'ensemble des docks change, pour
// invalider proprement une disposition sauvegardée devenue incompatible (`restoreState`).
constexpr int LAYOUT_VERSION =
    5;  // 5 : panneau Outils devenu Decors (barre d'outils + inspecteur), Textures sort du
        //     regroupement en onglets (LOT-57, amendement post-essai manuel)

// Clés de persistance (portée application ; l'organisation/appli sont fixées dans `main`,
// HMI/main.cpp).
constexpr char GEOMETRY_KEY[] = "mainWindow/geometry";
constexpr char STATE_KEY[] = "mainWindow/state";
// Réglage de mise en avant automatique des panneaux (LOT-57 TACHE-02) : local à MainWindow, pas
// une extension d'ApplicationTheme.cpp (qui concerne le thème, pas ce comportement).
constexpr char FOLLOW_ACTIVE_TOOL_KEY[] = "panels/followActiveTool";

}  // namespace

MainWindow::MainWindow(core::MemoryLogSink* sessionLog)
    : _ui(std::make_unique<Ui::EditorMainWindow>()),
      _stack(nullptr),
      _menu(nullptr),
      _options(nullptr),
      _editorContainer(nullptr),
      _viewport(new GameViewport()),
      _palette(nullptr),
      _levels(nullptr),
      _decors(nullptr),
      _links(nullptr),
      _textures(nullptr),
      _actions(nullptr),
      _toolBar(nullptr),
      _themeMenu(nullptr),
      _themeSystemAction(nullptr),
      _themeLightAction(nullptr),
      _themeDarkAction(nullptr),
      _loc(hmi::executableDirectory() / "Localization"),
      _sessionLog(sessionLog) {
    _ui->setupUi(this);  // barre de menus + docks (coquilles) depuis MainWindow.ui.

    // Catalogue de traduction : français par défaut (repli), langue active depuis les réglages.
    static_cast<void>(_loc.loadDefaultLanguage("fr"));
    const QString savedLanguage =
        QSettings().value(QStringLiteral("language"), QStringLiteral("fr")).toString();
    if (savedLanguage != QLatin1String("fr")) {
        static_cast<void>(_loc.loadLanguage(savedLanguage.toStdString()));
    }
    _viewport->setLocalization(&_loc);
    _editContext = _viewport;  ///< Seule implémentation aujourd'hui (LOT-57 TACHE-04).

    // `createWindowContainer` embarque la fenêtre native du viewport et en prend la propriété.
    _editorContainer = QWidget::createWindowContainer(_viewport, this);
    _editorContainer->setMinimumSize(320, 240);
    _editorContainer->setFocusPolicy(Qt::StrongFocus);

    // Central : menu principal, options et viewport empilés (remplace le centralHost du .ui).
    _menu = new MainMenu();
    _options =
        new OptionsPage(_viewport, hmi::executableDirectory() / "Settings" / "keybindings.json");
    _stack = new QStackedWidget(this);
    _stack->addWidget(_menu);
    _stack->addWidget(_options);
    _stack->addWidget(_editorContainer);
    setCentralWidget(_stack);

    buildUi();  // contenu des docks (panneaux) + branchement des actions de la barre de menus.

    // Sélectionner une tuile dans la palette définit le type peint au clic dans le viewport.
    connect(_palette, &PalettePanel::tileSelected, _viewport,
            [this](core::TileType type) { _viewport->setActiveTile(type); });
    // Raccourci clavier de l'outil « Texture par instance » (LOT-45, « touche dédiée ») :
    // resynchronise le panneau Décors (visibilité du sélecteur de placement) et la barre d'outils
    // (LOT-56 TACHE-04), sans reboucler (setActiveTool n'émet rien).
    connect(_viewport, &GameViewport::toolChanged, _decors, &DecorsPanel::setActiveTool);
    connect(_viewport, &GameViewport::toolChanged, _actions, &EditorActions::setActiveTool);
    // Les messages d'état du viewport (enregistrement, essai, erreurs) s'affichent en bas, puis
    // laissent la main à l'aide contextuelle (LOT-57 TACHE-01).
    connect(_viewport, &GameViewport::statusMessage, this,
            [this](const QString& message) { showTransientStatusMessage(message, 5000); });
    // Barre d'état : zones permanentes (LOT-57 TACHE-01), recalculées à chaque changement
    // pertinent -- outil, survol, zoom, brouillon (nom, modifications).
    connect(_viewport, &GameViewport::toolChanged, this, [this](hmi::EditorTool) { refreshStatusHelp(); });
    // Mise en avant du panneau pertinent selon l'outil actif (LOT-57 TACHE-02).
    connect(_viewport, &GameViewport::toolChanged, this, &MainWindow::applyPanelFocus);
    connect(_viewport, &GameViewport::hoveredCellChanged, this,
            [this](std::optional<core::GridPosition>) { refreshStatusHelp(); });
    connect(_viewport, &GameViewport::zoomChanged, this, [this](float) { refreshStatusHelp(); });
    // Ouvrir un niveau depuis le panneau : garde-fou des modifications non enregistrées d'abord.
    connect(_levels, &LevelBrowserPanel::levelOpenRequested, this, [this](const QString& path) {
        if (_viewport->isDirty()) {
            const QMessageBox::StandardButton answer = QMessageBox::question(
                this, text("dialog.unsaved_title"), text("dialog.unsaved_text"));
            if (answer != QMessageBox::Yes) {
                return;
            }
        }
        _viewport->openLevel(std::filesystem::path(path.toStdString()));
    });

    // Panneau Liens : reste synchronise avec le brouillon (LOT-37) ; selectionner une ligne
    // surligne la liaison dans le viewport, supprimer delegue au viewport (seul proprietaire).
    // Section « Fond » (LOT-44) : meme synchronisation -- le viewport reste seul proprietaire du
    // brouillon, le panneau ne fait que refleter fond/jeu de skins du niveau courant.
    connect(_viewport, &GameViewport::draftChanged, this, [this] {
        _links->refresh(_viewport->draft());
        _textures->setLevelProperties(_viewport->draft().background(), _viewport->draft().skinSet());
        _textures->refreshObjects(_viewport->draft());
        _decors->refreshDecors(_viewport->draft(), _viewport->selectedDecorIndex());
        refreshStatusHelp();  // nom du niveau et indicateur de modification (LOT-57 TACHE-01).
    });
    connect(_links, &LinkPanel::linkSelected, _viewport, &GameViewport::setHighlightedLink);
    connect(_links, &LinkPanel::deleteRequested, _viewport, &GameViewport::unlinkMechanism);
    _links->refresh(_viewport->draft());  // etat initial (avant tout draftChanged).

    // Section « Objets » (LOT-45) : meme separation que le panneau Liens -- choisir un asset arme
    // l'outil « Texture par instance », la selection d'une ligne surligne dans le viewport, le
    // retrait passe par le viewport (seul proprietaire du brouillon).
    connect(_textures, &TexturePanel::textureOverrideAssetSelected, _viewport,
            [this](const QString& fileName) {
                _viewport->setActiveTextureAsset(
                    fileName.isEmpty() ? std::nullopt : std::make_optional(fileName.toStdString()));
            });
    connect(_textures, &TexturePanel::textureOverrideSelectionChanged, _viewport,
            &GameViewport::setHighlightedTextureOverride);
    connect(_textures, &TexturePanel::textureOverrideRemoveRequested, _viewport,
            &GameViewport::removeTextureOverride);

    // Outil Décor (LOT-49 TACHE-04) : choisir un asset/une couche dans le panneau Décors arme le
    // clic de placement du viewport, même séparation que la section « Objets » ci-dessus.
    connect(_decors, &DecorsPanel::decorAssetSelected, _viewport, [this](const QString& fileName) {
        _viewport->setActiveDecorAsset(
            fileName.isEmpty() ? std::nullopt : std::make_optional(fileName.toStdString()));
    });
    connect(_decors, &DecorsPanel::decorLayerSelected, _viewport, &GameViewport::setActiveDecorLayer);
    connect(_decors, &DecorsPanel::decorSnapToGridChanged, _viewport,
            &GameViewport::setDecorSnapToGrid);
    _textures->refreshObjects(_viewport->draft());  // etat initial (avant tout draftChanged).

    // Inspecteur de décors (LOT-50 TACHE-04, déplacé dans le panneau Décors LOT-57) : sélection
    // croisée avec le canevas -- une seule source (`hmi::GameViewport::selectedDecorIndex`), les
    // deux vues ne font que la refléter. Les actions de la liste (réordonner/changer de
    // couche/supprimer/centrer) passent par les mêmes mutateurs que le canevas, donc annulables.
    connect(_decors, &DecorsPanel::decorSelected, _viewport, &GameViewport::selectDecor);
    connect(_viewport, &GameViewport::decorSelectionChanged, this,
            [this](std::optional<std::size_t> index) {
                _decors->refreshDecors(_viewport->draft(), index);
            });
    connect(_decors, &DecorsPanel::decorForwardRequested, _viewport,
            &GameViewport::bringDecorForward);
    connect(_decors, &DecorsPanel::decorBackwardRequested, _viewport,
            &GameViewport::sendDecorBackward);
    connect(_decors, &DecorsPanel::decorLayerChangeRequested, _viewport,
            &GameViewport::setDecorLayer);
    connect(_decors, &DecorsPanel::decorRemoveRequested, _viewport, &GameViewport::removeDecor);
    connect(_decors, &DecorsPanel::decorCenterRequested, _viewport,
            &GameViewport::centerCameraOnDecor);
    _decors->refreshDecors(_viewport->draft(), _viewport->selectedDecorIndex());

    // Panneau Textures : agit sur le catalogue dont le viewport est proprietaire, et lui signale
    // le jeu courant. Aucune scene n'est reconstruite -- l'apparence est resolue a la composition,
    // donc l'image suivante suffit a montrer le resultat (LOT-42).
    _textures->setCatalog(&_viewport->skinCatalog());
    _textures->setLevelProperties(_viewport->draft().background(), _viewport->draft().skinSet());

    // Section « Fond » (LOT-44) : les deux modifications passent par le viewport (seul
    // proprietaire du brouillon), exactement comme le panneau Liens ci-dessus.
    connect(_textures, &TexturePanel::backgroundChanged, _viewport, [this](const QString& name) {
        _viewport->setLevelBackground(name.isEmpty() ? std::nullopt
                                                      : std::make_optional(name.toStdString()));
    });
    connect(_textures, &TexturePanel::levelSkinSetChanged, _viewport, [this](const QString& name) {
        _viewport->setLevelSkinSet(name.isEmpty() ? std::nullopt
                                                   : std::make_optional(name.toStdString()));
    });

    // Palette fidele au canevas (EX-EDIT-027) : elle interroge le MEME catalogue, et se rafraichit
    // aux trois evenements qui rendent ses vignettes obsoletes -- bascule de mode, changement de
    // jeu, reassignation. Peindre sans voir ce que l'on pose serait une regression d'usage.
    _palette->setSkinSource(hmi::executableDirectory() / "Assets" / "Skins",
                            &_viewport->skinCatalog());
    _palette->refreshThumbnails(_viewport->renderMode(), _textures->currentSet());

    connect(_textures, &TexturePanel::assignmentsChanged, this, [this] {
        _viewport->setSkinSet(_textures->currentSet());
        _palette->refreshThumbnails(_viewport->renderMode(), _textures->currentSet());
    });
    connect(_viewport, &GameViewport::renderModeChanged, this,
            [this](RenderMode mode) { _palette->refreshThumbnails(mode, _textures->currentSet()); });

    // Rechargement a chaud (LOT-43 TACHE-03) : un asset modifie/renomme/ajoute hors de
    // l'application n'est repris qu'a la demande explicite -- une surveillance automatique de
    // dossier a ete ecartee (editeurs d'image externes ecrivant en plusieurs passes, risque de
    // recharger un fichier partiellement ecrit). Invalider le TextureCache PUIS vider les caches
    // de vignettes, dans cet ordre : les vignettes redecoderont depuis un cache deja purge.
    connect(_textures, &TexturePanel::reloadRequested, this, [this] {
        _viewport->reloadAssets();
        _textures->reloadAssets();
        _decors->reloadDecorThumbnails();  // LOT-57 : vignettes de l'inspecteur de decors.
        _palette->clearThumbnailCache();
        _palette->refreshThumbnails(_viewport->renderMode(), _textures->currentSet());
        showTransientStatusMessage(text("textures.reload_done"), 3000);
    });

    // Navigation depuis le menu principal.
    connect(_menu, &MainMenu::editorRequested, this, &MainWindow::showEditor);
    connect(_menu, &MainMenu::playRequested, this, &MainWindow::showGame);
    connect(_menu, &MainMenu::optionsRequested, this, &MainWindow::showOptions);
    connect(_menu, &MainMenu::quitRequested, this, &MainWindow::close);
    // Retour au menu à la fin d'une partie (ou Échap en mode jeu).
    connect(_viewport, &GameViewport::exitToMenuRequested, this, &MainWindow::showMenu);
    // Page Options : retour au menu, bascule plein écran, changement de langue, sauvegarde des
    // logs.
    connect(_options, &OptionsPage::backRequested, this, &MainWindow::showMenu);
    connect(_options, &OptionsPage::fullscreenRequested, this,
            [this](bool enabled) { enabled ? showFullScreen() : showNormal(); });
    connect(_options, &OptionsPage::languageChanged, this, &MainWindow::changeLanguage);
    connect(_options, &OptionsPage::saveLogsRequested, this, &MainWindow::saveSessionLogs);
    // Un remappage d'editeur (onglet Options > Éditeur, LOT-57 TACHE-04) doit se refleter
    // immediatement sur les raccourcis effectifs des actions (menu/barre d'outils).
    connect(_options, &OptionsPage::editorBindingsChanged, this,
            [this] { _actions->applyShortcuts(_viewport->editorBindings(), _loc); });

    // Navigation manette des menus : sondage périodique, actif seulement hors jeu/édition.
    _menuNavTimer = new QTimer(this);
    _menuNavTimer->setInterval(150);
    connect(_menuNavTimer, &QTimer::timeout, this, &MainWindow::pollMenuGamepad);

    resize(1280, 720);
    retranslateUi();  // applique la langue active à tous les textes construits ci-dessus.

    // Capture la disposition par défaut (après création des docks, avant restauration d'une
    // éventuelle disposition sauvegardée) : sert de cible à « Réinitialiser la disposition ».
    _defaultState = saveState(LAYOUT_VERSION);
    restoreLayout();

    showMenu();  // l'application démarre sur le menu principal.
}

void MainWindow::setDocksVisible(bool visible) {
    // TOUS les docks, retrouves dynamiquement, plutot qu'une liste ecrite a la main : celle-ci
    // laissait echapper silencieusement chaque dock ajoute ensuite, qui restait alors affiche
    // par-dessus le menu principal et le jeu (constate avec le dock « Textures » du LOT-42).
    // Les panneaux d'edition n'ont de sens qu'en mode edition (EX-IHM-010).
    // Bascule de mode (edition/jeu/menu), pas un choix d'onglet : ne doit pas etre pris pour un
    // "l'utilisateur a impose un panneau" (LOT-57 TACHE-02).
    _suppressPanelFocusTracking = true;
    for (QDockWidget* const dock : findChildren<QDockWidget*>()) {
        dock->setVisible(visible);
    }
    _suppressPanelFocusTracking = false;
}

void MainWindow::showMenu() {
    HMI_LOG_INFO("Navigation : menu principal.");
    _stack->setCurrentWidget(_menu);
    setDocksVisible(false);
    menuBar()->setVisible(false);  // pas de barre de menu sur l'écran d'accueil
    _toolBar->setVisible(false);
    _actions->setEditingCommandsEnabled(false);
    _statusMessageTimer->stop();
    refreshStatusHelp();  // hors édition : zones et aide vides (aucun résidu d'état d'édition).
    setMenuGamepadActive(true);
}

void MainWindow::showEditor() {
    HMI_LOG_INFO("Navigation : editeur.");
    _stack->setCurrentWidget(_editorContainer);
    setDocksVisible(true);
    menuBar()->setVisible(true);
    _toolBar->setVisible(true);
    _actions->setEditingCommandsEnabled(true);
    _statusMessageTimer->stop();
    refreshStatusHelp();
    _editorContainer->setFocus();
    setMenuGamepadActive(false);
}

void MainWindow::showGame() {
    HMI_LOG_INFO("Navigation : jeu.");
    setMenuGamepadActive(false);
    _stack->setCurrentWidget(_editorContainer);
    setDocksVisible(false);
    menuBar()->setVisible(false);
    _toolBar->setVisible(false);
    _actions->setEditingCommandsEnabled(false);
    _statusMessageTimer->stop();
    refreshStatusHelp();  // jeu : menuBar masquee -> contexte de niveau absent (pas de residu).
    _editorContainer->setFocus();

    // Séquence de niveaux démo (même ordre que le jeu historique) — Échap ou la fin revient au
    // menu.
    const std::filesystem::path levels = hmi::executableDirectory() / "Levels";
    _viewport->startGame({
        levels / "demo-deplacement.json",
        levels / "demo-saut.json",
        levels / "demo-double-saut.json",
        levels / "demo-wall-jump.json",
        levels / "demo-dash.json",
        levels / "demo-interrupteur.json",
        levels / "demo-plaque-pression.json",
        levels / "demo-bloc.json",
        levels / "demo-budget.json",
        levels / "demo-pente.json",
        levels / "demo-arrondi.json",
        levels / "demo-bloc-reduit.json",
        levels / "demo-dangers-avances.json",
        levels / "demo-final.json",
        levels / "demo-salles.json",
    });
}

void MainWindow::showOptions() {
    HMI_LOG_INFO("Navigation : options.");
    _stack->setCurrentWidget(_options);
    setDocksVisible(false);
    menuBar()->setVisible(false);
    _toolBar->setVisible(false);
    _actions->setEditingCommandsEnabled(false);
    _statusMessageTimer->stop();
    refreshStatusHelp();
    setMenuGamepadActive(true);
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    // Outils et commandes principales (LOT-56 TACHE-04) : une action unique par commande, partagée
    // entre la barre d'outils, le menu et son raccourci (plus de double définition). Icônes
    // construites depuis le thème d'éditeur actuellement effectif ; régénérées par
    // `EditorActions::refreshIcons` lors d'un changement de thème (TACHE-06).
    _actions = new EditorActions(hmi::currentEditorTokens(), this);
    // Raccourcis effectifs synchronises depuis les touches d'editeur remappables (LOT-57 TACHE-04) :
    // ActionCatalog reste sans dependance Qt (valeurs par defaut litterales), c'est ici que le
    // raccourci REELLEMENT actif est branche sur EditorKeyBindings.
    _actions->applyShortcuts(_viewport->editorBindings(), _loc);
    // Barre d'outils de l'éditeur : reste globale à la fenêtre principale, hors du panneau Décors
    // (LOT-57, amendement) -- une seule définition (`EditorActions`), un seul ancrage.
    _toolBar = addToolBar(QStringLiteral("EditorToolBar"));
    _toolBar->setObjectName(QStringLiteral("EditorToolBar"));
    _toolBar->setMovable(false);
    _actions->populateToolBar(*_toolBar);

    // Contenu des docks : les coquilles (`PalettePanel`/`DecorsPanel`/`LevelsPanel`) et leur
    // agencement viennent du `.ui` ; leurs widgets, paramétrés (chemins, dépendances), sont créés
    // en code.
    _palette = new PalettePanel(_ui->PalettePanel);
    _ui->PalettePanel->setWidget(_palette);
    // Panneau Décors (LOT-57, amendement) : placement + inspecteur, regroupés -- l'ancien panneau
    // « Outils » ne portait déjà plus que le décor (LOT-56 TACHE-04).
    _decors = new DecorsPanel(hmi::executableDirectory() / "Assets" / "Decors", _ui->DecorsPanel);
    _ui->DecorsPanel->setWidget(_decors);
    _levels = new LevelBrowserPanel(hmi::executableDirectory() / "Levels", _ui->LevelsPanel);
    _ui->LevelsPanel->setWidget(_levels);
    _links = new LinkPanel(_ui->LinksPanel);
    _ui->LinksPanel->setWidget(_links);
    // Panneau d'habillage (LOT-42) : écrit `skins.json` au chemin **déployé**, exactement comme
    // l'enregistrement d'un niveau — aucun nouveau mécanisme d'écriture.
    _textures = new TexturePanel(hmi::executableDirectory() / "Assets" / "Skins",
                                 hmi::executableDirectory() / "Assets" / "skins.json",
                                 hmi::executableDirectory() / "Assets" / "Backgrounds",
                                 hmi::executableDirectory() / "Assets" / "Objects", _ui->TexturesPanel);
    _ui->TexturesPanel->setWidget(_textures);

    // Regroupement par defaut des panneaux Niveaux/Liens en onglets (LOT-57 TACHE-02) : chacun
    // reste individuellement deplacable/detachable/refermable (EX-IHM-010), seule la disposition
    // par defaut change. Textures redevient un dock independant, comme Palette/Decors (LOT-57,
    // amendement post-essai manuel). Doit preceder la capture de _defaultState (constructeur,
    // apres buildUi()).
    tabifyDockWidget(_ui->LevelsPanel, _ui->LinksPanel);
    // Un changement de visibilite d'un de ces docks NON provoque par notre propre code (mise en
    // avant, bascule de mode, restauration de disposition -- toutes gardees par
    // _suppressPanelFocusTracking) ne peut venir que d'un choix explicite de l'utilisateur : cliquer
    // un onglet ou fermer/rouvrir le panneau. Meme principe pour un detachement (topLevelChanged),
    // toujours explicite, jamais gardee.
    for (QDockWidget* const dock : {_ui->LevelsPanel, _ui->LinksPanel}) {
        connect(dock, &QDockWidget::visibilityChanged, this, [this](bool) {
            if (!_suppressPanelFocusTracking) {
                _userPickedTab = true;
            }
        });
        connect(dock, &QDockWidget::topLevelChanged, this, [this](bool) { _userPickedTab = true; });
    }

    for (const hmi::EditorTool tool : {hmi::EditorTool::Paint, hmi::EditorTool::Rectangle,
                                       hmi::EditorTool::Selection, hmi::EditorTool::Link,
                                       hmi::EditorTool::TextureAssign, hmi::EditorTool::Decor}) {
        connect(_actions->toolAction(tool), &QAction::toggled, _viewport, [this, tool](bool on) {
            if (on) {
                _viewport->setTool(tool);
            }
        });
    }
    connect(_actions->action(hmi::IconId::Save), &QAction::triggered, _viewport,
            [this] { _viewport->save(); });
    connect(_actions->action(hmi::IconId::Playtest), &QAction::triggered, _viewport,
            [this] { _viewport->startPlaytest(); });
    // Annuler/Refaire/Copier/Coller dispatchent via le contexte d'edition actif (_editContext,
    // GameViewport aujourd'hui) plutot que directement sur _viewport : le seuil de dispatch que
    // LOT-54 reutilisera pour sa propre cible (LOT-57 TACHE-04, EX-IHM-062).
    connect(_actions->action(hmi::IconId::Undo), &QAction::triggered, this,
            [this] { _editContext->undo(); });
    connect(_actions->action(hmi::IconId::Redo), &QAction::triggered, this,
            [this] { _editContext->redo(); });
    connect(_actions->action(hmi::IconId::Copy), &QAction::triggered, this,
            [this] { _editContext->copy(); });
    connect(_actions->action(hmi::IconId::Paste), &QAction::triggered, this,
            [this] { _editContext->paste(); });
    connect(_actions->action(hmi::IconId::ToggleGrid), &QAction::triggered, _viewport,
            [this] { _viewport->toggleGrid(); });
    connect(_actions->action(hmi::IconId::ResetCamera), &QAction::triggered, _viewport,
            [this] { _viewport->resetCamera(); });
    connect(_actions->action(hmi::IconId::ToggleRenderMode), &QAction::triggered, _viewport,
            [this] { _viewport->toggleRenderMode(); });
    // Renommer le niveau ouvert (LOT-57 TACHE-04) : meme dialogue que LevelBrowserPanel::onRename,
    // pre-rempli du nom courant.
    connect(_actions->action(hmi::IconId::Rename), &QAction::triggered, this, [this] {
        bool accepted = false;
        const QString name = QInputDialog::getText(
            this, text("level.rename"), text("level.rename_prompt"), QLineEdit::Normal,
            QString::fromStdString(_viewport->draft().name()), &accepted);
        if (!accepted || name.isEmpty()) {
            return;
        }
        if (_viewport->renameOpenLevel(name.toStdString())) {
            _levels->refresh();  // le fichier a pu changer de nom dans le dossier liste.
        }
    });
    // Aperçu des raccourcis (LOT-57 TACHE-04, concretise EX-EDIT-015) : lit les raccourcis EFFECTIFS
    // des actions a l'ouverture, jamais un texte fige -- toujours a jour apres un remappage.
    connect(_actions->action(hmi::IconId::ShortcutsOverview), &QAction::triggered, this, [this] {
        QDialog dialog(this);
        dialog.setWindowTitle(text("dialog.shortcuts_title"));
        auto* const layout = new QVBoxLayout(&dialog);
        for (const hmi::EditorActionSpec& spec : hmi::editorActionCatalog()) {
            QAction* const act = _actions->action(spec.id);
            if (act->shortcut().isEmpty()) {
                continue;
            }
            layout->addWidget(new QLabel(
                act->text() + QStringLiteral(" — ") + act->shortcut().toString(QKeySequence::NativeText),
                &dialog));
        }
        auto* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        dialog.exec();
    });

    // Commandes principales egalement dans le menu "Niveau" (decouvrabilite, EX-EDIT-015) : les
    // memes actions que la barre d'outils, aucune seconde definition.
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Save));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Playtest));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Undo));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Redo));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Copy));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Paste));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::ToggleGrid));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::ResetCamera));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::ToggleRenderMode));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Rename));
    _ui->levelMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::ShortcutsOverview));
    _ui->levelMenu->insertSeparator(_ui->actResize);

    // Branchement du fonctionnel sur les actions restantes, déclarées dans le `.ui`.
    connect(_ui->actMainMenu, &QAction::triggered, this, &MainWindow::showMenu);
    connect(_ui->actQuit, &QAction::triggered, this, &MainWindow::close);
    connect(_ui->actResize, &QAction::triggered, this, [this] { openResizeDialog(); });
    connect(_ui->actResetLayout, &QAction::triggered, this, [this] {
        _suppressPanelFocusTracking = true;
        restoreState(_defaultState, LAYOUT_VERSION);
        _suppressPanelFocusTracking = false;
        _userPickedTab = false;  // repart sur la mise en avant automatique, disposition remise a neuf.
    });

    // Thème clair/sombre de l'éditeur (LOT-56 TACHE-06) : réglage Système/Clair/Sombre, persisté,
    // sans effet sur l'identité du jeu (menu principal/Options), qui reste toujours sombre.
    _themeMenu = new QMenu(this);
    _themeSystemAction = _themeMenu->addAction(QString());
    _themeLightAction = _themeMenu->addAction(QString());
    _themeDarkAction = _themeMenu->addAction(QString());
    auto* const themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);
    for (QAction* const act : {_themeSystemAction, _themeLightAction, _themeDarkAction}) {
        act->setCheckable(true);
        act->setActionGroup(themeGroup);
    }
    switch (hmi::editorThemeSetting()) {
        case hmi::EditorThemeSetting::Light:
            _themeLightAction->setChecked(true);
            break;
        case hmi::EditorThemeSetting::Dark:
            _themeDarkAction->setChecked(true);
            break;
        case hmi::EditorThemeSetting::System:
            _themeSystemAction->setChecked(true);
            break;
    }
    // Re-genere palette + feuille de style + icones depuis le theme desormais effectif : les seuls
    // elements qui suivent les jetons de couleur (les vignettes d'assets, elles, sont un contenu
    // de jeu independant du thème de l'IHM, cf. epic.md).
    const auto applyThemeSetting = [this](hmi::EditorThemeSetting setting) {
        hmi::setEditorThemeSetting(setting);
        hmi::reapplyEditorTheme();
        _actions->refreshIcons(hmi::currentEditorTokens());
    };
    connect(_themeSystemAction, &QAction::triggered, this,
            [applyThemeSetting] { applyThemeSetting(hmi::EditorThemeSetting::System); });
    connect(_themeLightAction, &QAction::triggered, this,
            [applyThemeSetting] { applyThemeSetting(hmi::EditorThemeSetting::Light); });
    connect(_themeDarkAction, &QAction::triggered, this,
            [applyThemeSetting] { applyThemeSetting(hmi::EditorThemeSetting::Dark); });
    // Reglage "Systeme" : reagit a un changement live du theme du systeme d'exploitation.
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
            [this](Qt::ColorScheme) {
                if (hmi::editorThemeSetting() == hmi::EditorThemeSetting::System) {
                    hmi::reapplyEditorTheme();
                    _actions->refreshIcons(hmi::currentEditorTokens());
                }
            });
    _ui->viewMenu->insertMenu(_ui->actResetLayout, _themeMenu);
    _ui->viewMenu->insertSeparator(_ui->actResetLayout);

    // Bascules de visibilité des docks (dynamiques) : insérées avant « Réinitialiser la disposition
    // ».
    _ui->viewMenu->insertAction(_ui->actResetLayout, _ui->PalettePanel->toggleViewAction());
    _ui->viewMenu->insertAction(_ui->actResetLayout, _ui->LevelsPanel->toggleViewAction());
    _ui->viewMenu->insertAction(_ui->actResetLayout, _ui->DecorsPanel->toggleViewAction());
    _ui->viewMenu->insertAction(_ui->actResetLayout, _ui->LinksPanel->toggleViewAction());
    _ui->viewMenu->insertAction(_ui->actResetLayout, _ui->TexturesPanel->toggleViewAction());
    _ui->viewMenu->insertSeparator(_ui->actResetLayout);

    // Mise en avant automatique des panneaux de droite selon l'outil actif (LOT-57 TACHE-02) :
    // reglage persiste, actif par defaut.
    _actFollowActiveTool = new QAction(this);
    _actFollowActiveTool->setCheckable(true);
    _actFollowActiveTool->setChecked(
        QSettings().value(QString::fromLatin1(FOLLOW_ACTIVE_TOOL_KEY), true).toBool());
    connect(_actFollowActiveTool, &QAction::toggled, this, [](bool enabled) {
        QSettings().setValue(QString::fromLatin1(FOLLOW_ACTIVE_TOOL_KEY), enabled);
    });
    _ui->viewMenu->insertAction(_ui->actResetLayout, _actFollowActiveTool);
    _ui->viewMenu->insertSeparator(_ui->actResetLayout);

    // Mode d'inspection par calque (LOT-57 TACHE-03) : deplace depuis l'onglet Calques du panneau
    // Textures -- DECOMPOSE le rendu pour auditer chaque calque, jamais lu par hmi::GameSession, a
    // l'inverse de F8 qui le COMPOSE tel que le joueur le verra (EX-REN-046, la bascule Physique/
    // Texture est traitee en TACHE-04). Cases dans l'ORDRE DE DESSIN (hmi::RenderLayer,
    // EX-REN-014), toutes cochees par defaut, jamais persistees entre deux sessions.
    constexpr std::array<hmi::RenderLayer, 7> LAYER_ORDER{
        hmi::RenderLayer::Background, hmi::RenderLayer::Decor,  hmi::RenderLayer::Shadow,
        hmi::RenderLayer::Tile,       hmi::RenderLayer::Object, hmi::RenderLayer::Player,
        hmi::RenderLayer::Foreground};
    for (std::size_t i = 0; i < LAYER_ORDER.size(); ++i) {
        const hmi::RenderLayer layer = LAYER_ORDER[i];
        QAction* const act = new QAction(this);
        act->setCheckable(true);
        act->setChecked(true);
        connect(act, &QAction::toggled, _viewport,
                [this, layer](bool checked) { _viewport->setLayerVisible(layer, checked); });
        _ui->viewMenu->insertAction(_ui->actResetLayout, act);
        _layerVisibilityActions[i] = act;
    }
    _actShowAllLayers = new QAction(this);
    connect(_actShowAllLayers, &QAction::triggered, this, [this] {
        _viewport->showAllLayers();
        // showAllLayers() n'emet pas de signal par calque : resynchronise les cases sans
        // redeclencher setLayerVisible sept fois (deja tout affiche cote rendu).
        for (QAction* const act : _layerVisibilityActions) {
            const QSignalBlocker blocker(act);
            act->setChecked(true);
        }
    });
    _ui->viewMenu->insertAction(_ui->actResetLayout, _actShowAllLayers);
    // Bascule Physique/Texture (LOT-57 TACHE-04) : l'action existe déjà (IconId::ToggleRenderMode,
    // LOT-56, toolbar + menu Niveau + F8) -- lui donner une présence ICI, à côté du mode
    // d'inspection par calque, remplace la case "Physique seul" retirée en TACHE-03 (même état,
    // une seule définition désormais, EX-IHM-062) sans en créer une seconde.
    _ui->viewMenu->insertAction(_ui->actResetLayout, _actions->action(hmi::IconId::ToggleRenderMode));
    _ui->viewMenu->insertSeparator(_ui->actResetLayout);

    // Barre d'état structurée (LOT-57 TACHE-01) : zones permanentes, ajoutées via
    // addPermanentWidget -- jamais recouvertes par un message transitoire (showMessage), à
    // l'inverse de l'ancienne chaîne unique `status.edit_help`. Largeur minimale sur les zones qui
    // changent au survol (case, zoom) : sans elle, la barre "saute" à chaque déplacement de souris.
    _statusLevel = new QLabel(this);
    _statusDirty = new QLabel(this);
    _statusTool = new QLabel(this);
    _statusHover = new QLabel(this);
    _statusHover->setMinimumWidth(fontMetrics().horizontalAdvance(QStringLiteral("(999, 999)")));
    _statusZoom = new QLabel(this);
    _statusZoom->setMinimumWidth(fontMetrics().horizontalAdvance(QStringLiteral("Zoom : 999%")));
    for (QLabel* const zone : {_statusLevel, _statusDirty, _statusTool, _statusHover, _statusZoom}) {
        statusBar()->addPermanentWidget(zone);
    }
    _statusMessageTimer = new QTimer(this);
    _statusMessageTimer->setSingleShot(true);
    connect(_statusMessageTimer, &QTimer::timeout, this, &MainWindow::refreshStatusHelp);
}

void MainWindow::openResizeDialog() {
    constexpr int MAX_DIMENSION = 100;  // plafond de taille de niveau (EX-EDIT-017).

    QDialog dialog(this);
    dialog.setWindowTitle(text("dialog.resize_title"));

    auto* const widthSpin = new QSpinBox(&dialog);
    widthSpin->setRange(1, MAX_DIMENSION);
    widthSpin->setValue(_viewport->levelWidth());
    auto* const heightSpin = new QSpinBox(&dialog);
    heightSpin->setRange(1, MAX_DIMENSION);
    heightSpin->setValue(_viewport->levelHeight());

    auto* const form = new QFormLayout(&dialog);
    form->addRow(text("dialog.width"), widthSpin);
    form->addRow(text("dialog.height"), heightSpin);
    auto* const buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const int width = widthSpin->value();
    const int height = heightSpin->value();
    // Confirmation si le redimensionnement supprimerait du contenu déjà posé (EX-EDIT-012).
    if (_viewport->wouldResizeDrop(width, height)) {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this, text("dialog.resize_title"), text("dialog.resize_drop").arg(width).arg(height));
        if (answer != QMessageBox::Yes) {
            return;
        }
    }
    _viewport->resizeLevel(width, height);
}

void MainWindow::restoreLayout() {
    const QSettings settings;
    const QByteArray geometry = settings.value(QString::fromLatin1(GEOMETRY_KEY)).toByteArray();
    const QByteArray state = settings.value(QString::fromLatin1(STATE_KEY)).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    if (!state.isEmpty()) {
        _suppressPanelFocusTracking = true;
        restoreState(state, LAYOUT_VERSION);
        _suppressPanelFocusTracking = false;
    }
}

void MainWindow::saveLayout() {
    QSettings settings;
    settings.setValue(QString::fromLatin1(GEOMETRY_KEY), saveGeometry());
    settings.setValue(QString::fromLatin1(STATE_KEY), saveState(LAYOUT_VERSION));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveLayout();
    QMainWindow::closeEvent(event);
}

void MainWindow::setMenuGamepadActive(bool active) {
    if (active) {
        _menuNavTimer->start();
    } else {
        _menuNavTimer->stop();
    }
}

void MainWindow::pollMenuGamepad() {
    _menuPad.poll(_menuPadInput);

    // Poste un événement clavier Qt à la cible focalisée (repli : page courante de la pile), pour
    // que la manette pilote la **navigation de focus** standard de Qt sans code de layout dédié.
    QWidget* const target = QApplication::focusWidget() != nullptr ? QApplication::focusWidget()
                                                                   : _stack->currentWidget();
    const auto post = [target](Qt::Key key, Qt::KeyboardModifiers mods) {
        if (target == nullptr) {
            return;
        }
        QApplication::postEvent(target, new QKeyEvent(QEvent::KeyPress, key, mods));
        QApplication::postEvent(target, new QKeyEvent(QEvent::KeyRelease, key, mods));
    };

    // Bas/Droite -> focus suivant ; Haut/Gauche -> focus précédent ; A -> activer.
    if (_menuPadInput.gamepadButtonPressed(GamepadButton::Down) ||
        _menuPadInput.gamepadButtonPressed(GamepadButton::Right)) {
        post(Qt::Key_Tab, Qt::NoModifier);
    }
    if (_menuPadInput.gamepadButtonPressed(GamepadButton::Up) ||
        _menuPadInput.gamepadButtonPressed(GamepadButton::Left)) {
        post(Qt::Key_Backtab, Qt::ShiftModifier);
    }
    if (_menuPadInput.gamepadButtonPressed(GamepadButton::A)) {
        post(Qt::Key_Return, Qt::NoModifier);
    }
    // B : retour contextuel (depuis Options vers le menu), sans quitter depuis le menu principal.
    if (_menuPadInput.gamepadButtonPressed(GamepadButton::B) &&
        _stack->currentWidget() == _options) {
        showMenu();
    }

    _menuPadInput.beginFrame();
}

QString MainWindow::text(const char* key) const {
    return QString::fromStdString(_loc.text(key));
}

void MainWindow::refreshStatusHelp() {
    EditorStatusContext context;
    // Contexte de niveau seulement en édition (pas en jeu/essai ni au menu/Options) : même
    // condition que l'ancien rechargement de `status.edit_help` en changement de langue.
    if (_stack->currentWidget() == _editorContainer && menuBar()->isVisible()) {
        LevelStatusInfo level;
        level.name = _viewport->draft().name();
        level.dirty = _viewport->isDirty();
        level.tool = _viewport->activeTool();
        level.hoveredCell = _viewport->hoveredCell();
        level.zoom = _viewport->zoom();
        context.level = level;
    }
    const EditorStatusLines lines = editorStatusLines(context, _loc);
    _statusLevel->setText(QString::fromStdString(lines.permanent[0]));
    _statusDirty->setText(QString::fromStdString(lines.permanent[1]));
    _statusTool->setText(QString::fromStdString(lines.permanent[2]));
    _statusHover->setText(QString::fromStdString(lines.permanent[3]));
    _statusZoom->setText(QString::fromStdString(lines.permanent[4]));
    statusBar()->showMessage(QString::fromStdString(lines.help));
}

void MainWindow::showTransientStatusMessage(const QString& message, int timeoutMs) {
    // Timeout 0 (par defaut de showMessage) : le message reste affiche jusqu'a restauration
    // explicite par _statusMessageTimer, plutot que d'etre vide silencieusement par Qt (defaut
    // corrige, LOT-57 TACHE-01).
    statusBar()->showMessage(message);
    _statusMessageTimer->start(timeoutMs);
}

void MainWindow::applyPanelFocus(hmi::EditorTool tool) {
    if (!_actFollowActiveTool->isChecked() || _userPickedTab) {
        return;  // reglage desactive, ou l'utilisateur a deja impose un onglet pour la session.
    }
    const std::optional<hmi::PanelId> panel = hmi::panelForTool(tool);
    if (!panel) {
        return;  // cet outil n'a pas de panneau dedie (ex. Decor : panneau Outils, jamais masque).
    }
    QDockWidget* dock = nullptr;
    switch (*panel) {
        case hmi::PanelId::Levels:
            dock = _ui->LevelsPanel;
            break;
        case hmi::PanelId::Links:
            dock = _ui->LinksPanel;
            break;
        case hmi::PanelId::Textures:
            dock = _ui->TexturesPanel;
            break;
    }
    // raise() met l'onglet au premier plan sans voler le focus clavier au canevas -- une
    // suggestion, jamais une confiscation (ligne rouge de cette tache).
    _suppressPanelFocusTracking = true;
    dock->raise();
    _suppressPanelFocusTracking = false;
}

void MainWindow::retranslateUi() {
    setWindowTitle(text("window.title"));

    // Panneaux dockables (les actions « toggle » du menu Affichage suivent le titre du dock).
    _ui->PalettePanel->setWindowTitle(text("dock.palette"));
    _ui->DecorsPanel->setWindowTitle(text("dock.decors"));
    _ui->LevelsPanel->setWindowTitle(text("dock.levels"));
    _ui->LinksPanel->setWindowTitle(text("dock.links"));
    _ui->TexturesPanel->setWindowTitle(text("dock.textures"));

    // Barre de menus.
    _ui->appMenu->setTitle(text("menubar.application"));
    _ui->actMainMenu->setText(text("menubar.main_menu"));
    _ui->actQuit->setText(text("menubar.quit"));
    _ui->levelMenu->setTitle(text("menubar.level"));
    _ui->actResize->setText(text("menubar.resize"));
    _ui->viewMenu->setTitle(text("menubar.view"));
    _themeMenu->setTitle(text("menubar.theme"));
    _themeSystemAction->setText(text("menubar.theme_system"));
    _themeLightAction->setText(text("menubar.theme_light"));
    _themeDarkAction->setText(text("menubar.theme_dark"));
    _actFollowActiveTool->setText(text("menubar.follow_active_tool"));
    static constexpr const char* LAYER_ACTION_KEYS[] = {
        "menubar.layer_background", "menubar.layer_decor_background", "menubar.layer_shadow",
        "menubar.layer_tile_skin",  "menubar.layer_objects",           "menubar.layer_player",
        "menubar.layer_decor_foreground"};
    for (std::size_t i = 0; i < _layerVisibilityActions.size(); ++i) {
        _layerVisibilityActions[i]->setText(text(LAYER_ACTION_KEYS[i]));
    }
    _actShowAllLayers->setText(text("menubar.layer_show_all"));
    _ui->actResetLayout->setText(text("menubar.reset_layout"));
    _actions->retranslateUi(_loc);

    // Panneaux et pages (chacun retraduit son propre contenu depuis le catalogue).
    _menu->retranslateUi(_loc);
    _options->retranslateUi(_loc);
    _palette->retranslateUi(_loc);
    _decors->retranslateUi(_loc);
    _levels->retranslateUi(_loc);
    _links->retranslateUi(_loc);
    _textures->retranslateUi(_loc);

    // Recalcule la barre d'état dans la nouvelle langue (zones + aide) : un changement de langue ne
    // doit pas rester sur une aide figée dans l'ancienne (LOT-57 TACHE-01).
    refreshStatusHelp();
}

void MainWindow::changeLanguage(const QString& code) {
    if (code.toStdString() == _loc.activeLanguage()) {
        return;
    }
    if (!_loc.loadLanguage(code.toStdString())) {
        HMI_LOG_WARNING("Langue introuvable, conservee : " + code.toStdString());
        return;
    }
    QSettings().setValue(QStringLiteral("language"), code);
    HMI_LOG_INFO("Langue changee : " + code.toStdString());
    retranslateUi();
}

void MainWindow::saveSessionLogs() {
    if (_sessionLog == nullptr) {
        showTransientStatusMessage(text("options.logs_unavailable"), 5000);
        return;
    }
    // Nom de fichier horodaté, à côté de l'exécutable (dossier Logs créé au besoin).
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    char stamp[32] = {};
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &local);
    const std::filesystem::path path =
        hmi::executableDirectory() / "Logs" / (std::string("session_") + stamp + ".log");

    if (hmi::saveSessionLog(_sessionLog->entries(), path)) {
        HMI_LOG_INFO("Journaux de session enregistres : " + path.string());
        showTransientStatusMessage(
            text("options.logs_saved").arg(QString::fromStdString(path.filename().string())), 5000);
    } else {
        HMI_LOG_ERROR("Echec de l'enregistrement des journaux : " + path.string());
        showTransientStatusMessage(text("options.logs_failed"), 5000);
    }
}

}  // namespace hmi
