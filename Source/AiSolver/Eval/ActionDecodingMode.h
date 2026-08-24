// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file AiSolver/Eval/ActionDecodingMode.h
 * @brief Mode de décodage d'une politique évaluée (`LOT-ANNEXE-15`, TACHE-01, `EX-IA-016`).
 */

namespace aisolver::eval {

/**
 * @brief `Argmax` (déterministe, réutilise `decodeArgmax`, `LOT-ANNEXE-07`) ou `Stochastic`
 * (échantillonnage selon la distribution produite, réutilise `decodeStochastic`).
 *
 * Toute politique n'accepte pas les deux modes : `TrainedPolicy::supportsMode` documente, par
 * famille d'algorithme, lequel est valide (voir `EvolutionaryTrainedPolicy`,
 * `AdvancedAlgorithmTrainedPolicy`).
 */
enum class ActionDecodingMode { Argmax, Stochastic };

}  // namespace aisolver::eval
