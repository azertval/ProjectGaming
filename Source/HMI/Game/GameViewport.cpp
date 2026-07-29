#include "HMI/Game/GameViewport.h"

#include <QEvent>
#include <QExposeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPlatformSurfaceEvent>
#include <QResizeEvent>
#include <QSettings>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <optional>
#include <utility>

#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/LinkGeometry.h"
#include "HMI/Editor/LinkGesture.h"
#include "HMI/Input/QtKeyMap.h"
// GraphicsDevice tire <Windows.h>/<d3d11.h> (HWND, device D3D11). Inclus après les en-têtes Qt.
#include "HMI/Graphics/AssetPaths.h"
#include "HMI/Graphics/DraftRenderer.h"
#include "HMI/Graphics/GraphicsDevice.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"
#include "HMI/HmiLog.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {
// Cle de preference du mode de rendu, dans la meme portee QSettings que la langue et la
// disposition des docks (EX-IHM-011) : aucun nouveau mecanisme de persistance a inventer.
const char* const RENDER_MODE_SETTINGS_KEY = "render_mode";
}  // namespace

namespace {

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
      _camera(1280, 720) {
    // Restaure le mode de rendu du dernier lancement (EX-IHM-011). Une preference absente, vide ou
    // corrompue retombe silencieusement sur le defaut (Texture) : perdre son mode d'affichage ne
    // doit jamais empecher de demarrer.
    _renderMode =
        renderModeFromName(QSettings()
                               .value(QString::fromLatin1(RENDER_MODE_SETTINGS_KEY), QString{})
                               .toString()
                               .toStdString());
}

GameViewport::~GameViewport() = default;

// Choisit le mode de rendu et persiste le choix (EX-REN-046, EX-IHM-011).
void GameViewport::setRenderMode(RenderMode mode) {
    if (_renderMode == mode) {
        return;  // aucun changement : ni ecriture de preference, ni message d'etat inutile.
    }
    _renderMode = mode;
    // Ecriture a la bascule, jamais dans la boucle de rendu : QSettings bufferise, le cout est
    // marginal et le choix survit meme a une fermeture brutale.
    QSettings().setValue(QString::fromLatin1(RENDER_MODE_SETTINGS_KEY),
                         QString::fromLatin1(renderModeName(mode)));
    HMI_LOG_INFO(std::string{"Rendu : mode "} + renderModeName(mode) + ".");
    // Aucune invalidation du brouillon : la scene ECS est inchangee, seule la resolution de
    // l'apparence differe (LOT-41).
}

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
    HMI_LOG_INFO("Viewport : initialisation Direct3D 11 (" + std::to_string(pixelWidth()) + "x" +
                 std::to_string(pixelHeight()) + ").");
    _graphics = std::make_unique<hmi::GraphicsDevice>(handle, pixelWidth(), pixelHeight());
    _graphics->setVSyncEnabled(_vsync);
    _spriteBatch = std::make_unique<hmi::SpriteBatch>(_graphics->device(), _graphics->context());
    _atlas = std::make_unique<hmi::TextureAtlas>(_graphics->device());
    // Registre des textures nommees (LOT-40) : proprietaire du damier de repli du mode Texture,
    // et point d'entree des skins a partir du LOT-42.
    _textureCache = std::make_unique<hmi::TextureCache>(
        _graphics->device(), hmi::AssetPaths{hmi::executableDirectory() / "Assets"});
    _draftRenderer = std::make_unique<hmi::DraftRenderer>(*_spriteBatch, *_atlas, *_textureCache);

    // Catalogue des skins (LOT-42), lu a cote de l'executable comme les niveaux et les traductions.
    // Fichier absent ou illisible : catalogue vide, tout retombe sur le damier -- un etat de depart
    // legitime, pas une erreur bloquante (EX-NFR-040).
    hmi::SkinCatalogResult skins =
        hmi::SkinCatalog::loadFromFile(hmi::executableDirectory() / "Assets" / "skins.json");
    if (skins.ok()) {
        _skins = std::move(*skins.catalog);
    }
    _draftRenderer->setSkins(&_skins);

    // LOT-35 : ouvre un niveau de démonstration comme brouillon éditable (le sélecteur de niveaux
    // arrive au LOT-36). Échec récupérable : on garde le brouillon vierge.
    const std::filesystem::path levelPath =
        hmi::executableDirectory() / "Levels" / "demo-deplacement.json";
    core::LevelLoadResult result = core::LevelLoader::loadFromFile(levelPath);
    if (result.ok()) {
        _draft = core::LevelDraft::fromLevel(*result.level);
        markDraftMutated();
    } else {
        HMI_LOG_WARNING("Editeur : echec du chargement du niveau de demo : " + result.error);
    }
}

void GameViewport::updateEditCamera() {
    const int levelWidth = _draft.tileMap().width();
    const int levelHeight = _draft.tileMap().height();
    _camera.setViewportSize(pixelWidth(), pixelHeight());
    _camera.setZoom(hmi::Camera2D::fitZoom(
        static_cast<float>(pixelWidth()), static_cast<float>(pixelHeight()),
        static_cast<float>(levelWidth), static_cast<float>(levelHeight), 0.92f));
    _camera.setCenter(core::Vector2{static_cast<float>(levelWidth) * 0.5f,
                                    static_cast<float>(levelHeight) * 0.5f});
}

std::optional<core::GridPosition> GameViewport::cellAt(const QMouseEvent* event) {
    updateEditCamera();  // s'assure que la conversion écran→monde utilise le cadrage courant.
    const qreal ratio = devicePixelRatio();
    const core::Vector2 world =
        _camera.screenToWorld(core::Vector2{static_cast<float>(event->position().x() * ratio),
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
        markDraftMutated();
    }
}

core::GridPosition GameViewport::clampedCell(const QMouseEvent* event) {
    updateEditCamera();
    const qreal ratio = devicePixelRatio();
    const core::Vector2 world =
        _camera.screenToWorld(core::Vector2{static_cast<float>(event->position().x() * ratio),
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
    markDraftMutated();
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
    emit statusMessage(
        statusText("status.region_copied").arg(mx.column - mn.column + 1).arg(mx.row - mn.row + 1));
}

void GameViewport::pasteClipboard() {
    if (_clipboard.empty()) {
        return;
    }
    _draft.paintRegion(_hoverCell.column, _hoverCell.row, _clipboard);
    _dirty = true;
    markDraftMutated();
    emit statusMessage(statusText("status.region_pasted"));
}

std::optional<std::pair<core::GridPosition, core::GridPosition>> GameViewport::highlight() const {
    if (_dragging) {
        return std::make_pair(core::GridPosition{std::min(_dragStart.column, _dragCurrent.column),
                                                 std::min(_dragStart.row, _dragCurrent.row)},
                              core::GridPosition{std::max(_dragStart.column, _dragCurrent.column),
                                                 std::max(_dragStart.row, _dragCurrent.row)});
    }
    return _selection;
}

void GameViewport::markDraftMutated() {
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
    emit draftChanged();
}

void GameViewport::setHighlightedLink(
    std::optional<std::pair<core::GridPosition, core::GridPosition>> link) {
    _selectedLink = std::move(link);  // presentation seule : aucune mutation du brouillon.
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
}

void GameViewport::unlinkMechanism(core::GridPosition targetPosition) {
    _draft.unlinkMechanism(targetPosition);
    _dirty = true;
    markDraftMutated();
}

bool GameViewport::linkExists(core::GridPosition switchPosition,
                              core::GridPosition targetPosition) const {
    for (const core::Mechanism& mechanism : _draft.mechanisms()) {
        if (mechanism.switchPosition == switchPosition &&
            mechanism.doorPosition == targetPosition) {
            return true;
        }
    }
    for (const core::DangerLink& dangerLink : _draft.dangerLinks()) {
        if (dangerLink.triggerPosition == switchPosition &&
            dangerLink.dangerPosition == targetPosition) {
            return true;
        }
    }
    return false;
}

void GameViewport::handleLinkClick(const QMouseEvent* event) {
    const std::optional<core::GridPosition> cell = cellAt(event);
    if (!cell) {
        return;
    }
    const core::TileType clickedType = _draft.tileMap().tile(cell->column, cell->row);

    std::optional<hmi::PendingLink> pending;
    if (_pendingLink) {
        pending = hmi::PendingLink{*_pendingLink,
                                   _draft.tileMap().tile(_pendingLink->column, _pendingLink->row)};
    }

    // N'a de sens que si l'attente est toujours un declencheur/une cible valide : resolu de la
    // meme facon que resolveLinkClick determinera lequel des deux est le declencheur.
    bool alreadyLinked = false;
    if (pending &&
        (hmi::isTriggerTile(pending->tileType) || hmi::isLinkTargetTile(pending->tileType))) {
        const bool pendingIsTrigger = hmi::isTriggerTile(pending->tileType);
        const core::GridPosition switchPosition = pendingIsTrigger ? pending->cell : *cell;
        const core::GridPosition targetPosition = pendingIsTrigger ? *cell : pending->cell;
        alreadyLinked = linkExists(switchPosition, targetPosition);
    }

    const hmi::LinkGestureDecision decision =
        hmi::resolveLinkClick(pending, *cell, clickedType, alreadyLinked);
    switch (decision.action) {
        case hmi::LinkGestureAction::Ignore:
            break;
        case hmi::LinkGestureAction::SetPending:
        case hmi::LinkGestureAction::ReplacePending:
            _pendingLink = decision.cell;
            if (_draftRenderer) {
                _draftRenderer->invalidate();  // affiche la nouvelle case en attente.
            }
            break;
        case hmi::LinkGestureAction::Link:
            _draft.linkMechanism(decision.switchPosition, decision.targetPosition);
            _pendingLink.reset();
            _dirty = true;
            markDraftMutated();
            break;
        case hmi::LinkGestureAction::Unlink:
            _draft.unlinkMechanism(decision.targetPosition);
            _pendingLink.reset();
            _dirty = true;
            markDraftMutated();
            break;
    }
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
        _session->render(pixelWidth(), pixelHeight(), _renderMode, _timestep.interpolationAlpha());
    } else {
        updateEditCamera();
        hmi::LinkOverlayState linkOverlay;
        linkOverlay.hoveredCell = _hoverCell;
        linkOverlay.pendingLink = _pendingLink;
        linkOverlay.selectedLink = _selectedLink;
        _draftRenderer->render(_draft, _camera, _showGrid, highlight(), linkOverlay, _renderMode);
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
                _textureCache.reset();  // libere les textures avant le device qui les a creees
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
    // Bascule du mode de rendu : traitee **avant** toute autre branche, pour etre active en
    // edition, en essai et en jeu reel (EX-REN-046). Touche fixe, jamais remappable -- meme parti
    // pris que F10 (grille de repere) ; hmi::Key n'a d'ailleurs aucune valeur F8, donc la touche
    // ne peut structurellement pas entrer dans une table de remappage.
    if (event->key() == Qt::Key_F8) {
        setRenderMode(_renderMode == RenderMode::Physique ? RenderMode::Texture
                                                          : RenderMode::Physique);
        return;
    }

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
            if (const std::optional<hmi::Key> key = qtKeyToHmiKey(event->key())) {
                _input.onKeyDown(*key);
            }
        }
        return;
    }

    // Mode édition : raccourcis (annuler/refaire, enregistrer, essai).
    if (event->key() == Qt::Key_Escape && _pendingLink) {
        // Annule la liaison en cours (outil Lien) : efface l'attente sans toucher au brouillon.
        _pendingLink.reset();
        if (_draftRenderer) {
            _draftRenderer->invalidate();
        }
        return;
    }
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Z && _draft.undo()) {
            _dirty = true;
            markDraftMutated();
            return;
        }
        if (event->key() == Qt::Key_Y && _draft.redo()) {
            _dirty = true;
            markDraftMutated();
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
    if (const std::optional<hmi::Key> key = qtKeyToHmiKey(event->key())) {
        _input.onKeyDown(*key);
    }
}

void GameViewport::save() {
    const core::LevelLoadResult validated = _draft.toLevel();
    if (!validated.ok()) {
        HMI_LOG_WARNING("Editeur : enregistrement refuse (brouillon invalide) : " +
                        validated.error);
        emit statusMessage(
            statusText("status.save_failed").arg(QString::fromStdString(validated.error)));
        return;
    }
    const std::filesystem::path path =
        hmi::executableDirectory() / "Levels" / (_draft.name() + ".json");
    if (core::LevelWriter::saveToFile(*validated.level, path)) {
        _dirty = false;
        HMI_LOG_INFO("Editeur : niveau enregistre : " + path.string());
        emit statusMessage(
            statusText("status.level_saved").arg(QString::fromStdString(path.filename().string())));
    } else {
        HMI_LOG_ERROR("Editeur : echec d'ecriture du niveau : " + path.string());
        emit statusMessage(statusText("status.write_failed"));
    }
}

void GameViewport::openLevel(const std::filesystem::path& path) {
    core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        HMI_LOG_WARNING("Editeur : ouverture impossible (" + path.string() + ") : " + loaded.error);
        emit statusMessage(
            statusText("status.open_failed").arg(QString::fromStdString(loaded.error)));
        return;
    }
    stopPlaytest();  // sort d'un éventuel essai en cours
    _draft = core::LevelDraft::fromLevel(*loaded.level);
    _dirty = false;
    _pendingLink.reset();
    _selectedLink.reset();
    markDraftMutated();
    HMI_LOG_INFO("Editeur : niveau ouvert : " + path.string());
    emit statusMessage(
        statusText("status.level_opened").arg(QString::fromStdString(path.filename().string())));
}

void GameViewport::startPlaytest() {
    if (_session) {
        return;  // déjà en essai
    }
    const core::LevelLoadResult validated = _draft.toLevel();
    if (!validated.ok()) {
        HMI_LOG_WARNING("Editeur : essai refuse (brouillon invalide) : " + validated.error);
        emit statusMessage(
            statusText("status.playtest_failed").arg(QString::fromStdString(validated.error)));
        return;
    }
    ensureResources();
    _session.emplace(*_spriteBatch, *_atlas, *_textureCache, pixelWidth(), pixelHeight(),
                     std::move(*validated.level), _gameBindings, _gamepadBindings);
    // Meme habillage qu'en edition : l'essai doit montrer exactement le canevas de l'editeur.
    _session->setSkins(&_skins);
    HMI_LOG_INFO("Editeur : essai immediat demarre.");
    emit statusMessage(statusText("status.playtesting"));
}

void GameViewport::stopPlaytest() {
    if (!_session) {
        return;
    }
    _session.reset();
    if (_draftRenderer) {
        _draftRenderer->invalidate();  // ré-affiche le brouillon (intact).
    }
    emit statusMessage(statusText("status.back_to_edit"));
}

void GameViewport::setVSync(bool enabled) noexcept {
    _vsync = enabled;
    HMI_LOG_INFO(std::string("Options : V-Sync ") + (enabled ? "activee." : "desactivee."));
    if (_graphics) {
        _graphics->setVSyncEnabled(enabled);
    }
}

void GameViewport::startGame(std::vector<std::filesystem::path> levels) {
    ensureResources();
    _gameLevels = std::move(levels);
    _gameMode = true;
    HMI_LOG_INFO("Jeu : demarrage de la sequence (" + std::to_string(_gameLevels.size()) +
                 " niveaux).");
    loadGameLevel(0);
}

void GameViewport::loadGameLevel(std::size_t index) {
    if (index >= _gameLevels.size()) {
        // Séquence terminée : retour au menu.
        _gameMode = false;
        _session.reset();
        HMI_LOG_INFO("Jeu : sequence terminee, retour au menu.");
        emit exitToMenuRequested();
        return;
    }
    core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(_gameLevels[index]);
    if (!loaded.ok()) {
        HMI_LOG_WARNING("Jeu : niveau illisible ignore (" + _gameLevels[index].string() +
                        ") : " + loaded.error);
        loadGameLevel(index + 1);  // niveau illisible : passe au suivant (robustesse)
        return;
    }
    _gameLevel = index;
    HMI_LOG_INFO("Jeu : niveau " + std::to_string(index) +
                 " charge : " + _gameLevels[index].filename().string());
    _session.emplace(*_spriteBatch, *_atlas, *_textureCache, pixelWidth(), pixelHeight(),
                     std::move(*loaded.level), _gameBindings, _gamepadBindings);
    _session->setSkins(&_skins);
}

void GameViewport::resizeLevel(int width, int height) {
    _draft.resize(width, height);
    _dirty = true;
    markDraftMutated();
    emit statusMessage(statusText("status.level_resized").arg(width).arg(height));
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

QString GameViewport::statusText(const char* key) const {
    return _loc != nullptr ? QString::fromStdString(_loc->text(key)) : QString::fromLatin1(key);
}

void GameViewport::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return;
    }
    if (const std::optional<hmi::Key> key = qtKeyToHmiKey(event->key())) {
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
        case hmi::EditorTool::Link:
            handleLinkClick(event);
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
            _selection =
                std::make_pair(core::GridPosition{std::min(_dragStart.column, _dragCurrent.column),
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

}  // namespace hmi
