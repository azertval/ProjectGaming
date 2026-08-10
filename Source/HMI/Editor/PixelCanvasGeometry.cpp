#include "HMI/Editor/PixelCanvasGeometry.h"

#include <cmath>

namespace hmi {

PixelScreenRect imagePixelScreenRect(const PixelCanvasView& view, int x, int y) noexcept {
    const double zoom = static_cast<double>(view.zoom);
    return PixelScreenRect{
        static_cast<double>(x - view.panX) * zoom,
        static_cast<double>(y - view.panY) * zoom,
        zoom,
        zoom,
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
        thumbnailPixelSize(imageWidth * zoom, scaleFactor),
        thumbnailPixelSize(imageHeight * zoom, scaleFactor),
    };
}

}  // namespace hmi
