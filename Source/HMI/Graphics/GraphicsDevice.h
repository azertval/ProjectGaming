#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

/**
 * @file HMI/Graphics/GraphicsDevice.h
 * @brief Initialisation Direct3D 11 et présentation de l'image.
 */

namespace hmi {

/**
 * @brief Encapsule le device Direct3D 11, sa swap chain et la cible de rendu (RAII).
 *
 * Toutes les ressources Direct3D sont gérées par des pointeurs intelligents COM
 * (ComPtr) : leur libération est automatique. La classe fournit l'effacement de
 * l'écran, la présentation synchronisée (V-Sync, activable/désactivable — `EX-REN-022`) et le
 * redimensionnement.
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

    /// Présente l'image à l'écran, synchronisée (V-Sync) selon `vsyncEnabled()`.
    void present();

    /// @return true si la présentation est synchronisée au rafraîchissement de l'écran (V-Sync).
    [[nodiscard]] bool vsyncEnabled() const noexcept {
        return _vsyncEnabled;
    }

    /**
     * @brief Active ou désactive la synchronisation verticale (`EX-REN-022`), effective au
     *        prochain `present()`.
     * @param enabled true = présentation calée sur le rafraîchissement (pas de tearing) ;
     *                false = présentation immédiate.
     */
    void setVSyncEnabled(bool enabled) noexcept {
        _vsyncEnabled = enabled;
    }

    /// @return Le device Direct3D 11 (non possédé par l'appelant), pour créer des ressources.
    [[nodiscard]] ID3D11Device* device() const {
        return _device.Get();
    }

    /// @return Le contexte immédiat Direct3D 11 (non possédé par l'appelant), pour dessiner.
    [[nodiscard]] ID3D11DeviceContext* context() const {
        return _context.Get();
    }

    /// @return Largeur courante de la zone de rendu, en pixels.
    [[nodiscard]] int width() const {
        return _width;
    }

    /// @return Hauteur courante de la zone de rendu, en pixels.
    [[nodiscard]] int height() const {
        return _height;
    }

private:
    /// Crée la render target view à partir du back buffer et configure le viewport.
    void createRenderTarget();

    Microsoft::WRL::ComPtr<ID3D11Device> _device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _renderTargetView;
    int _width;
    int _height;
    bool _vsyncEnabled = true;
};

}  // namespace hmi
