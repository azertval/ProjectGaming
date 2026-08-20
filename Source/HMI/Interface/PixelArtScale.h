#pragma once

/**
 * @file HMI/Interface/PixelArtScale.h
 * @brief Facteur d'agrandissement **entier** des écrans du jeu (`LOT-68`, `EX-IHM-070`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `ProjectGaming` et directement dans `UnitTests`, comme
 * `HMI/Interface/DesignTokens.cpp`. Même découpage que `hmi::thumbnailPixelSize` : cette fonction
 * décide *de combien* agrandir, la couche Qt (`hmi::applyStyleSheet`) décide *quoi* multiplier.
 *
 * Le pixel art n'admet pas d'agrandissement fractionnaire : à 1,5×, une bordure d'un pixel devient
 * une bordure d'un pixel et demi, que le rastériseur rend tantôt sur un pixel, tantôt sur deux. La
 * contrainte est technique (`EX-IHM-053`, filtrage au plus proche voisin), pas esthétique.
 */

namespace hmi {

/// Hauteur, en pixels **logiques**, de la maquette à l'échelle 1. Les écrans sont dessinés pour
/// cette hauteur ; tout le reste en est un multiple entier.
inline constexpr int PIXEL_ART_BASE_HEIGHT = 360;

/// Plus grand facteur admis. Au-delà, les écrans cessent de gagner en lisibilité et commencent à
/// perdre en contenu visible : trois entrées de menu par écran ne rendent service à personne.
inline constexpr int PIXEL_ART_MAX_SCALE = 3;

/**
 * @brief Facteur d'agrandissement des écrans du jeu pour une hauteur de fenêtre donnée.
 *
 * La hauteur attendue est celle en pixels **logiques**, pas en pixels réels : la mise à l'échelle
 * du système d'exploitation est appliquée par Qt **par-dessus** ce facteur, sur une disposition
 * exprimée en unités logiques. Multiplier une seconde fois par `devicePixelRatio` ici produirait
 * une interface deux fois trop grande sur un écran réglé à 200 %.
 *
 * Le résultat est toujours un entier d'au moins 1, même pour une fenêtre plus petite que la
 * maquette : une fenêtre étroite doit rogner l'écran, jamais le rendre à une échelle fractionnaire.
 *
 * @param windowLogicalHeight Hauteur de la fenêtre, en pixels logiques.
 * @return Le facteur entier, dans `[1, PIXEL_ART_MAX_SCALE]`.
 */
[[nodiscard]] int pixelArtScale(int windowLogicalHeight) noexcept;

}  // namespace hmi
