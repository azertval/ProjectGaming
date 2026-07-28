#include "HMI/Interface/MainMenu.h"

#include <QPushButton>

#include "HMI/Localization/Localization.h"
#include "ui_MainMenu.h"

namespace hmi {

MainMenu::MainMenu(QWidget* parent) : QWidget(parent), _ui(std::make_unique<Ui::MainMenu>()) {
    setObjectName(QStringLiteral("MainMenu"));   // ciblé par le thème (theme.qss)
    setAttribute(Qt::WA_StyledBackground, true);  // pour que la couleur de fond du thème s'applique
    _ui->setupUi(this);

    connect(_ui->playButton, &QPushButton::clicked, this, &MainMenu::playRequested);
    connect(_ui->editorButton, &QPushButton::clicked, this, &MainMenu::editorRequested);
    connect(_ui->optionsButton, &QPushButton::clicked, this, &MainMenu::optionsRequested);
    connect(_ui->quitButton, &QPushButton::clicked, this, &MainMenu::quitRequested);
}

MainMenu::~MainMenu() = default;

void MainMenu::retranslateUi(const Localization& loc) {
    _ui->menuTitle->setText(QString::fromStdString(loc.text("menu.title")));
    _ui->playButton->setText(QString::fromStdString(loc.text("menu.play")));
    _ui->editorButton->setText(QString::fromStdString(loc.text("menu.edit_mode")));
    _ui->optionsButton->setText(QString::fromStdString(loc.text("menu.options")));
    _ui->quitButton->setText(QString::fromStdString(loc.text("menu.quit")));
}

}  // namespace hmi
