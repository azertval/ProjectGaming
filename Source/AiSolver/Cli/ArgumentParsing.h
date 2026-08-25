// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

/**
 * @file AiSolver/Cli/ArgumentParsing.h
 * @brief Analyse minimale d'arguments `--nom valeur` maison (`LOT-ANNEXE-19`, TACHE-01,
 * `EX-IA-020`), sans dépendance tierce.
 */

namespace aisolver::cli {

/**
 * @brief Cherche `--name valeur` dans @p args et retourne `valeur`.
 * @param args Liste d'arguments bruts (typiquement `argv[1..]`, sans le nom de sous-commande).
 * @param name Nom complet de l'option, avec son préfixe `--` (ex. `"--level"`).
 * @return La valeur associée, ou `std::nullopt` si @p name est absent ou porté par le dernier
 *         élément de @p args (valeur manquante).
 */
[[nodiscard]] std::optional<std::string> findOption(const std::vector<std::string>& args,
                                                    std::string_view name);

/// @return `true` si @p name (un drapeau sans valeur, ex. `"--verbose"`) apparaît dans @p args.
[[nodiscard]] bool hasFlag(const std::vector<std::string>& args, std::string_view name);

}  // namespace aisolver::cli
