// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <utility>

#include "HMI/Editor/ThumbnailGeometry.h"

/**
 * @file HMI/Editor/PixelCanvasGeometry.h
 * @brief Conversions pures vue ↔ image du canevas pixel art (LOT-54 TACHE-03, `EX-EDIT-045`).
 *
 * Aucune dépendance Qt/GPU (`EX-NFR-010`) : `hmi::PixelCanvas` (widget) se contente d'appeler ces
 * fonctions, qui restent seules responsables de la justesse à tout zoom, décalage de vue et
 * échelle d'affichage — c'est elle qui détermine le pixel survolé affiché dans la barre d'état
 * (TACHE-04) et doit donc rester juste dans tous les cas.
 */

namespace hmi {

/// Zoom minimal (1 pixel image = 1 pixel écran) et maximal admis par les commandes de zoom du
/// canevas — toujours **entier**, pour que les pixels restent carrés (`EX-ARCH-022`).
inline constexpr int PIXEL_CANVAS_MIN_ZOOM = 1;
inline constexpr int PIXEL_CANVAS_MAX_ZOOM = 32;

/// Facteur de zoom à partir duquel la grille de pixels est dessinée : en-deçà, elle ne ferait que
/// du bruit visuel sur des pixels trop petits pour la distinguer du contenu.
inline constexpr int PIXEL_CANVAS_GRID_VISIBLE_MIN_ZOOM = 4;

/// État de la vue du canevas : zoom entier et décalage de défilement, en pixels **image**.
struct PixelCanvasView {
    int zoom = 8;
    int panX = 0;  ///< Colonne de l'image affichée au coin haut-gauche du viewport.
    int panY = 0;  ///< Ligne de l'image affichée au coin haut-gauche du viewport.

    [[nodiscard]] friend bool operator==(const PixelCanvasView&,
                                         const PixelCanvasView&) noexcept = default;
};

/// Rectangle écran (pixels **logiques**, relatifs au coin haut-gauche du widget) d'un pixel image.
struct PixelScreenRect {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

/**
 * @brief Rectangle écran du pixel image `(x, y)`, au zoom et décalage de vue donnés.
 *
 * Fonction **pure** : ne dépend que de @p view, jamais des bornes de l'image — c'est elle qui
 * garantit que la grille, le pixel survolé et le tracé de sélection (TACHE-06) s'accordent
 * toujours exactement, puisqu'ils l'appellent tous.
 */
[[nodiscard]] PixelScreenRect imagePixelScreenRect(const PixelCanvasView& view, int x,
                                                   int y) noexcept;

/**
 * @brief Pixel image sous une position écran donnée.
 * @param view        Vue courante (zoom, décalage).
 * @param imageWidth  Largeur de l'image, en pixels.
 * @param imageHeight Hauteur de l'image, en pixels.
 * @param screenX     Position écran, en pixels logiques (ex. `QMouseEvent::position().x()`).
 * @param screenY     Position écran, en pixels logiques.
 * @return La position `(colonne, ligne)`, ou `std::nullopt` si elle tombe hors de l'image —
 *         jamais un indice tronqué au bord.
 */
[[nodiscard]] std::optional<std::pair<int, int>> screenToImagePixel(const PixelCanvasView& view,
                                                                    int imageWidth, int imageHeight,
                                                                    double screenX,
                                                                    double screenY) noexcept;

/// Zoom borné à `[PIXEL_CANVAS_MIN_ZOOM, PIXEL_CANVAS_MAX_ZOOM]`.
[[nodiscard]] constexpr int clampPixelCanvasZoom(int zoom) noexcept {
    if (zoom < PIXEL_CANVAS_MIN_ZOOM) {
        return PIXEL_CANVAS_MIN_ZOOM;
    }
    if (zoom > PIXEL_CANVAS_MAX_ZOOM) {
        return PIXEL_CANVAS_MAX_ZOOM;
    }
    return zoom;
}

/// Zoom suivant (plus grand), borné — toujours entier.
[[nodiscard]] constexpr int pixelCanvasZoomIn(int zoom) noexcept {
    return clampPixelCanvasZoom(zoom + 1);
}

/// Zoom précédent (plus petit), borné — toujours entier.
[[nodiscard]] constexpr int pixelCanvasZoomOut(int zoom) noexcept {
    return clampPixelCanvasZoom(zoom - 1);
}

/// `true` si la grille de pixels doit être dessinée à ce zoom
/// (`PIXEL_CANVAS_GRID_VISIBLE_MIN_ZOOM`).
[[nodiscard]] constexpr bool pixelCanvasGridVisible(int zoom) noexcept {
    return zoom >= PIXEL_CANVAS_GRID_VISIBLE_MIN_ZOOM;
}

/// Taille en pixels **réels** de l'image entière affichée (`largeur/hauteur image × zoom`, à
/// l'échelle d'affichage réelle).
struct PixelCanvasRealSize {
    int width = 0;
    int height = 0;
};

/**
 * @brief Taille en pixels réels du canevas entier à un facteur d'échelle d'affichage donné.
 *
 * Réutilise **directement** `hmi::thumbnailPixelSize` (`LOT-56` TACHE-05) — jamais réécrite :
 * c'est la même fonction qui dimensionne les vignettes de la bibliothèque et du panneau Textures,
 * pour que le canevas ne redevienne pas flou à 125 %/150 % le jour où elle évolue sans lui.
 * @param imageWidth  Largeur de l'image, en pixels.
 * @param imageHeight Hauteur de l'image, en pixels.
 * @param zoom        Facteur de zoom entier du canevas.
 * @param scaleFactor Facteur d'échelle du support d'affichage (`devicePixelRatio`).
 */
[[nodiscard]] PixelCanvasRealSize pixelCanvasRealSize(int imageWidth, int imageHeight, int zoom,
                                                      double scaleFactor) noexcept;

}  // namespace hmi
