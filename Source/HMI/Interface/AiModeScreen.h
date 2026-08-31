// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QElapsedTimer>
#include <QString>
#include <QThread>
#include <QWidget>
#include <memory>
#include <optional>

#include "AiSolver/Cli/TrainingConfig.h"
#include "HMI/Ai/EvaluationWorker.h"
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

    /// @return `true` si une évaluation est actuellement active.
    ///
    /// Même enjeu que `trainingActive` : une campagne de trente répétitions dure aussi longtemps
    /// qu'un entraînement court, et fermer la fenêtre pendant joint son fil sans rien dire.
    [[nodiscard]] bool evaluationActive() const noexcept;

    /// Demande l'arrêt de l'évaluation en cours, sans attendre ; sans effet si aucune n'est active.
    void stopEvaluationIfActive();

signals:
    void backRequested();
    /// Émis pour « Voir en jeu » (aperçu) et « Lancer le rejeu » (onglet Rejeu) — `MainWindow`
    /// bascule vers le viewport et appelle `GameViewport::startReplay(path)`.
    void replayRequested(QString replayPath);

private slots:
    void onLaunchTraining();
    void onStopTraining();
    void onTrainingProgress(hmi::TrainingProgress step);
    void onTrainingPreviewReady(QString replayPath, QString algorithmId, QString levelPath,
                                int generation);
    void onTrainingFinished(bool solved, QString modelPath, QString statsPath, QString configPath,
                            QString replayPath, bool replayExported, int generationsRun);
    void onTrainingFailed(QString messageKey, QString detail);
    void onEvaluate();
    void onStopEvaluation();
    void onEvaluationProgress(int completed, int total);
    void onEvaluationFinished(hmi::EvaluationOutcome outcome);
    void onEvaluationFailed();
    void onBrowseModel();
    void onExportReport();
    void onReuseRunSettings();
    void onSaveModel();
    void onExportReplay();
    void onLaunchReplay();
    void onOpenRunFolder();
    void onBrowseRunsRoot();
    void onLoadConfig();
    void onSaveConfig();
    void onResetDefaults();

private:
    [[nodiscard]] QString selectedAlgo() const;
    /// Recopie les champs du formulaire d'entraînement dans une configuration résolue, et
    /// l'inverse — un seul endroit qui connaît la correspondance champ ↔ hyperparamètre, partagé
    /// par le lancement d'un run, les presets et la reprise des réglages d'un run passé.
    [[nodiscard]] aisolver::cli::TrainingConfig configFromForm() const;
    void applyConfigToForm(const aisolver::cli::TrainingConfig& config);
    /// Charge dans le formulaire le `config.json` de @p runDir (bouton « Reprendre les réglages »,
    /// et chargement d'un preset) ; sans effet si le fichier est absent ou illisible.
    /// Nombre maximal de lignes conservées dans le tableau de statistiques.
    ///
    /// L'historique complet vit dans le `stats.csv` du run ; ce tableau montre ce qui vient de se
    /// passer. Sans plafond, un run de plusieurs milliers d'épisodes alloue autant de fois huit
    /// cellules que personne ne fera jamais défiler.
    static constexpr int MAX_STATS_ROWS = 500;

    void loadConfigFile(const QString& path);

    /// @return Le dossier de runs choisi dans le formulaire, ou celui à côté de l'exécutable si le
    /// champ est vide. Sert à l'écriture **et** à la lecture des runs.
    [[nodiscard]] std::filesystem::path selectedRunsRoot() const;
    /// Met à jour l'estimation de temps restant (`trainingEtaLabel`) à partir des instants des
    /// générations déjà reçues ; efface l'étiquette tant que la durée totale est inconnue.
    void updateEta(int completedSteps);
    /// Assemble la requête d'évaluation à partir de l'onglet Validation, ou `std::nullopt` si
    /// aucun modèle/niveau n'est sélectionné.
    [[nodiscard]] std::optional<EvaluationRequest> evaluationRequestFromForm() const;
    void setEvaluationControlsEnabled(bool enabled);
    void teardownEvaluationWorker();
    /// Chemin du modèle et algorithme du run actuellement choisi dans `runCombo` — l'algorithme
    /// est relu dans le `config.json` du run, jamais déduit des boutons radio de l'onglet
    /// Entraînement (qui décrivent le PROCHAIN entraînement).
    /// @return L'identifiant d'algorithme lu dans le `config.json` voisin de @p modelPath, ou une
    /// chaîne vide si le chemin est vide. Statique : ne dépend d'aucun état de l'écran, seulement
    /// du modèle désigné.
    [[nodiscard]] static QString algorithmOfModel(const QString& modelPath);
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
    /// Début du run courant et nombre d'étapes déjà reçues : base de l'estimation de temps
    /// restant, calculée ici et non par le travailleur (une durée n'a de sens qu'affichée).
    QElapsedTimer _trainingClock;
    std::unique_ptr<QThread> _evaluationThread;
    /// Possédé par `_evaluationThread` (deleteLater), pas `_ui` ; nul hors évaluation.
    EvaluationWorker* _evaluationWorker = nullptr;
    /// Dernier résultat d'évaluation affiché, avec la requête qui l'a produit : le rapport CSV
    /// nomme l'algorithme et le niveau mesurés, que l'écran ne doit pas re-déduire au moment de
    /// l'export (l'utilisateur a pu changer la sélection entre-temps).
    std::optional<EvaluationOutcome> _lastEvaluation;
    std::optional<EvaluationRequest> _lastEvaluationRequest;
};

}  // namespace hmi
