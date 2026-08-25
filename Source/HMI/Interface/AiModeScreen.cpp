// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/AiModeScreen.h"

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

#include "AiSolver/Replay/ReplayFile.h"
#include "HMI/Ai/EvaluationHelper.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "ui_AiModeScreen.h"

/**
 * @file HMI/Interface/AiModeScreen.cpp
 * @brief Voir `AiModeScreen.h`. Ne référence jamais `AiSolver/Training`/`Nn`/`Optim` directement
 * (amendement de `LOT-ANNEXE-18` limité à `HMI/Ai`, voir `epic.md` de `LOT-ANNEXE-21`) : délègue à
 * `hmi::TrainingWorker` (thread, entraînement) et `hmi::evaluateModel` (`HMI/Ai/EvaluationHelper`).
 */

namespace hmi {

namespace {

constexpr const char* kRunsRoot = "TrainingRuns";
constexpr const char* kReplaysDirName = "Replays";
constexpr const char* kLevelsDirName = "Levels";

std::filesystem::path levelsDir() {
    return executableDirectory() / kLevelsDirName;
}

std::filesystem::path runsRootDir() {
    return executableDirectory() / kRunsRoot;
}

std::filesystem::path replaysDir() {
    return executableDirectory() / kReplaysDirName;
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

    // Infobulles (LOT-ANNEXE-21) : une par champ/bouton des onglets Entraînement et Validation &
    // sauvegarde, texte de référence fixé lors de la revue des maquettes.
    _ui->levelCombo->setToolTip(tr(
        "Fichier de niveau sur lequel l'IA s'entraîne. Un modèle est entraîné niveau par niveau, "
        "jamais sur plusieurs à la fois."));
    _ui->algoEvoRadio->setToolTip(
        tr("Population de réseaux, sélection puis mutation à chaque génération. Le plus rapide à "
           "obtenir un résultat ; sert de ligne de base."));
    _ui->algoPgRadio->setToolTip(
        tr("Apprentissage par gradient de politique (policy gradient) : un réseau appris par "
           "rétropropagation, pas une recherche aveugle."));
    _ui->algoAcRadio->setToolTip(
        tr("Variante de REINFORCE avec un second réseau (le critique) qui réduit la variance de "
           "l'estimation du gradient."));
    _ui->algoAvanceRadio->setToolTip(
        tr("Q-learning profond : mémoire de rejeu et réseau cible, l'algorithme le plus avancé."));
    _ui->populationSpin->setToolTip(
        tr("Nombre d'individus (réseaux) par génération — évolutif uniquement. Plus grand explore "
           "plus large mais coûte plus cher par génération."));
    _ui->mutationRateSpin->setToolTip(tr(
        "Probabilité qu'un poids du réseau soit perturbé aléatoirement à chaque génération. Trop "
        "bas : convergence lente. Trop haut : instable."));
    _ui->episodesSpin->setToolTip(
        tr("Plafond de générations (évolutif) ou nombre d'épisodes à jouer (autres algorithmes) — "
           "critère de secours, pas un objectif."));
    _ui->learningRateSpin->setToolTip(
        tr("Taille du pas de mise à jour des poids à chaque optimisation (algorithmes par "
           "gradient uniquement)."));
    _ui->gammaSpin->setToolTip(
        tr("Facteur d'actualisation : poids donné aux récompenses futures par rapport aux "
           "récompenses immédiates (algorithmes par gradient uniquement)."));
    _ui->optimizerCombo->setToolTip(
        tr("Règle de mise à jour des poids à partir du gradient calculé."));
    _ui->seedSpin->setToolTip(
        tr("Graine du générateur aléatoire. Deux runs avec la même graine et les mêmes paramètres "
           "produisent un résultat strictement identique."));
    _ui->launchTrainingButton->setToolTip(
        tr("Lance l'entraînement en arrière-plan ; la fenêtre reste réactive."));
    _ui->stopTrainingButton->setToolTip(
        tr("Arrête l'entraînement à la prochaine génération/épisode. Le meilleur individu obtenu "
           "jusqu'ici reste sauvegardé."));
    _ui->generationCombo->setToolTip(
        tr("Génération (évolutif) ou épisode (autres algorithmes) dont l'aperçu sera rejoué par "
           "« Voir en jeu »."));
    _ui->previewButton->setToolTip(
        tr("Rejoue le meilleur individu de la génération/l'épisode sélectionné dans la scène du "
           "niveau."));

    _ui->runCombo->setToolTip(
        tr("Run entraîné à évaluer (dossier TrainingRuns le plus récent en tête de liste)."));
    _ui->repetitionsSpin->setToolTip(
        tr("Nombre d'épisodes rejoués pour mesurer le taux de réussite. Plus élevé = mesure plus "
           "fiable, plus lent."));
    _ui->evaluateButton->setToolTip(
        tr("Rejoue le modèle en mode déterministe (Argmax) sur le niveau d'origine, N fois, et "
           "calcule les statistiques ci-dessous."));
    _ui->saveModelButton->setToolTip(
        tr("Copie les poids du modèle (déjà sauvegardés dans le dossier du run) vers un fichier "
           "de votre choix."));
    _ui->exportReplayButton->setToolTip(
        tr("Copie le rejeu produit par ce run vers Elements/Replays, sélectionnable ensuite dans "
           "l'onglet Rejeu."));

    setTrainingControlsEnabled(true);
    refreshLevelList();
    refreshRunsAndReplays();
}

AiModeScreen::~AiModeScreen() {
    teardownWorker();
}

void AiModeScreen::retranslateUi(const Localization& loc) {
    const auto t = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };
    _ui->titleLabel->setText(t("ai_mode.title"));
    _ui->backButton->setText(t("options.back"));
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
    if (_ui->algoPgRadio->isChecked()) {
        return QStringLiteral("pg");
    }
    if (_ui->algoAcRadio->isChecked()) {
        return QStringLiteral("ac");
    }
    if (_ui->algoAvanceRadio->isChecked()) {
        return QStringLiteral("avance");
    }
    return QStringLiteral("evo");
}

void AiModeScreen::setTrainingControlsEnabled(bool enabled) {
    _ui->levelCombo->setEnabled(enabled);
    _ui->algoEvoRadio->setEnabled(enabled);
    _ui->algoPgRadio->setEnabled(enabled);
    _ui->algoAcRadio->setEnabled(enabled);
    _ui->algoAvanceRadio->setEnabled(enabled);
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
    request.algo = selectedAlgo();
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
    _ui->trainingStatusLabel->setText(tr("Entraînement en cours…"));

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
    _ui->trainingStatusLabel->setText(tr("Arrêt demandé…"));
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
    _ui->trainingStatusLabel->setText(tr("Génération/épisode %1 — meilleure récompense %2")
                                          .arg(index)
                                          .arg(QString::number(bestReward, 'f', 3)));
}

void AiModeScreen::onTrainingPreviewReady(QString replayPath, QString /*algo*/,
                                          QString /*levelPath*/, int generation) {
    // Un aperçu par génération/épisode, jamais écrasé (voir TrainingWorker::run) : on suit la
    // dernière génération reçue tant que l'utilisateur n'a pas sélectionné une génération
    // antérieure pour l'examiner pendant que l'entraînement continue.
    const bool wasFollowingLatest = _ui->generationCombo->count() == 0 ||
                                    _ui->generationCombo->currentIndex() ==
                                        _ui->generationCombo->count() - 1;
    _ui->generationCombo->addItem(tr("Génération %1").arg(generation), replayPath);
    if (wasFollowingLatest) {
        _ui->generationCombo->setCurrentIndex(_ui->generationCombo->count() - 1);
    }
    _ui->generationCombo->setEnabled(true);
    _ui->previewButton->setEnabled(true);
}

void AiModeScreen::onTrainingFinished(bool solved, QString modelPath, QString /*statsPath*/,
                                      QString /*configPath*/, QString replayPath,
                                      bool replayExported) {
    _lastRunModelPath = modelPath;
    _lastRunReplayPath = replayExported ? replayPath : QString();
    _lastRunAlgo = selectedAlgo();
    _ui->trainingStatusLabel->setText(solved ? tr("Entraînement terminé : niveau résolu.")
                                             : tr("Entraînement terminé : non résolu (arrêté ou "
                                                  "plafond atteint)."));
    teardownWorker();
    setTrainingControlsEnabled(true);
    refreshRunsAndReplays();
}

void AiModeScreen::onTrainingFailed(QString message) {
    _ui->trainingStatusLabel->setText(tr("Erreur : %1").arg(message));
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

    const std::optional<EvaluationOutcome> outcome = evaluateModel(
        QString::fromStdString(modelPath.string()), QString::fromStdString(levelPath.string()),
        selectedAlgo(), _ui->repetitionsSpin->value());
    if (!outcome) {
        QMessageBox::warning(this, tr("Évaluation"),
                             tr("Impossible de charger ce modèle (algorithme incompatible ?)."));
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
    const QString destination = QFileDialog::getSaveFileName(
        this, tr("Sauvegarder le modèle"), QStringLiteral("model.bin"), tr("Modèle (*.bin)"));
    if (destination.isEmpty()) {
        return;
    }
    std::error_code error;
    std::filesystem::copy_file(source, destination.toStdString(),
                               std::filesystem::copy_options::overwrite_existing, error);
    _ui->saveStatusLabel->setText(error ? tr("Échec de la sauvegarde.") : tr("Modèle sauvegardé."));
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
        _ui->saveStatusLabel->setText(tr("Ce run n'a pas produit de rejeu (niveau non résolu)."));
        return;
    }
    std::filesystem::create_directories(replaysDir(), error);
    const std::filesystem::path runPath(runDir.toStdString());
    const std::string destinationName =
        runPath.parent_path().filename().string() + "_" + runPath.filename().string() + ".json";
    std::filesystem::copy_file(source, replaysDir() / destinationName,
                               std::filesystem::copy_options::overwrite_existing, error);
    _ui->saveStatusLabel->setText(error ? tr("Échec de l'export.") : tr("Rejeu publié."));
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
