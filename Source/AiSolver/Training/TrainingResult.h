// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Training/Evolutionary/Individual.h"

/**
 * @file AiSolver/Training/TrainingResult.h
 * @brief Issue d'une session d'entraînement niveau-par-niveau (`LOT-ANNEXE-11`, `EX-IA-012`).
 */

namespace aisolver::training {

/**
 * @brief Distingue un arrêt par résolution d'un arrêt par plafond de générations, et donne accès
 * au meilleur individu final (`LevelTrainingSession::run`).
 */
struct TrainingResult {
    /// `true` si le meilleur individu a résolu le niveau et est resté invaincu comme champion
    /// pendant le nombre de générations consécutives requis ; `false` si l'arrêt provient du
    /// plafond de générations.
    bool solved = false;
    /// Nombre de générations effectivement exécutées avant l'arrêt.
    unsigned generationsRun = 0;
    /// Meilleur individu connu au moment de l'arrêt (poids figés, copie indépendante de la
    /// population interne de `LevelTrainingSession`).
    evolutionary::Individual bestIndividual;
};

}  // namespace aisolver::training
