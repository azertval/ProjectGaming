// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/AiModeScreen.h"

#include <QApplication>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <filesystem>
#include <system_error>

#include "AiSolver/Cli/TrainingConfig.h"
#include "AiSolver/Replay/ReplayFile.h"
#include "HMI/Ai/ModelEvaluation.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/PixelFocusCaret.h"
#include "HMI/Interface/TrainingChartWidget.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "ui_AiModeScreen.h"

/**
 * @file HMI/Interface/AiModeScreen.cpp
 * @brief Voir `AiModeScreen.h`. Ne référence jamais `AiSolver/Training`/`Nn`/`Optim` directement
 * (amendement de `LOT-ANNEXE-18` limité à `HMI/Ai`, voir `epic.md` de `LOT-ANNEXE-21`) : délègue à
 * `hmi::TrainingWorker` (thread, entraînement) et `hmi::evaluateModel` (`HMI/Ai/ModelEvaluation`).
 */

namespace hmi {

namespace {

constexpr const char* RUNS_ROOT = "TrainingRuns";
constexpr const char* REPLAYS_DIR_NAME = "Replays";
constexpr const char* LEVELS_DIR_NAME = "Levels";

std::filesystem::path levelsDir() {
    return executableDirectory() / LEVELS_DIR_NAME;
}

std::filesystem::path runsRootDir() {
    return executableDirectory() / RUNS_ROOT;
}

std::filesystem::path replaysDir() {
    return executableDirectory() / REPLAYS_DIR_NAME;
}

}  // namespace

AiModeScreen::AiModeScreen(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::AiModeScreen>()) {
    setObjectName(QStringLiteral("AiModeScreen"));  // ciblé par le thème (theme-identity.qss)
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);

    const SpacingTokens& spacing = identityTokens().spacing;
    _ui->outerLayout->setContentsMargins(spacing.extraLarge * 2, spacing.extraLarge,
                                         spacing.extraLarge * 2, spacing.extraLarge);

    // Marque explicite de focus (EX-IHM-071). Le suivi est branche sur le focus de
    // L'APPLICATION, mais filtre par l'ecran : PixelFocusCaret::follow se masque de lui-meme des
    // que le controle focalise n'est pas un descendant de son hote, donc l'ouverture d'un autre
    // ecran ou d'une boite de dialogue efface la marque sans traitement particulier ici.
    _focusCaret = new PixelFocusCaret(this);
    connect(qApp, &QApplication::focusChanged, this,
            [this](QWidget*, QWidget* now) { _focusCaret->follow(now); });

    connect(_ui->backButton, &QPushButton::clicked, this, &AiModeScreen::backRequested);
    _ui->backButton->setAutoDefault(true);

    connect(_ui->launchTrainingButton, &QPushButton::clicked, this,
            &AiModeScreen::onLaunchTraining);
    connect(_ui->stopTrainingButton, &QPushButton::clicked, this, &AiModeScreen::onStopTraining);
    connect(_ui->previewButton, &QPushButton::clicked, this, [this] {
        const QString path = _ui->generationCombo->currentData().toString();
        if (!path.isEmpty()) {
            emit replayRequested(path);
        }
    });

    connect(_ui->evaluateButton, &QPushButton::clicked, this, &AiModeScreen::onEvaluate);
    connect(_ui->stopEvaluationButton, &QPushButton::clicked, this,
            &AiModeScreen::onStopEvaluation);
    connect(_ui->browseModelButton, &QPushButton::clicked, this, &AiModeScreen::onBrowseModel);
    connect(_ui->exportReportButton, &QPushButton::clicked, this, &AiModeScreen::onExportReport);
    connect(_ui->reuseRunSettingsButton, &QPushButton::clicked, this,
            &AiModeScreen::onReuseRunSettings);
    connect(_ui->saveModelButton, &QPushButton::clicked, this, &AiModeScreen::onSaveModel);
    connect(_ui->exportReplayButton, &QPushButton::clicked, this, &AiModeScreen::onExportReplay);
    connect(_ui->launchReplayButton, &QPushButton::clicked, this, &AiModeScreen::onLaunchReplay);
    connect(_ui->openRunFolderButton, &QPushButton::clicked, this, &AiModeScreen::onOpenRunFolder);
    connect(_ui->browseRunsRootButton, &QPushButton::clicked, this,
            &AiModeScreen::onBrowseRunsRoot);
    connect(_ui->loadConfigButton, &QPushButton::clicked, this, &AiModeScreen::onLoadConfig);
    connect(_ui->saveConfigButton, &QPushButton::clicked, this, &AiModeScreen::onSaveConfig);
    connect(_ui->resetDefaultsButton, &QPushButton::clicked, this, &AiModeScreen::onResetDefaults);

    // Le formulaire OUVRE sur les defauts du moteur, jamais sur les litteraux du .ui (EX-IHM-083).
    // Ces derniers ne sont qu'un repli de conception : trois d'entre eux avaient diverge du code
    // (taux d'apprentissage 0,01 contre 0,003 ; gamma 0,99 contre 0,995 ; optimiseur "sgd" contre
    // "adam"), si bien que l'ecran decrivait au demarrage un run que `aisolver-cli train` sans
    // option n'aurait pas produit -- et que "Reinitialiser aux defauts" CHANGEAIT trois champs
    // au lieu de n'en changer aucun. Meme appel que le bouton : une seule source de verite.
    applyConfigToForm(aisolver::cli::TrainingConfig{});

    // Le dossier des runs est modifiable, mais part de l'emplacement historique (a cote de
    // l'executable) : un utilisateur qui ne touche a rien retrouve ses runs precedents. Pose
    // APRES applyConfigToForm, qui ne connait pas cet emplacement.
    _ui->runsRootEdit->setText(QString::fromStdString(runsRootDir().string()));

    // Menu evolutif (LOT-ANNEXE-21) : chaque radio d'algorithme ne montre que les groupes de
    // parametres qui lui sont utiles (evite qu'un run DQN affiche "Taille de population", par
    // exemple, un champ sans aucun effet pour cet algorithme).
    for (QRadioButton* radio : {_ui->evolutionaryAlgorithmRadio, _ui->reinforceAlgorithmRadio,
                                _ui->actorCriticAlgorithmRadio, _ui->advancedAlgorithmRadio}) {
        connect(radio, &QRadioButton::toggled, this, [this](bool checked) {
            if (checked) {
                updateFieldVisibility();
            }
        });
    }
    // En-tetes des deux tables : aucune n'etait dimensionnee. Huit colonnes aux libelles longs
    // (« Meilleure recompense », « Recompense moyenne »...) restaient a la largeur par defaut, donc
    // tronquees, avec un defilement horizontal permanent pour lire ce qui aurait tenu. Et l'en-tete
    // VERTICAL affichait des numeros de ligne qui repetent la colonne « Generation » -- laquelle
    // porte deja l'index, et avec la bonne valeur.
    for (QTableWidget* const table : {_ui->statsTable, _ui->replayTable}) {
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->horizontalHeader()->setStretchLastSection(true);
        table->verticalHeader()->setVisible(false);
    }

    connect(_ui->replayTable, &QTableWidget::itemSelectionChanged, this, [this] {
        _ui->launchReplayButton->setEnabled(!_ui->replayTable->selectedItems().isEmpty());
    });
    // Choisir un run pre-remplit modele et niveau de l'onglet Validation, sans les y figer :
    // l'evaluation croisee (le meme modele sur un AUTRE niveau) est precisement ce que la ligne de
    // commande permet et que cet onglet interdisait.
    connect(_ui->runCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        const QString runDir = _ui->runCombo->currentData().toString();
        const bool hasRun = !runDir.isEmpty();
        _ui->saveModelButton->setEnabled(hasRun);
        _ui->reuseRunSettingsButton->setEnabled(hasRun);
        if (!hasRun) {
            return;
        }
        const std::filesystem::path runPath(runDir.toStdString());
        _ui->evalModelEdit->setText(QString::fromStdString((runPath / "model.bin").string()));
        const QString levelName = QString::fromStdString(runPath.parent_path().filename().string());
        const int levelIndex = _ui->evalLevelCombo->findText(levelName + ".json");
        if (levelIndex >= 0) {
            _ui->evalLevelCombo->setCurrentIndex(levelIndex);
        }
    });
    // L'export de rejeu ne depend plus du run selectionne mais du couple modele/niveau saisi.
    connect(_ui->evalModelEdit, &QLineEdit::textChanged, this,
            [this](const QString& path) { _ui->exportReplayButton->setEnabled(!path.isEmpty()); });

    setTrainingControlsEnabled(true);
    updateFieldVisibility();
    refreshLevelList();
    refreshRunsAndReplays();
}

AiModeScreen::~AiModeScreen() {
    // Les DEUX fils, pas seulement l'entrainement : detruire un `QThread` encore en cours abandonne
    // le processus (`QThread: Destroyed while thread is still running` puis `terminate()`), et une
    // evaluation dure assez longtemps pour que la fenetre se ferme pendant.
    teardownWorker();
    teardownEvaluationWorker();
}

QString AiModeScreen::text(const char* key) const {
    return _loc == nullptr ? QString() : QString::fromStdString(_loc->text(key));
}

void AiModeScreen::retranslateUi(const Localization& loc) {
    _loc = &loc;
    const auto t = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };

    _ui->titleLabel->setText(t("ai_mode.title"));
    _ui->backButton->setText(t("options.back"));

    _ui->tabs->setTabText(0, t("ai_mode.tab_training"));
    _ui->tabs->setTabText(1, t("ai_mode.tab_validate"));
    _ui->tabs->setTabText(2, t("ai_mode.tab_replay"));

    // Onglet Entraînement.
    _ui->commonGroup->setTitle(t("ai_mode.group_common"));
    _ui->evolutionaryGroup->setTitle(t("ai_mode.group_evolutionary"));
    _ui->gradientGroup->setTitle(t("ai_mode.group_gradient"));
    _ui->dqnGroup->setTitle(t("ai_mode.group_dqn"));
    _ui->levelLabel->setText(t("ai_mode.level"));
    _ui->algorithmLabel->setText(t("ai_mode.algorithm"));
    _ui->evolutionaryAlgorithmRadio->setText(t("ai_mode.algo_evo"));
    _ui->reinforceAlgorithmRadio->setText(t("ai_mode.algo_pg"));
    _ui->actorCriticAlgorithmRadio->setText(t("ai_mode.algo_ac"));
    _ui->advancedAlgorithmRadio->setText(t("ai_mode.algo_dqn"));
    _ui->hiddenSizeLabel->setText(t("ai_mode.hidden_size"));
    _ui->runsRootLabel->setText(t("ai_mode.runs_root"));
    _ui->browseRunsRootButton->setText(t("ai_mode.browse"));
    _ui->populationLabel->setText(t("ai_mode.population"));
    _ui->tournamentSizeLabel->setText(t("ai_mode.tournament_size"));
    _ui->mutationRateLabel->setText(t("ai_mode.mutation_rate"));
    _ui->mutationStrengthLabel->setText(t("ai_mode.mutation_strength"));
    _ui->maxGenerationsLabel->setText(t("ai_mode.max_generations"));
    _ui->requiredSuccessesLabel->setText(t("ai_mode.required_successes"));
    _ui->loadConfigButton->setText(t("ai_mode.load_config"));
    _ui->saveConfigButton->setText(t("ai_mode.save_config"));
    _ui->resetDefaultsButton->setText(t("ai_mode.reset_defaults"));
    _ui->stabilityLabel->setText(t("ai_mode.stability"));
    _ui->episodesLabel->setText(t("ai_mode.episodes"));
    _ui->learningRateLabel->setText(t("ai_mode.learning_rate"));
    _ui->gammaLabel->setText(t("ai_mode.gamma"));
    _ui->optimizerLabel->setText(t("ai_mode.optimizer"));
    _ui->criticLearningRateLabel->setText(t("ai_mode.critic_learning_rate"));
    _ui->crossoverRateLabel->setText(t("ai_mode.crossover_rate"));
    _ui->batchEpisodesLabel->setText(t("ai_mode.batch_episodes"));
    _ui->entropyLabel->setText(t("ai_mode.entropy"));
    _ui->explorationFloorLabel->setText(t("ai_mode.exploration_floor"));
    _ui->gradientClipNormLabel->setText(t("ai_mode.gradient_clip"));
    _ui->actionRepeatLabel->setText(t("ai_mode.action_repeat"));
    _ui->maxStepsBudgetLabel->setText(t("ai_mode.step_budget"));
    _ui->stuckThresholdLabel->setText(t("ai_mode.stuck_threshold"));
    _ui->criticLearningRateSpin->setToolTip(t("ai_mode.critic_learning_rate_tip"));
    _ui->crossoverRateSpin->setToolTip(t("ai_mode.crossover_rate_tip"));
    _ui->batchEpisodesSpin->setToolTip(t("ai_mode.batch_episodes_tip"));
    _ui->entropySpin->setToolTip(t("ai_mode.entropy_tip"));
    _ui->explorationFloorSpin->setToolTip(t("ai_mode.exploration_floor_tip"));
    _ui->gradientClipNormSpin->setToolTip(t("ai_mode.gradient_clip_tip"));
    _ui->actionRepeatSpin->setToolTip(t("ai_mode.action_repeat_tip"));
    _ui->stepBudgetSpin->setToolTip(t("ai_mode.step_budget_tip"));
    _ui->stuckThresholdSpin->setToolTip(t("ai_mode.stuck_threshold_tip"));
    _ui->seedLabel->setText(t("ai_mode.seed"));
    _ui->dqnReplayCapacityLabel->setText(t("ai_mode.dqn_replay_capacity"));
    _ui->dqnBatchSizeLabel->setText(t("ai_mode.dqn_batch_size"));
    _ui->dqnWarmupSizeLabel->setText(t("ai_mode.dqn_warmup_size"));
    _ui->dqnUpdatePeriodLabel->setText(t("ai_mode.dqn_update_period"));
    _ui->dqnTargetSyncPeriodLabel->setText(t("ai_mode.dqn_target_sync_period"));
    _ui->dqnEpsilonStartLabel->setText(t("ai_mode.dqn_epsilon_start"));
    _ui->dqnEpsilonEndLabel->setText(t("ai_mode.dqn_epsilon_end"));
    _ui->dqnEpsilonDecayLabel->setText(t("ai_mode.dqn_epsilon_decay"));
    _ui->dqnEpsilonCurrentLabel->setText(t("ai_mode.dqn_epsilon_current"));
    _ui->launchTrainingButton->setText(t("ai_mode.launch_training"));
    _ui->stopTrainingButton->setText(t("ai_mode.stop_training"));
    _ui->previewButton->setText(t("ai_mode.preview"));
    _ui->openRunFolderButton->setText(t("ai_mode.open_run_folder"));
    _ui->statsTable->setHorizontalHeaderLabels(
        {t("ai_mode.column_generation"), t("ai_mode.column_best_reward"),
         t("ai_mode.column_mean_reward"), t("ai_mode.column_worst_reward"),
         t("ai_mode.column_reward_stddev"), t("ai_mode.column_best_steps"),
         t("ai_mode.column_success_rate"), t("ai_mode.column_seed")});
    _ui->trainingChart->setEmptyLabel(t("ai_mode.chart_empty"));
    _ui->trainingChart->setSeriesLabels(
        t("ai_mode.column_best_reward"), t("ai_mode.column_mean_reward"),
        t("ai_mode.chart_moving_average"), t("ai_mode.column_success_rate"));

    // Onglet Validation & sauvegarde.
    _ui->runLabel->setText(t("ai_mode.run"));
    _ui->evalModelLabel->setText(t("ai_mode.eval_model"));
    _ui->browseModelButton->setText(t("ai_mode.browse"));
    _ui->evalLevelLabel->setText(t("ai_mode.eval_level"));
    _ui->repetitionsLabel->setText(t("ai_mode.repetitions"));
    _ui->maxStepsLabel->setText(t("ai_mode.max_steps"));
    _ui->evalSeedLabel->setText(t("ai_mode.eval_seed"));
    _ui->decodingLabel->setText(t("ai_mode.decoding"));
    _ui->evaluateButton->setText(t("ai_mode.evaluate"));
    _ui->stopEvaluationButton->setText(t("ai_mode.stop_evaluation"));
    _ui->successRateLabel->setText(t("ai_mode.success_rate"));
    _ui->meanStepsLabel->setText(t("ai_mode.mean_steps"));
    _ui->meanStepsAllLabel->setText(t("ai_mode.mean_steps_all"));
    _ui->varianceLabel->setText(t("ai_mode.variance"));
    _ui->repetitionsRunLabel->setText(t("ai_mode.repetitions_run"));
    _ui->saveModelButton->setText(t("ai_mode.save_model"));
    _ui->exportReplayButton->setText(t("ai_mode.export_replay"));
    _ui->exportReportButton->setText(t("ai_mode.export_report"));
    _ui->reuseRunSettingsButton->setText(t("ai_mode.reuse_run_settings"));

    // Onglet Rejeu.
    _ui->replayTable->setHorizontalHeaderLabels(
        {t("ai_mode.column_level"), t("ai_mode.column_algorithm"), t("ai_mode.column_reward"),
         t("ai_mode.column_exported")});
    _ui->launchReplayButton->setText(t("ai_mode.launch_replay"));
    _ui->replayHintLabel->setText(t("ai_mode.replay_hint"));

    // Infobulles : une par champ/bouton, rejouees ici comme le reste — une infobulle posee une
    // seule fois à la construction resterait dans la langue de depart.
    _ui->levelCombo->setToolTip(t("ai_mode.level_tip"));
    _ui->evolutionaryAlgorithmRadio->setToolTip(t("ai_mode.algo_evo_tip"));
    _ui->reinforceAlgorithmRadio->setToolTip(t("ai_mode.algo_pg_tip"));
    _ui->actorCriticAlgorithmRadio->setToolTip(t("ai_mode.algo_ac_tip"));
    _ui->advancedAlgorithmRadio->setToolTip(t("ai_mode.algo_dqn_tip"));
    _ui->hiddenSizeSpin->setToolTip(t("ai_mode.hidden_size_tip"));
    _ui->runsRootEdit->setToolTip(t("ai_mode.runs_root_tip"));
    _ui->browseRunsRootButton->setToolTip(t("ai_mode.runs_root_tip"));
    _ui->populationSpin->setToolTip(t("ai_mode.population_tip"));
    _ui->tournamentSizeSpin->setToolTip(t("ai_mode.tournament_size_tip"));
    _ui->mutationRateSpin->setToolTip(t("ai_mode.mutation_rate_tip"));
    _ui->mutationStrengthSpin->setToolTip(t("ai_mode.mutation_strength_tip"));
    _ui->maxGenerationsSpin->setToolTip(t("ai_mode.max_generations_tip"));
    _ui->requiredSuccessesSpin->setToolTip(t("ai_mode.required_successes_tip"));
    _ui->loadConfigButton->setToolTip(t("ai_mode.load_config_tip"));
    _ui->saveConfigButton->setToolTip(t("ai_mode.save_config_tip"));
    _ui->resetDefaultsButton->setToolTip(t("ai_mode.reset_defaults_tip"));
    _ui->episodesSpin->setToolTip(t("ai_mode.episodes_tip"));
    _ui->learningRateSpin->setToolTip(t("ai_mode.learning_rate_tip"));
    _ui->gammaSpin->setToolTip(t("ai_mode.gamma_tip"));
    _ui->optimizerCombo->setToolTip(t("ai_mode.optimizer_tip"));
    _ui->seedSpin->setToolTip(t("ai_mode.seed_tip"));
    _ui->dqnReplayCapacitySpin->setToolTip(t("ai_mode.dqn_replay_capacity_tip"));
    _ui->dqnBatchSizeSpin->setToolTip(t("ai_mode.dqn_batch_size_tip"));
    _ui->dqnWarmupSizeSpin->setToolTip(t("ai_mode.dqn_warmup_size_tip"));
    _ui->dqnUpdatePeriodSpin->setToolTip(t("ai_mode.dqn_update_period_tip"));
    _ui->dqnTargetSyncPeriodSpin->setToolTip(t("ai_mode.dqn_target_sync_period_tip"));
    _ui->dqnEpsilonStartSpin->setToolTip(t("ai_mode.dqn_epsilon_start_tip"));
    _ui->dqnEpsilonEndSpin->setToolTip(t("ai_mode.dqn_epsilon_end_tip"));
    _ui->dqnEpsilonDecaySpin->setToolTip(t("ai_mode.dqn_epsilon_decay_tip"));
    _ui->openRunFolderButton->setToolTip(t("ai_mode.open_run_folder_tip"));
    _ui->launchTrainingButton->setToolTip(t("ai_mode.launch_training_tip"));
    _ui->stopTrainingButton->setToolTip(t("ai_mode.stop_training_tip"));
    setPreviewAvailable(_ui->previewButton->isEnabled());
    _ui->runCombo->setToolTip(t("ai_mode.run_tip"));
    _ui->repetitionsSpin->setToolTip(t("ai_mode.repetitions_tip"));
    _ui->evalModelEdit->setToolTip(t("ai_mode.eval_model_tip"));
    _ui->browseModelButton->setToolTip(t("ai_mode.eval_model_tip"));
    _ui->evalLevelCombo->setToolTip(t("ai_mode.eval_level_tip"));
    _ui->maxStepsSpin->setToolTip(t("ai_mode.max_steps_tip"));
    _ui->evalSeedSpin->setToolTip(t("ai_mode.eval_seed_tip"));
    _ui->decodingCombo->setToolTip(t("ai_mode.decoding_tip"));
    _ui->stopEvaluationButton->setToolTip(t("ai_mode.stop_evaluation_tip"));
    _ui->exportReportButton->setToolTip(t("ai_mode.export_report_tip"));
    _ui->reuseRunSettingsButton->setToolTip(t("ai_mode.reuse_run_settings_tip"));
    _ui->evaluateButton->setToolTip(t("ai_mode.evaluate_tip"));
    _ui->saveModelButton->setToolTip(t("ai_mode.save_model_tip"));
    _ui->exportReplayButton->setToolTip(t("ai_mode.export_replay_tip"));

    // L'etiquette d'etat n'est reinitialisee que hors entrainement : ecraser un message de
    // progression au changement de langue ferait perdre l'information a l'ecran.
    if (_worker == nullptr) {
        _ui->trainingStatusLabel->setText(t("ai_mode.status_idle"));
    }
}

void AiModeScreen::focusDefaultAction() {
    _ui->backButton->setFocus();
}

void AiModeScreen::refreshLevelList() {
    _ui->levelCombo->clear();
    _ui->evalLevelCombo->clear();
    std::error_code error;
    if (!std::filesystem::exists(levelsDir(), error)) {
        return;
    }
    for (const auto& entry : std::filesystem::directory_iterator(levelsDir(), error)) {
        if (entry.path().extension() == ".json") {
            const QString name = QString::fromStdString(entry.path().filename().string());
            const QString path = QString::fromStdString(entry.path().string());
            _ui->levelCombo->addItem(name, path);
            // Meme catalogue pour l'evaluation : un modele peut etre mesure sur un autre niveau
            // que celui qui l'a entraine (generalisation), comme `evaluate --level` le permet.
            _ui->evalLevelCombo->addItem(name, path);
        }
    }
    _ui->levelCombo->model()->sort(0);
    _ui->evalLevelCombo->model()->sort(0);
}

std::filesystem::path AiModeScreen::selectedRunsRoot() const {
    // Le dossier choisi par l'utilisateur, pas le dossier fige a cote de l'executable : `runsRoot`
    // pilotait l'ECRITURE des runs sans piloter leur LECTURE, si bien qu'un run ecrit ailleurs
    // n'apparaissait jamais dans l'onglet Validation.
    const QString chosen = _ui->runsRootEdit->text().trimmed();
    return chosen.isEmpty() ? runsRootDir() : std::filesystem::path(chosen.toStdString());
}

void AiModeScreen::refreshRunsAndReplays() {
    _ui->runCombo->clear();
    std::error_code error;
    const std::filesystem::path runsRoot = selectedRunsRoot();
    if (std::filesystem::exists(runsRoot, error)) {
        // Une entree par run : runsRoot/<niveau>/<runId>/model.bin (LOT-ANNEXE-09 TACHE-04).
        for (const auto& levelDir : std::filesystem::directory_iterator(runsRoot, error)) {
            if (!levelDir.is_directory()) {
                continue;
            }
            for (const auto& runDir : std::filesystem::directory_iterator(levelDir.path(), error)) {
                const std::filesystem::path modelPath = runDir.path() / "model.bin";
                if (!std::filesystem::exists(modelPath, error)) {
                    continue;
                }
                // Libelle enrichi : le seul couple niveau/identifiant ne disait pas quel
                // algorithme avait produit le modele, alors que c'est ce qui determine comment il
                // se recharge. Relu dans le config.json du run, comme partout ailleurs.
                const std::string algorithmId =
                    aisolver::cli::loadTrainingConfig(runDir.path() / "config.json",
                                                      aisolver::cli::CommandLineOverrides{})
                        .algorithmId;
                const QString label =
                    QString::fromStdString(levelDir.path().filename().string() + " · " +
                                           runDir.path().filename().string() + " · " + algorithmId);
                _ui->runCombo->addItem(label, QString::fromStdString(runDir.path().string()));
            }
        }
    }
    _ui->saveModelButton->setEnabled(_ui->runCombo->currentData().isValid());
    _ui->exportReplayButton->setEnabled(_ui->runCombo->currentData().isValid());

    _ui->replayTable->setRowCount(0);
    if (std::filesystem::exists(replaysDir(), error)) {
        for (const auto& entry : std::filesystem::directory_iterator(replaysDir(), error)) {
            if (entry.path().extension() != ".json") {
                continue;
            }
            const aisolver::ReplayLoadResult loaded = aisolver::readReplay(entry.path());
            if (!loaded.ok()) {
                continue;
            }
            const int row = _ui->replayTable->rowCount();
            _ui->replayTable->insertRow(row);
            _ui->replayTable->setItem(
                row, 0, new QTableWidgetItem(QString::fromStdString(loaded.replay->levelPath)));
            _ui->replayTable->setItem(
                row, 1, new QTableWidgetItem(QString::fromStdString(loaded.replay->algorithmId)));
            _ui->replayTable->setItem(
                row, 2, new QTableWidgetItem(QString::number(loaded.replay->finalReward, 'f', 2)));
            QTableWidgetItem* pathItem =
                new QTableWidgetItem(QString::fromStdString(entry.path().filename().string()));
            pathItem->setData(Qt::UserRole, QString::fromStdString(entry.path().string()));
            _ui->replayTable->setItem(row, 3, pathItem);
        }
    }
    _ui->launchReplayButton->setEnabled(false);
}

bool AiModeScreen::trainingActive() const noexcept {
    return _worker != nullptr;
}

void AiModeScreen::stopTrainingIfActive() {
    if (_worker) {
        _worker->requestStop();
    }
}

bool AiModeScreen::evaluationActive() const noexcept {
    return _evaluationWorker != nullptr;
}

void AiModeScreen::stopEvaluationIfActive() {
    if (_evaluationWorker != nullptr) {
        _evaluationWorker->requestStop();
    }
}

// Correspondance champ <-> hyperparametre, ecrite UNE fois. Le lancement d'un run, les presets et
// la reprise des reglages d'un run passe la traversent tous les trois : trois recopies separees
// divergeraient au premier hyperparametre ajoute.
aisolver::cli::TrainingConfig AiModeScreen::configFromForm() const {
    aisolver::cli::TrainingConfig config;
    config.algorithmId = selectedAlgo().toStdString();
    config.hiddenSize = static_cast<std::size_t>(_ui->hiddenSizeSpin->value());
    config.evolutionary.populationSize = static_cast<std::size_t>(_ui->populationSpin->value());
    config.evolutionary.tournamentSize = _ui->tournamentSizeSpin->value();
    config.evolutionary.mutationRate = static_cast<float>(_ui->mutationRateSpin->value());
    config.evolutionary.mutationStrength = static_cast<float>(_ui->mutationStrengthSpin->value());
    config.stopping.maxGenerations = _ui->maxGenerationsSpin->value();
    config.stopping.requiredConsecutiveSuccesses = _ui->requiredSuccessesSpin->value();
    config.episodes = static_cast<std::size_t>(_ui->episodesSpin->value());
    config.learningRate = static_cast<float>(_ui->learningRateSpin->value());
    config.gamma = static_cast<float>(_ui->gammaSpin->value());
    config.optimizer = _ui->optimizerCombo->currentText().toStdString();
    config.dqnReplayCapacity = static_cast<std::size_t>(_ui->dqnReplayCapacitySpin->value());
    config.dqnBatchSize = static_cast<std::size_t>(_ui->dqnBatchSizeSpin->value());
    config.dqnWarmupSize = static_cast<std::size_t>(_ui->dqnWarmupSizeSpin->value());
    config.dqnUpdatePeriodSteps = static_cast<std::size_t>(_ui->dqnUpdatePeriodSpin->value());
    config.dqnTargetSyncPeriodSteps =
        static_cast<std::size_t>(_ui->dqnTargetSyncPeriodSpin->value());
    config.dqnEpsilonStart = static_cast<float>(_ui->dqnEpsilonStartSpin->value());
    config.dqnEpsilonEnd = static_cast<float>(_ui->dqnEpsilonEndSpin->value());
    config.dqnEpsilonDecaySteps = static_cast<std::size_t>(_ui->dqnEpsilonDecaySpin->value());
    config.criticLearningRate = static_cast<float>(_ui->criticLearningRateSpin->value());
    config.evolutionary.crossoverRate = static_cast<float>(_ui->crossoverRateSpin->value());
    config.tuning.batchEpisodes = static_cast<std::size_t>(_ui->batchEpisodesSpin->value());
    config.tuning.entropyCoefficient = static_cast<float>(_ui->entropySpin->value());
    config.tuning.explorationFloor = static_cast<float>(_ui->explorationFloorSpin->value());
    config.tuning.gradientClipNorm = static_cast<float>(_ui->gradientClipNormSpin->value());
    config.tuning.actionRepeat = _ui->actionRepeatSpin->value();
    config.maxSteps = _ui->stepBudgetSpin->value();
    config.stuckThreshold = _ui->stuckThresholdSpin->value();
    return config;
}

void AiModeScreen::applyConfigToForm(const aisolver::cli::TrainingConfig& config) {
    if (config.algorithmId == "pg") {
        _ui->reinforceAlgorithmRadio->setChecked(true);
    } else if (config.algorithmId == "ac") {
        _ui->actorCriticAlgorithmRadio->setChecked(true);
    } else if (config.algorithmId == "avance") {
        _ui->advancedAlgorithmRadio->setChecked(true);
    } else {
        _ui->evolutionaryAlgorithmRadio->setChecked(true);
    }
    _ui->hiddenSizeSpin->setValue(static_cast<int>(config.hiddenSize));
    _ui->populationSpin->setValue(static_cast<int>(config.evolutionary.populationSize));
    _ui->tournamentSizeSpin->setValue(config.evolutionary.tournamentSize);
    _ui->mutationRateSpin->setValue(config.evolutionary.mutationRate);
    _ui->mutationStrengthSpin->setValue(config.evolutionary.mutationStrength);
    _ui->maxGenerationsSpin->setValue(config.stopping.maxGenerations);
    _ui->requiredSuccessesSpin->setValue(config.stopping.requiredConsecutiveSuccesses);
    _ui->episodesSpin->setValue(static_cast<int>(config.episodes));
    _ui->learningRateSpin->setValue(config.learningRate);
    _ui->gammaSpin->setValue(config.gamma);
    _ui->optimizerCombo->setCurrentText(QString::fromStdString(config.optimizer));
    _ui->dqnReplayCapacitySpin->setValue(static_cast<int>(config.dqnReplayCapacity));
    _ui->dqnBatchSizeSpin->setValue(static_cast<int>(config.dqnBatchSize));
    _ui->dqnWarmupSizeSpin->setValue(static_cast<int>(config.dqnWarmupSize));
    _ui->dqnUpdatePeriodSpin->setValue(static_cast<int>(config.dqnUpdatePeriodSteps));
    _ui->dqnTargetSyncPeriodSpin->setValue(static_cast<int>(config.dqnTargetSyncPeriodSteps));
    _ui->dqnEpsilonStartSpin->setValue(config.dqnEpsilonStart);
    _ui->dqnEpsilonEndSpin->setValue(config.dqnEpsilonEnd);
    _ui->dqnEpsilonDecaySpin->setValue(static_cast<int>(config.dqnEpsilonDecaySteps));
    _ui->criticLearningRateSpin->setValue(config.criticLearningRate);
    _ui->crossoverRateSpin->setValue(config.evolutionary.crossoverRate);
    _ui->batchEpisodesSpin->setValue(static_cast<int>(config.tuning.batchEpisodes));
    _ui->entropySpin->setValue(config.tuning.entropyCoefficient);
    _ui->explorationFloorSpin->setValue(config.tuning.explorationFloor);
    _ui->gradientClipNormSpin->setValue(config.tuning.gradientClipNorm);
    _ui->actionRepeatSpin->setValue(config.tuning.actionRepeat);
    _ui->stepBudgetSpin->setValue(config.maxSteps);
    _ui->stuckThresholdSpin->setValue(config.stuckThreshold);
    updateFieldVisibility();
}

void AiModeScreen::loadConfigFile(const QString& path) {
    if (path.isEmpty()) {
        return;
    }
    // Existence verifiee AVANT d'appliquer quoi que ce soit : `loadTrainingConfig` part des defauts
    // documentes et n'ecrase que les cles presentes, ce qui est la bonne regle pour un fichier
    // PARTIEL -- mais transforme un fichier absent ou illisible en « remise a zero de tout le
    // formulaire », silencieuse, alors que l'utilisateur croyait charger des reglages.
    std::error_code error;
    const std::filesystem::path configPath(path.toStdString());
    if (!std::filesystem::exists(configPath, error) || error) {
        _ui->trainingStatusLabel->setText(text("ai_mode.config_load_failed"));
        return;
    }
    applyConfigToForm(
        aisolver::cli::loadTrainingConfig(configPath, aisolver::cli::CommandLineOverrides{}));
}

void AiModeScreen::onLoadConfig() {
    loadConfigFile(QFileDialog::getOpenFileName(this, text("ai_mode.load_config_title"),
                                                QString::fromStdString(selectedRunsRoot().string()),
                                                text("ai_mode.config_filter")));
}

void AiModeScreen::onSaveConfig() {
    const QString destination =
        QFileDialog::getSaveFileName(this, text("ai_mode.save_config_title"),
                                     QStringLiteral("config.json"), text("ai_mode.config_filter"));
    if (destination.isEmpty()) {
        return;
    }
    const bool written = aisolver::cli::writeTrainingConfigJson(
        configFromForm(), std::filesystem::path(destination.toStdString()));
    _ui->trainingStatusLabel->setText(written ? text("ai_mode.config_saved")
                                              : text("ai_mode.config_save_failed"));
}

void AiModeScreen::onResetDefaults() {
    // Les defauts sont ceux du code partage avec la CLI, jamais une seconde liste de valeurs
    // inscrite dans l'ecran : l'ecran au demarrage doit decrire le meme run que
    // `aisolver-cli train` sans option.
    applyConfigToForm(aisolver::cli::TrainingConfig{});
}

void AiModeScreen::onBrowseRunsRoot() {
    const QString chosen = QFileDialog::getExistingDirectory(this, text("ai_mode.runs_root_title"),
                                                             _ui->runsRootEdit->text());
    if (!chosen.isEmpty()) {
        _ui->runsRootEdit->setText(chosen);
    }
}

QString AiModeScreen::selectedAlgo() const {
    if (_ui->reinforceAlgorithmRadio->isChecked()) {
        return QStringLiteral("pg");
    }
    if (_ui->actorCriticAlgorithmRadio->isChecked()) {
        return QStringLiteral("ac");
    }
    if (_ui->advancedAlgorithmRadio->isChecked()) {
        return QStringLiteral("avance");
    }
    return QStringLiteral("evo");
}

void AiModeScreen::updateFieldVisibility() {
    const QString algo = selectedAlgo();
    _ui->evolutionaryGroup->setVisible(algo == "evo");
    // Gradient (episodes/learning rate/gamma/optimizer) sert aux trois algorithmes par gradient,
    // DQN inclus -- DQN a en plus son propre groupe d'hyperparametres.
    _ui->gradientGroup->setVisible(algo != "evo");
    _ui->dqnGroup->setVisible(algo == "avance");
}

void AiModeScreen::setTrainingControlsEnabled(bool enabled) {
    _ui->levelCombo->setEnabled(enabled);
    _ui->evolutionaryAlgorithmRadio->setEnabled(enabled);
    _ui->reinforceAlgorithmRadio->setEnabled(enabled);
    _ui->actorCriticAlgorithmRadio->setEnabled(enabled);
    _ui->advancedAlgorithmRadio->setEnabled(enabled);
    _ui->hiddenSizeSpin->setEnabled(enabled);
    _ui->runsRootEdit->setEnabled(enabled);
    _ui->browseRunsRootButton->setEnabled(enabled);
    _ui->populationSpin->setEnabled(enabled);
    _ui->tournamentSizeSpin->setEnabled(enabled);
    _ui->mutationRateSpin->setEnabled(enabled);
    _ui->mutationStrengthSpin->setEnabled(enabled);
    _ui->maxGenerationsSpin->setEnabled(enabled);
    _ui->requiredSuccessesSpin->setEnabled(enabled);
    _ui->loadConfigButton->setEnabled(enabled);
    _ui->saveConfigButton->setEnabled(enabled);
    _ui->resetDefaultsButton->setEnabled(enabled);
    _ui->episodesSpin->setEnabled(enabled);
    _ui->learningRateSpin->setEnabled(enabled);
    _ui->gammaSpin->setEnabled(enabled);
    _ui->optimizerCombo->setEnabled(enabled);
    _ui->criticLearningRateSpin->setEnabled(enabled);
    _ui->crossoverRateSpin->setEnabled(enabled);
    _ui->batchEpisodesSpin->setEnabled(enabled);
    _ui->entropySpin->setEnabled(enabled);
    _ui->explorationFloorSpin->setEnabled(enabled);
    _ui->gradientClipNormSpin->setEnabled(enabled);
    _ui->actionRepeatSpin->setEnabled(enabled);
    _ui->stepBudgetSpin->setEnabled(enabled);
    _ui->stuckThresholdSpin->setEnabled(enabled);
    _ui->seedSpin->setEnabled(enabled);
    _ui->dqnReplayCapacitySpin->setEnabled(enabled);
    _ui->dqnBatchSizeSpin->setEnabled(enabled);
    _ui->dqnWarmupSizeSpin->setEnabled(enabled);
    _ui->dqnUpdatePeriodSpin->setEnabled(enabled);
    _ui->dqnTargetSyncPeriodSpin->setEnabled(enabled);
    _ui->dqnEpsilonStartSpin->setEnabled(enabled);
    _ui->dqnEpsilonEndSpin->setEnabled(enabled);
    _ui->dqnEpsilonDecaySpin->setEnabled(enabled);
    _ui->launchTrainingButton->setVisible(enabled);
    _ui->stopTrainingButton->setVisible(!enabled);
}

void AiModeScreen::setPreviewAvailable(bool available) {
    // L'etat ET son explication sont poses ENSEMBLE : un bouton grise sans raison affichee
    // laisse croire a une panne. Ici l'attente est normale -- le premier apercu n'est ecrit
    // qu'apres quelques secondes d'entrainement -- et le dire coute une infobulle.
    _ui->previewButton->setEnabled(available);
    _ui->generationCombo->setEnabled(available);
    _ui->previewButton->setToolTip(available ? text("ai_mode.preview_tip")
                                             : text("ai_mode.preview_tip_waiting"));
    _ui->generationCombo->setToolTip(available ? text("ai_mode.generation_tip")
                                               : text("ai_mode.preview_tip_waiting"));
}

void AiModeScreen::onLaunchTraining() {
    if (_worker || _ui->levelCombo->currentData().isNull()) {
        return;
    }

    const aisolver::cli::TrainingConfig form = configFromForm();

    TrainingRequest request;
    // Conversion a la FRONTIERE : la requete est de la donnee pure (compilee dans UnitTests, que
    // le job sanitize construit sans Qt), c'est donc a l'ecran de traduire ce que Qt lui donne.
    request.levelPath = _ui->levelCombo->currentData().toString().toStdString();
    request.algorithmId = selectedAlgo().toStdString();
    request.seed = static_cast<std::uint64_t>(_ui->seedSpin->value());
    request.runsRoot = _ui->runsRootEdit->text().toStdString();
    request.hiddenSize = form.hiddenSize;
    request.populationSize = form.evolutionary.populationSize;
    request.tournamentSize = form.evolutionary.tournamentSize;
    request.mutationRate = form.evolutionary.mutationRate;
    request.mutationStrength = form.evolutionary.mutationStrength;
    request.maxGenerations = form.stopping.maxGenerations;
    request.requiredConsecutiveSuccesses = form.stopping.requiredConsecutiveSuccesses;
    request.episodes = form.episodes;
    request.learningRate = form.learningRate;
    request.gamma = form.gamma;
    request.optimizer = _ui->optimizerCombo->currentText().toStdString();
    // Reglages communs a toutes les familles d'algorithmes. Ils etaient lus par configFromForm()
    // puis JETES : TrainingRequest n'avait pas de champ pour les recevoir, si bien que les regler
    // ne changeait rien -- et que le config.json du run decrivait un run qui n'avait pas tourne,
    // d'ou "Reprendre les reglages de ce run" rechargeait des valeurs fausses. `IHM superset CLI`
    // (LOT-ANNEXE-22) vaut dans les deux sens : un reglage expose atteint le moteur, ou ne
    // s'expose pas (EX-IHM-083).
    request.criticLearningRate = form.criticLearningRate;
    request.crossoverRate = form.evolutionary.crossoverRate;
    request.batchEpisodes = form.tuning.batchEpisodes;
    request.entropyCoefficient = form.tuning.entropyCoefficient;
    request.explorationFloor = form.tuning.explorationFloor;
    request.gradientClipNorm = form.tuning.gradientClipNorm;
    request.actionRepeat = form.tuning.actionRepeat;
    request.maxSteps = form.maxSteps;
    request.stuckThreshold = form.stuckThreshold;
    if (request.algorithmId == "avance") {
        request.dqnReplayCapacity = form.dqnReplayCapacity;
        request.dqnBatchSize = form.dqnBatchSize;
        request.dqnWarmupSize = form.dqnWarmupSize;
        request.dqnUpdatePeriodSteps = form.dqnUpdatePeriodSteps;
        request.dqnTargetSyncPeriodSteps = form.dqnTargetSyncPeriodSteps;
        request.dqnEpsilonStart = form.dqnEpsilonStart;
        request.dqnEpsilonEnd = form.dqnEpsilonEnd;
        request.dqnEpsilonDecaySteps = form.dqnEpsilonDecaySteps;
    }

    _ui->statsTable->setRowCount(0);
    _ui->trainingChart->clearChart();
    setPreviewAvailable(false);
    _ui->generationCombo->clear();
    _ui->openRunFolderButton->setEnabled(false);
    _ui->trainingRunFolderValue->setText(QString());
    // Les deux familles bornent leur run par un champ different de l'onglet : plafond de
    // generations pour l'evolutionniste, budget d'episodes pour les algorithmes par gradient. Un
    // run peut toujours s'arreter avant (resolution, interruption), auquel cas la barre n'atteint
    // simplement jamais son maximum -- elle mesure le budget consomme, pas l'avancement vers une
    // solution.
    _ui->trainingProgressBar->setRange(0, request.algorithmId == "evo"
                                              ? _ui->maxGenerationsSpin->value()
                                              : _ui->episodesSpin->value());
    _ui->trainingProgressBar->setValue(0);
    _ui->trainingEtaLabel->setText(QString());
    _ui->stabilityValue->setText(QStringLiteral("—"));
    _ui->dqnEpsilonCurrentValue->setText(QStringLiteral("—"));
    _trainingClock.start();
    setTrainingControlsEnabled(false);
    _ui->trainingStatusLabel->setText(text("ai_mode.status_running"));

    _workerThread = std::make_unique<QThread>();
    _worker = new TrainingWorker(std::move(request));
    _worker->moveToThread(_workerThread.get());
    connect(_workerThread.get(), &QThread::started, _worker, &TrainingWorker::run);
    connect(_worker, &TrainingWorker::progress, this, &AiModeScreen::onTrainingProgress);
    connect(_worker, &TrainingWorker::previewReady, this, &AiModeScreen::onTrainingPreviewReady);
    connect(_worker, &TrainingWorker::finished, this, &AiModeScreen::onTrainingFinished);
    connect(_worker, &TrainingWorker::failed, this, &AiModeScreen::onTrainingFailed);
    _workerThread->start();
}

void AiModeScreen::onStopTraining() {
    stopTrainingIfActive();
    _ui->trainingStatusLabel->setText(text("ai_mode.status_stopping"));
}

void AiModeScreen::onTrainingProgress(hmi::TrainingProgress step) {
    // Toutes les colonnes du `stats.csv` du run, pas une selection : c'est la meme information,
    // affichee au moment ou elle est produite plutot que relue apres coup dans le fichier.
    //
    // Fenetre glissante : un run de plusieurs milliers d'episodes allouait autant de fois huit
    // cellules, sans que personne ne fasse jamais defiler jusqu'aux premieres -- le CSV du run
    // garde l'historique complet, ce tableau n'a qu'a montrer ce qui vient de se passer.
    while (_ui->statsTable->rowCount() >= MAX_STATS_ROWS) {
        _ui->statsTable->removeRow(0);
    }
    const int row = _ui->statsTable->rowCount();
    _ui->statsTable->insertRow(row);
    _ui->statsTable->setItem(row, 0, new QTableWidgetItem(QString::number(step.index)));
    _ui->statsTable->setItem(row, 1,
                             new QTableWidgetItem(QString::number(step.bestReward, 'f', 3)));
    _ui->statsTable->setItem(row, 2,
                             new QTableWidgetItem(QString::number(step.meanReward, 'f', 3)));
    _ui->statsTable->setItem(row, 3,
                             new QTableWidgetItem(QString::number(step.worstReward, 'f', 3)));
    _ui->statsTable->setItem(row, 4,
                             new QTableWidgetItem(QString::number(step.rewardStdDev, 'f', 3)));
    _ui->statsTable->setItem(row, 5, new QTableWidgetItem(QString::number(step.bestStepCount)));
    _ui->statsTable->setItem(
        row, 6, new QTableWidgetItem(QString::number(step.successRate * 100.0, 'f', 1) + "%"));
    _ui->statsTable->setItem(
        row, 7, new QTableWidgetItem(QString::number(static_cast<qulonglong>(step.seed))));
    _ui->statsTable->scrollToBottom();
    _ui->trainingStatusLabel->setText(text("ai_mode.status_progress")
                                          .arg(step.index)
                                          .arg(QString::number(step.bestReward, 'f', 3)));
    _ui->trainingChart->addPoint(step.index, step.bestReward, step.meanReward,
                                 step.movingAverageReward, step.successRate);

    // Une etape journalisee d'index N signifie N+1 etapes consommees : la barre et l'estimation
    // comptent un budget consomme, pas un numero d'etape.
    const int completedSteps = step.index + 1;
    _ui->trainingProgressBar->setValue(completedSteps);
    updateEta(completedSteps);

    if (step.consecutiveStable.has_value() && step.requiredStable.has_value()) {
        _ui->stabilityValue->setText(
            QStringLiteral("%1 / %2").arg(*step.consecutiveStable).arg(*step.requiredStable));
    }
    if (step.epsilon.has_value()) {
        _ui->dqnEpsilonCurrentValue->setText(QString::number(*step.epsilon, 'f', 3));
    }
}

void AiModeScreen::updateEta(int completedSteps) {
    const int total = _ui->trainingProgressBar->maximum();
    if (completedSteps <= 0 || completedSteps >= total) {
        _ui->trainingEtaLabel->setText(QString());
        return;
    }
    // Duree moyenne par etape depuis le debut du run plutot qu'un ecart instantane : le cout
    // d'une generation varie fortement (un episode qui echoue tot est bien plus rapide qu'un
    // episode qui va au bout), et une moyenne cumulee ne fait pas osciller l'affichage.
    const qint64 elapsedMs = _trainingClock.elapsed();
    // Multiplication AVANT division : `elapsedMs / completedSteps` est une division entiere, nulle
    // des qu'une etape dure moins d'une milliseconde -- l'estimation affichait alors « 0 min 00 s »
    // pendant tout le run.
    const qint64 remainingMs = elapsedMs * (total - completedSteps) / completedSteps;
    const qint64 remainingSeconds = remainingMs / 1000;
    _ui->trainingEtaLabel->setText(text("ai_mode.eta")
                                       .arg(remainingSeconds / 60)
                                       .arg(remainingSeconds % 60, 2, 10, QLatin1Char('0')));
}

void AiModeScreen::onTrainingPreviewReady(QString replayPath, QString /*algorithmId*/,
                                          QString /*levelPath*/, int generation) {
    // Un aperçu par génération/épisode, jamais écrasé (voir TrainingWorker::run) : on suit la
    // dernière génération reçue tant que l'utilisateur n'a pas sélectionné une génération
    // antérieure pour l'examiner pendant que l'entraînement continue.
    const bool wasFollowingLatest =
        _ui->generationCombo->count() == 0 ||
        _ui->generationCombo->currentIndex() == _ui->generationCombo->count() - 1;
    _ui->generationCombo->addItem(text("ai_mode.generation_item").arg(generation), replayPath);
    if (wasFollowingLatest) {
        _ui->generationCombo->setCurrentIndex(_ui->generationCombo->count() - 1);
    }
    setPreviewAvailable(true);
}

// Seul `solved` guide le message affiche ; les autres chemins decrivent le run qui vient de
// finir et servent a `_lastRunDir` (bouton "Ouvrir le dossier du run") -- l'ecran continue par
// ailleurs de relire les runs anterieurs depuis `runCombo` apres `refreshRunsAndReplays()`.
void AiModeScreen::onTrainingFinished(bool solved, QString modelPath, QString /*statsPath*/,
                                      QString /*configPath*/, QString /*replayPath*/,
                                      bool /*replayExported*/, int generationsRun) {
    _ui->trainingStatusLabel->setText(
        (solved ? text("ai_mode.status_done_solved") : text("ai_mode.status_done_unsolved"))
            .arg(generationsRun));
    _ui->trainingEtaLabel->setText(QString());
    _lastRunDir = QString::fromStdString(
        std::filesystem::path(modelPath.toStdString()).parent_path().string());
    _ui->trainingRunFolderValue->setText(_lastRunDir);
    _ui->openRunFolderButton->setEnabled(!_lastRunDir.isEmpty());
    teardownWorker();
    setTrainingControlsEnabled(true);
    refreshRunsAndReplays();
}

void AiModeScreen::onOpenRunFolder() {
    if (_lastRunDir.isEmpty()) {
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(_lastRunDir));
}

void AiModeScreen::onTrainingFailed(QString messageKey, QString detail) {
    // Le worker ne connait pas le catalogue : il nomme l'echec, l'ecran le traduit.
    _ui->trainingStatusLabel->setText(
        text("ai_mode.status_error").arg(text(messageKey.toUtf8().constData()).arg(detail)));
    teardownWorker();
    setTrainingControlsEnabled(true);
}

void AiModeScreen::teardownWorker() {
    if (!_workerThread) {
        return;
    }
    if (_worker) {
        _worker->requestStop();
    }
    _workerThread->quit();
    _workerThread->wait();
    // `delete` direct, jamais `deleteLater()` : la boucle d'evenements du fil vient de se terminer
    // et ne traitera plus aucun evenement differe -- l'objet fuyait, un par run. Le fil est arrete
    // et joint, plus personne ne touche au worker.
    delete _worker;
    _worker = nullptr;
    _workerThread.reset();
}

QString AiModeScreen::algorithmOfModel(const QString& modelPath) {
    if (modelPath.isEmpty()) {
        return QString();
    }
    // Algorithme relu dans le `config.json` DU MODELE, jamais dans celui du run selectionne dans la
    // liste : un modele parcouru au disque (« Parcourir... ») n'a aucune raison d'appartenir au run
    // affiche, et l'evaluer sur la mauvaise topologie le refuse ou, pire, le charge en supposant
    // l'evolutionniste par defaut.
    const std::filesystem::path runDirectory =
        std::filesystem::path(modelPath.toStdString()).parent_path();
    return QString::fromStdString(
        aisolver::cli::loadTrainingConfig(runDirectory / "config.json",
                                          aisolver::cli::CommandLineOverrides{})
            .algorithmId);
}

std::optional<EvaluationRequest> AiModeScreen::evaluationRequestFromForm() const {
    const QString modelPath = _ui->evalModelEdit->text();
    const QString levelPath = _ui->evalLevelCombo->currentData().toString();
    if (modelPath.isEmpty() || levelPath.isEmpty()) {
        return std::nullopt;
    }
    EvaluationRequest request;
    request.modelPath = modelPath;
    request.levelPath = levelPath;
    request.algorithmId = algorithmOfModel(modelPath);
    request.repetitions = _ui->repetitionsSpin->value();
    request.maxStepsPerEpisode = _ui->maxStepsSpin->value();
    request.seed = static_cast<std::uint64_t>(_ui->evalSeedSpin->value());
    request.stochasticDecoding = _ui->decodingCombo->currentText() == QLatin1String("stochastic");
    return request;
}

void AiModeScreen::setEvaluationControlsEnabled(bool enabled) {
    _ui->runCombo->setEnabled(enabled);
    _ui->evalModelEdit->setEnabled(enabled);
    _ui->browseModelButton->setEnabled(enabled);
    _ui->evalLevelCombo->setEnabled(enabled);
    _ui->repetitionsSpin->setEnabled(enabled);
    _ui->maxStepsSpin->setEnabled(enabled);
    _ui->evalSeedSpin->setEnabled(enabled);
    _ui->decodingCombo->setEnabled(enabled);
    _ui->evaluateButton->setVisible(enabled);
    _ui->stopEvaluationButton->setVisible(!enabled);
    // Les actions de l'onglet aussi : elles lisent le meme modele et le meme dossier de run que la
    // campagne en cours, et « Exporter le rejeu » rejoue le niveau SUR LE FIL DE L'INTERFACE --
    // cliquer dessus pendant une evaluation figeait la fenetre le temps du rollout.
    _ui->saveModelButton->setEnabled(enabled);
    _ui->exportReportButton->setEnabled(enabled);
    _ui->reuseRunSettingsButton->setEnabled(enabled);
    _ui->exportReplayButton->setEnabled(enabled && !_ui->evalModelEdit->text().isEmpty());
}

// Sur son propre thread, comme l'entrainement : une campagne rejoue le niveau `repetitions` fois,
// chacune jusqu'a `maxStepsPerEpisode` pas -- assez pour figer la fenetre plusieurs minutes si
// elle etait lancee ici.
void AiModeScreen::onEvaluate() {
    if (_evaluationWorker != nullptr) {
        return;
    }
    const std::optional<EvaluationRequest> request = evaluationRequestFromForm();
    if (!request.has_value()) {
        return;
    }

    _lastEvaluation.reset();
    _lastEvaluationRequest = request;
    _ui->exportReportButton->setEnabled(false);
    _ui->evaluationProgressBar->setRange(0, request->repetitions);
    _ui->evaluationProgressBar->setValue(0);
    setEvaluationControlsEnabled(false);

    _evaluationThread = std::make_unique<QThread>();
    _evaluationWorker = new EvaluationWorker(*request);
    _evaluationWorker->moveToThread(_evaluationThread.get());
    connect(_evaluationThread.get(), &QThread::started, _evaluationWorker, &EvaluationWorker::run);
    connect(_evaluationWorker, &EvaluationWorker::progress, this,
            &AiModeScreen::onEvaluationProgress);
    connect(_evaluationWorker, &EvaluationWorker::finished, this,
            &AiModeScreen::onEvaluationFinished);
    connect(_evaluationWorker, &EvaluationWorker::failed, this, &AiModeScreen::onEvaluationFailed);
    _evaluationThread->start();
}

void AiModeScreen::onStopEvaluation() {
    if (_evaluationWorker != nullptr) {
        _evaluationWorker->requestStop();
    }
}

void AiModeScreen::onEvaluationProgress(int completed, int /*total*/) {
    _ui->evaluationProgressBar->setValue(completed);
}

void AiModeScreen::onEvaluationFinished(hmi::EvaluationOutcome outcome) {
    _lastEvaluation = outcome;
    _ui->successRateValue->setText(QString::number(outcome.successRate * 100.0, 'f', 1) + "%");
    _ui->meanStepsValue->setText(QString::number(outcome.meanStepsOnSuccess, 'f', 1));
    _ui->meanStepsAllValue->setText(QString::number(outcome.meanStepsAll, 'f', 1));
    _ui->varianceValue->setText(QString::number(outcome.stepVariance, 'f', 2));
    _ui->repetitionsRunValue->setText(QString::number(outcome.repetitionsRun));
    _ui->exportReportButton->setEnabled(true);
    teardownEvaluationWorker();
    setEvaluationControlsEnabled(true);
}

void AiModeScreen::onEvaluationFailed() {
    teardownEvaluationWorker();
    setEvaluationControlsEnabled(true);
    QMessageBox::warning(this, text("ai_mode.eval_title"), text("ai_mode.eval_failed"));
}

void AiModeScreen::teardownEvaluationWorker() {
    if (!_evaluationThread) {
        return;
    }
    if (_evaluationWorker != nullptr) {
        _evaluationWorker->requestStop();
    }
    _evaluationThread->quit();
    _evaluationThread->wait();
    // Meme raison que `teardownWorker` : la boucle d'evenements est terminee, un `deleteLater()`
    // ne serait jamais traite.
    delete _evaluationWorker;
    _evaluationWorker = nullptr;
    _evaluationThread.reset();
}

void AiModeScreen::onBrowseModel() {
    const QString chosen = QFileDialog::getOpenFileName(
        this, text("ai_mode.browse_model_title"),
        QString::fromStdString(selectedRunsRoot().string()), text("ai_mode.model_filter"));
    if (!chosen.isEmpty()) {
        _ui->evalModelEdit->setText(chosen);
    }
}

void AiModeScreen::onExportReport() {
    if (!_lastEvaluation.has_value() || !_lastEvaluationRequest.has_value()) {
        return;
    }
    const QString destination =
        QFileDialog::getSaveFileName(this, text("ai_mode.export_report_title"),
                                     QStringLiteral("benchmark.csv"), text("ai_mode.csv_filter"));
    if (destination.isEmpty()) {
        return;
    }
    // Meme rapport que `aisolver-cli evaluate --report`. La mise en forme vit dans HMI/Ai, seul
    // autorise a referencer AiSolver/Eval (amendement de LOT-ANNEXE-18).
    const bool written =
        writeEvaluationReport(*_lastEvaluationRequest, *_lastEvaluation, destination);
    _ui->saveStatusLabel->setText(written ? text("ai_mode.report_exported")
                                          : text("ai_mode.report_export_failed"));
}

void AiModeScreen::onReuseRunSettings() {
    const QString runDir = _ui->runCombo->currentData().toString();
    if (runDir.isEmpty()) {
        return;
    }
    loadConfigFile(QString::fromStdString(
        (std::filesystem::path(runDir.toStdString()) / "config.json").string()));
    _ui->tabs->setCurrentIndex(0);
}

void AiModeScreen::onSaveModel() {
    const QString runDir = _ui->runCombo->currentData().toString();
    if (runDir.isEmpty()) {
        return;
    }
    const std::filesystem::path source = std::filesystem::path(runDir.toStdString()) / "model.bin";
    const QString destination =
        QFileDialog::getSaveFileName(this, text("ai_mode.save_dialog_title"),
                                     QStringLiteral("model.bin"), text("ai_mode.save_filter"));
    if (destination.isEmpty()) {
        return;
    }
    std::error_code error;
    std::filesystem::copy_file(source, destination.toStdString(),
                               std::filesystem::copy_options::overwrite_existing, error);
    _ui->saveStatusLabel->setText(error ? text("ai_mode.save_failed") : text("ai_mode.save_ok"));
}

// Vrai export-replay (rejeu argmax du modele), pas une copie du replay.json du run : celle-ci ne
// pouvait rien produire pour un niveau autre que celui du run, ni pour un run dont le fichier
// manque -- alors que le modele sauvegarde reste rejouable. Meme refus qu'en ligne de commande
// pour un modele qui ne resout pas le niveau (decision de cadrage de LOT-ANNEXE-11).
void AiModeScreen::onExportReplay() {
    const std::optional<EvaluationRequest> request = evaluationRequestFromForm();
    if (!request.has_value()) {
        return;
    }
    std::error_code error;
    std::filesystem::create_directories(replaysDir(), error);
    const std::string defaultName =
        std::filesystem::path(request->levelPath.toStdString()).stem().string() + "_" +
        request->algorithmId.toStdString() + ".json";
    const QString destination =
        QFileDialog::getSaveFileName(this, text("ai_mode.export_replay_title"),
                                     QString::fromStdString((replaysDir() / defaultName).string()),
                                     text("ai_mode.replay_filter"));
    if (destination.isEmpty()) {
        return;
    }

    switch (exportModelReplay(*request, destination)) {
        case ReplayExportOutcome::Exported:
            _ui->saveStatusLabel->setText(text("ai_mode.export_ok"));
            break;
        case ReplayExportOutcome::NotSolved:
            _ui->saveStatusLabel->setText(text("ai_mode.export_not_solved"));
            break;
        case ReplayExportOutcome::Failed:
            _ui->saveStatusLabel->setText(text("ai_mode.export_failed"));
            break;
    }
    refreshRunsAndReplays();
}

void AiModeScreen::onLaunchReplay() {
    const QList<QTableWidgetItem*> selected = _ui->replayTable->selectedItems();
    if (selected.isEmpty()) {
        return;
    }
    const int row = selected.first()->row();
    QTableWidgetItem* pathItem = _ui->replayTable->item(row, 3);
    if (!pathItem) {
        return;
    }
    emit replayRequested(pathItem->data(Qt::UserRole).toString());
}

}  // namespace hmi
