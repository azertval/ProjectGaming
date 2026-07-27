#include "Editor/MainMenu.h"

#include <QPushButton>

#include "ui_MainMenu.h"

namespace editor {

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

}  // namespace editor
