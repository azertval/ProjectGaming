#pragma once

#include <vector>

/**
 * @file HMI/Interface/PixelFrameGeometry.h
 * @brief Géométrie du cadre pixel art des écrans du jeu (`LOT-68`, `EX-IHM-070`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — même patron que `hmi::iconGeometry` : cette fonction décide *quoi* dessiner,
 * `hmi::PixelFrameWidget` (Qt) décide *comment* le peindre, en résolvant chaque rôle depuis les
 * jetons de la portée identité.
 *
 * Aucun fichier d'image n'est livré. Un cadre 9 tranches en PNG figerait ses couleurs hors des
 * jetons (`EX-IHM-051`) et devrait être réexporté à chaque changement de palette ; tracé par code,
 * il suit la palette sans qu'on y pense — exactement le raisonnement qui a mené aux icônes
 * vectorielles du `LOT-56`.
 */

namespace hmi {

/// Rôle de couleur d'un pavé, résolu depuis les jetons au moment du rendu.
enum class PixelFrameRole {
    Fill,        ///< Intérieur du cadre (`surface`).
    Outline,     ///< Contour extérieur, la couleur la plus sombre de la palette.
    BevelLight,  ///< Biseau haut et gauche : la lumière vient d'en haut à gauche.
    BevelDark,   ///< Biseau bas et droite.
};

/// Un pavé plein, en pixels, relatif au coin haut-gauche du widget.
struct PixelFrameQuad {
    PixelFrameRole role;
    int x;
    int y;
    int width;
    int height;

    [[nodiscard]] friend bool operator==(const PixelFrameQuad&, const PixelFrameQuad&) noexcept =
        default;
};

/**
 * @brief Pavés composant un cadre, dans l'**ordre de dessin** (les derniers recouvrent les
 *        premiers).
 *
 * Les coins sont **entaillés** et non arrondis : le contour laisse un pavé de côté à chaque angle.
 * C'est ce détail, et non l'épaisseur du trait, qui distingue un cadre pixel art d'une simple
 * bordure carrée — un `border-radius`, même d'un pixel, produit un dégradé d'anticrénelage que le
 * filtrage au plus proche voisin ne peut pas rendre.
 *
 * Tous les pavés retournés sont contenus dans `[0, width] x [0, height]`, y compris lorsque le
 * cadre est trop petit pour porter ses bordures : dans ce cas la géométrie se réduit, elle ne
 * déborde jamais.
 *
 * @param width  Largeur du cadre, en pixels.
 * @param height Hauteur du cadre, en pixels.
 * @param scale  Facteur d'agrandissement entier (`hmi::pixelArtScale`) : l'épaisseur du contour et
 *               celle du biseau valent chacune un pixel de maquette, donc @p scale pixels réels.
 * @return Les pavés à peindre ; vide si @p width ou @p height est nul ou négatif.
 */
[[nodiscard]] std::vector<PixelFrameQuad> pixelFrameQuads(int width, int height, int scale);

/**
 * @brief Pavés du **curseur de focus** — un triangle plein pointant à droite, inscrit dans un carré
 *        de côté @p size (`LOT-68`, `EX-IHM-071`).
 *
 * Tous les pavés portent le rôle `Fill` : le curseur est monochrome, et c'est le peintre qui
 * décide de sa couleur (l'accent). Un triangle tracé en pavés plutôt qu'en polygone parce qu'un
 * polygone incliné serait anticrénelé, ce que le pixel art ne tolère pas.
 *
 * @param size Côté du carré englobant, en pixels. Ramené au multiple de 8 immédiatement inférieur
 *             pour que les huit rangées du dessin restent d'épaisseur égale ; en dessous de 8, le
 *             curseur est vide plutôt que déformé.
 * @return Les pavés à peindre, relatifs au coin haut-gauche du carré.
 */
[[nodiscard]] std::vector<PixelFrameQuad> pixelCaretQuads(int size);

}  // namespace hmi
