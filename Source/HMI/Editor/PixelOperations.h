#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "HMI/Graphics/TextureLoader.h"

/**
 * @file HMI/Editor/PixelOperations.h
 * @brief Opérations pures sur un tampon de pixels RGBA (atelier pixel art, LOT-54 TACHE-02).
 *
 * Fonctions **libres**, sans état, sans Qt ni GPU (`EX-NFR-010`) : ce que fait chaque outil du
 * canevas (TACHE-03) sur `hmi::DecodedImage` — le même type que `decodeImageFile`/`encodeImageFile`
 * (TACHE-01), pour que le canevas édite, charge et enregistre le même tampon sans conversion.
 */

namespace hmi {

/**
 * @brief Rectangle de pixels modifié par une opération (bornes incluses).
 *
 * Sert à borner le coût mémoire de l'historique (`hmi::PixelHistory`) : une entrée mémorise le
 * contenu de cette région avant/après plutôt qu'une copie complète de l'image.
 */
struct PixelRegion {
    int minX = 0;
    int minY = 0;
    int maxX = -1;  ///< `maxX < minX` (valeur par défaut) : région vide.
    int maxY = -1;

    /// @return `true` si la région ne couvre aucun pixel.
    [[nodiscard]] constexpr bool empty() const noexcept {
        return maxX < minX || maxY < minY;
    }
    /// @return Largeur de la région, `0` si vide.
    [[nodiscard]] constexpr int width() const noexcept {
        return empty() ? 0 : (maxX - minX + 1);
    }
    /// @return Hauteur de la région, `0` si vide.
    [[nodiscard]] constexpr int height() const noexcept {
        return empty() ? 0 : (maxY - minY + 1);
    }

    /// @return `true` si les deux régions couvrent exactement les mêmes pixels.
    [[nodiscard]] constexpr bool operator==(const PixelRegion& other) const noexcept {
        return (empty() && other.empty()) ||
              (minX == other.minX && minY == other.minY && maxX == other.maxX &&
               maxY == other.maxY);
    }
};

/// @return `true` si `(x, y)` est dans les bornes d'une image `width` x `height`.
[[nodiscard]] constexpr bool pixelInBounds(int width, int height, int x, int y) noexcept {
    return x >= 0 && y >= 0 && x < width && y < height;
}

/**
 * @brief Union de deux régions (la plus petite région couvrant les deux).
 *
 * Une région vide n'affecte pas l'union : sert à accumuler la région touchée par un geste complet
 * (`hmi::PixelCanvas`, TACHE-03) à partir des régions renvoyées par chacune de ses sous-opérations
 * (ex. plusieurs `drawLine` successifs pendant un glisser).
 */
[[nodiscard]] constexpr PixelRegion unionPixelRegion(const PixelRegion& a,
                                                      const PixelRegion& b) noexcept {
    if (a.empty()) {
        return b;
    }
    if (b.empty()) {
        return a;
    }
    return PixelRegion{
        a.minX < b.minX ? a.minX : b.minX,
        a.minY < b.minY ? a.minY : b.minY,
        a.maxX > b.maxX ? a.maxX : b.maxX,
        a.maxY > b.maxY ? a.maxY : b.maxY,
    };
}

/**
 * @brief Pose un pixel d'une couleur donnée.
 * @param image Image modifiée en place.
 * @param x     Colonne (0-based).
 * @param y     Ligne (0-based).
 * @param color Couleur `R8G8B8A8_UNORM` posée.
 * @return La région modifiée (le seul pixel `(x, y)`), vide si `(x, y)` est hors bornes.
 */
PixelRegion setPixel(DecodedImage& image, int x, int y, std::uint32_t color);

/**
 * @brief Efface un pixel (alpha nul), sans changer sa couleur RVB.
 *
 * Alpha nul, RVB **conservé** plutôt que mis à zéro : un pixel effacé puis re-coloré sans jamais
 * changer sa teinte reproduit l'attente naturelle d'une gomme.
 * @param image Image modifiée en place.
 * @param x     Colonne (0-based).
 * @param y     Ligne (0-based).
 * @return La région modifiée (le seul pixel `(x, y)`), vide si `(x, y)` est hors bornes.
 */
PixelRegion erasePixel(DecodedImage& image, int x, int y);

/**
 * @brief Trace une ligne pleine entre deux positions (algorithme de Bresenham).
 *
 * Sans elle, un glisser rapide de souris entre deux positions successives d'un même geste laisse
 * des pixels isolés au lieu d'un trait continu — le défaut le plus visible d'un canevas pixel art
 * fait naïvement.
 * @param image Image modifiée en place.
 * @param x0    Colonne de départ.
 * @param y0    Ligne de départ.
 * @param x1    Colonne d'arrivée.
 * @param y1    Ligne d'arrivée.
 * @param color Couleur posée sur chaque pixel du tracé.
 * @return La région englobante des pixels effectivement posés (vide si tout le segment est hors
 *         bornes).
 */
PixelRegion drawLine(DecodedImage& image, int x0, int y0, int x1, int y1, std::uint32_t color);

/**
 * @brief Efface une ligne pleine entre deux positions (même tracé que `drawLine`, alpha nul plutôt
 *        qu'une couleur posée).
 *
 * Pendant de `drawLine` pour la gomme : un glisser rapide avec la gomme doit laisser une trace
 * continue effacée, pas des pixels effacés isolés — même défaut, même correction.
 * @param image Image modifiée en place.
 * @param x0    Colonne de départ.
 * @param y0    Ligne de départ.
 * @param x1    Colonne d'arrivée.
 * @param y1    Ligne d'arrivée.
 * @return La région englobante des pixels effectivement effacés (vide si tout le segment est hors
 *         bornes).
 */
PixelRegion eraseLine(DecodedImage& image, int x0, int y0, int x1, int y1);

/**
 * @brief Remplit, de proche en proche, la zone connexe (4-connexité) de même couleur que le pixel
 *        de départ.
 *
 * Itératif (file explicite), **jamais récursif** : une récursion sur une grande zone déborderait
 * la pile. Sans effet — et sans parcourir quoi que ce soit — si `(x, y)` est hors bornes ou si la
 * couleur de départ est déjà @p color, ce qui évite à la fois une opération d'historique vide et
 * une boucle sans fin sur une image déjà uniforme.
 * @param image Image modifiée en place.
 * @param x     Colonne du point de départ.
 * @param y     Ligne du point de départ.
 * @param color Couleur de remplissage.
 * @return La région englobante des pixels effectivement remplis (vide si sans effet).
 */
PixelRegion floodFill(DecodedImage& image, int x, int y, std::uint32_t color);

/**
 * @brief Couleur du pixel à une position donnée (outil pipette).
 * @param image Image consultée.
 * @param x     Colonne.
 * @param y     Ligne.
 * @return La couleur du pixel, ou `std::nullopt` si `(x, y)` est hors bornes.
 */
[[nodiscard]] std::optional<std::uint32_t> pickColor(const DecodedImage& image, int x,
                                                      int y) noexcept;

/**
 * @brief Copie les pixels d'une région, ligne par ligne — capture le « avant » ou l'« après »
 *        d'une entrée d'historique (`hmi::PixelHistory`).
 * @param image  Image consultée.
 * @param region Région à copier (doit être contenue dans les bornes de @p image).
 * @return Les pixels de la région, taille `region.width() * region.height()` ; vide si
 *         @p region est vide.
 */
[[nodiscard]] std::vector<std::uint32_t> readRegion(const DecodedImage& image,
                                                     const PixelRegion& region);

/**
 * @brief Écrit des pixels dans une région — inverse de `readRegion`, utilisé par
 *        `hmi::PixelHistory` pour restaurer un instantané annulé/rétabli.
 * @param image  Image modifiée en place.
 * @param region Région ciblée (doit être contenue dans les bornes de @p image).
 * @param data   Pixels à écrire, taille attendue `region.width() * region.height()`.
 */
void writeRegion(DecodedImage& image, const PixelRegion& region,
                 const std::vector<std::uint32_t>& data);

}  // namespace hmi
