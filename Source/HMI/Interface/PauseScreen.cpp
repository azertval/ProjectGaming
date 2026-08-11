#include "HMI/Interface/PauseScreen.h"

#include <QKeyEvent>
#include <QPushButton>

#include "HMI/Localization/Localization.h"
#include "ui_PauseScreen.h"

namespace hmi {

PauseScreen::PauseScreen(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::PauseScreen>()) {
    setObjectName(QStringLiteral("PauseScreen"));
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);

    // Fond translucide pose ici plutot que dans le theme (theme.qss) : propre a ce recouvrement
    // par-dessus la scene figee, pas un jeton de theme editeur (LOT-56) -- la carte du menu garde
    // un fond opaque pour rester lisible par-dessus n'importe quel niveau.
    setStyleSheet(
        QStringLiteral("#PauseScreen { background-color: rgba(0, 0, 0, 160); }"
                       "#menuCard { background-color: rgba(32, 32, 32, 230); "
                       "border-radius: 8px; padding: 24px; }"
                       "#menuCard QLabel, #menuCard QPushButton { color: white; }"));

    connect(_ui->resumeButton, &QPushButton::clicked, this, &PauseScreen::resumeRequested);
    connect(_ui->restartButton, &QPushButton::clicked, this, &PauseScreen::restartRequested);
    connect(_ui->optionsButton, &QPushButton::clicked, this, &PauseScreen::optionsRequested);
    connect(_ui->quitButton, &QPushButton::clicked, this, &PauseScreen::quitToMenuRequested);

    // autoDefault (LOT-59 TACHE-07, bug réel trouvé en jeu : Entrée sans effet) : Qt ne l'active
    // par défaut que pour un bouton dont un ancêtre est un vrai QDialog -- ce widget n'en est pas
    // un (même affiché comme fenêtre Qt::Dialog, cf. MainWindow), donc Entrée ne déclenchait aucun
    // bouton, même focus (seule Espace fonctionnait). Posé explicitement sur chacun pour que le
    // bouton *ayant le focus* réponde à Entrée, quel qu'il soit.
    for (QPushButton* const button : findChildren<QPushButton*>()) {
        button->setAutoDefault(true);
    }
}

PauseScreen::~PauseScreen() = default;

void PauseScreen::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit resumeRequested();
        return;
    }
    QWidget::keyPressEvent(event);
}

void PauseScreen::focusDefaultAction() {
    _ui->resumeButton->setFocus();
}

void PauseScreen::retranslateUi(const Localization& loc) {
    _ui->pauseTitle->setText(QString::fromStdString(loc.text("pause.title")));
    _ui->resumeButton->setText(QString::fromStdString(loc.text("pause.resume")));
    _ui->restartButton->setText(QString::fromStdString(loc.text("pause.restart")));
    _ui->optionsButton->setText(QString::fromStdString(loc.text("pause.options")));
    _ui->quitButton->setText(QString::fromStdString(loc.text("pause.quit_to_menu")));
}

}  // namespace hmi
