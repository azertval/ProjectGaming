#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

/**
 * @file HMI/GraphicsDevice.h
 * @brief Initialisation Direct3D 11 et présentation de l'image.
 */

namespace hmi {

/**
 * @brief Encapsule le device Direct3D 11, sa swap chain et la cible de rendu (RAII).
 *
 * Toutes les ressources Direct3D sont gérées par des pointeurs intelligents COM
 * (ComPtr) : leur libération est automatique. La classe fournit l'effacement de
 * l'écran, la présentation synchronisée (V-Sync) et le redimensionnement.
 */
class GraphicsDevice {
public:
    /**
     * @brief Initialise Direct3D 11 pour la fenêtre donnée.
     * @param window Handle de la fenêtre cible.
     * @param width  Largeur initiale de la zone client, en pixels.
     * @param height Hauteur initiale de la zone client, en pixels.
     */
    GraphicsDevice(HWND window, int width, int height);

    ~GraphicsDevice() = default;

    GraphicsDevice(const GraphicsDevice&) = delete;
    GraphicsDevice& operator=(const GraphicsDevice&) = delete;

    /**
     * @brief Redimensionne les buffers de la swap chain.
     * @param width  Nouvelle largeur, en pixels (ignoré si nul).
     * @param height Nouvelle hauteur, en pixels (ignoré si nul).
     */
    void resize(int width, int height);

    /**
     * @brief Efface la cible de rendu à une couleur.
     * @param red   Composante rouge [0, 1].
     * @param green Composante verte [0, 1].
     * @param blue  Composante bleue [0, 1].
     * @param alpha Composante alpha [0, 1].
     */
    void clear(float red, float green, float blue, float alpha);

    /// Présente l'image à l'écran avec synchronisation verticale (V-Sync).
    void present();

private:
    /// Crée la render target view à partir du back buffer et configure le viewport.
    void createRenderTarget();

    Microsoft::WRL::ComPtr<ID3D11Device> _device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _renderTargetView;
    int _width;
    int _height;
};

}  // namespace hmi
