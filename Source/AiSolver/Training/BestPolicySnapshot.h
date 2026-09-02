// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <vector>

#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/DeterministicReplay.h"

/**
 * @file AiSolver/Training/BestPolicySnapshot.h
 * @brief Conservation du **meilleur** réseau rencontré pendant un entraînement par gradient
 * (REINFORCE, acteur-critique, DQN), plutôt que du réseau tel qu'il se trouve au dernier épisode.
 *
 * Les familles par gradient entraînent un réseau **en place** : à la fin du run, ce réseau porte
 * l'état du dernier épisode joué, qui n'est pas le meilleur du run — une exploration
 * `epsilon`-greedy (DQN) ou une mise à jour de politique malheureuse suffit à dégrader les poids
 * juste avant l'arrêt, et le modèle sauvegardé est alors un échec alors que des générations
 * antérieures réussissaient le niveau (défaut constaté sur le niveau « deplacement »). Le chemin
 * évolutionniste n'a jamais eu ce problème : il sauvegarde `TrainingResult::bestIndividual`, pas
 * la dernière population. Cette classe donne aux chemins par gradient l'équivalent de ce
 * `bestIndividual`.
 */

namespace aisolver::training {

/**
 * @brief Note comparable d'un rejeu **déterministe** (`argmaxRollout`/`replayBestIndividual`).
 *
 * Mesurée sur la politique gloutonne, jamais sur l'épisode d'entraînement : la récompense d'un
 * épisode d'entraînement DQN dépend de l'exploration aléatoire de cet épisode et ne dit rien de ce
 * que vaut le réseau une fois `epsilon` retiré — c'est-à-dire de ce qui sera sauvegardé et rejoué.
 */
struct PolicyScore {
    /// Le rejeu a atteint la sortie (`EpisodeStatus::Won`).
    bool solved = false;
    /// Récompense cumulée du rejeu.
    float reward = 0.0f;
    /// Longueur du rejeu, en pas de simulation.
    int stepCount = 0;

    /**
     * @brief Ordre total du « meilleur » : une réussite bat toujours un échec ; entre deux
     * réussites, la plus courte gagne (à longueur égale, la mieux récompensée) ; entre deux échecs,
     * la mieux récompensée gagne — la seule grandeur qui distingue deux tentatives ratées.
     */
    [[nodiscard]] bool betterThan(const PolicyScore& other) const noexcept;
};

/**
 * @brief Retient une copie profonde des poids du réseau au moment de sa meilleure note, avec le
 * rejeu déterministe qui l'a établie.
 *
 * Le rejeu est conservé avec les poids, et pas seulement les poids : il a déjà été calculé pour
 * noter le candidat, et c'est exactement celui que l'export doit publier — le recalculer après
 * restauration coûterait un rejeu complet pour un résultat identique par construction.
 */
class BestPolicySnapshot {
public:
    /**
     * @brief Note @p replay et, s'il bat le meilleur retenu jusqu'ici, mémorise les poids courants
     * de @p network (copie profonde, `Tensor::clone()`) et @p replay.
     * @return `true` si ce candidat est devenu le nouveau meilleur.
     */
    bool consider(const nn::Network& network, const DeterministicReplayResult& replay);

    /// @return `true` si au moins un candidat a été retenu.
    [[nodiscard]] bool hasSnapshot() const noexcept {
        return _replay.has_value();
    }

    /// @return La note du meilleur candidat retenu ; sans objet si `hasSnapshot()` est faux.
    [[nodiscard]] const PolicyScore& bestScore() const noexcept {
        return _bestScore;
    }

    /// @return Le rejeu déterministe du meilleur candidat retenu, ou `std::nullopt`.
    [[nodiscard]] const std::optional<DeterministicReplayResult>& bestReplay() const noexcept {
        return _replay;
    }

    /**
     * @brief Réécrit dans @p network les poids mémorisés (copie profonde : le cliché reste
     * réutilisable après l'appel).
     * @pre @p network a la topologie de celui passé à `consider` (même nombre de paramètres, mêmes
     *      formes) — c'est toujours le même réseau en pratique, entraîné en place.
     * @return `false` si aucun candidat n'a été retenu ou si la topologie ne correspond pas
     *         (@p network alors inchangé) ; `true` si la restauration a eu lieu.
     */
    bool restore(nn::Network& network) const;

private:
    std::vector<Tensor<float>> _parameters;
    PolicyScore _bestScore;
    std::optional<DeterministicReplayResult> _replay;
};

}  // namespace aisolver::training
