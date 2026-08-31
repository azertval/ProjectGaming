// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>

#include "AiSolver/Training/PolicyGradientLoss.h"

/**
 * @file AiSolver/Training/PolicyGradientTuning.h
 * @brief Réglages communs aux deux familles de policy gradient (REINFORCE, acteur-critique).
 */

namespace aisolver::training {

/// Facteur d'actualisation par défaut des deux familles de policy gradient.
///
/// `0,99` donne un horizon effectif de `1 / (1 - gamma) = 100` pas, soit `1,7 s` de jeu : le bonus
/// de complétion versé au bout d'un épisode de plusieurs milliers de pas y était escompté à
/// `0,99^4000 ~ 1e-18`, donc rigoureusement invisible depuis le départ. `0,995` porte cet horizon à
/// `200` pas sans rendre les retours ingérables ; c'est la récompense de progression, dense, qui
/// relaie le bonus sur les distances plus longues.
inline constexpr float DEFAULT_GAMMA = 0.995f;

/// Nombre d'épisodes collectés avant chaque mise à jour de poids.
///
/// Un lot réduit la variance de la direction estimée, mais il divise d'autant le nombre de mises à
/// jour obtenues pour un même coût de simulation — et une trajectoire coûte des milliers de pas de
/// physique. Mesure faite sur `demo-saut.json`, `400` épisodes, graine fixée : `75` victoires à
/// `batchEpisodes = 1` contre `31` à `8`, pour exactement le même budget d'épisodes.
///
/// La valeur retenue est donc `1`, et le centrage-réduction porte alors sur les retours **d'un
/// même épisode** — la forme classique du *reward-to-go* centré, qui suffit à supprimer la
/// pathologie « tous les retours négatifs, donc on ne fait que décourager ». Le réglage reste
/// exposé : c'est par lui qu'on rejoue la comparaison ci-dessus, et un lot plus large redevient
/// intéressant si le taux d'apprentissage est augmenté en conséquence (non mesuré ici).
inline constexpr std::size_t DEFAULT_BATCH_EPISODES = 1;

/// Norme globale maximale du gradient accumulé sur un lot (`optim::clipGradientNorm`).
inline constexpr float DEFAULT_GRADIENT_CLIP_NORM = 1.0f;

/// Nombre d'images fixes pendant lesquelles une action décidée est maintenue.
///
/// À `1`, la politique retire parmi les `48` actions à chaque image de `1/60 s` : les directions
/// tirées se compensent, le déplacement espéré est nul, et l'exploration ne quitte jamais le
/// voisinage du point de départ — quel que soit le nombre d'épisodes. Traverser une case demande
/// `20` images de direction constante (`moveSpeed = 3`), en tirer `20` de suite au hasard parmi
/// trois directions a une probabilité de `3^-20`.
///
/// `4` images (`67 ms`) est le compromis usuel : assez pour que l'exploration soit
/// directionnelle, assez court pour ne pas rendre inatteignables les enchaînements précis
/// (un dash dure `0,15 s`, soit `9` images).
inline constexpr int DEFAULT_ACTION_REPEAT = 4;

/// Part de tirage uniforme mélangée à la politique **à l'échantillonnage** (entraînement seul).
///
/// Le terme d'entropie ne suffit pas à empêcher une politique de se figer : sa dérivée s'annule
/// justement quand la distribution est déjà saturée, c'est-à-dire quand on en aurait le plus
/// besoin. Mesure faite avant correction, `1 500` épisodes sur `demo-double-saut.json` et
/// `demo-wall-jump.json` : **1 499 épisodes bit à bit identiques**, malgré une graine différente à
/// chacun — l'entraînement tournait sans qu'aucune autre action ne soit plus jamais essayée.
///
/// Mélanger `epsilon` d'uniforme à la distribution échantillonnée garantit à chaque action une
/// probabilité minimale de `epsilon / 48`, quelle que soit la saturation du `softmax`. Le gradient,
/// lui, reste calculé sur la politique elle-même — le décalage est celui d'un `epsilon`-greedy, et
/// il disparaît du rejeu final, décodé en `argmax`.
inline constexpr float DEFAULT_EXPLORATION_FLOOR = 0.05f;

/**
 * @brief Réglages partagés par `ReinforceConfig` et `ActorCriticConfig`.
 *
 * Les deux familles restent sans lien de code (décision de cadrage de `LOT-ANNEXE-13`), mais elles
 * partagent déjà la **même** formule de perte (`PolicyGradientLoss.h`) et le **même** collecteur de
 * trajectoire : ce qui règle ces deux pièces se règle donc au même endroit, sinon une correction
 * apportée à l'une manquerait silencieusement à l'autre.
 */
struct PolicyGradientTuning {
    /// Épisodes collectés avant chaque pas d'optimisation (`DEFAULT_BATCH_EPISODES`).
    std::size_t batchEpisodes = DEFAULT_BATCH_EPISODES;
    /// Poids du terme d'entropie de la perte (`DEFAULT_ENTROPY_COEFFICIENT`) ; `0` le désactive.
    float entropyCoefficient = DEFAULT_ENTROPY_COEFFICIENT;
    /// Norme globale maximale du gradient (`DEFAULT_GRADIENT_CLIP_NORM`) ; `0` désactive
    /// l'écrêtage.
    float gradientClipNorm = DEFAULT_GRADIENT_CLIP_NORM;
    /// Images pendant lesquelles une action décidée est maintenue (`DEFAULT_ACTION_REPEAT`).
    int actionRepeat = DEFAULT_ACTION_REPEAT;
    /// Part d'uniforme mélangée à l'échantillonnage (`DEFAULT_EXPLORATION_FLOOR`) ; `0` la
    /// désactive.
    float explorationFloor = DEFAULT_EXPLORATION_FLOOR;
};

}  // namespace aisolver::training
