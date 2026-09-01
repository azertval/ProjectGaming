// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>

#include "AiSolver/Stats/MovingAverage.h"

/**
 * @file AiSolver/Stats/TrainingStatsRecorder.h
 * @brief Journalisation CSV incrémentale des statistiques d'un entraînement, partagée par tout
 * algorithme (évolutionniste ou par gradient) (`LOT-ANNEXE-09`, `EX-IA-010`).
 */

namespace aisolver {

/**
 * @brief Une ligne de statistiques : une génération (évolutionniste) ou un épisode/lot d'épisodes
 * (par gradient). Structure unique, sans branchement par famille d'algorithme.
 */
struct TrainingStatsRow {
    /// Index de génération (évolutionniste) ou d'épisode (par gradient), à partir de `0`.
    int index = 0;
    /// Meilleure récompense de la génération/du lot d'épisodes.
    float bestReward = 0.0f;
    /// Récompense moyenne.
    float meanReward = 0.0f;
    /// Pire récompense.
    float worstReward = 0.0f;
    /// Écart-type des récompenses.
    float rewardStdDev = 0.0f;
    /// Nombre de pas de l'individu/épisode le meilleur.
    int bestStepCount = 0;
    /// Taux de réussite (`EpisodeStatus::Won`) du lot, dans `[0, 1]`.
    float successRate = 0.0f;
    /// Graine utilisée par ce lot/épisode.
    std::uint64_t seed = 0;
    /// Nom du niveau en cours d'entraînement (dupliqué depuis le chemin de fichier, voir
    /// `TrainingRunPath.h` : un CSV doit rester interprétable seul).
    std::string levelName;
};

/**
 * @brief Grandeurs **dérivées** par l'enregistreur au fil des lignes, écrites dans le CSV.
 *
 * Distinctes de `TrainingStatsRow`, qui est ce que l'appelant *fournit* : celles-ci, l'enregistreur
 * les calcule, parce qu'elles dépendent de l'historique du run et non de la seule génération
 * courante. Elles étaient calculées, écrites, puis perdues — l'observateur ne recevait que la
 * ligne brute, si bien qu'une IHM voulant tracer une courbe lissée devait relire le fichier que
 * l'enregistreur venait d'écrire, ou refaire le calcul.
 */
struct TrainingStatsDerived {
    /// Moyenne mobile de `bestReward` sur la fenêtre passée au constructeur.
    float movingAverageReward = 0.0f;
    /// Variation de cette moyenne depuis la ligne précédente ; `0` sur la première ligne.
    float rewardDelta = 0.0f;
};

/**
 * @brief Enregistreur CSV incrémental : un fichier par instance, une ligne par appel à `record`,
 * jamais rouvert en ajout (voir `makeTrainingRunPath`/`generateRunId`, `TrainingRunPath.h`, pour la
 * construction du chemin d'un nouveau run).
 *
 * Aucune dépendance à un module du dossier `Source/AiSolver/Training` : cette classe ignore tout
 * des algorithmes qui l'appellent, condition pour qu'un même format serve indifféremment aux
 * générations 2 et 3, puis au harnais de benchmark (`LOT-ANNEXE-15`).
 */
class TrainingStatsRecorder {
public:
    /**
     * @param outputCsvPath Chemin du fichier CSV à créer (écrase un fichier existant au même
     * chemin) ; les dossiers parents manquants sont créés. L'en-tête de colonnes est écrit
     * immédiatement.
     * @param movingAverageWindow Taille de la fenêtre de moyenne mobile sur `bestReward` (voir
     * `MovingAverageTracker`) ; documentée, pas une constante magique enfouie.
     */
    explicit TrainingStatsRecorder(const std::filesystem::path& outputCsvPath,
                                   int movingAverageWindow = 20);

    /**
     * @brief Ajoute une ligne au fichier CSV et force son écriture sur disque (`flush`).
     *
     * Un `flush` après chaque ligne a un coût (une écriture disque par génération/épisode),
     * volontairement accepté : la fréquence d'appel reste très inférieure au temps de calcul d'une
     * génération/d'un épisode complet, et garantit qu'un arrêt prématuré du processus ne perde
     * aucune ligne déjà journalisée (`EX-NFR-040`). Ce coût ne dépend jamais de la taille de
     * population : une seule ligne par génération, quel que soit le nombre d'individus évalués —
     * la trace reste donc toujours écrite, quelle que soit la taille de la population.
     */
    void record(const TrainingStatsRow& row);

    /**
     * @brief Observateur optionnel, appelé à la fin de chaque `record()` avec la ligne qui vient
     * d'être journalisée (`LOT-ANNEXE-21`) : point d'observation unique, déjà traversé par les
     * quatre familles d'algorithmes (évolutionniste, REINFORCE, acteur-critique, DQN), pour ne
     * jamais dupliquer la boucle d'entraînement dans l'IHM — celle-ci n'a besoin que d'observer,
     * jamais de réimplémenter.
     * @param callback Invoqué après l'écriture disque de chaque ligne, avec la ligne **et** les
     *        grandeurs dérivées qui l'accompagnent dans le CSV ; `nullptr` (défaut) pour ne rien
     *        observer, comportement inchangé.
     */
    void setOnRecord(
        std::function<void(const TrainingStatsRow&, const TrainingStatsDerived&)> callback) {
        onRecord_ = std::move(callback);
    }

private:
    std::ofstream csvFile_;
    MovingAverageTracker movingAverage_;
    bool hasPreviousMovingAverage_ = false;
    float previousMovingAverage_ = 0.0f;
    std::function<void(const TrainingStatsRow&, const TrainingStatsDerived&)> onRecord_;
};

}  // namespace aisolver
