#pragma once

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "Core/Ecs/Components/Sprite.h"  // core::Color

/**
 * @file HMI/Graphics/BitmapFont.h
 * @brief Police bitmap à chasse fixe, générée en code, et dessin de texte via SpriteBatch.
 */

namespace hmi {

class SpriteBatch;

/**
 * @brief Police bitmap **monospace** générée en mémoire, et rendu de texte en espace écran.
 *
 * En l'absence d'asset de police, la texture est produite en code (`EX-REN-032`) : chaque
 * caractère occupe une **cellule de taille fixe** dessinée à partir de données de glyphes
 * 5×7 intégrées (déterministes). Les lettres **accentuées** nécessaires au français sont
 * composées d'une lettre de base et d'un signe diacritique (accent aigu, grave, circonflexe,
 * tréma, cédille), ce qui couvre les libellés du catalogue de traduction sans dupliquer les
 * glyphes. Les glyphes sont **blancs** sur fond transparent : la teinte passée à `drawText`
 * les colore (le nuanceur multiplie texture × couleur). Le texte est rendu en **espace écran**
 * (pixels), indépendamment de la caméra du monde, avec échantillonnage *nearest* (`EX-ARCH-022`).
 * Ressources Direct3D en RAII (`ComPtr`).
 */
class BitmapFont {
public:
    /// Largeur d'une cellule de glyphe, en pixels. En chasse fixe, c'est aussi l'avance.
    static constexpr int CELL_WIDTH = 6;

    /// Hauteur d'une cellule de glyphe, en pixels (zone d'accents + corps 5×7 + cédille).
    static constexpr int CELL_HEIGHT = 10;

    /**
     * @brief Génère la texture de police et crée la ressource Direct3D associée.
     * @param device Device Direct3D 11 (crée la texture et sa vue de ressource).
     */
    explicit BitmapFont(ID3D11Device* device);

    ~BitmapFont() = default;

    BitmapFont(const BitmapFont&) = delete;
    BitmapFont& operator=(const BitmapFont&) = delete;

    /// @return La vue de ressource de la texture de police (non possédée par l'appelant).
    [[nodiscard]] ID3D11ShaderResourceView* textureView() const {
        return _view.Get();
    }

    /**
     * @brief Dessine une chaîne UTF-8 dans le lot courant, en espace écran (pixels).
     * @param batch Lot **déjà démarré** (`begin`) avec la texture de police (`textureView()`)
     *              et une projection écran (`screenProjection`).
     * @param text  Texte à dessiner, encodé en **UTF-8**.
     * @param x     Abscisse du coin haut-gauche du texte, en pixels.
     * @param y     Ordonnée du coin haut-gauche du texte, en pixels.
     * @param scale Facteur d'échelle (entier recommandé pour la netteté pixel art).
     * @param color Teinte appliquée aux glyphes (blancs à l'origine, donc colorés par elle).
     *
     * Émet un quad par caractère couvert. Un caractère non couvert est **ignoré** (il avance
     * comme une espace, sans dessin ni plantage). Le saut de ligne `\n` revient à l'abscisse
     * de départ, une ligne plus bas.
     */
    void drawText(SpriteBatch& batch, std::string_view text, float x, float y, float scale,
                  const core::Color& color) const;

    /**
     * @brief Largeur en pixels qu'occuperait @p text à l'échelle @p scale (chasse fixe).
     * @param text  Texte mesuré (UTF-8) ; les sauts de ligne ne sont pas pris en compte.
     * @param scale Facteur d'échelle.
     * @return La largeur, utile pour centrer un libellé.
     */
    [[nodiscard]] float textWidth(std::string_view text, float scale) const;

    /// @return Hauteur d'une ligne de texte à l'échelle @p scale, en pixels.
    [[nodiscard]] float lineHeight(float scale) const noexcept;

    /**
     * @brief Construit une projection orthographique **espace écran → clip**.
     * @param viewportWidth  Largeur de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur de la surface de rendu, en pixels.
     * @return Matrice mappant (0,0) au coin haut-gauche et (w,h) au coin bas-droit (Y vers le bas).
     *
     * À passer à `SpriteBatch::begin` pour dessiner l'interface en pixels, indépendamment de
     * la caméra du monde.
     */
    [[nodiscard]] static DirectX::XMFLOAT4X4 screenProjection(int viewportWidth,
                                                              int viewportHeight) noexcept;

private:
    /// @return Indice de cellule couvrant @p codePoint, ou -1 si non couvert.
    [[nodiscard]] int cellForCodePoint(char32_t codePoint) const;

    int _columns = 0;       ///< Nombre de cellules par ligne dans la texture de police.
    int _textureWidth = 0;  ///< Largeur de la texture de police, en pixels.
    int _textureHeight = 0;  ///< Hauteur de la texture de police, en pixels.
    std::unordered_map<char32_t, int> _cellIndex;  ///< Code point → indice de cellule.
    Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _view;
};

}  // namespace hmi
