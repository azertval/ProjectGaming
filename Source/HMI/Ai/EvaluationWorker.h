// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <atomic>

#include "HMI/Ai/ModelEvaluation.h"

/**
 * @file HMI/Ai/EvaluationWorker.h
 * @brief Exécute une évaluation (`LOT-ANNEXE-22`) sur un `QThread` séparé, pour que l'onglet
 * Validation & sauvegarde reste réactif pendant une campagne potentiellement longue.
 *
 * Même raison d'être que `TrainingWorker`, appliquée à l'autre capacité coûteuse de l'écran : une
 * campagne rejoue le niveau `repetitions` fois, chacune jusqu'à `maxStepsPerEpisode` pas, et fige
 * donc la fenêtre aussi sûrement qu'un entraînement si elle est lancée sur le thread d'IHM.
 * Délègue intégralement à `hmi::evaluateModel` : aucune règle d'évaluation n'est réimplémentée
 * ici, uniquement l'observation (progression) et l'interruption propre.
 */

namespace hmi {

/**
 * @brief Travailleur `QObject`, déplacé sur un `QThread` par l'appelant (`AiModeScreen`) :
 * `run()` bloque jusqu'à la fin de la campagne ou une interruption via `requestStop()`.
 */
class EvaluationWorker : public QObject {
    Q_OBJECT

public:
    explicit EvaluationWorker(EvaluationRequest request, QObject* parent = nullptr);

public slots:
    /// @brief Lance l'évaluation (bloquant) ; à connecter au signal `QThread::started`.
    void run();

    /// @brief Demande l'arrêt à la fin de la répétition en cours (`EX-NFR-040` : jamais de
    /// terminaison brutale du thread). Thread-safe (indicateur atomique).
    void requestStop();

signals:
    /// @brief Une répétition vient de se terminer.
    void progress(int completed, int total);

    /// @brief Campagne terminée (complète ou interrompue) : @p outcome porte alors les seules
    /// répétitions jouées (voir `EvaluationOutcome::repetitionsRun`).
    void finished(hmi::EvaluationOutcome outcome);

    /// @brief Modèle illisible (chemin invalide, topologie incompatible) — jamais d'exception
    /// traversant `run()` (`EX-NFR-040`).
    void failed();

private:
    EvaluationRequest _request;
    std::atomic<bool> _stopRequested{false};
};

}  // namespace hmi

// Voir `TrainingWorker.h` : le signal traverse la frontière de thread du worker, Qt doit donc
// savoir sérialiser son argument.
Q_DECLARE_METATYPE(hmi::EvaluationOutcome)
