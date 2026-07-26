#include "Editor/GameViewport.h"

#include <algorithm>

#include <QExposeEvent>
#include <QResizeEvent>

// GraphicsDevice tire <Windows.h>/<d3d11.h> (HWND, device D3D11). Inclus après les en-têtes Qt
// pour éviter les conflits de macros (NOMINMAX/WIN32_LEAN_AND_MEAN sont définis par HmiRuntime).
#include "HMI/Graphics/GraphicsDevice.h"

namespace editor {

GameViewport::GameViewport(QWindow* parent) : QWindow(parent) {
    // Direct3D 11 présente directement sur le `HWND` natif via sa propre swap chain : on n'utilise
    // aucun `QBackingStore` Qt (aucun `QSurface` géré par Qt n'est créé), la surface par défaut
    // (Raster) sert seulement de fenêtre native hôte.
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

void GameViewport::renderFrame() {
    if (!isExposed()) {
        return;
    }
    ensureDevice();
    // Même couleur de fond que la boucle historique (parité visuelle).
    _graphics->clear(0.10f, 0.12f, 0.16f, 1.0f);
    _graphics->present();
}

void GameViewport::exposeEvent(QExposeEvent*) {
    if (isExposed()) {
        renderFrame();
    }
}

void GameViewport::resizeEvent(QResizeEvent*) {
    if (_graphics) {
        _graphics->resize(pixelWidth(), pixelHeight());
        renderFrame();
    }
}

}  // namespace editor
