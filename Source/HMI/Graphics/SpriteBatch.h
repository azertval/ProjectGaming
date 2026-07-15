#pragma once

#include <cstddef>
#include <vector>

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

/**
 * @file HMI/Graphics/SpriteBatch.h
 * @brief Pipeline de rendu 2D : accumule des quads texturés et les dessine par lots.
 */

namespace hmi {

/**
 * @brief Un quad texturé à dessiner : rectangle en **unités monde**, région de texture
 *        (coordonnées normalisées) et teinte.
 *
 * Le coin est haut-gauche, l'axe Y va vers le bas (convention du projet). Les coordonnées
 * de texture `u,v` sont normalisées dans [0, 1] ; la conversion depuis une région d'atlas
 * en pixels est faite en amont (par le rendu des sprites).
 */
struct SpriteQuad {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

/**
 * @brief Dessine des quads texturés en Direct3D 11, avec transparence et échantillonnage
 *        *nearest* (pixel art).
 *
 * Usage : `begin(projection, texture)`, puis un ou plusieurs `draw(quad)`, puis `end()`.
 * Les quads sont accumulés dans un tampon puis émis en un minimum d'appels de dessin
 * (*batching*). Toutes les ressources Direct3D sont en RAII (`ComPtr`). L'objet ne possède
 * ni le device ni le contexte : ils doivent lui survivre.
 */
class SpriteBatch {
public:
    /**
     * @brief Construit le pipeline (shaders, buffers, états de rendu).
     * @param device  Device Direct3D 11 (crée les ressources).
     * @param context Contexte immédiat (émet les commandes de dessin), non possédé.
     */
    SpriteBatch(ID3D11Device* device, ID3D11DeviceContext* context);

    ~SpriteBatch() = default;

    SpriteBatch(const SpriteBatch&) = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    /**
     * @brief Démarre un lot : configure les états et la texture, et la projection caméra.
     * @param projection Matrice de projection monde → clip (fournie par la caméra).
     * @param texture    Vue de la texture d'atlas à échantillonner (non possédée).
     */
    void begin(const DirectX::XMFLOAT4X4& projection, ID3D11ShaderResourceView* texture);

    /**
     * @brief Ajoute un quad au lot courant.
     * @param quad Quad à dessiner (unités monde, UV normalisées, teinte).
     */
    void draw(const SpriteQuad& quad);

    /// Termine le lot : émet le dessin des quads accumulés.
    void end();

private:
    /// Sommet envoyé au GPU : position monde (x, y), UV, couleur RVBA.
    struct Vertex {
        float x;
        float y;
        float u;
        float v;
        float r;
        float g;
        float b;
        float a;
    };

    /// Nombre maximal de quads par appel de dessin (dimensionne les buffers).
    static constexpr std::size_t MAXIMUM_QUADS = 2048;

    /// Émet le dessin des quads accumulés et vide le tampon.
    void flush();

    ID3D11DeviceContext* _context;  // non possédé
    Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> _inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> _vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> _indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> _constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11BlendState> _blendState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState;
    ID3D11ShaderResourceView* _texture = nullptr;  // texture du lot courant, non possédée
    std::vector<Vertex> _vertices;
};

}  // namespace hmi
