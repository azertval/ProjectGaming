// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/AiModeScreen.h"

#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <filesystem>
#include <system_error>

#include "AiSolver/Cli/TrainingConfig.h"
#include "AiSolver/Replay/ReplayFile.h"
#include "HMI/Ai/ModelEvaluation.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/PixelFocusCaret.h"
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
    setObjectName(QStringLiteral("AiModeScreen"));  // ciblé par le thème (theme.qss)
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
    connect(_ui->saveModelButton, &QPushButton::clicked, this, &AiModeScreen::onSaveModel);
    connect(_ui->exportReplayButton, &QPushButton::clicked, this, &AiModeScreen::onExportReplay);
    connect(_ui->launchReplayButton, &QPushButton::clicked, this, &AiModeScreen::onLaunchReplay);
    connect(_ui->replayTable, &QTableWidget::itemSelectionChanged, this, [this] {
        _ui->launchReplayButton->setEnabled(!_ui->replayTable->selectedItems().isEmpty());
    });
    connect(_ui->runCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        _ui->saveModelButton->setEnabled(_ui->runCombo->currentData().isValid());
        _ui->exportReplayButton->setEnabled(_ui->runCombo->currentData().isValid());
    });

    setTrainingControlsEnabled(true);
    refreshLevelList();
    refreshRunsAndReplays();
}

AiModeScreen::~AiModeScreen() {
    teardownWorker();
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
    _ui->levelLabel->setText(t("ai_mode.level"));
    _ui->algorithmLabel->setText(t("ai_mode.algorithm"));
    _ui->evolutionaryAlgorithmRadio->setText(t("ai_mode.algo_evo"));
    _ui->reinforceAlgorithmRadio->setText(t("ai_mode.algo_pg"));
    _ui->actorCriticAlgorithmRadio->setText(t("ai_mode.algo_ac"));
    _ui->advancedAlgorithmRadio->setText(t("ai_mode.algo_dqn"));
    _ui->populationLabel->setText(t("ai_mode.population"));
    _ui->mutationRateLabel->setText(t("ai_mode.mutation_rate"));
    _ui->episodesLabel->setText(t("ai_mode.episodes"));
    _ui->learningRateLabel->setText(t("ai_mode.learning_rate"));
    _ui->gammaLabel->setText(t("ai_mode.gamma"));
    _ui->optimizerLabel->setText(t("ai_mode.optimizer"));
    _ui->seedLabel->setText(t("ai_mode.seed"));
    _ui->launchTrainingButton->setText(t("ai_mode.launch_training"));
    _ui->stopTrainingButton->setText(t("ai_mode.stop_training"));
    _ui->previewButton->setText(t("ai_mode.preview"));
    _ui->statsTable->setHorizontalHeaderLabels(
        {t("ai_mode.column_generation"), t("ai_mode.column_best_reward"),
         t("ai_mode.column_mean_reward"), t("ai_mode.column_success_rate")});

    // Onglet Validation & sauvegarde.
    _ui->runLabel->setText(t("ai_mode.run"));
    _ui->repetitionsLabel->setText(t("ai_mode.repetitions"));
    _ui->evaluateButton->setText(t("ai_mode.evaluate"));
    _ui->successRateLabel->setText(t("ai_mode.success_rate"));
    _ui->meanStepsLabel->setText(t("ai_mode.mean_steps"));
    _ui->varianceLabel->setText(t("ai_mode.variance"));
    _ui->saveModelButton->setText(t("ai_mode.save_model"));
    _ui->exportReplayButton->setText(t("ai_mode.export_replay"));

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
    _ui->populationSpin->setToolTip(t("ai_mode.population_tip"));
    _ui->mutationRateSpin->setToolTip(t("ai_mode.mutation_rate_tip"));
    _ui->episodesSpin->setToolTip(t("ai_mode.episodes_tip"));
    _ui->learningRateSpin->setToolTip(t("ai_mode.learning_rate_tip"));
    _ui->gammaSpin->setToolTip(t("ai_mode.gamma_tip"));
    _ui->optimizerCombo->setToolTip(t("ai_mode.optimizer_tip"));
    _ui->seedSpin->setToolTip(t("ai_mode.seed_tip"));
    _ui->launchTrainingButton->setToolTip(t("ai_mode.launch_training_tip"));
    _ui->stopTrainingButton->setToolTip(t("ai_mode.stop_training_tip"));
    _ui->generationCombo->setToolTip(t("ai_mode.generation_tip"));
    _ui->previewButton->setToolTip(t("ai_mode.preview_tip"));
    _ui->runCombo->setToolTip(t("ai_mode.run_tip"));
    _ui->repetitionsSpin->setToolTip(t("ai_mode.repetitions_tip"));
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
    std::error_code error;
    if (!std::filesystem::exists(levelsDir(), error)) {
        return;
    }
    for (const auto& entry : std::filesystem::directory_iterator(levelsDir(), error)) {
        if (entry.path().extension() == ".json") {
            _ui->levelCombo->addItem(QString::fromStdString(entry.path().filename().string()),
                                     QString::fromStdString(entry.path().string()));
        }
    }
    _ui->levelCombo->model()->sort(0);
}

void AiModeScreen::refreshRunsAndReplays() {
    _ui->runCombo->clear();
    std::error_code error;
    if (std::filesystem::exists(runsRootDir(), error)) {
        // Une entree par run : runsRoot/<niveau>/<runId>/model.bin (LOT-ANNEXE-09 TACHE-04).
        for (const auto& levelDir : std::filesystem::directory_iterator(runsRootDir(), error)) {
            if (!levelDir.is_directory()) {
                continue;
            }
            for (const auto& runDir : std::filesystem::directory_iterator(levelDir.path(), error)) {
                const std::filesystem::path modelPath = runDir.path() / "model.bin";
                if (!std::filesystem::exists(modelPath, error)) {
                    continue;
                }
                const QString label =
                    QString::fromStdString(levelDir.path().filename().string() + " · " +
                                           runDir.path().filename().string());
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

void AiModeScreen::setTrainingControlsEnabled(bool enabled) {
    _ui->levelCombo->setEnabled(enabled);
    _ui->evolutionaryAlgorithmRadio->setEnabled(enabled);
    _ui->reinforceAlgorithmRadio->setEnabled(enabled);
    _ui->actorCriticAlgorithmRadio->setEnabled(enabled);
    _ui->advancedAlgorithmRadio->setEnabled(enabled);
    _ui->populationSpin->setEnabled(enabled);
    _ui->mutationRateSpin->setEnabled(enabled);
    _ui->episodesSpin->setEnabled(enabled);
    _ui->learningRateSpin->setEnabled(enabled);
    _ui->gammaSpin->setEnabled(enabled);
    _ui->optimizerCombo->setEnabled(enabled);
    _ui->seedSpin->setEnabled(enabled);
    _ui->launchTrainingButton->setVisible(enabled);
    _ui->stopTrainingButton->setVisible(!enabled);
}

void AiModeScreen::onLaunchTraining() {
    if (_worker || _ui->levelCombo->currentData().isNull()) {
        return;
    }

    TrainingRequest request;
    request.levelPath = _ui->levelCombo->currentData().toString();
    request.algorithmId = selectedAlgo();
    request.seed = static_cast<std::uint64_t>(_ui->seedSpin->value());
    request.runsRoot = QString::fromStdString(runsRootDir().string());
    request.populationSize = static_cast<std::size_t>(_ui->populationSpin->value());
    request.mutationRate = static_cast<float>(_ui->mutationRateSpin->value());
    request.episodes = static_cast<std::size_t>(_ui->episodesSpin->value());
    request.learningRate = static_cast<float>(_ui->learningRateSpin->value());
    request.gamma = static_cast<float>(_ui->gammaSpin->value());
    request.optimizer = _ui->optimizerCombo->currentText();

    _ui->statsTable->setRowCount(0);
    _ui->previewButton->setEnabled(false);
    _ui->generationCombo->setEnabled(false);
    _ui->generationCombo->clear();
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

void AiModeScreen::onTrainingProgress(int index, double bestReward, double meanReward,
                                      double successRate) {
    const int row = _ui->statsTable->rowCount();
    _ui->statsTable->insertRow(row);
    _ui->statsTable->setItem(row, 0, new QTableWidgetItem(QString::number(index)));
    _ui->statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(bestReward, 'f', 3)));
    _ui->statsTable->setItem(row, 2, new QTableWidgetItem(QString::number(meanReward, 'f', 3)));
    _ui->statsTable->setItem(
        row, 3, new QTableWidgetItem(QString::number(successRate * 100.0, 'f', 1) + "%"));
    _ui->statsTable->scrollToBottom();
    _ui->trainingStatusLabel->setText(
        text("ai_mode.status_progress").arg(index).arg(QString::number(bestReward, 'f', 3)));
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
    _ui->generationCombo->setEnabled(true);
    _ui->previewButton->setEnabled(true);
}

// Seul `solved` est lu : les chemins du signal decrivent le run qui vient de finir, mais l'ecran
// les relit depuis `runCombo` apres `refreshRunsAndReplays()`, qui voit aussi les runs anterieurs.
void AiModeScreen::onTrainingFinished(bool solved, QString /*modelPath*/, QString /*statsPath*/,
                                      QString /*configPath*/, QString /*replayPath*/,
                                      bool /*replayExported*/) {
    _ui->trainingStatusLabel->setText(solved ? text("ai_mode.status_done_solved")
                                             : text("ai_mode.status_done_unsolved"));
    teardownWorker();
    setTrainingControlsEnabled(true);
    refreshRunsAndReplays();
}

void AiModeScreen::onTrainingFailed(QString message) {
    _ui->trainingStatusLabel->setText(text("ai_mode.status_error").arg(message));
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
    if (_worker) {
        _worker->deleteLater();
        _worker = nullptr;
    }
    _workerThread.reset();
}

void AiModeScreen::onEvaluate() {
    const QString runDir = _ui->runCombo->currentData().toString();
    if (runDir.isEmpty()) {
        return;
    }
    const std::filesystem::path runPath(runDir.toStdString());
    const std::filesystem::path modelPath = runPath / "model.bin";
    const std::string levelName = runPath.parent_path().filename().string();
    const std::filesystem::path levelPath = levelsDir() / (levelName + ".json");

    // Algorithme du run evalue, relu dans son config.json : les boutons radio de l'onglet
    // Entrainement decrivent le PROCHAIN entrainement, pas celui qui a produit ce modele.
    const QString runAlgo =
        QString::fromStdString(aisolver::cli::loadTrainingConfig(
                                   runPath / "config.json", aisolver::cli::CommandLineOverrides{})
                                   .algorithmId);
    const std::optional<EvaluationOutcome> outcome = evaluateModel(
        QString::fromStdString(modelPath.string()), QString::fromStdString(levelPath.string()),
        runAlgo, _ui->repetitionsSpin->value());
    if (!outcome) {
        QMessageBox::warning(this, text("ai_mode.eval_title"), text("ai_mode.eval_failed"));
        return;
    }
    _ui->successRateValue->setText(QString::number(outcome->successRate * 100.0, 'f', 1) + "%");
    _ui->meanStepsValue->setText(QString::number(outcome->meanStepsOnSuccess, 'f', 1));
    _ui->varianceValue->setText(QString::number(outcome->stepVariance, 'f', 2));
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

void AiModeScreen::onExportReplay() {
    const QString runDir = _ui->runCombo->currentData().toString();
    if (runDir.isEmpty()) {
        return;
    }
    const std::filesystem::path source =
        std::filesystem::path(runDir.toStdString()) / "replay.json";
    std::error_code error;
    if (!std::filesystem::exists(source, error)) {
        _ui->saveStatusLabel->setText(text("ai_mode.no_replay"));
        return;
    }
    std::filesystem::create_directories(replaysDir(), error);
    const std::filesystem::path runPath(runDir.toStdString());
    const std::string destinationName =
        runPath.parent_path().filename().string() + "_" + runPath.filename().string() + ".json";
    std::filesystem::copy_file(source, replaysDir() / destinationName,
                               std::filesystem::copy_options::overwrite_existing, error);
    _ui->saveStatusLabel->setText(error ? text("ai_mode.export_failed")
                                        : text("ai_mode.export_ok"));
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
