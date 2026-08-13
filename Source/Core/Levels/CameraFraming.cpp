#include "Core/Levels/CameraFraming.h"

namespace core {

// Nom JSON du mode (voir en-tete).
std::string_view cameraFramingModeName(CameraFramingMode mode) noexcept {
    switch (mode) {
        case CameraFramingMode::WholeLevel:
            return "wholeLevel";
        case CameraFramingMode::PerRoom:
            return "perRoom";
        case CameraFramingMode::Follow:
            return "follow";
    }
    return "wholeLevel";
}

// Reconnait le nom JSON d'un mode (voir en-tete).
std::optional<CameraFramingMode> parseCameraFramingMode(std::string_view name) noexcept {
    if (name == "wholeLevel") {
        return CameraFramingMode::WholeLevel;
    }
    if (name == "perRoom") {
        return CameraFramingMode::PerRoom;
    }
    if (name == "follow") {
        return CameraFramingMode::Follow;
    }
    return std::nullopt;
}

// Regle de repli (EX-LVL-006, voir en-tete) : reproduit EXACTEMENT la regle historique, deduite
// de la comparaison entre les dimensions du niveau et la taille de salle par defaut -- jamais une
// salle personnalisee, puisqu'un niveau sans champ declare n'en a par definition pas.
CameraFramingConfig resolveCameraFraming(const std::optional<CameraFramingConfig>& declared,
                                         int levelWidth, int levelHeight) noexcept {
    if (declared) {
        return *declared;
    }
    if (levelWidth <= kDefaultRoomWidthTiles && levelHeight <= kDefaultRoomHeightTiles) {
        return CameraFramingConfig{.mode = CameraFramingMode::WholeLevel};
    }
    return CameraFramingConfig{.mode = CameraFramingMode::PerRoom};
}

// Valide un cadrage declare (EX-LVL-004, voir en-tete).
std::optional<std::string> validateCameraFramingConfig(const CameraFramingConfig& config,
                                                       int levelWidth, int levelHeight) {
    if (config.mode != CameraFramingMode::PerRoom) {
        if (config.roomWidthTiles) {
            return "cameraFraming.roomWidthTiles renseigne pour un mode qui n'est pas 'perRoom'";
        }
        if (config.roomHeightTiles) {
            return "cameraFraming.roomHeightTiles renseigne pour un mode qui n'est pas 'perRoom'";
        }
        return std::nullopt;
    }
    if (config.roomWidthTiles) {
        if (*config.roomWidthTiles <= 0) {
            return "cameraFraming.roomWidthTiles doit etre superieur a zero";
        }
        if (*config.roomWidthTiles > levelWidth) {
            return "cameraFraming.roomWidthTiles depasse la largeur du niveau";
        }
    }
    if (config.roomHeightTiles) {
        if (*config.roomHeightTiles <= 0) {
            return "cameraFraming.roomHeightTiles doit etre superieur a zero";
        }
        if (*config.roomHeightTiles > levelHeight) {
            return "cameraFraming.roomHeightTiles depasse la hauteur du niveau";
        }
    }
    return std::nullopt;
}

}  // namespace core
