#include "HMI/Interface/LevelCompleteScreen.h"

#include <QPushButton>

#include "HMI/Localization/Localization.h"
#include "ui_LevelCompleteScreen.h"

namespace hmi {

LevelCompleteScreen::LevelCompleteScreen(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::LevelCompleteScreen>()) {
    setObjectName(QStringLiteral("LevelCompleteScreen"));
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);

    // Même fond translucide que PauseScreen (LOT-59 TACHE-02) : les deux recouvrements posent le
    // même style, pas un jeton de theme.qss (éditeur).
    setStyleSheet(
        QStringLiteral("#LevelCompleteScreen { background-color: rgba(0, 0, 0, 160); }"
                       "#menuCard { background-color: rgba(32, 32, 32, 230); "
                       "border-radius: 8px; padding: 24px; }"
                       "#menuCard QLabel, #menuCard QPushButton { color: white; }"));

    connect(_ui->continueButton, &QPushButton::clicked, this,
            &LevelCompleteScreen::continueRequested);
    connect(_ui->replayButton, &QPushButton::clicked, this, &LevelCompleteScreen::replayRequested);
    connect(_ui->returnToMenuButton, &QPushButton::clicked, this,
            &LevelCompleteScreen::returnToMenuRequested);
}

LevelCompleteScreen::~LevelCompleteScreen() = default;

void LevelCompleteScreen::configure(bool sequenceComplete, const QString& levelName) {
    _sequenceComplete = sequenceComplete;
    _levelName = levelName;
    applyTexts();
}

void LevelCompleteScreen::retranslateUi(const Localization& loc) {
    _loc = &loc;
    applyTexts();
}

void LevelCompleteScreen::applyTexts() {
    if (_loc == nullptr) {
        return;
    }
    _ui->continueButton->setText(QString::fromStdString(_loc->text("level_complete.continue")));
    _ui->replayButton->setText(QString::fromStdString(_loc->text("level_complete.replay")));
    _ui->returnToMenuButton->setText(
        QString::fromStdString(_loc->text("level_complete.return_to_menu")));

    // Fin de séquence : ni « Continuer » (rien où mener après le dernier tableau) ni « Rejouer »
    // (le message de fin porte sur la séquence entière, pas un tableau précis) -- seul le retour
    // au menu a un sens, conformément à la tâche (« message de fin, retour au menu »).
    _ui->continueButton->setVisible(!_sequenceComplete);
    _ui->replayButton->setVisible(!_sequenceComplete);

    if (_sequenceComplete) {
        _ui->titleLabel->setText(
            QString::fromStdString(_loc->text("level_complete.title_sequence")));
    } else {
        _ui->titleLabel->setText(
            QString::fromStdString(_loc->text("level_complete.title_level")).arg(_levelName));
    }
}

void LevelCompleteScreen::focusDefaultAction() {
    if (_sequenceComplete) {
        _ui->returnToMenuButton->setFocus();
    } else {
        _ui->continueButton->setFocus();
    }
}

}  // namespace hmi
