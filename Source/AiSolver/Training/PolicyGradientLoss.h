// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

/**
 * @file AiSolver/Training/PolicyGradientLoss.h
 * @brief Formule de perte de policy gradient commune (`LOT-ANNEXE-13`, TACHE-02), partagée entre
 * `Training/PolicyGradient` (poids = retour brut, `LOT-ANNEXE-12`) et `Training/ActorCritic` (poids
 * = avantage, `LOT-ANNEXE-13`) sans faire dépendre l'un de l'autre.
 */

namespace aisolver::training {

/// Coefficient d'entropie par défaut de la perte de policy gradient.
///
/// La perte nue `-log(pi) * poids` n'a aucun terme qui s'oppose à la spécialisation : une politique
/// dont un logit prend l'avantage voit sa probabilité tendre vers `1`, l'échantillonnage devient
/// déterministe, et plus aucun autre couple (état, action) n'est jamais visité — l'entraînement
/// continue de tourner sans que rien ne puisse plus changer. La mesure faite avant correction :
/// des trajectoires **bit à bit identiques** dès l'épisode ~50, sur les 9 950 épisodes suivants,
/// malgré une graine différente à chaque épisode.
///
/// La valeur est un compromis usuel : assez pour maintenir une distribution vivante, assez peu
/// pour ne pas dominer le signal de récompense une fois les retours centrés-réduits.
inline constexpr float DEFAULT_ENTROPY_COEFFICIENT = 0.01f;

/**
 * @brief Construit, sur `trajectory`, le graphe de la moyenne par pas de la perte pondérée
 *        moins le terme d'entropie : `-log(probabilite de l'action) x poids - beta x entropie`.
 *
 * Rejoue le passage avant de `policy` pas à pas (poids actuels) : jamais de dépendance à
 * `TrajectoryStep::logProbability`, valeur détachée sans historique de graphe. Utilisée telle
 * quelle par `computeReinforceLoss` (`weights` = retours) et `computeActorCriticLoss` (`weights` =
 * avantages) — seule la nature du poids par pas change, jamais la formule.
 * @param policy     Réseau rejoué pas à pas (poids **non modifiés** par cet appel).
 * @param trajectory Trajectoire collectée (`TrajectoryCollector`).
 * @param weights    Poids par pas (retour ou avantage), même longueur que `trajectory.steps`.
 * @param entropyCoefficient Poids du terme d'entropie (`beta`) ; `0` le désactive entièrement
 *        (aucun nœud construit), ce qui restitue la formule d'origine.
 * @return Nœud scalaire (forme `[1]`) de la perte moyenne, prêt pour `autodiff::backward()`.
 * @pre `weights.size() == trajectory.steps.size()`, `!trajectory.steps.empty()`.
 */
[[nodiscard]] autodiff::NodePtr computeWeightedPolicyGradientLoss(
    nn::Network& policy, const Trajectory& trajectory, const std::vector<float>& weights,
    float entropyCoefficient = DEFAULT_ENTROPY_COEFFICIENT);

}  // namespace aisolver::training
