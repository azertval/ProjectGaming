#include "Editor/GameViewport.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <optional>
#include <utility>

#include <QEvent>
#include <QExposeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPlatformSurfaceEvent>
#include <QResizeEvent>
#include <QWheelEvent>

#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
// GraphicsDevice tire <Windows.h>/<d3d11.h> (HWND, device D3D11). Inclus après les en-têtes Qt.
#include "HMI/Graphics/DraftRenderer.h"
#include "HMI/Graphics/GraphicsDevice.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace editor {

namespace {

[[nodiscard]] std::optional<hmi::Key> mapQtKey(int qtKey) {
    switch (qtKey) {
        case Qt::Key_Escape:
            return hmi::Key::Escape;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            return hmi::Key::Tab;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            return hmi::Key::Enter;
        case Qt::Key_Backspace:
            return hmi::Key::Backspace;
        case Qt::Key_Shift:
            return hmi::Key::Shift;
        case Qt::Key_Control:
            return hmi::Key::Control;
        case Qt::Key_Space:
            return hmi::Key::Space;
        case Qt::Key_Left:
            return hmi::Key::Left;
        case Qt::Key_Up:
            return hmi::Key::Up;
        case Qt::Key_Right:
            return hmi::Key::Right;
        case Qt::Key_Down:
            return hmi::Key::Down;
        case Qt::Key_F1:
            return hmi::Key::F1;
        case Qt::Key_F2:
            return hmi::Key::F2;
        case Qt::Key_F10:
            return hmi::Key::F10;
        default:
            break;
    }
    if ((qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9) || (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)) {
        return static_cast<hmi::Key>(qtKey);
    }
    return std::nullopt;
}

[[nodiscard]] std::optional<hmi::MouseButton> mapQtMouseButton(Qt::MouseButton button) {
    switch (button) {
        case Qt::LeftButton:
            return hmi::MouseButton::Left;
        case Qt::RightButton:
            return hmi::MouseButton::Right;
        case Qt::MiddleButton:
            return hmi::MouseButton::Middle;
        default:
            return std::nullopt;
    }
}

[[nodiscard]] std::filesystem::path keybindingsPath() {
    return hmi::executableDirectory() / "Settings" / "keybindings.json";
}

}  // namespace

GameViewport::GameViewport(QWindow* parent)
    : QWindow(parent),
      _gameBindings(hmi::GameKeyBindings::load(keybindingsPath())),
      _gamepadBindings(hmi::GamepadBindings::load(keybindingsPath())),
      _draft(core::LevelDraft::empty("Nouveau niveau", 24, 14)),
      _camera(1280, 720) {}

GameViewport::~GameViewport() = default;

int GameViewport::pixelWidth() const {
    return std::max(1, static_cast<int>(width() * devicePixelRatio()));
}

int GameViewport::pixelHeight() const {
    return std::max(1, static_cast<int>(height() * devicePixelRatio()));
}

void GameViewport::ensureResources() {
    if (_graphics) {
        return;
    }
    const HWND handle = reinterpret_cast<HWND>(winId());
    _graphics = std::make_unique<hmi::GraphicsDevice>(handle, pixelWidth(), pixelHeight());
    _graphics->setVSyncEnabled(_vsync);
    _spriteBatch = std::make_unique<hmi::SpriteBatch>(_graphics->device(), _graphics->context());
    _atlas = std::make_unique<hmi::TextureAtlas>(_graphics->device());
    _draftRenderer = std::make_unique<hmi::DraftRenderer>(*_spriteBatch, *_atlas);

    // LOT-35 : ouvre un niveau de démonstration comme brouillon éditable (le sélecteur de niveaux
    // arrive au LOT-36). Échec récupérable : on garde le brouillon vierge.
    const std::filesystem::path levelPath =
        hmi::executableDirectory() / "Levels" / "demo-deplacement.json";
    core::LevelLoadResult result = core::LevelLoader::loadFromFile(levelPath);
    if (result.ok()) {
        _draft = core::LevelDraft::fromLevel(*result.level);
        _draftRenderer->invalidate();
    } else {
        HMI_LOG_WARNING("Editeur : echec du chargement du niveau de demo : " + result.error);
    }
}

void GameViewport::updateEditCamera() {
    const int levelWidth = _draft.tileMap().width();
    const int levelHeight = _draft.tileMap().height();
    _camera.setViewportSize(pixelWidth(), pixelHeight());
    _camera.setZoom(hmi::Camera2D::fitZoom(static_cast<float>(pixelWidth()),
                                           static_cast<float>(pixelHeight()),
                                           static_cast<float>(levelWidth),
                                           static_cast<float>(levelHeight), 0.92f));
    _camera.setCenter(core::Vector2{static_cast<float>(levelWidth) * 0.5f,
                                    static_cast<float>(levelHeight) * 0.5f});
}

std::optional<core::GridPosition> GameViewport::cellAt(const QMouseEvent* event) {
    updateEditCamera();  // s'assure que la conversion écran→monde utilise le cadrage courant.
    const qreal ratio = devicePixelRatio();
    const core::Vector2 world = _camera.screenToWorld(
        core::Vector2{static_cast<float>(event->position().x() * ratio),
                      static_cast<float>(event->position().y() * ratio)});
    const int column = static_cast<int>(std::floor(world.x));
    const int row = static_cast<int>(std::floor(world.y));
    if (!_draft.tileMap().inBounds(column, row)) {
        return std::nullopt;
    }
    return core::GridPosition{column, row};
}

void GameViewport::paintAt(const QMouseEvent* event) {
    if (const std::optional<core::GridPosition> cell = cellAt(event)) {
        _draft.paintTile(cell->column, cell->row, _activeTile);
        _dirty = true;
        if (_draftRenderer) {
            _draftRenderer->invalidate();
        }
    }
}

core::GridPosition GameViewport::clampedCell(const QMouseEvent* event) {
    updateEditCamera();
    const qreal ratio = devicePixelRatio();
    const core::Vector2 world = _camera.screenToWorld(
        core::Vector2{static_cast<float>(event->position().x() * ratio),
                      static_cast<float>(event->position().y() * ratio)});
    const int width = _draft.tileMap().width();
    const int height = _draft.tileMap().height();
    return core::GridPosition{std::clamp(static_cast<int>(std::floor(world.x)), 0, width - 1),
                              std::clamp(static_cast<int>(std::floor(world.y)), 0, height - 1)};
}

void GameViewport::applyRectangle(core::GridPosition a, core::GridPosition b) {
    const int minColumn = std::min(a.column, b.column);
    const int maxColumn = std::max(a.column, b.column);
    const int minRow = std::min(a.row, b.row);
    const int maxRow = std::max(a.row, b.row);
    const std::vector<std::vector<core::TileType>> block(
        static_cast<std::size_t>(maxRow - minRow + 1),
        std::vector<core::TileType>(static_cast<std::size_t>(maxColumn - minColumn + 1),
                                    _activeTile));
    _draft.paintRegion(minColumn, minRow, block);  // un seul snapshot undo pour tout le rectangle
    _dirty = true;
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
}

void GameViewport::copySelection() {
    if (!_selection) {
        return;
    }
    const core::GridPosition mn = _selection->first;
    const core::GridPosition mx = _selection->second;
    const core::TileMap& map = _draft.tileMap();
    _clipboard.clear();
    for (int row = mn.row; row <= mx.row; ++row) {
        std::vector<core::TileType> line;
        for (int column = mn.column; column <= mx.column; ++column) {
            line.push_back(map.tile(column, row));
        }
        _clipboard.push_back(std::move(line));
    }
    emit statusMessage(QStringLiteral("Zone copiée (%1 × %2).")
                           .arg(mx.column - mn.column + 1)
                           .arg(mx.row - mn.row + 1));
}

void GameViewport::pasteClipboard() {
    if (_clipboard.empty()) {
        return;
    }
    _draft.paintRegion(_hoverCell.column, _hoverCell.row, _clipboard);
    _dirty = true;
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
    emit statusMessage(QStringLiteral("Zone collée."));
}

std::optional<std::pair<core::GridPosition, core::GridPosition>> GameViewport::highlight() const {
    if (_dragging) {
        return std::make_pair(
            core::GridPosition{std::min(_dragStart.column, _dragCurrent.column),
                               std::min(_dragStart.row, _dragCurrent.row)},
            core::GridPosition{std::max(_dragStart.column, _dragCurrent.column),
                               std::max(_dragStart.row, _dragCurrent.row)});
    }
    return _selection;
}

void GameViewport::tick() {
    if (!isExposed()) {
        return;
    }
    ensureResources();

    const Clock::time_point now = Clock::now();
    const float elapsedSeconds = std::chrono::duration<float>(now - _previousFrame).count();
    _previousFrame = now;

    _gamepad.poll(_input);

    const int steps = _timestep.advance(elapsedSeconds);
    const float fixedDelta = _timestep.fixedDeltaSeconds();
    for (int step = 0; step < steps; ++step) {
        if (_session && _session->update(_input, fixedDelta) == core::LevelOutcome::Won) {
            if (_gameMode) {
                loadGameLevel(_gameLevel + 1);  // enchaîne le niveau suivant de la séquence
            } else {
                stopPlaytest();  // essai éditeur : retour à l'édition
            }
        }
        _input.beginFrame();
    }

    renderFrame();
    requestUpdate();
}

void GameViewport::renderFrame() {
    if (!isExposed()) {
        return;
    }
    ensureResources();
    _graphics->clear(0.10f, 0.12f, 0.16f, 1.0f);
    if (_session) {
        _session->render(pixelWidth(), pixelHeight(), _timestep.interpolationAlpha());
    } else {
        updateEditCamera();
        _draftRenderer->render(_draft, _camera, _showGrid, highlight());
    }
    _graphics->present();
}

bool GameViewport::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::UpdateRequest:
            tick();
            return true;
        case QEvent::FocusOut:
            _input.releaseAll();
            break;
        case QEvent::PlatformSurface:
            // Libère les ressources Direct3D 11 tant que la surface native existe encore (crash de
            // fermeture, cf. LOT-34). Ordre : session/renderer avant le device.
            if (static_cast<QPlatformSurfaceEvent*>(event)->surfaceEventType() ==
                QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
                _session.reset();
                _draftRenderer.reset();
                _atlas.reset();
                _spriteBatch.reset();
                _graphics.reset();
                _loopStarted = false;
            }
            break;
        default:
            break;
    }
    return QWindow::event(event);
}

void GameViewport::exposeEvent(QExposeEvent*) {
    if (!isExposed()) {
        return;
    }
    ensureResources();
    if (!_loopStarted) {
        _loopStarted = true;
        _previousFrame = Clock::now();
    }
    requestUpdate();
}

void GameViewport::resizeEvent(QResizeEvent*) {
    if (_graphics) {
        _graphics->resize(pixelWidth(), pixelHeight());
        renderFrame();
    }
}

void GameViewport::keyPressEvent(QKeyEvent* event) {
    // Mode jeu/essai : Échap sort ; les autres touches alimentent le jeu.
    if (_session) {
        if (event->key() == Qt::Key_Escape) {
            if (_gameMode) {
                _gameMode = false;
                _session.reset();
                emit exitToMenuRequested();
            } else {
                stopPlaytest();
            }
            return;
        }
        if (!event->isAutoRepeat()) {
            if (const std::optional<hmi::Key> key = mapQtKey(event->key())) {
                _input.onKeyDown(*key);
            }
        }
        return;
    }

    // Mode édition : raccourcis (annuler/refaire, enregistrer, essai).
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Z && _draft.undo()) {
            _dirty = true;
            if (_draftRenderer) {
                _draftRenderer->invalidate();
            }
            return;
        }
        if (event->key() == Qt::Key_Y && _draft.redo()) {
            _dirty = true;
            if (_draftRenderer) {
                _draftRenderer->invalidate();
            }
            return;
        }
        if (event->key() == Qt::Key_S) {
            save();
            return;
        }
        if (event->key() == Qt::Key_C) {
            copySelection();
            return;
        }
        if (event->key() == Qt::Key_V) {
            pasteClipboard();
            return;
        }
    }
    if (event->key() == Qt::Key_P) {
        startPlaytest();
        return;
    }
    if (event->key() == Qt::Key_F10) {
        _showGrid = !_showGrid;  // bascule de la grille de repère (comme l'éditeur historique).
        return;
    }
    if (event->isAutoRepeat()) {
        return;
    }
    if (const std::optional<hmi::Key> key = mapQtKey(event->key())) {
        _input.onKeyDown(*key);
    }
}

void GameViewport::save() {
    const core::LevelLoadResult validated = _draft.toLevel();
    if (!validated.ok()) {
        emit statusMessage(QStringLiteral("Enregistrement impossible : ") +
                           QString::fromStdString(validated.error));
        return;
    }
    const std::filesystem::path path =
        hmi::executableDirectory() / "Levels" / (_draft.name() + ".json");
    if (core::LevelWriter::saveToFile(*validated.level, path)) {
        _dirty = false;
        emit statusMessage(QStringLiteral("Niveau enregistré : ") +
                           QString::fromStdString(path.filename().string()));
    } else {
        emit statusMessage(QStringLiteral("Échec de l'écriture du fichier."));
    }
}

void GameViewport::openLevel(const std::filesystem::path& path) {
    core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        emit statusMessage(QStringLiteral("Ouverture impossible : ") +
                           QString::fromStdString(loaded.error));
        return;
    }
    stopPlaytest();  // sort d'un éventuel essai en cours
    _draft = core::LevelDraft::fromLevel(*loaded.level);
    _dirty = false;
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
    emit statusMessage(QStringLiteral("Niveau ouvert : ") +
                       QString::fromStdString(path.filename().string()));
}

void GameViewport::startPlaytest() {
    if (_session) {
        return;  // déjà en essai
    }
    const core::LevelLoadResult validated = _draft.toLevel();
    if (!validated.ok()) {
        emit statusMessage(QStringLiteral("Essai impossible : ") +
                           QString::fromStdString(validated.error));
        return;
    }
    ensureResources();
    _session.emplace(*_spriteBatch, *_atlas, pixelWidth(), pixelHeight(),
                     std::move(*validated.level), _gameBindings, _gamepadBindings);
    emit statusMessage(QStringLiteral("Essai en cours — Échap pour revenir à l'édition."));
}

void GameViewport::stopPlaytest() {
    if (!_session) {
        return;
    }
    _session.reset();
    if (_draftRenderer) {
        _draftRenderer->invalidate();  // ré-affiche le brouillon (intact).
    }
    emit statusMessage(QStringLiteral("Retour à l'édition."));
}

void GameViewport::setVSync(bool enabled) noexcept {
    _vsync = enabled;
    if (_graphics) {
        _graphics->setVSyncEnabled(enabled);
    }
}

void GameViewport::startGame(std::vector<std::filesystem::path> levels) {
    ensureResources();
    _gameLevels = std::move(levels);
    _gameMode = true;
    loadGameLevel(0);
}

void GameViewport::loadGameLevel(std::size_t index) {
    if (index >= _gameLevels.size()) {
        // Séquence terminée : retour au menu.
        _gameMode = false;
        _session.reset();
        emit exitToMenuRequested();
        return;
    }
    core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(_gameLevels[index]);
    if (!loaded.ok()) {
        loadGameLevel(index + 1);  // niveau illisible : passe au suivant (robustesse)
        return;
    }
    _gameLevel = index;
    _session.emplace(*_spriteBatch, *_atlas, pixelWidth(), pixelHeight(),
                     std::move(*loaded.level), _gameBindings, _gamepadBindings);
}

void GameViewport::resizeLevel(int width, int height) {
    _draft.resize(width, height);
    _dirty = true;
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
    emit statusMessage(
        QStringLiteral("Niveau redimensionné : %1 × %2").arg(width).arg(height));
}

bool GameViewport::wouldResizeDrop(int width, int height) const {
    return _draft.wouldResizeDropContent(width, height);
}

int GameViewport::levelWidth() const {
    return _draft.tileMap().width();
}

int GameViewport::levelHeight() const {
    return _draft.tileMap().height();
}

void GameViewport::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return;
    }
    if (const std::optional<hmi::Key> key = mapQtKey(event->key())) {
        _input.onKeyUp(*key);
    }
}

void GameViewport::updateMousePosition(const QMouseEvent* event) {
    const qreal ratio = devicePixelRatio();
    _input.onMouseMove(static_cast<int>(event->position().x() * ratio),
                       static_cast<int>(event->position().y() * ratio));
}

void GameViewport::mousePressEvent(QMouseEvent* event) {
    updateMousePosition(event);
    if (const std::optional<hmi::MouseButton> button = mapQtMouseButton(event->button())) {
        _input.onMouseButtonDown(*button);
    }
    if (_session || event->button() != Qt::LeftButton) {
        return;
    }
    // Mode édition : dispatch selon l'outil actif.
    switch (_tool) {
        case hmi::EditorTool::Paint:
            _painting = true;
            paintAt(event);
            break;
        case hmi::EditorTool::Rectangle:
        case hmi::EditorTool::Selection:
            _dragging = true;
            _dragStart = clampedCell(event);
            _dragCurrent = _dragStart;
            break;
    }
}

void GameViewport::mouseReleaseEvent(QMouseEvent* event) {
    updateMousePosition(event);
    if (const std::optional<hmi::MouseButton> button = mapQtMouseButton(event->button())) {
        _input.onMouseButtonUp(*button);
    }
    if (event->button() != Qt::LeftButton) {
        return;
    }
    if (_dragging) {
        _dragCurrent = clampedCell(event);
        if (_tool == hmi::EditorTool::Rectangle) {
            applyRectangle(_dragStart, _dragCurrent);
        } else if (_tool == hmi::EditorTool::Selection) {
            _selection = std::make_pair(
                core::GridPosition{std::min(_dragStart.column, _dragCurrent.column),
                                   std::min(_dragStart.row, _dragCurrent.row)},
                core::GridPosition{std::max(_dragStart.column, _dragCurrent.column),
                                   std::max(_dragStart.row, _dragCurrent.row)});
        }
        _dragging = false;
    }
    _painting = false;
}

void GameViewport::mouseMoveEvent(QMouseEvent* event) {
    updateMousePosition(event);
    if (_session) {
        return;
    }
    if (const std::optional<core::GridPosition> cell = cellAt(event)) {
        _hoverCell = *cell;  // cible du collage (Ctrl+V)
    }
    if (_painting) {
        paintAt(event);  // glisser de peinture
    } else if (_dragging) {
        _dragCurrent = clampedCell(event);  // aperçu du rectangle/de la sélection
    }
}

void GameViewport::wheelEvent(QWheelEvent* event) {
    _input.onMouseWheel(event->angleDelta().y());
}

}  // namespace editor
