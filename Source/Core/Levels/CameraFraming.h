#pragma once

#include <optional>
#include <string>
#include <string_view>

/**
 * @file Core/Levels/CameraFraming.h
 * @brief Mode de cadrage de caméra porté par le niveau (`EX-LVL-006`, `EX-REN-016`, LOT-64).
 */

namespace core {

/**
 * @brief Mode de cadrage de caméra d'un niveau (`EX-REN-016`) : décision de **conception**, pas
 *        une règle déduite de ses dimensions.
 */
enum class CameraFramingMode {
    WholeLevel,  ///< Le niveau entier tient dans le cadre (`EX-REN-013`).
    PerRoom,     ///< Cadrage salle par salle, coupure nette au franchissement (`EX-REN-015`).
    Follow,      ///< Caméra de suivi du personnage (zone morte, anticipation, bornage).
};

/// Nom JSON du mode (symétrique à `parseCameraFramingMode`), pour la sérialisation.
[[nodiscard]] std::string_view cameraFramingModeName(CameraFramingMode mode) noexcept;

/// Reconnaît le nom JSON d'un mode ; `std::nullopt` si non reconnu (mode inconnu, `EX-LVL-004`).
[[nodiscard]] std::optional<CameraFramingMode> parseCameraFramingMode(
    std::string_view name) noexcept;

/**
 * @brief Taille de salle par défaut du mode *par salle*, en cases.
 *
 * Mêmes valeurs historiques que `hmi::RoomGrid` (`LOT-32`), désormais une **valeur par défaut**
 * plutôt que la seule vérité (`tache-01`) : un niveau peut la personnaliser. Dupliquées ici plutôt
 * que lues depuis `hmi::RoomGrid` : `Core` ne dépend jamais de `HMI` (`EX-ARCH-011`).
 */
inline constexpr int kDefaultRoomWidthTiles = 24;
inline constexpr int kDefaultRoomHeightTiles = 14;

/**
 * @brief Cadrage de caméra résolu d'un niveau : le mode retenu et, pour le mode *par salle*, une
 *        taille de salle éventuellement personnalisée.
 *
 * `roomWidthTiles`/`roomHeightTiles` n'ont de sens qu'en mode `PerRoom` ; `std::nullopt` signifie
 * « taille par défaut » (`kDefaultRoomWidthTiles`/`kDefaultRoomHeightTiles`), jamais une taille de
 * zéro case.
 */
struct CameraFramingConfig {
    CameraFramingMode mode = CameraFramingMode::WholeLevel;
    std::optional<int> roomWidthTiles;
    std::optional<int> roomHeightTiles;

    [[nodiscard]] bool operator==(const CameraFramingConfig&) const noexcept = default;
};

/**
 * @brief Résout le cadrage effectif d'un niveau à partir du champ **déclaré** (peut être absent).
 *
 * Unique endroit incarnant la règle de repli (`EX-LVL-006`) : un niveau sans mode déclaré se
 * comporte **exactement** comme avant ce lot -- niveau entier s'il tient dans une salle de taille
 * par défaut, par salle sinon (la règle historique, `LOT-16`/`LOT-32`). Un mode déclaré
 * explicitement est retourné tel quel, sans substitution.
 * @param declared    Cadrage déclaré dans le fichier, ou `std::nullopt` si le champ est absent.
 * @param levelWidth  Largeur du niveau, en cases (> 0).
 * @param levelHeight Hauteur du niveau, en cases (> 0).
 * @return Le cadrage effectif à appliquer.
 */
[[nodiscard]] CameraFramingConfig resolveCameraFraming(
    const std::optional<CameraFramingConfig>& declared, int levelWidth, int levelHeight) noexcept;

/**
 * @brief Valide un cadrage déclaré (`EX-LVL-004`) : taille de salle nulle/négative, supérieure au
 *        niveau, ou paramètre étranger au mode retenu.
 * @param config      Cadrage déclaré à valider.
 * @param levelWidth  Largeur du niveau, en cases.
 * @param levelHeight Hauteur du niveau, en cases.
 * @return Un message d'erreur exploitable nommant le champ fautif, ou `std::nullopt` si valide.
 */
[[nodiscard]] std::optional<std::string> validateCameraFramingConfig(
    const CameraFramingConfig& config, int levelWidth, int levelHeight);

}  // namespace core
