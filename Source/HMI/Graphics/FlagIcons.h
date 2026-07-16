#pragma once

#include <string_view>

#include <d3d11.h>
#include <wrl/client.h>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion

/**
 * @file HMI/Graphics/FlagIcons.h
 * @brief Icônes de drapeaux (générées en code) pour le sélecteur de langue.
 */

namespace hmi {

/**
 * @brief Texture d'icônes de drapeaux **générée en code**, indexée par identifiant de langue.
 *
 * En l'absence d'asset graphique, les drapeaux sont produits en mémoire : le drapeau français
 * (trois bandes verticales) et un drapeau du Royaume-Uni approché (croix + sautoir) pour
 * l'anglais. Chaque drapeau occupe une région de la texture, exposée par langue (« fr »,
 * « en »). Ressources Direct3D en RAII (`ComPtr`). Remplaçable plus tard par un chargement de
 * fichiers si le nombre de langues croît.
 */
class FlagIcons {
public:
    /// Largeur d'un drapeau, en pixels de texture.
    static constexpr int FLAG_WIDTH = 24;
    /// Hauteur d'un drapeau, en pixels de texture.
    static constexpr int FLAG_HEIGHT = 16;

    /**
     * @brief Génère la texture des drapeaux et crée la ressource Direct3D associée.
     * @param device Device Direct3D 11 (crée la texture et sa vue de ressource).
     */
    explicit FlagIcons(ID3D11Device* device);

    /// @return La vue de ressource de la texture des drapeaux (non possédée par l'appelant).
    [[nodiscard]] ID3D11ShaderResourceView* textureView() const {
        return _view.Get();
    }

    /// @return Largeur totale de la texture, en pixels.
    [[nodiscard]] int width() const noexcept {
        return _width;
    }

    /// @return Hauteur totale de la texture, en pixels.
    [[nodiscard]] int height() const noexcept {
        return FLAG_HEIGHT;
    }

    /**
     * @brief Région du drapeau associé à une langue.
     * @param language Identifiant de langue (« fr » ou « en »).
     * @return La région (en pixels) ; le drapeau français par défaut si la langue est inconnue.
     */
    [[nodiscard]] core::AtlasRegion region(std::string_view language) const;

private:
    int _width = 0;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _view;
};

}  // namespace hmi
