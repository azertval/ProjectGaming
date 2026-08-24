// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string_view>

/**
 * @file AiSolver/Replay/LevelFingerprint.h
 * @brief Empreinte déterministe d'un fichier de niveau, utilisée pour valider un rejeu à la
 * lecture (`LOT-ANNEXE-17`, TACHE-01, `EX-IA-018`).
 */

namespace aisolver {

/// @brief Empreinte d'un fichier de niveau (hash FNV-1a 64 bits, voir `computeLevelFingerprint`).
using LevelFingerprint = std::uint64_t;

/**
 * @brief Calcule l'empreinte du contenu **brut** (octets tels que lus, sans reformatage ni
 * reparsing JSON) d'un fichier de niveau, via FNV-1a 64 bits.
 *
 * Algorithme fixe, sans bibliothèque tierce, implémenté à la main : simplicité et
 * reproductibilité l'emportent sur une notion d'équivalence sémantique (deux fichiers
 * textuellement différents mais sémantiquement identiques produisent des empreintes
 * différentes, accepté délibérément). Choix délibérément non cryptographique — rien à sécuriser,
 * seulement détecter un changement accidentel.
 *
 * Convention d'octets stricte à respecter par toute réimplémentation (notamment
 * `LOT-ANNEXE-20`, garde-fou CI en Python pur) : @p levelFileContent doit être exactement le
 * contenu du fichier tel que lu (encodage UTF-8, aucune normalisation de fin de ligne).
 *
 * @param levelFileContent Contenu brut du fichier de niveau. Fonction pure, testable sans accès
 *        disque — c'est à l'appelant de lire le fichier.
 * @return L'empreinte FNV-1a 64 bits de @p levelFileContent.
 */
[[nodiscard]] LevelFingerprint computeLevelFingerprint(std::string_view levelFileContent) noexcept;

}  // namespace aisolver
