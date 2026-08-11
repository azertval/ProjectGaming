#include "HMI/Interface/MainMenu.h"

#include <QPushButton>

#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"
#include "ui_MainMenu.h"

namespace hmi {

MainMenu::MainMenu(QWidget* parent) : QWidget(parent), _ui(std::make_unique<Ui::MainMenu>()) {
    setObjectName(QStringLiteral("MainMenu"));    // ciblé par le thème (theme.qss)
    setAttribute(Qt::WA_StyledBackground, true);  // pour que la couleur de fond du thème s'applique
    _ui->setupUi(this);

    // Marges de la mise en page, depuis l'echelle d'espacement des jetons (LOT-56 TACHE-03) --
    // remplace les nombres jusqu'ici figes dans MainMenu.ui.
    const SpacingTokens& spacing = identityTokens().spacing;
    _ui->verticalLayout->setContentsMargins(spacing.extraLarge * 3, spacing.extraLarge * 3,
                                            spacing.extraLarge * 2, spacing.extraLarge * 2);

    connect(_ui->continueButton, &QPushButton::clicked, this, &MainMenu::continueRequested);
    connect(_ui->newGameButton, &QPushButton::clicked, this, &MainMenu::newGameRequested);
    connect(_ui->selectLevelButton, &QPushButton::clicked, this, &MainMenu::selectLevelRequested);
    connect(_ui->editorButton, &QPushButton::clicked, this, &MainMenu::editorRequested);
    connect(_ui->optionsButton, &QPushButton::clicked, this, &MainMenu::optionsRequested);
    connect(_ui->quitButton, &QPushButton::clicked, this, &MainMenu::quitRequested);
}

MainMenu::~MainMenu() = default;

void MainMenu::retranslateUi(const Localization& loc) {
    _ui->menuTitle->setText(QString::fromStdString(loc.text("menu.title")));
    _ui->continueButton->setText(QString::fromStdString(loc.text("menu.continue")));
    _ui->newGameButton->setText(QString::fromStdString(loc.text("menu.new_game")));
    _ui->selectLevelButton->setText(QString::fromStdString(loc.text("menu.select_level")));
    _ui->editorButton->setText(QString::fromStdString(loc.text("menu.edit_mode")));
    _ui->optionsButton->setText(QString::fromStdString(loc.text("menu.options")));
    _ui->quitButton->setText(QString::fromStdString(loc.text("menu.quit")));
}

void MainMenu::setContinueEnabled(bool enabled) {
    _ui->continueButton->setEnabled(enabled);
}

}  // namespace hmi
