// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <atomic>
#include <cstdint>
#include <optional>

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

/// Paramètres d'un run, saisis dans l'onglet Entraînement — mêmes champs que
/// `aisolver::cli::TrainArgs`/`CommandLineOverrides` (`LOT-ANNEXE-19`), en `QString`/valeurs
/// simples pour rester utilisables sans dépendre de `AiSolver/Cli` depuis un en-tête Qt.
struct TrainingRequest {
    QString levelPath;
    QString algorithmId;  ///< `"evo"`, `"pg"`, `"ac"` ou `"avance"`.
    std::uint64_t seed = 0;
    QString runsRoot;  ///< Vide : défaut `aisolver::training::DEFAULT_TRAINING_RUNS_ROOT`.
    std::optional<std::size_t> populationSize;
    std::optional<float> mutationRate;
    std::optional<std::size_t> episodes;
    std::optional<float> learningRate;
    std::optional<float> gamma;
    QString optimizer;  ///< Vide : défaut (`"sgd"`).
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
    void progress(int index, double bestReward, double meanReward, double successRate);

    /// @brief Un aperçu du champion courant a été écrit sur disque (`LOT-ANNEXE-21`), sous un
    /// fichier propre à `generation` (jamais écrasé par l'aperçu suivant) : `replayPath` se charge
    /// directement via `aisolver::readReplay`, `generation` est le numéro de génération
    /// (évolutif) ou d'épisode (par gradient) auquel il correspond, permettant à l'IHM de
    /// proposer un choix parmi les aperçus déjà reçus plutôt que le seul plus récent.
    void previewReady(QString replayPath, QString algorithmId, QString levelPath, int generation);

    /// @brief Run terminé (résolu, interrompu, ou plafond atteint) sans erreur récupérable.
    void finished(bool solved, QString modelPath, QString statsPath, QString configPath,
                  QString replayPath, bool replayExported);

    /// @brief Erreur récupérable (niveau introuvable, échec d'écriture...) — jamais d'exception
    /// traversant `run()` (`EX-NFR-040`).
    void failed(QString message);

private:
    TrainingRequest _request;
    std::atomic<bool> _stopRequested{false};
};

}  // namespace hmi
