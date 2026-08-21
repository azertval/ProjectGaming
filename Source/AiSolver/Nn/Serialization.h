// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>

#include "AiSolver/Nn/Network.h"

/**
 * @file AiSolver/Nn/Serialization.h
 * @brief Format binaire versionné de sauvegarde/chargement des poids d'un `Network` (LOT-ANNEXE-03).
 *
 * Ne stocke aucune information de structure au-delà des formes (pas de type d'activation, pas de
 * schéma d'initialisation) : reconstruire un `Network` compatible (mêmes couches, mêmes
 * activations, dans le même ordre) reste la responsabilité de l'appelant avant `loadWeights`.
 */

namespace aisolver::nn {

/// Nombre magique du format (ASCII `"AINN"`), premier champ de tout fichier valide.
inline constexpr std::uint32_t WEIGHTS_FILE_MAGIC = 0x41494E4Eu;

/// Version du format binaire ; à incrémenter (jamais réutiliser) dès que le format change.
inline constexpr std::uint32_t WEIGHTS_FILE_VERSION = 1;

/**
 * @brief Écrit les poids de `network` dans `path` (magique, version, nombre de couches, puis par
 * couche : forme et valeurs des poids, forme et valeurs du biais).
 * @return `false` si le fichier ne peut pas être ouvert en écriture (erreur récupérable, pas
 * d'exception) ; `true` sinon.
 */
[[nodiscard]] bool saveWeights(const Network& network, const std::filesystem::path& path);

/**
 * @brief Recharge dans `network` les poids sauvegardés dans `path`.
 *
 * Valide magique, version, nombre de couches et forme de chaque couche **avant** toute
 * modification de `network` : en cas d'échec (fichier absent, magique/version invalide, structure
 * incompatible), `network` reste inchangé.
 * @return `false` en cas d'échec (fichier absent ou invalide, structure incompatible) ; `true` si
 * `network` a été entièrement rechargé.
 */
[[nodiscard]] bool loadWeights(Network& network, const std::filesystem::path& path);

}  // namespace aisolver::nn
