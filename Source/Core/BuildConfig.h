// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/BuildConfig.h
 * @brief Indicateurs de configuration de build (développement vs release).
 */

namespace core {

/**
 * @brief Vrai dans les builds de **développement** (Debug), faux en **Release**.
 *
 * S'appuie sur `NDEBUG` (défini par le compilateur en Release). Sert à n'inclure que dans les
 * builds de développement des éléments d'outillage (par exemple des boutons de débogage de
 * l'interface), à utiliser de préférence avec `if constexpr` pour éliminer le code en Release.
 */
#if defined(NDEBUG)
inline constexpr bool kDeveloperBuild = false;
#else
inline constexpr bool kDeveloperBuild = true;
#endif

}  // namespace core
