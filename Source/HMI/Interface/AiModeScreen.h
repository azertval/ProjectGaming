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
 *
 * Écran **du jeu** : il relève donc de l'identité pixel art (`EX-IHM-070`) — police bitmap,
 * couleurs et facteur d'agrandissement entier reçus de `theme.qss`, cadre à bordure franche
 * (`PixelFrameWidget`, posé dans le `.ui`) — et de la marque explicite de focus (`EX-IHM-071`),
 * peinte par `PixelFocusCaret` pour les contrôles Qt ordinaires et par `PixelMenuButton` pour
 * l'entrée « Retour ».
 */

namespace Ui {
class AiModeScreen;
}

namespace hmi {

class Localization;
class PixelFocusCaret;

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
    void onTrainingPreviewReady(QString replayPath, QString algorithmId, QString levelPath,
                                int generation);
    void onTrainingFinished(bool solved, QString modelPath, QString statsPath, QString configPath,
                            QString replayPath, bool replayExported);
    void onTrainingFailed(QString message);
    void onEvaluate();
    void onSaveModel();
    void onExportReplay();
    void onLaunchReplay();
    void onOpenRunFolder();

private:
    [[nodiscard]] QString selectedAlgo() const;
    /// @return Traduction de @p key dans la langue courante, vide tant que `retranslateUi` n'a pas
    ///         été appelé (l'écran affiche alors les libellés du `.ui`).
    [[nodiscard]] QString text(const char* key) const;
    void setTrainingControlsEnabled(bool enabled);
    void teardownWorker();
    /// Affiche uniquement les groupes de paramètres pertinents pour `selectedAlgo()` (menu
    /// évolutif de l'onglet Entraînement, `LOT-ANNEXE-21`) : Commun+Évolutionniste pour `evo`,
    /// Commun+Gradient pour `pg`/`ac`, Commun+Gradient+DQN avancé pour `avance`.
    void updateFieldVisibility();

    std::unique_ptr<Ui::AiModeScreen> _ui;
    /// Marque explicite de focus (`EX-IHM-071`) pour les contrôles Qt ordinaires de l'écran, que
    /// leur feuille de style ne peut pas signaler autrement que par la teinte. Enfant de l'écran,
    /// donc détruit avec lui.
    PixelFocusCaret* _focusCaret = nullptr;
    std::unique_ptr<QThread> _workerThread;
    TrainingWorker* _worker = nullptr;  ///< Possédé par `_workerThread` (deleteLater), pas `_ui`.
    /// Catalogue courant, mémorisé par `retranslateUi` : les messages d'état produits pendant un
    /// entraînement doivent être traduits eux aussi, longtemps après la construction.
    const Localization* _loc = nullptr;
    /// Dossier du dernier run terminé (`onTrainingFinished`), pour le bouton « Ouvrir le dossier du
    /// run » — vide tant qu'aucun run n'a terminé dans cette session de l'écran.
    QString _lastRunDir;
};

}  // namespace hmi
