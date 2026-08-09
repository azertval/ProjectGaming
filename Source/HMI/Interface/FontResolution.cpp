#include "HMI/Interface/FontResolution.h"

namespace hmi {

FontFamilyResolution resolveFontFamily(bool fontRegistered, const std::string& embeddedFamilyName) {
    if (!fontRegistered) {
        return FontFamilyResolution{};  // pas de nom : l'appelant retombe sur une famille générique.
    }
    return FontFamilyResolution{true, embeddedFamilyName};
}

}  // namespace hmi
