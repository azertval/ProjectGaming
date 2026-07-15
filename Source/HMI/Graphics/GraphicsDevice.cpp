#include "HMI/Graphics/GraphicsDevice.h"

#include <iterator>
#include <stdexcept>

namespace hmi {

namespace {
/// Lève une exception si un appel Direct3D a échoué (frontière de démarrage).
void throwIfFailed(HRESULT result, const char* message) {
    if (FAILED(result)) {
        throw std::runtime_error(message);
    }
}
}  // namespace

/**
 * @brief Initialise Direct3D 11 pour la fenêtre donnée.
 * @param window Handle de la fenêtre cible.
 * @param width  Largeur initiale de la zone client, en pixels.
 * @param height Hauteur initiale de la zone client, en pixels.
 */
GraphicsDevice::GraphicsDevice(HWND window, int width, int height) : _width(width), _height(height) {
    DXGI_SWAP_CHAIN_DESC description{};
    description.BufferCount = 1;
    description.BufferDesc.Width = static_cast<UINT>(width);
    description.BufferDesc.Height = static_cast<UINT>(height);
    description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.BufferDesc.RefreshRate.Numerator = 60;
    description.BufferDesc.RefreshRate.Denominator = 1;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.OutputWindow = window;
    description.SampleDesc.Count = 1;
    description.SampleDesc.Quality = 0;
    description.Windowed = TRUE;
    description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
#ifdef _DEBUG
    // La couche de debug facilite le suivi des fuites et des mauvais usages.
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL requestedLevels[] = {D3D_FEATURE_LEVEL_11_0};

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, requestedLevels,
        static_cast<UINT>(std::size(requestedLevels)), D3D11_SDK_VERSION, &description, &_swapChain,
        &_device, nullptr, &_context);

#ifdef _DEBUG
    // La couche de debug n'est disponible que si les « Graphics Tools » de Windows
    // sont installés. En leur absence, on retente sans elle plutôt que d'échouer.
    if (FAILED(result) && (flags & D3D11_CREATE_DEVICE_DEBUG) != 0) {
        flags &= ~static_cast<UINT>(D3D11_CREATE_DEVICE_DEBUG);
        result = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, requestedLevels,
            static_cast<UINT>(std::size(requestedLevels)), D3D11_SDK_VERSION, &description,
            &_swapChain, &_device, nullptr, &_context);
    }
#endif

    throwIfFailed(result, "Echec de l'initialisation de Direct3D 11");
    createRenderTarget();
}

/// Crée la render target view à partir du back buffer et configure le viewport.
void GraphicsDevice::createRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    throwIfFailed(_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)),
                  "Echec de recuperation du back buffer");
    throwIfFailed(_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &_renderTargetView),
                  "Echec de creation de la render target view");
    _context->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), nullptr);

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(_width);
    viewport.Height = static_cast<float>(_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    _context->RSSetViewports(1, &viewport);
}

/**
 * @brief Redimensionne les buffers de la swap chain.
 * @param width  Nouvelle largeur, en pixels (ignoré si nul).
 * @param height Nouvelle hauteur, en pixels (ignoré si nul).
 */
void GraphicsDevice::resize(int width, int height) {
    // Une fenêtre minimisée a une taille client nulle : on ne redimensionne pas.
    if (width <= 0 || height <= 0) {
        return;
    }

    // La render target view référence le back buffer : elle doit être libérée
    // avant ResizeBuffers, sinon l'opération échoue.
    _renderTargetView.Reset();
    throwIfFailed(_swapChain->ResizeBuffers(0, static_cast<UINT>(width), static_cast<UINT>(height),
                                            DXGI_FORMAT_UNKNOWN, 0),
                  "Echec du redimensionnement de la swap chain");

    _width = width;
    _height = height;
    createRenderTarget();
}

/**
 * @brief Efface la cible de rendu à une couleur.
 * @param red   Composante rouge [0, 1].
 * @param green Composante verte [0, 1].
 * @param blue  Composante bleue [0, 1].
 * @param alpha Composante alpha [0, 1].
 */
void GraphicsDevice::clear(float red, float green, float blue, float alpha) {
    const float color[4] = {red, green, blue, alpha};
    _context->ClearRenderTargetView(_renderTargetView.Get(), color);
}

/// Présente l'image à l'écran avec synchronisation verticale (V-Sync).
void GraphicsDevice::present() {
    // Intervalle de synchronisation = 1 : présentation calée sur le rafraîchissement
    // de l'écran, ce qui supprime le tearing.
    _swapChain->Present(1, 0);
}

}  // namespace hmi
