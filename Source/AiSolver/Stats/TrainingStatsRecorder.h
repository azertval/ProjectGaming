// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
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
     * aucune ligne déjà journalisée (`EX-NFR-040`).
     */
    void record(const TrainingStatsRow& row);

private:
    std::ofstream csvFile_;
    MovingAverageTracker movingAverage_;
    bool hasPreviousMovingAverage_ = false;
    float previousMovingAverage_ = 0.0f;
};

}  // namespace aisolver
