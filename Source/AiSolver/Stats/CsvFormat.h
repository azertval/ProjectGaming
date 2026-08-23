// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <string_view>

#include "AiSolver/Stats/TrainingStatsRecorder.h"

/**
 * @file AiSolver/Stats/CsvFormat.h
 * @brief Format exact des colonnes CSV écrites par `TrainingStatsRecorder`, réutilisé sans
 * changement par le harnais de benchmark (`LOT-ANNEXE-15`) (`LOT-ANNEXE-09`, `EX-IA-010`).
 */

namespace aisolver {

/**
 * @brief En-tête de colonnes, fixe et dans cet ordre exact.
 *
 * Aucune dépendance à une bibliothèque CSV tierce : un seul champ textuel (`levelName`) est à
 * risque d'échappement dans tout le schéma, une fonction dédiée de quelques lignes suffit
 * (conforme à la contrainte « from scratch, aucune dépendance tierce nouvelle » du programme).
 */
[[nodiscard]] std::string csvHeader();

/**
 * @brief Sérialise une ligne de statistiques, valeurs séparées par des virgules.
 *
 * `levelName` est entouré de guillemets s'il contient une virgule ou un guillemet (un guillemet
 * interne est alors doublé, convention CSV usuelle) ; toute autre colonne est numérique et ne
 * nécessite aucun échappement.
 * @param row Ligne fournie par l'appelant (voir `TrainingStatsRow`).
 * @param movingAverageReward Moyenne mobile de `bestReward`, calculée par le recorder
 * (`MovingAverageTracker`), jamais par l'appelant.
 * @param rewardDelta Delta de la moyenne mobile par rapport à l'appel précédent, calculé par le
 * recorder.
 * @param timestampIso8601 Horodatage d'enregistrement, format ISO 8601 (`AAAA-MM-JJThh:mm:ssZ`) :
 * trié lexicographiquement dans le même ordre que chronologiquement.
 * @return La ligne CSV, sans saut de ligne final.
 */
[[nodiscard]] std::string csvRow(const TrainingStatsRow& row, float movingAverageReward,
                                 float rewardDelta, std::string_view timestampIso8601);

}  // namespace aisolver
