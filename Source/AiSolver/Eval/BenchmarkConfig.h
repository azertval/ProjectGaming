// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

#include "AiSolver/Eval/ActionDecodingMode.h"

/**
 * @file AiSolver/Eval/BenchmarkConfig.h
 * @brief Paramètres d'une campagne d'exécution répétée (`LOT-ANNEXE-15`, TACHE-01, `EX-IA-016`).
 */

namespace aisolver::eval {

struct BenchmarkConfig {
    /// Nombre de répétitions de la politique sur le niveau.
    int repetitions = 30;
    /// Graine de base ; la répétition `i` utilise `deriveSeed(rngSeedBase, i)`.
    std::uint64_t rngSeedBase = 0;
    /// Budget de pas dur par épisode (timeout) ; `0` = dérivé du niveau (`Env/StepBudget.h`).
    ///
    /// Dérivé par défaut, comme à l'entraînement : mesurer un modèle sous un budget plus court que
    /// celui qui l'a produit le déclare incapable de terminer un niveau qu'il termine. `2 000` pas
    /// fixes étaient déjà inférieurs aux ~`4 000` que demande `demo-final.json`.
    int maxStepsPerEpisode = 0;
    /// Mode de décodage utilisé pour toutes les répétitions de la campagne.
    ActionDecodingMode decodingMode = ActionDecodingMode::Stochastic;
};

}  // namespace aisolver::eval
