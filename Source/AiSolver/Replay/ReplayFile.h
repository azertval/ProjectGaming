// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Core/Physics/PlayerInput.h"

/**
 * @file AiSolver/Replay/ReplayFile.h
 * @brief Format de rejeu v1 : séquence d'actions gagnante exportée par un entraînement
 * (`LOT-ANNEXE-07`, `EX-IA-008`).
 */

namespace aisolver {

/**
 * @brief Version courante du format de rejeu (`EX-IA-008`), même principe que
 *        `core::kLevelFormatVersion` (`EX-LVL-005`) : un fichier sans champ `formatVersion` est lu
 *        comme la version initiale (0), sans erreur.
 *
 * Passée à `2` par `LOT-ANNEXE-17` (TACHE-02, stabilisation du format v1) : ajout de
 * `totalDurationSeconds` et `algorithmId`. Un fichier `formatVersion == 1` (écrit avant ce lot,
 * sans ces deux champs) reste lisible sans erreur — ils sont alors pris à leur valeur sentinelle
 * (`0.0f` et `""`).
 */
inline constexpr std::uint32_t kReplayFormatVersion = 2;

/**
 * @brief Un fichier de rejeu : séquence déterministe de `core::PlayerInput` rejouable en jeu sans
 *        aucune inférence live, avec ses métadonnées d'entraînement.
 *
 * Format JSON (schéma) :
 * @code
 * {
 *   "formatVersion": 2,
 *   "levelPath": "demo-deplacement.json",
 *   "levelFingerprint": 0,
 *   "algorithmName": "evolutionnaire",
 *   "exportedAtIso8601": "2026-08-23T12:00:00Z",
 *   "seed": 42,
 *   "finalReward": 1.0,
 *   "totalDurationSeconds": 1.5,
 *   "algorithmId": "evo",
 *   "steps": [
 *     {"moveX": 1.0, "jumpPressed": false, "jumpHeld": false, "moveY": 0.0,
 *      "dashPressed": false, "interactPressed": false, "interactHeld": false,
 *      "interactReleased": false},
 *     ...
 *   ]
 * }
 * @endcode
 */
struct ReplayFile {
    /// Numéro de version explicite du format (voir `kReplayFormatVersion`).
    std::uint32_t formatVersion = kReplayFormatVersion;
    /// Chemin **relatif** (à `Source/Elements/Levels`) du niveau d'origine — jamais un chemin
    /// absolu, pour rester portable d'une machine à l'autre.
    std::string levelPath;
    /// Empreinte du niveau d'origine (`computeLevelFingerprint`, `LOT-ANNEXE-17`), vérifiée à la
    /// lecture par `validateReplay` (`LevelFingerprint.h`/`ReplayValidation.h`).
    std::uint64_t levelFingerprint = 0;
    /// Séquence ordonnée d'entrées, une par pas fixe (`1/60 s`).
    std::vector<core::PlayerInput> steps;
    /// Nom de l'algorithme ayant produit ce rejeu (ex. `"evolutionnaire"`, `"reinforce"`).
    std::string algorithmName;
    /// Horodatage d'export, format ISO 8601 (ex. `"2026-08-23T12:00:00Z"`).
    std::string exportedAtIso8601;
    /// Graine utilisée par l'entraînement ayant produit ce rejeu.
    std::uint64_t seed = 0;
    /// Statistique finale de l'entraînement (ex. récompense cumulée du dernier épisode réussi).
    float finalReward = 0.0f;
    /// Durée totale du rejeu (`steps.size() × delta de pas fixe`), calculée une fois à l'export
    /// plutôt que recalculée à chaque lecture — commodité d'affichage/de reporting, jamais une
    /// source de vérité (le nombre de pas réel de `steps` fait foi pour la simulation).
    /// `0.0f` (valeur sentinelle) sur un fichier `formatVersion == 1` (`LOT-ANNEXE-17`).
    float totalDurationSeconds = 0.0f;
    /// Identifiant court de l'algorithme d'origine (ex. `"evo"`, `"pg"`, `"ac"`, `"avance"` —
    /// mêmes valeurs que l'argument `--algo` du CLI, `LOT-ANNEXE-19`), pour retrouver a posteriori
    /// de quel algorithme un rejeu est issu sans consulter les journaux d'entraînement. Chaîne
    /// libre, pas une énumération figée. `""` (valeur sentinelle) sur un fichier
    /// `formatVersion == 1` (`LOT-ANNEXE-17`).
    std::string algorithmId;
};

/**
 * @brief Résultat d'une lecture de fichier de rejeu : soit un `ReplayFile`, soit une erreur décrite.
 *
 * Sur le même modèle que `core::LevelLoadResult` : aucune exception ne traverse jamais
 * `readReplay` (`EX-NFR-040`), une erreur (fichier introuvable, JSON malformé) est toujours
 * récupérable.
 */
struct ReplayLoadResult {
    std::optional<ReplayFile> replay;
    std::string error;

    /// @return `true` si la lecture a réussi.
    [[nodiscard]] bool ok() const noexcept {
        return replay.has_value();
    }
};

/**
 * @brief Écrit un fichier de rejeu au format JSON.
 * @param path  Chemin du fichier à (re)créer ; les dossiers parents manquants sont créés.
 * @param replay Contenu à écrire.
 * @return `true` si l'écriture a réussi, `false` sinon (jamais d'exception).
 */
[[nodiscard]] bool writeReplay(const std::filesystem::path& path, const ReplayFile& replay);

/**
 * @brief Lit un fichier de rejeu au format JSON.
 *
 * Aucune validation de cohérence n'est effectuée ici (empreinte de niveau, budget de pas…) :
 * ajoutée par `LOT-ANNEXE-17`, qui étend ce format. Un fichier sans champ `formatVersion` est lu
 * comme la version initiale (`0`), sans erreur.
 * @param path Chemin du fichier à lire.
 * @return Résultat de la lecture, jamais d'exception (`EX-NFR-040`).
 */
[[nodiscard]] ReplayLoadResult readReplay(const std::filesystem::path& path);

}  // namespace aisolver
