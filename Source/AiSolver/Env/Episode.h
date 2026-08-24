// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Levels/LevelOutcome.h"

/**
 * @file AiSolver/Env/Episode.h
 * @brief Classification de fin d'épisode partagée par tout algorithme d'apprentissage
 * (`LOT-ANNEXE-08`, `EX-IA-009`).
 */

namespace aisolver {

/**
 * @brief Statut d'un épisode d'entraînement, plus riche que `core::LevelOutcome` (qui ignore
 * blocage et plafond de pas, inutiles au jeu mais nécessaires à un entraînement automatisé).
 */
enum class EpisodeStatus {
    Ongoing,
    Won,
    Lost,
    TimedOut,
    Stuck,
};

/**
 * @brief Classe la fin d'épisode à partir de l'issue du jeu et des compteurs de pas/progression.
 *
 * Fonction pure, sans accès à `HeadlessLevelEnvironment` : reçoit ses compteurs en paramètres,
 * testable sans construire d'environnement réel. Ordre de priorité explicite : victoire/défaite
 * réelles (`outcome`) priment toujours sur les critères artificiels (`TimedOut`/`Stuck`), qui
 * restent deux bornes indépendantes (un agent qui progresse lentement mais sûrement ne doit pas
 * être coupé aussi tôt qu'un agent qui stagne complètement).
 * @param outcome Issue du niveau (`core::LevelOutcome`) à ce pas.
 * @param stepIndex Nombre de pas simulés depuis le dernier `reset`.
 * @param stepsSinceProgress Nombre de pas consécutifs sans amélioration de la meilleure distance
 * atteinte (`HeadlessLevelEnvironment::stepsSinceProgress`).
 * @param hardStepBudget Plafond dur de pas (`EnvironmentConfig::maxSteps`).
 * @param stuckThreshold Seuil de pas sans progression au-delà duquel l'épisode est jugé bloqué.
 * @return Le statut de l'épisode à ce pas.
 */
[[nodiscard]] EpisodeStatus classifyEpisode(core::LevelOutcome outcome, int stepIndex,
                                            int stepsSinceProgress, int hardStepBudget,
                                            int stuckThreshold);

}  // namespace aisolver
