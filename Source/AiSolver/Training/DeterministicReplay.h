// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <vector>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Training/Evolutionary/FitnessEvaluator.h"
#include "AiSolver/Training/Evolutionary/Individual.h"
#include "Core/Physics/PlayerInput.h"

/**
 * @file AiSolver/Training/DeterministicReplay.h
 * @brief Rejeu déterministe du meilleur individu d'un entraînement (`LOT-ANNEXE-11`, `EX-IA-012`).
 */

namespace aisolver::training {

/// Séquence d'entrées rejouable en jeu, une par pas fixe (`1/60 s`) — même type que
/// `ReplayFile::steps` (`LOT-ANNEXE-07`, `Source/AiSolver/Replay`), consommé tel quel par
/// `exportReplay` (`ReplayExport.h`).
using ActionSequence = std::vector<core::PlayerInput>;

/// Issue complète d'un rejeu déterministe : la séquence d'actions produite, le statut de fin
/// d'épisode et la récompense cumulée sur ce rejeu (destinée à `ReplayFile::finalReward`).
struct DeterministicReplayResult {
    ActionSequence steps;
    EpisodeStatus status = EpisodeStatus::Ongoing;
    float finalReward = 0.0f;
};

/**
 * @brief Rejoue `individual` de façon strictement déterministe sur `environment`/`levelPath`,
 * produisant la séquence d'actions destinée à l'export (`ReplayExport.h`).
 *
 * Réutilise **exactement** le même code de décodage (`decodeArgmax`) et de calcul de récompense que
 * `FitnessEvaluator::evaluateFitness` (`LOT-ANNEXE-10`) — décision de cadrage de l'épic : élimine
 * toute possibilité de divergence entre « ce que l'algorithme a mesuré comme réussite » et « ce qui
 * est effectivement exporté ». Le réseau de `individual` n'est jamais modifié (poids inchangés
 * avant/après l'appel) ; `individual` n'est pris par référence non constante que parce que
 * `nn::Network::forward` (`LOT-ANNEXE-03`) ne l'est pas — même déviation pragmatique que
 * `FitnessEvaluator::evaluateFitness`, documentée ici plutôt que dans l'épic.
 * @param individual  Individu à rejouer (poids figés, lecture seule au sens fonctionnel).
 * @param environment Environnement réinitialisé en tout premier (aucun état ne fuit d'un appel au
 *                    suivant).
 * @param levelPath   Niveau à charger (`HeadlessLevelEnvironment::reset`).
 * @return La séquence d'actions produite, son statut de fin et sa récompense cumulée.
 */
[[nodiscard]] DeterministicReplayResult replayBestIndividual(
    evolutionary::Individual& individual, HeadlessLevelEnvironment& environment,
    const std::filesystem::path& levelPath);

}  // namespace aisolver::training
