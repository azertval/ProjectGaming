#include "HMI/Interface/MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <ctime>
#include <filesystem>
#include <vector>

#include "Core/Diagnostics/MemoryLogSink.h"
#include "HMI/Diagnostics/SessionLog.h"
#include "HMI/Editor/LevelBrowserPanel.h"
#include "HMI/Editor/LinkPanel.h"
#include "HMI/Editor/PalettePanel.h"
#include "HMI/Editor/ToolPanel.h"
#include "HMI/Game/GameViewport.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/GamepadButton.h"
#include "HMI/Interface/MainMenu.h"
#include "HMI/Interface/OptionsPage.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "ui_MainWindow.h"

namespace hmi {

namespace {

// Version de la disposition sérialisée : à incrémenter si l'ensemble des docks change, pour
// invalider proprement une disposition sauvegardée devenue incompatible (`restoreState`).
constexpr int LAYOUT_VERSION =
    3;  // 3 : dock « Liens » ajouté (LOT-37, invalide les dispositions v2)

// Clés de persistance (portée application ; l'organisation/appli sont fixées dans `main`,
// HMI/main.cpp).
constexpr char GEOMETRY_KEY[] = "mainWindow/geometry";
constexpr char STATE_KEY[] = "mainWindow/state";

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
      _tools(nullptr),
      _links(nullptr),
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
                this, text("dialog.unsaved_title"), text("dialog.unsaved_text"));
            if (answer != QMessageBox::Yes) {
                return;
            }
        }
        _viewport->openLevel(std::filesystem::path(path.toStdString()));
    });

    // Panneau Liens : reste synchronise avec le brouillon (LOT-37) ; selectionner une ligne
    // surligne la liaison dans le viewport, supprimer delegue au viewport (seul proprietaire).
    connect(_viewport, &GameViewport::draftChanged, this,
            [this] { _links->refresh(_viewport->draft()); });
    connect(_links, &LinkPanel::linkSelected, _viewport, &GameViewport::setHighlightedLink);
    connect(_links, &LinkPanel::deleteRequested, _viewport, &GameViewport::unlinkMechanism);
    _links->refresh(_viewport->draft());  // etat initial (avant tout draftChanged).

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
    for (QDockWidget* const dock :
         {_ui->PalettePanel, _ui->ToolPanel, _ui->LevelsPanel, _ui->LinksPanel}) {
        dock->setVisible(visible);
    }
}

void MainWindow::showMenu() {
    HMI_LOG_INFO("Navigation : menu principal.");
    _stack->setCurrentWidget(_menu);
    setDocksVisible(false);
    menuBar()->setVisible(false);  // pas de barre de menu sur l'écran d'accueil
    statusBar()->clearMessage();
    setMenuGamepadActive(true);
}

void MainWindow::showEditor() {
    HMI_LOG_INFO("Navigation : editeur.");
    _stack->setCurrentWidget(_editorContainer);
    setDocksVisible(true);
    menuBar()->setVisible(true);
    statusBar()->showMessage(text("status.edit_help"));
    _editorContainer->setFocus();
    setMenuGamepadActive(false);
}

void MainWindow::showGame() {
    HMI_LOG_INFO("Navigation : jeu.");
    setMenuGamepadActive(false);
    _stack->setCurrentWidget(_editorContainer);
    setDocksVisible(false);
    menuBar()->setVisible(false);
    statusBar()->clearMessage();
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
    statusBar()->clearMessage();
    setMenuGamepadActive(true);
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    // Contenu des docks : les coquilles (`PalettePanel`/`ToolPanel`/`LevelsPanel`) et leur
    // agencement viennent du `.ui` ; leurs widgets, paramétrés (chemins, dépendances), sont créés
    // en code.
    _palette = new PalettePanel(_ui->PalettePanel);
    _ui->PalettePanel->setWidget(_palette);
    _tools = new ToolPanel(_ui->ToolPanel);
    _ui->ToolPanel->setWidget(_tools);
    _levels = new LevelBrowserPanel(hmi::executableDirectory() / "Levels", _ui->LevelsPanel);
    _ui->LevelsPanel->setWidget(_levels);
    _links = new LinkPanel(_ui->LinksPanel);
    _ui->LinksPanel->setWidget(_links);

    // Branchement du fonctionnel sur les actions déclarées dans le `.ui` (libellés posés par
    // retranslateUi ; les raccourcis Ctrl+S/P/Ctrl+Z sont gérés par le viewport, ces entrées
    // servent la découvrabilité).
    connect(_ui->actMainMenu, &QAction::triggered, this, &MainWindow::showMenu);
    connect(_ui->actQuit, &QAction::triggered, this, &MainWindow::close);
    connect(_ui->actSave, &QAction::triggered, _viewport, [this] { _viewport->save(); });
    connect(_ui->actPlaytest, &QAction::triggered, _viewport,
            [this] { _viewport->startPlaytest(); });
    connect(_ui->actResize, &QAction::triggered, this, [this] { openResizeDialog(); });
    connect(_ui->actResetLayout, &QAction::triggered, this,
            [this] { restoreState(_defaultState, LAYOUT_VERSION); });

    // Bascules de visibilité des docks (dynamiques) : insérées avant « Réinitialiser la disposition
    // ».
    _ui->viewMenu->insertAction(_ui->actResetLayout, _ui->PalettePanel->toggleViewAction());
    _ui->viewMenu->insertAction(_ui->actResetLayout, _ui->LevelsPanel->toggleViewAction());
    _ui->viewMenu->insertAction(_ui->actResetLayout, _ui->ToolPanel->toggleViewAction());
    _ui->viewMenu->insertAction(_ui->actResetLayout, _ui->LinksPanel->toggleViewAction());
    _ui->viewMenu->insertSeparator(_ui->actResetLayout);
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

void MainWindow::retranslateUi() {
    setWindowTitle(text("window.title"));

    // Panneaux dockables (les actions « toggle » du menu Affichage suivent le titre du dock).
    _ui->PalettePanel->setWindowTitle(text("dock.palette"));
    _ui->ToolPanel->setWindowTitle(text("dock.tools"));
    _ui->LevelsPanel->setWindowTitle(text("dock.levels"));
    _ui->LinksPanel->setWindowTitle(text("dock.links"));

    // Barre de menus.
    _ui->appMenu->setTitle(text("menubar.application"));
    _ui->actMainMenu->setText(text("menubar.main_menu"));
    _ui->actQuit->setText(text("menubar.quit"));
    _ui->levelMenu->setTitle(text("menubar.level"));
    _ui->actSave->setText(text("menubar.save"));
    _ui->actPlaytest->setText(text("menubar.playtest"));
    _ui->actResize->setText(text("menubar.resize"));
    _ui->viewMenu->setTitle(text("menubar.view"));
    _ui->actResetLayout->setText(text("menubar.reset_layout"));

    // Panneaux et pages (chacun retraduit son propre contenu depuis le catalogue).
    _menu->retranslateUi(_loc);
    _options->retranslateUi(_loc);
    _palette->retranslateUi(_loc);
    _tools->retranslateUi(_loc);
    _levels->retranslateUi(_loc);
    _links->retranslateUi(_loc);

    // Rafraîchit le message d'aide s'il est visible (mode éditeur).
    if (_stack->currentWidget() == _editorContainer && menuBar()->isVisible()) {
        statusBar()->showMessage(text("status.edit_help"));
    }
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
        statusBar()->showMessage(text("options.logs_unavailable"), 5000);
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
        statusBar()->showMessage(
            text("options.logs_saved").arg(QString::fromStdString(path.filename().string())), 5000);
    } else {
        HMI_LOG_ERROR("Echec de l'enregistrement des journaux : " + path.string());
        statusBar()->showMessage(text("options.logs_failed"), 5000);
    }
}

}  // namespace hmi
