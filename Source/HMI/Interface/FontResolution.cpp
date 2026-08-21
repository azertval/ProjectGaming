// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/FontResolution.h"

namespace hmi {

FontFamilyResolution resolveFontFamily(bool fontRegistered, const std::string& embeddedFamilyName) {
    if (!fontRegistered) {
        return FontFamilyResolution{};  // pas de nom : l'appelant retombe sur une famille
                                        // générique.
    }
    return FontFamilyResolution{.useEmbeddedFamily = true, .embeddedFamily = embeddedFamilyName};
}

}  // namespace hmi
