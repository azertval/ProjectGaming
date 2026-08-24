// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <optional>

#include "AiSolver/Replay/ReplayFile.h"

/**
 * @file AiSolver/Replay/ReplayValidation.h
 * @brief Validation d'un rejeu déjà désérialisé, à la lecture, avant toute utilisation par un
 * appelant (jeu, CLI, script) (`LOT-ANNEXE-17`, TACHE-01, `EX-IA-018`).
 */

namespace aisolver {

/// @brief Raison pour laquelle un rejeu est invalide (voir `validateReplay`).
enum class ReplayValidationError {
    /// Le fichier de niveau référencé par le rejeu n'existe pas sous le répertoire de niveaux.
    LevelFileMissing,
    /// Le fichier de niveau existe mais son empreinte diverge de celle enregistrée à l'export.
    LevelFingerprintMismatch,
};

/**
 * @brief Valide un rejeu déjà désérialisé : le niveau qu'il référence doit exister et son
 * empreinte doit correspondre à celle enregistrée à l'export.
 *
 * Ne lit jamais le fichier de rejeu lui-même — @p replay est déjà désérialisé par `readReplay`
 * (`LOT-ANNEXE-07`), dont la responsabilité (fichier de rejeu introuvable, JSON malformé) reste
 * entièrement la sienne, jamais dupliquée ici. Ne lève jamais d'exception (`EX-NFR-040`) : le
 * résultat porte une erreur optionnelle, à l'appelant de décider de la suite.
 *
 * @param replay Rejeu déjà chargé (`readReplay`), dont on valide `levelPath`/`levelFingerprint`.
 * @param levelsDir Répertoire sous lequel résoudre `replay.levelPath`.
 * @return `ReplayValidationError::LevelFileMissing` si le fichier de niveau n'existe pas,
 *         `ReplayValidationError::LevelFingerprintMismatch` si son empreinte diverge,
 *         `std::nullopt` si le rejeu est valide.
 */
[[nodiscard]] std::optional<ReplayValidationError> validateReplay(
    const ReplayFile& replay, const std::filesystem::path& levelsDir);

}  // namespace aisolver
