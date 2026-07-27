#include "Editor/MainWindow.h"

#include <vector>

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFormLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QString>
#include <QWidget>

#include <filesystem>

#include "Editor/GameViewport.h"
#include "Editor/LevelBrowserPanel.h"
#include "Editor/MainMenu.h"
#include "Editor/OptionsPage.h"
#include "Editor/PalettePanel.h"
#include "Editor/ToolPanel.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace editor {

namespace {

// Version de la disposition sérialisée : à incrémenter si l'ensemble des docks change, pour
// invalider proprement une disposition sauvegardée devenue incompatible (`restoreState`).
constexpr int LAYOUT_VERSION = 1;

// Clés de persistance (portée application ; l'organisation/appli sont fixées dans `main_qt`).
constexpr char GEOMETRY_KEY[] = "mainWindow/geometry";
constexpr char STATE_KEY[] = "mainWindow/state";

// Crée un panneau docké nommé, au contenu provisoire (rempli aux tâches suivantes).
[[nodiscard]] QDockWidget* makePanel(const QString& title, const QString& objectName,
                                     const QString& placeholder) {
    auto* const panel = new QDockWidget(title);
    panel->setObjectName(objectName);  // indispensable à saveState/restoreState.
    auto* const content = new QLabel(placeholder);
    content->setAlignment(Qt::AlignCenter);
    content->setMargin(12);
    content->setWordWrap(true);
    panel->setWidget(content);
    return panel;
}

}  // namespace

MainWindow::MainWindow()
    : _stack(nullptr),
      _menu(nullptr),
      _options(nullptr),
      _editorContainer(nullptr),
      _viewport(new GameViewport()),
      _palettePanel(nullptr),
      _palette(nullptr),
      _levelsPanel(nullptr),
      _levels(nullptr),
      _toolPanel(nullptr),
      _tools(nullptr),
      _statusPanel(nullptr) {
    setObjectName(QStringLiteral("EditorMainWindow"));

    // `createWindowContainer` embarque la fenêtre native du viewport et en prend la propriété.
    _editorContainer = QWidget::createWindowContainer(_viewport, this);
    _editorContainer->setMinimumSize(320, 240);
    _editorContainer->setFocusPolicy(Qt::StrongFocus);

    // Central : menu principal, options et viewport empilés ; l'application démarre sur le menu.
    _menu = new MainMenu();
    _options = new OptionsPage(_viewport, hmi::executableDirectory() / "Settings" / "keybindings.json");
    _stack = new QStackedWidget(this);
    _stack->addWidget(_menu);
    _stack->addWidget(_options);
    _stack->addWidget(_editorContainer);
    setCentralWidget(_stack);

    setDockNestingEnabled(true);
    createDockPanels();
    createMenus();

    // Sélectionner une tuile dans la palette définit le type peint au clic dans le viewport.
    connect(_palette, &PalettePanel::tileSelected, _viewport,
            [this](core::TileType type) { _viewport->setActiveTile(type); });
    // Changer d'outil dans le panneau Outils met à jour l'outil actif du viewport.
    connect(_tools, &ToolPanel::toolSelected, _viewport,
            [this](hmi::EditorTool tool) { _viewport->setTool(tool); });
    // Les messages d'état du viewport (enregistrement, essai, erreurs) s'affichent en bas.
    connect(_viewport, &GameViewport::statusMessage, this,
            [this](const QString& message) { statusBar()->showMessage(message, 5000); });
    // Ouvrir un niveau depuis le panneau : garde-fou des modifications non enregistrées d'abord.
    connect(_levels, &LevelBrowserPanel::levelOpenRequested, this, [this](const QString& path) {
        if (_viewport->isDirty()) {
            const QMessageBox::StandardButton answer = QMessageBox::question(
                this, QStringLiteral("Modifications non enregistrées"),
                QStringLiteral("Le niveau courant a des modifications non enregistrées. "
                               "Ouvrir un autre niveau et les perdre ?"));
            if (answer != QMessageBox::Yes) {
                return;
            }
        }
        _viewport->openLevel(std::filesystem::path(path.toStdString()));
    });
    statusBar()->showMessage(QStringLiteral("Éditeur — peindre : clic gauche · Ctrl+Z/Y · "
                                            "Ctrl+S enregistrer · P essayer"));

    // Navigation depuis le menu principal.
    connect(_menu, &MainMenu::editorRequested, this, &MainWindow::showEditor);
    connect(_menu, &MainMenu::playRequested, this, &MainWindow::showGame);
    connect(_menu, &MainMenu::optionsRequested, this, &MainWindow::showOptions);
    connect(_menu, &MainMenu::quitRequested, this, &MainWindow::close);
    // Retour au menu à la fin d'une partie (ou Échap en mode jeu).
    connect(_viewport, &GameViewport::exitToMenuRequested, this, &MainWindow::showMenu);
    // Page Options : retour au menu, bascule plein écran.
    connect(_options, &OptionsPage::backRequested, this, &MainWindow::showMenu);
    connect(_options, &OptionsPage::fullscreenRequested, this,
            [this](bool enabled) { enabled ? showFullScreen() : showNormal(); });

    setWindowTitle(QStringLiteral("ProjectGaming — Éditeur (Qt)"));
    resize(1280, 720);

    // Capture la disposition par défaut (après création des docks, avant restauration d'une
    // éventuelle disposition sauvegardée) : sert de cible à « Réinitialiser la disposition ».
    _defaultState = saveState(LAYOUT_VERSION);
    restoreLayout();

    showMenu();  // l'application démarre sur le menu principal.
}

void MainWindow::setDocksVisible(bool visible) {
    for (QDockWidget* const dock : {_palettePanel, _toolPanel, _levelsPanel, _statusPanel}) {
        dock->setVisible(visible);
    }
}

void MainWindow::showMenu() {
    _stack->setCurrentWidget(_menu);
    setDocksVisible(false);
    menuBar()->setVisible(false);  // pas de barre de menu sur l'écran d'accueil
    statusBar()->clearMessage();
}

void MainWindow::showEditor() {
    _stack->setCurrentWidget(_editorContainer);
    setDocksVisible(true);
    menuBar()->setVisible(true);
    _editorContainer->setFocus();
}

void MainWindow::showGame() {
    _stack->setCurrentWidget(_editorContainer);
    setDocksVisible(false);
    menuBar()->setVisible(false);
    statusBar()->clearMessage();
    _editorContainer->setFocus();

    // Séquence de niveaux démo (même ordre que le jeu historique) — Échap ou la fin revient au menu.
    const std::filesystem::path levels = hmi::executableDirectory() / "Levels";
    _viewport->startGame({
        levels / "demo-deplacement.json", levels / "demo-saut.json",
        levels / "demo-double-saut.json", levels / "demo-wall-jump.json",
        levels / "demo-dash.json", levels / "demo-interrupteur.json",
        levels / "demo-plaque-pression.json", levels / "demo-bloc.json",
        levels / "demo-budget.json", levels / "demo-pente.json", levels / "demo-arrondi.json",
        levels / "demo-bloc-reduit.json", levels / "demo-dangers-avances.json",
        levels / "demo-final.json", levels / "demo-salles.json",
    });
}

void MainWindow::showOptions() {
    _stack->setCurrentWidget(_options);
    setDocksVisible(false);
    menuBar()->setVisible(false);
    statusBar()->clearMessage();
}

MainWindow::~MainWindow() = default;

void MainWindow::createDockPanels() {
    // Panneau « Palette » : dock hôte contenant l'arbre de sélection de tuiles.
    _palettePanel = new QDockWidget(QStringLiteral("Palette"));
    _palettePanel->setObjectName(QStringLiteral("PalettePanel"));
    _palette = new PalettePanel(_palettePanel);
    _palettePanel->setWidget(_palette);

    // Panneau « Niveaux » : liste/gestion des fichiers de niveaux (LOT-36).
    _levelsPanel = new QDockWidget(QStringLiteral("Niveaux"));
    _levelsPanel->setObjectName(QStringLiteral("LevelsPanel"));
    _levels = new LevelBrowserPanel(hmi::executableDirectory() / "Levels", _levelsPanel);
    _levelsPanel->setWidget(_levels);

    // Panneau « Outils » : sélecteur d'outil d'édition (pinceau/rectangle/sélection).
    _toolPanel = new QDockWidget(QStringLiteral("Outils"));
    _toolPanel->setObjectName(QStringLiteral("ToolPanel"));
    _tools = new ToolPanel(_toolPanel);
    _toolPanel->setWidget(_tools);

    _statusPanel = makePanel(QStringLiteral("Statut"), QStringLiteral("StatusPanel"),
                             QStringLiteral("Statut"));

    addDockWidget(Qt::LeftDockWidgetArea, _palettePanel);
    addDockWidget(Qt::LeftDockWidgetArea, _toolPanel);
    addDockWidget(Qt::RightDockWidgetArea, _levelsPanel);
    addDockWidget(Qt::BottomDockWidgetArea, _statusPanel);
}

void MainWindow::createMenus() {
    // Menu « Application » : retour au menu principal, quitter.
    QMenu* const appMenu = menuBar()->addMenu(QStringLiteral("Application"));
    connect(appMenu->addAction(QStringLiteral("Menu principal")), &QAction::triggered, this,
            &MainWindow::showMenu);
    appMenu->addSeparator();
    connect(appMenu->addAction(QStringLiteral("Quitter")), &QAction::triggered, this,
            &MainWindow::close);

    // Menu « Niveau » : actions d'édition. Les raccourcis (Ctrl+S, P, Ctrl+Z/Y) sont gérés par le
    // viewport lui-même (fenêtre native) ; ces entrées de menu servent la découvrabilité.
    QMenu* const levelMenu = menuBar()->addMenu(QStringLiteral("Niveau"));
    connect(levelMenu->addAction(QStringLiteral("Enregistrer (Ctrl+S)")), &QAction::triggered,
            _viewport, [this] { _viewport->save(); });
    connect(levelMenu->addAction(QStringLiteral("Essayer (P)")), &QAction::triggered, _viewport,
            [this] { _viewport->startPlaytest(); });
    connect(levelMenu->addAction(QStringLiteral("Redimensionner…")), &QAction::triggered, this,
            [this] { openResizeDialog(); });

    // Menu « Affichage » : visibilité des panneaux + réinitialisation de la disposition.
    QMenu* const viewMenu = menuBar()->addMenu(QStringLiteral("Affichage"));
    viewMenu->addAction(_palettePanel->toggleViewAction());
    viewMenu->addAction(_levelsPanel->toggleViewAction());
    viewMenu->addAction(_toolPanel->toggleViewAction());
    viewMenu->addAction(_statusPanel->toggleViewAction());
    viewMenu->addSeparator();
    QAction* const reset = viewMenu->addAction(QStringLiteral("Réinitialiser la disposition"));
    connect(reset, &QAction::triggered, this,
            [this] { restoreState(_defaultState, LAYOUT_VERSION); });
}

void MainWindow::openResizeDialog() {
    constexpr int MAX_DIMENSION = 100;  // plafond de taille de niveau (EX-EDIT-017).

    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Redimensionner le niveau"));

    auto* const widthSpin = new QSpinBox(&dialog);
    widthSpin->setRange(1, MAX_DIMENSION);
    widthSpin->setValue(_viewport->levelWidth());
    auto* const heightSpin = new QSpinBox(&dialog);
    heightSpin->setRange(1, MAX_DIMENSION);
    heightSpin->setValue(_viewport->levelHeight());

    auto* const form = new QFormLayout(&dialog);
    form->addRow(QStringLiteral("Largeur (cases)"), widthSpin);
    form->addRow(QStringLiteral("Hauteur (cases)"), heightSpin);
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
            this, QStringLiteral("Redimensionner"),
            QStringLiteral("Réduire à %1 × %2 supprimera du contenu (entrée, sortie ou liaisons). "
                           "Continuer ?")
                .arg(width)
                .arg(height));
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
        restoreState(state, LAYOUT_VERSION);
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

}  // namespace editor
