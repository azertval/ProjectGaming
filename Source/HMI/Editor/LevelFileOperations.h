// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <string>
#include <vector>

/**
 * @file HMI/Editor/LevelFileOperations.h
 * @brief Opérations fichiers sur les niveaux (créer/renommer/dupliquer/supprimer), sans Qt
 * (LOT-36).
 */

namespace hmi {

/// Résultat d'une opération fichier : succès (avec le chemin résultant) ou erreur lisible.
struct FileOpResult {
    bool ok = false;
    std::string error;
    std::filesystem::path path;  ///< Chemin créé/renommé/dupliqué en cas de succès.

    [[nodiscard]] static FileOpResult success(std::filesystem::path resultPath) {
        return {true, {}, std::move(resultPath)};
    }
    [[nodiscard]] static FileOpResult failure(std::string message) {
        return {false, std::move(message), {}};
    }
};

/**
 * @brief Opérations sur les fichiers de niveaux d'un dossier, **sans dépendance Qt/GPU**.
 *
 * Logique pure et testable (`EX-NFR-010`, `EX-IHM-021`) : réutilise `hmi::isValidLevelName`
 * (validation de nom, `EX-EDIT-009`), `core::LevelLoader`/`LevelWriter` (format) et
 * `core::LevelDraft` (aucune règle de niveau dupliquée, `EX-EDIT-010`). Chaque opération renvoie un
 * `FileOpResult` — jamais d'exception vers l'appelant (`EX-NFR-040`).
 */
class LevelFileOperations {
public:
    explicit LevelFileOperations(std::filesystem::path levelsDir);

    /// @return Les fichiers `.json` du dossier, triés par nom (vide si le dossier n'existe pas).
    [[nodiscard]] std::vector<std::filesystem::path> list() const;

    /// Crée un niveau minimal valide (grille vide + entrée/sortie) nommé @p name.
    [[nodiscard]] FileOpResult create(const std::string& name, int width, int height) const;

    /// Renomme le niveau @p source en @p newName (met à jour le nom interne).
    [[nodiscard]] FileOpResult rename(const std::filesystem::path& source,
                                      const std::string& newName) const;

    /// Duplique le niveau @p source sous un nom unique (« … (copie) », « … (copie 2) », …).
    [[nodiscard]] FileOpResult duplicate(const std::filesystem::path& source) const;

    /// Supprime le fichier de niveau @p source.
    [[nodiscard]] static FileOpResult remove(const std::filesystem::path& source);

private:
    /// Chemin du fichier `.json` correspondant à un nom de niveau, dans le dossier géré.
    [[nodiscard]] std::filesystem::path pathForName(const std::string& name) const;

    std::filesystem::path _dir;
};

}  // namespace hmi
