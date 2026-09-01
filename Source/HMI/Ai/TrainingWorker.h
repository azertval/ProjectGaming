// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMetaType>
#include <QObject>
#include <QString>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "HMI/Ai/TrainingRequest.h"

/**
 * @file HMI/Ai/TrainingWorker.h
 * @brief Exécute un entraînement (`LOT-ANNEXE-21`, `EX-IA-022`) sur un `QThread` séparé, pour que
 * l'onglet Entraînement du menu Mode IA reste réactif pendant un run potentiellement long.
 *
 * Délègue intégralement aux types déjà cadrés par le programme Lot-Annexe (`LevelTrainingSession`,
 * `ReinforceTrainer`, `ActorCriticTrainer`, `DqnTrainer`) — même construction, mêmes défauts que
 * `aisolver::cli::runTrain` (`LOT-ANNEXE-19`) : aucune règle d'apprentissage n'est réimplémentée
 * ici, uniquement l'observation (progression, aperçu) et l'interruption propre, que la CLI
 * (synchrone, jamais interrompue) n'a pas besoin d'exposer.
 */

namespace hmi {

/**
 * @brief Instantané d'une génération/d'un épisode, transporté par le signal
 * `TrainingWorker::progress`.
 *
 * Reprend **toute** la ligne journalisée (`aisolver::TrainingStatsRow`) plutôt qu'une sélection :
 * le CSV du run porte déjà la pire récompense, l'écart-type, le nombre de pas du meilleur et la
 * graine du lot, et n'en transmettre qu'une partie obligerait l'écran à relire le fichier qu'il
 * vient lui-même de faire écrire. S'y ajoutent deux grandeurs qui n'existent que pendant le run,
 * absentes du CSV commun : l'exploration courante (DQN) et l'avancement du critère de stabilité
 * (évolutionniste).
 */
struct TrainingProgress {
    /// Index de génération (évolutionniste) ou d'épisode (par gradient), à partir de `0`.
    int index = 0;
    double bestReward = 0.0;
    double meanReward = 0.0;
    double worstReward = 0.0;
    double rewardStdDev = 0.0;
    int bestStepCount = 0;
    double successRate = 0.0;
    std::uint64_t seed = 0;
    /// Moyenne mobile de la meilleure récompense, et sa variation depuis l'étape précédente
    /// (`aisolver::TrainingStatsDerived`). L'enregistreur les calculait déjà pour le CSV du run,
    /// mais ne les transmettait pas : l'écran ne pouvait donc tracer qu'une courbe brute, sur
    /// laquelle une progression lente est indiscernable du bruit d'une génération à l'autre.
    double movingAverageReward = 0.0;
    double rewardDelta = 0.0;
    /// Pas de simulation cumulés depuis le début du run, renseignés sur le seul chemin DQN
    /// (`std::nullopt` ailleurs) : c'est de ce compteur que dépend la décroissance d'`epsilon`.
    std::optional<std::size_t> totalSteps;
    /// `epsilon` courant, renseigné sur le seul chemin DQN (`std::nullopt` ailleurs).
    std::optional<float> epsilon;
    /// Générations consécutives déjà stables et seuil exigé, renseignés sur le seul chemin
    /// évolutionniste (`std::nullopt` ailleurs) — voir
    /// `aisolver::training::LevelTrainingSession::setOnStabilityChanged`.
    std::optional<int> consecutiveStable;
    std::optional<int> requiredStable;
};

/**
 * @brief Travailleur `QObject`, déplacé sur un `QThread` par l'appelant (`AiModeScreen`) :
 * `run()` bloque jusqu'à la fin de l'entraînement ou une interruption via `requestStop()`.
 */
class TrainingWorker : public QObject {
    Q_OBJECT

public:
    explicit TrainingWorker(TrainingRequest request, QObject* parent = nullptr);

public slots:
    /// @brief Lance l'entraînement (bloquant) ; à connecter au signal `QThread::started`.
    void run();

    /// @brief Demande l'arrêt à la prochaine génération/épisode (`EX-NFR-040` : jamais de
    /// terminaison brutale du thread) ; sans effet si l'entraînement est déjà terminé.
    /// Thread-safe (indicateur atomique) : appelable directement depuis le thread appelant.
    void requestStop();

signals:
    /// @brief Une génération (évolutionniste) ou un épisode (par gradient) vient d'être
    /// journalisé (`TrainingStatsRecorder::setOnRecord`, `LOT-ANNEXE-09`/`21`).
    void progress(hmi::TrainingProgress step);

    /// @brief Un aperçu du champion courant a été écrit sur disque (`LOT-ANNEXE-21`), sous un
    /// fichier propre à `generation` (jamais écrasé par l'aperçu suivant) : `replayPath` se charge
    /// directement via `aisolver::readReplay`, `generation` est le numéro de génération
    /// (évolutif) ou d'épisode (par gradient) auquel il correspond, permettant à l'IHM de
    /// proposer un choix parmi les aperçus déjà reçus plutôt que le seul plus récent.
    void previewReady(QString replayPath, QString algorithmId, QString levelPath, int generation);

    /// @brief Run terminé (résolu, interrompu, ou plafond atteint) sans erreur récupérable.
    /// @param generationsRun Générations (évolutionniste) ou épisodes (par gradient) réellement
    ///        exécutés — distinct du plafond demandé dès qu'un run s'arrête par résolution ou par
    ///        interruption.
    void finished(bool solved, QString modelPath, QString statsPath, QString configPath,
                  QString replayPath, bool replayExported, int generationsRun);

    /// @brief Erreur récupérable (niveau introuvable, échec d'écriture...) — jamais d'exception
    /// traversant `run()` (`EX-NFR-040`).
    /// @param messageKey Clé du catalogue de traduction décrivant l'échec (`ai_mode.error_*`),
    ///        jamais un libellé : le worker n'a pas accès au catalogue, et un libellé écrit ici
    ///        s'afficherait en français quelle que soit la langue choisie.
    /// @param detail Détail substitué à `%1` dans le libellé traduit (chemin fautif, le plus
    ///        souvent).
    void failed(QString messageKey, QString detail);

private:
    TrainingRequest _request;
    std::atomic<bool> _stopRequested{false};
};

}  // namespace hmi

// Le signal `progress` traverse la frontière de thread du worker : Qt sérialise alors l'argument
// dans la file d'événements du thread d'IHM, ce qu'il ne sait faire que pour un type enregistré
// auprès de son système de métatypes.
Q_DECLARE_METATYPE(hmi::TrainingProgress)
