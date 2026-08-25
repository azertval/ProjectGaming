// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QThread>
#include <QWidget>
#include <memory>

#include "HMI/Ai/TrainingWorker.h"

/**
 * @file HMI/Interface/AiModeScreen.h
 * @brief Écran « Mode IA » du menu principal (`LOT-ANNEXE-21`, `EX-IA-022`) : trois onglets
 * (Entraînement, Validation & sauvegarde, Rejeu), équivalent IHM de `aisolver-cli`
 * (`LOT-ANNEXE-19`).
 */

namespace Ui {
class AiModeScreen;
}

namespace hmi {

class Localization;

/**
 * @brief Page du `QStackedWidget` (comme `MainMenu`/`CreditsScreen`), pas un recouvrement.
 * Possède le `TrainingWorker` et son `QThread` : créés à la demande (« Lancer l'entraînement »),
 * jamais avant ; arrêtés proprement (`requestStop` puis `wait()`) avant toute destruction.
 */
class AiModeScreen : public QWidget {
    Q_OBJECT

public:
    explicit AiModeScreen(QWidget* parent = nullptr);
    ~AiModeScreen() override;

    /// Applique la langue active aux libellés et infobulles.
    void retranslateUi(const Localization& loc);

    /// Donne le focus clavier au bouton « Retour ».
    void focusDefaultAction();

    /// Repeuple le sélecteur de niveaux (appelé à chaque ouverture de l'écran, `LOT-ANNEXE-21`) :
    /// un niveau ajouté/retiré du dossier pendant que l'écran était fermé doit apparaître.
    void refreshLevelList();

    /// Repeuple la liste des runs entraînés (onglet Validation & sauvegarde) et des rejeux publiés
    /// (onglet Rejeu) — même raison que `refreshLevelList`.
    void refreshRunsAndReplays();

    /// @return `true` si un entraînement est actuellement actif (empêche la fermeture immédiate
    /// de la fenêtre sans arrêter le thread proprement — voir `MainWindow`).
    [[nodiscard]] bool trainingActive() const noexcept;

    /// Demande l'arrêt de l'entraînement en cours, sans attendre (voir `trainingActive`) ; sans
    /// effet si aucun entraînement n'est actif.
    void stopTrainingIfActive();

signals:
    void backRequested();
    /// Émis pour « Voir en jeu » (aperçu) et « Lancer le rejeu » (onglet Rejeu) — `MainWindow`
    /// bascule vers le viewport et appelle `GameViewport::startReplay(path)`.
    void replayRequested(QString replayPath);

private slots:
    void onLaunchTraining();
    void onStopTraining();
    void onTrainingProgress(int index, double bestReward, double meanReward, double successRate);
    void onTrainingPreviewReady(QString replayPath, QString algo, QString levelPath,
                                int generation);
    void onTrainingFinished(bool solved, QString modelPath, QString statsPath, QString configPath,
                            QString replayPath, bool replayExported);
    void onTrainingFailed(QString message);
    void onEvaluate();
    void onSaveModel();
    void onExportReplay();
    void onLaunchReplay();

private:
    [[nodiscard]] QString selectedAlgo() const;
    void setTrainingControlsEnabled(bool enabled);
    void teardownWorker();

    std::unique_ptr<Ui::AiModeScreen> _ui;
    std::unique_ptr<QThread> _workerThread;
    TrainingWorker* _worker = nullptr;  ///< Possédé par `_workerThread` (deleteLater), pas `_ui`.
    QString _lastRunModelPath;
    QString _lastRunReplayPath;
    QString _lastRunAlgo;
};

}  // namespace hmi
