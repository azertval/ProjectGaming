#include "Editor/MainMenu.h"

#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace editor {

MainMenu::MainMenu(QWidget* parent) : QWidget(parent) {
    auto* const title = new QLabel(QStringLiteral("ProjectGaming"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);

    auto* const play = new QPushButton(QStringLiteral("Jouer"), this);
    auto* const editor = new QPushButton(QStringLiteral("Éditeur de niveaux"), this);
    auto* const options = new QPushButton(QStringLiteral("Options"), this);
    auto* const keybindings = new QPushButton(QStringLiteral("Touches"), this);
    auto* const quit = new QPushButton(QStringLiteral("Quitter"), this);
    for (QPushButton* const button : {play, editor, options, keybindings, quit}) {
        button->setMinimumWidth(220);
    }

    connect(play, &QPushButton::clicked, this, &MainMenu::playRequested);
    connect(editor, &QPushButton::clicked, this, &MainMenu::editorRequested);
    connect(options, &QPushButton::clicked, this, &MainMenu::optionsRequested);
    connect(keybindings, &QPushButton::clicked, this, &MainMenu::keybindingsRequested);
    connect(quit, &QPushButton::clicked, this, &MainMenu::quitRequested);

    // Colonne de boutons centrée.
    auto* const column = new QVBoxLayout();
    column->setSpacing(10);
    column->addWidget(title);
    column->addSpacing(20);
    column->addWidget(play, 0, Qt::AlignCenter);
    column->addWidget(editor, 0, Qt::AlignCenter);
    column->addWidget(options, 0, Qt::AlignCenter);
    column->addWidget(keybindings, 0, Qt::AlignCenter);
    column->addWidget(quit, 0, Qt::AlignCenter);

    auto* const layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addLayout(column);
    layout->addStretch();
}

}  // namespace editor
