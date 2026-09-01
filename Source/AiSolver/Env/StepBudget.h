// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Levels/Level.h"

/**
 * @file AiSolver/Env/StepBudget.h
 * @brief Budget de pas et seuil de blocage **dérivés du niveau**, jamais d'une constante globale.
 */

namespace aisolver {

/// Coût nominal, en pas fixes, d'une case de la chaîne d'objectifs.
///
/// Une case de marche pure coûte `1 / (moveSpeed × 1/60) = 20` pas (`core::PhysicsConfig::
/// moveSpeed = 3`). Les montées au saut de mur, les attentes devant un danger temporisé (période
/// de `120` pas) et les trajets en plateforme mobile (`1` case/s, soit `60` pas/case) portent le
/// coût moyen constaté au-dessus de ce plancher : `demo-final.json` compte `137` cases de chaîne
/// pour un tracé scripté de près de `4 000` pas, soit `29` pas par case. La valeur retenue est
/// **fixée par le test de calibration** `StepBudgetTest.BudgetCouvreChaqueTraceScripte`
/// (`Source/Test/Integration/test_budget_pas.cpp`), qui la confronte aux 22 tracés joués par le
/// test système — elle n'est pas estimée à la lecture.
inline constexpr int STEPS_PER_OBJECTIVE_CELL = 30;

/// Marge appliquée au coût nominal, en pourcentage.
///
/// Un agent n'emprunte pas le plus court chemin : il doit pouvoir tâtonner sans être coupé avant
/// d'avoir eu une chance d'atteindre l'objectif suivant. La marge borne ce tâtonnement sans
/// laisser un épisode tourner indéfiniment.
inline constexpr int STEP_BUDGET_MARGIN_PERCENT = 60;

/// Plancher du budget : en deçà, un niveau trivial n'offrirait pas de quoi corriger une erreur.
///
/// C'est lui, et non le coût par case, qui borne les niveaux courts dont le temps est dominé par
/// une **attente** : `demo-plateforme.json` ne compte que `16` cases de chaîne mais son tracé de
/// référence demande `730` pas, l'essentiel passé à attendre la plateforme (`1` case/s, soit `60`
/// pas par case parcourue).
inline constexpr int MIN_STEP_BUDGET = 1200;

/// Plafond du budget, quelle que soit la chaîne mesurée — y compris pour un niveau dont la sortie
/// est inatteignable, où la chaîne n'a pas de longueur. Fixe aussi la borne haute de la pénalité
/// de temps cumulée d'un épisode (voir `RewardConfig`, `Reward.h`).
inline constexpr int MAX_STEP_BUDGET = 15000;

/// Plancher du seuil de blocage.
///
/// Une stagnation du champ d'objectif n'a rien d'anormal : attendre qu'un danger temporisé
/// s'éteigne coûte jusqu'à `120` pas, monter un puits au saut de mur n'en gagne aucune pendant la
/// remontée, et une plateforme mobile met `60` pas à franchir une case. Mesure faite sur les
/// tracés de référence : `114` pas de stagnation sur `demo-plateforme.json`, **`807`** sur
/// `demo-final.json` — un joueur qui résout le niveau, pas un agent qui tourne en rond.
inline constexpr int MIN_STUCK_THRESHOLD = 400;

/// Fraction du budget accordée à une stagnation avant de déclarer l'épisode bloqué.
///
/// Un quart, pas un huitième : à un huitième, `demo-final.json` obtenait `834` pas contre les
/// `807` que consomme déjà son tracé **parfait** — trois pour cent de marge, quand un agent qui
/// apprend hésite forcément plus qu'un script. Le budget dur reste la borne qui garantit la
/// terminaison ; ce seuil-ci n'est là que pour ne pas payer un épisode entier à regarder un agent
/// immobile.
inline constexpr int STUCK_THRESHOLD_BUDGET_DIVISOR = 4;

/**
 * @brief Longueur, en cases de grille, de la **chaîne d'objectifs** du niveau.
 *
 * Parcourt la même succession d'objectifs immédiats que la récompense
 * (`buildObjectiveDistanceField`, `Reward.h`) : depuis l'entrée, on rejoint la cible la plus
 * proche parmi la sortie et les déclencheurs dont la porte est encore fermée ; si c'est un
 * déclencheur, on l'active (toutes les portes qui partagent cette position s'ouvrent) et on
 * recommence depuis lui. Le budget dérivé couvre donc exactement l'itinéraire que la récompense
 * demande à l'agent de suivre, sans qu'aucun des deux ne connaisse l'ordre de résolution attendu.
 *
 * @param level Niveau chargé.
 * @return La somme des distances de plus court chemin de chaque tronçon, ou `-1` si un objectif
 *         de la chaîne est inatteignable (niveau sans solution, ou clé enfermée derrière la porte
 *         qu'elle ouvre).
 */
[[nodiscard]] int objectiveChainLength(const core::Level& level);

/**
 * @brief Budget de pas fixes d'un épisode sur @p level.
 *
 * `chaîne × STEPS_PER_OBJECTIVE_CELL`, majoré de `STEP_BUDGET_MARGIN_PERCENT`, borné à
 * `[MIN_STEP_BUDGET, MAX_STEP_BUDGET]`. Une chaîne sans solution rend `MAX_STEP_BUDGET` : c'est à
 * l'issue de l'épisode de dire qu'un niveau est infaisable, pas à son budget.
 * @param level Niveau chargé.
 */
[[nodiscard]] int estimateStepBudget(const core::Level& level);

/**
 * @brief Seuil de blocage associé à @p stepBudget : `max(MIN_STUCK_THRESHOLD, budget / 8)`.
 *
 * Proportionnel au budget, parce qu'un long niveau comporte de longues portions sans gain de
 * distance (attente d'un danger temporisé, trajet en plateforme, remontée d'un puits) qu'un seuil
 * fixe couperait au milieu.
 * @param stepBudget Budget de pas de l'épisode.
 */
[[nodiscard]] int stuckThresholdForBudget(int stepBudget);

}  // namespace aisolver
