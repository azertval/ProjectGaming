#include "Editor/GameViewport.h"

#include <algorithm>
#include <optional>

#include <QEvent>
#include <QExposeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWheelEvent>

// GraphicsDevice tire <Windows.h>/<d3d11.h> (HWND, device D3D11). Inclus après les en-têtes Qt
// pour éviter les conflits de macros (NOMINMAX/WIN32_LEAN_AND_MEAN sont définis par HmiRuntime).
#include "HMI/Graphics/GraphicsDevice.h"

namespace editor {

namespace {

// Traduit un code de touche Qt en `hmi::Key` (code virtuel Win32). Les lettres/chiffres/espace
// partagent déjà la même valeur entre Qt et Win32 (Qt::Key_A == VK 'A' == 0x41) ; seules les
// touches spéciales demandent une correspondance explicite. `nullopt` = touche non suivie.
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
    // Lettres A–Z et chiffres 0–9 : Qt reprend les valeurs ASCII majuscules, identiques aux codes
    // virtuels Win32 — conversion directe.
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

}  // namespace

GameViewport::GameViewport(QWindow* parent) : QWindow(parent) {
    // Direct3D 11 présente directement sur le `HWND` natif via sa propre swap chain : on n'utilise
    // aucun `QBackingStore` Qt, la surface par défaut (Raster) sert seulement de fenêtre native hôte.
}

GameViewport::~GameViewport() = default;

int GameViewport::pixelWidth() const {
    return std::max(1, static_cast<int>(width() * devicePixelRatio()));
}

int GameViewport::pixelHeight() const {
    return std::max(1, static_cast<int>(height() * devicePixelRatio()));
}

void GameViewport::ensureDevice() {
    if (_graphics) {
        return;
    }
    const HWND handle = reinterpret_cast<HWND>(winId());
    _graphics = std::make_unique<hmi::GraphicsDevice>(handle, pixelWidth(), pixelHeight());
}

void GameViewport::tick() {
    if (!isExposed()) {
        return;  // fenêtre masquée : la boucle reprend au prochain exposeEvent.
    }
    ensureDevice();

    const Clock::time_point now = Clock::now();
    const float elapsedSeconds = std::chrono::duration<float>(now - _previousFrame).count();
    _previousFrame = now;

    // 1. Manette (sondée, pas événementielle) fusionnée dans l'état courant.
    _gamepad.poll(_input);

    // 2. Accumulateur : temps réel -> nombre entier de pas fixes (déterminisme, EX-NFR-002). Les
    //    fronts d'entrée n'avancent qu'APRÈS chaque pas consommé (LOT-33), jamais par frame de rendu.
    const int steps = _timestep.advance(elapsedSeconds);
    for (int step = 0; step < steps; ++step) {
        // TACHE-04 : mise à jour de la session de jeu ici (input courant, _timestep.fixedDelta()).
        _input.beginFrame();
    }

    // 3. Rendu une fois par frame réelle (interpolation exploitée à la TACHE-04).
    renderFrame();

    // 4. Reprogramme le prochain tick (calé sur la présentation/compositeur).
    requestUpdate();
}

void GameViewport::renderFrame() {
    if (!isExposed()) {
        return;
    }
    ensureDevice();
    _graphics->clear(0.10f, 0.12f, 0.16f, 1.0f);
    _graphics->present();
}

bool GameViewport::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::UpdateRequest:
            tick();
            return true;
        case QEvent::FocusOut:
            // Perte de focus : relâche toutes les entrées maintenues (pas de touche « collée » au
            // retour d'un Alt+Tab), sans front parasite (LOT-33).
            _input.releaseAll();
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
    ensureDevice();
    if (!_loopStarted) {
        _loopStarted = true;
        _previousFrame = Clock::now();
    }
    requestUpdate();  // (re)démarre la boucle.
}

void GameViewport::resizeEvent(QResizeEvent*) {
    if (_graphics) {
        _graphics->resize(pixelWidth(), pixelHeight());
        renderFrame();
    }
}

void GameViewport::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return;  // un maintien ne doit pas produire de fronts répétés.
    }
    if (const std::optional<hmi::Key> key = mapQtKey(event->key())) {
        _input.onKeyDown(*key);
    }
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
}

void GameViewport::mouseReleaseEvent(QMouseEvent* event) {
    updateMousePosition(event);
    if (const std::optional<hmi::MouseButton> button = mapQtMouseButton(event->button())) {
        _input.onMouseButtonUp(*button);
    }
}

void GameViewport::mouseMoveEvent(QMouseEvent* event) {
    updateMousePosition(event);
}

void GameViewport::wheelEvent(QWheelEvent* event) {
    // angleDelta().y() est en 1/8 de degré, 120 par cran — même unité que Win32 (`WHEEL_DELTA`).
    _input.onMouseWheel(event->angleDelta().y());
}

}  // namespace editor
