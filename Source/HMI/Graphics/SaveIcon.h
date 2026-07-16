#pragma once

#include <d3d11.h>
#include <wrl/client.h>

/**
 * @file HMI/Graphics/SaveIcon.h
 * @brief Icône « enregistrer » (flèche de téléchargement) générée en code.
 */

namespace hmi {

/**
 * @brief Icône d'enregistrement **générée en code** : une flèche descendant vers un support.
 *
 * Petite icône monochrome (blanche sur fond transparent) symbolisant l'enregistrement des
 * logs. Blanche, elle se **teinte** au dessin. Ressources Direct3D en RAII (`ComPtr`).
 */
class SaveIcon {
public:
    /// Côté de l'icône, en pixels de texture.
    static constexpr int SIZE = 16;

    /**
     * @brief Génère la texture de l'icône et crée la ressource Direct3D associée.
     * @param device Device Direct3D 11 (crée la texture et sa vue de ressource).
     */
    explicit SaveIcon(ID3D11Device* device);

    /// @return La vue de ressource de la texture de l'icône (non possédée par l'appelant).
    [[nodiscard]] ID3D11ShaderResourceView* textureView() const {
        return _view.Get();
    }

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _view;
};

}  // namespace hmi
