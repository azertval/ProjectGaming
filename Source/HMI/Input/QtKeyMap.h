#pragma once

#include <optional>

#include "HMI/Input/InputState.h"  // hmi::Key

/**
 * @file HMI/Input/QtKeyMap.h
 * @brief Traduction d'un code de touche Qt en `hmi::Key` (code virtuel Win32).
 */

namespace hmi {

/**
 * @brief Traduit un code `Qt::Key` en `hmi::Key`.
 *
 * Les lettres/chiffres/espace partagent déjà la même valeur entre Qt et Win32
 * (`Qt::Key_A == VK 'A' == 0x41`) ; seules les touches spéciales (flèches, Échap, Tab, Maj, Ctrl,
 * F1/F2/F10) passent par une correspondance explicite. `nullopt` pour une touche non suivie.
 */
[[nodiscard]] std::optional<hmi::Key> qtKeyToHmiKey(int qtKey);

}  // namespace hmi
