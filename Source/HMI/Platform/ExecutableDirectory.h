// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>

/**
 * @file HMI/Platform/ExecutableDirectory.h
 * @brief Localisation du dossier contenant l'exécutable en cours.
 */

namespace hmi {

/**
 * @brief Dossier contenant l'exécutable en cours, pour localiser les ressources copiées à côté
 *        (niveaux, catalogues de traduction, logs).
 * @return Le chemin du dossier de l'exécutable.
 */
[[nodiscard]] std::filesystem::path executableDirectory();

}  // namespace hmi
