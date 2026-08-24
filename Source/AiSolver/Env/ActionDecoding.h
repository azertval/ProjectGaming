// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"

/**
 * @file AiSolver/Env/ActionDecoding.h
 * @brief Décodage d'une distribution de probabilité en action concrète (`LOT-ANNEXE-07`,
 * `EX-IA-007`).
 */

namespace aisolver {

/**
 * @brief Sélectionne l'action de probabilité maximale (décodage déterministe).
 *
 * Utilisé pour le rejeu final exporté (`LOT-ANNEXE-11`) et pour tout modèle évolutionniste
 * (`LOT-ANNEXE-10`, qui ne produit pas de distribution stochastique à proprement parler).
 *
 * En cas d'égalité stricte entre plusieurs indices, retourne systématiquement l'action du
 * **premier** indice au sens de l'ordre d'énumération de `ActionSpace` (déterministe, ne dépend
 * d'aucun état externe).
 * @param distribution Tenseur de rang 1, taille `actionCount()`, supposé déjà normalisé (somme à
 *        `1`, produit d'une couche `softmax`) — ni renormalisé ni validé ici.
 */
[[nodiscard]] Action decodeArgmax(const Tensor<float>& distribution);

/**
 * @brief Échantillonne une action selon la distribution, pondérée par une température.
 *
 * Applique la température à la distribution d'entrée (`p_i^(1/temperature) / somme`,
 * `temperature = 1.0` la laisse inchangée), puis tire l'indice résultant par la méthode de la
 * roulette (somme cumulée) à partir d'un flottant uniforme tiré de @p rng. Toute source
 * d'aléatoire passe par @p rng, fournie par l'appelant (jamais de graine interne à cette fonction,
 * cohérent avec `LOT-ANNEXE-01`) : deux appels avec la même instance de `Rng` dans le même état
 * produisent la même action.
 *
 * Utilisé pour l'exploration pendant l'entraînement par gradient (génération 3) : une température
 * basse rend ce décodage quasi-équivalent à `decodeArgmax`, une température haute aplatit la
 * distribution vers un tirage quasi uniforme.
 * @param distribution Tenseur de rang 1, taille `actionCount()`, supposé déjà normalisé (voir
 *        `decodeArgmax`).
 * @param temperature  Strictement positive.
 * @param rng          Générateur déterministe fourni par l'appelant.
 */
[[nodiscard]] Action decodeStochastic(const Tensor<float>& distribution, float temperature,
                                      Rng& rng);

}  // namespace aisolver
