// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/PixelCanvasGeometry.h"

#include <cmath>

namespace hmi {

PixelScreenRect imagePixelScreenRect(const PixelCanvasView& view, int x, int y) noexcept {
    const auto zoom = static_cast<double>(view.zoom);
    return PixelScreenRect{
        .x = static_cast<double>(x - view.panX) * zoom,
        .y = static_cast<double>(y - view.panY) * zoom,
        .width = zoom,
        .height = zoom,
    };
}

std::optional<std::pair<int, int>> screenToImagePixel(const PixelCanvasView& view, int imageWidth,
                                                      int imageHeight, double screenX,
                                                      double screenY) noexcept {
    if (view.zoom <= 0) {
        return std::nullopt;
    }
    // floor(), jamais une division/cast tronquant vers zero : une position ecran negative doit
    // produire une colonne/ligne strictement decroissante, pas revenir vers 0.
    const int column = view.panX + static_cast<int>(std::floor(screenX / view.zoom));
    const int row = view.panY + static_cast<int>(std::floor(screenY / view.zoom));
    if (column < 0 || row < 0 || column >= imageWidth || row >= imageHeight) {
        return std::nullopt;
    }
    return std::make_pair(column, row);
}

PixelCanvasRealSize pixelCanvasRealSize(int imageWidth, int imageHeight, int zoom,
                                        double scaleFactor) noexcept {
    return PixelCanvasRealSize{
        .width = thumbnailPixelSize(imageWidth * zoom, scaleFactor),
        .height = thumbnailPixelSize(imageHeight * zoom, scaleFactor),
    };
}

}  // namespace hmi
