#include "Editor/MainWindow.h"

#include <QAction>
#include <QCloseEvent>
#include <QDockWidget>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QString>
#include <QWidget>

#include "Editor/GameViewport.h"

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
    : _viewport(new GameViewport()),
      _palettePanel(nullptr),
      _toolPanel(nullptr),
      _statusPanel(nullptr) {
    setObjectName(QStringLiteral("EditorMainWindow"));

    // `createWindowContainer` embarque la fenêtre native du viewport et en prend la propriété.
    QWidget* const container = QWidget::createWindowContainer(_viewport, this);
    container->setMinimumSize(320, 240);
    container->setFocusPolicy(Qt::StrongFocus);
    setCentralWidget(container);

    setDockNestingEnabled(true);
    createDockPanels();
    createViewMenu();

    setWindowTitle(QStringLiteral("ProjectGaming — Éditeur (Qt)"));
    resize(1280, 720);

    // Capture la disposition par défaut (après création des docks, avant restauration d'une
    // éventuelle disposition sauvegardée) : sert de cible à « Réinitialiser la disposition ».
    _defaultState = saveState(LAYOUT_VERSION);
    restoreLayout();
}

MainWindow::~MainWindow() = default;

void MainWindow::createDockPanels() {
    _palettePanel = makePanel(QStringLiteral("Palette"), QStringLiteral("PalettePanel"),
                              QStringLiteral("Palette de tuiles\n(LOT-35 TACHE-02)"));
    _toolPanel = makePanel(QStringLiteral("Outils"), QStringLiteral("ToolPanel"),
                           QStringLiteral("Outils\n(LOT-35 TACHE-03)"));
    _statusPanel = makePanel(QStringLiteral("Statut"), QStringLiteral("StatusPanel"),
                             QStringLiteral("Statut"));

    addDockWidget(Qt::LeftDockWidgetArea, _palettePanel);
    addDockWidget(Qt::LeftDockWidgetArea, _toolPanel);
    addDockWidget(Qt::BottomDockWidgetArea, _statusPanel);
}

void MainWindow::createViewMenu() {
    QMenu* const viewMenu = menuBar()->addMenu(QStringLiteral("Affichage"));
    // Bascule de visibilité de chaque panneau (action fournie par le dock lui-même).
    viewMenu->addAction(_palettePanel->toggleViewAction());
    viewMenu->addAction(_toolPanel->toggleViewAction());
    viewMenu->addAction(_statusPanel->toggleViewAction());
    viewMenu->addSeparator();
    QAction* const reset = viewMenu->addAction(QStringLiteral("Réinitialiser la disposition"));
    connect(reset, &QAction::triggered, this,
            [this] { restoreState(_defaultState, LAYOUT_VERSION); });
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
