// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Cli/TrainingConfig.h"
#include "HMI/Ai/TrainingRequest.h"

/**
 * @file HMI/Ai/TrainingOverrides.h
 * @brief Traduction d'une `TrainingRequest` en surcharges `aisolver::cli::CommandLineOverrides`
 *        (`LOT-73`, `EX-IHM-083`).
 *
 * Logique **pure** : aucune dépendance à `QObject`, à `moc` ni à un fil d'exécution — compilée à la
 * fois dans `ProjectGaming` et directement dans `UnitTests`, comme
 * `HMI/Interface/DesignTokens.cpp`.
 *
 * Cette fonction existe parce que c'est ici, et nulle part ailleurs, qu'un réglage se perd. Neuf
 * l'ont été : lus par l'écran, ils n'avaient pas de champ où atterrir, et les régler ne changeait
 * rien — pendant que le `config.json` du run affirmait le contraire. Extraite du corps de
 * `TrainingWorker::run()`, la traduction devient vérifiable champ par champ, sans lancer
 * d'entraînement.
 */

namespace hmi {

/**
 * @brief Surcharges de ligne de commande équivalentes à @p request.
 *
 * `optimizer` reste `std::nullopt` : l'écran le fournit en `QString`, et l'appelant l'applique
 * après coup sur la configuration résolue — un `std::string` vide n'y serait pas distinguable d'un
 * choix explicite.
 *
 * @param request Requête saisie dans l'onglet Entraînement.
 * @return Les surcharges correspondantes, champ pour champ.
 */
[[nodiscard]] aisolver::cli::CommandLineOverrides overridesFor(const TrainingRequest& request);

}  // namespace hmi
