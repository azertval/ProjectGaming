// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/ThumbnailGeometry.h"

#include <algorithm>
#include <cmath>

namespace hmi {

int thumbnailPixelSize(int logicalSize, double scaleFactor) noexcept {
    const double pixels = std::round(static_cast<double>(logicalSize) * scaleFactor);
    return std::max(1, static_cast<int>(pixels));
}

}  // namespace hmi
