#include "Editor/MainMenu.h"

#include <QColor>
#include <QFont>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QVBoxLayout>

namespace editor {

namespace {

// Palette reprise de l'ancien menu (MenuScreen) : fond sombre, titre quasi-blanc, options gris-bleu,
// surbrillance ambre sur l'option survolée/active.
constexpr const char* BACKGROUND = "#1a1f29";  // ~ (0.10, 0.12, 0.16)
constexpr const char* TITLE_COLOR = "#f2f2ff";
constexpr const char* OPTION_COLOR = "#b3b8c7";
constexpr const char* SELECTED_COLOR = "#ffd133";  // ~ (1.0, 0.82, 0.20)

}  // namespace

MainMenu::MainMenu(QWidget* parent) : QWidget(parent) {
    // Fond sombre plein (comme l'effacement de l'ancien écran de menu).
    setAutoFillBackground(true);
    QPalette background = palette();
    background.setColor(QPalette::Window, QColor(BACKGROUND));
    setPalette(background);

    // Options en boutons **plats** alignés à gauche, gris-bleu, ambre au survol/focus — dans l'esprit
    // de la liste de l'ancien menu (pas des boutons encadrés).
    setStyleSheet(QStringLiteral(
                      "QLabel#menuTitle { color: %1; }"
                      "QPushButton { color: %2; background: transparent; border: none;"
                      " text-align: left; padding: 8px 0; }"
                      "QPushButton:hover, QPushButton:focus { color: %3; }")
                      .arg(TITLE_COLOR, OPTION_COLOR, SELECTED_COLOR));

    auto* const title = new QLabel(QStringLiteral("ProjectGaming"), this);
    title->setObjectName(QStringLiteral("menuTitle"));
    QFont titleFont = title->font();
    titleFont.setPointSize(48);
    titleFont.setBold(true);
    title->setFont(titleFont);

    const QFont optionFont(QStringLiteral("Consolas"), 22, QFont::Bold);  // chasse fixe, évoque le pixel art

    auto* const play = new QPushButton(QStringLiteral("Jouer"), this);
    auto* const editor = new QPushButton(QStringLiteral("Éditeur de niveaux"), this);
    auto* const options = new QPushButton(QStringLiteral("Options"), this);
    auto* const keybindings = new QPushButton(QStringLiteral("Touches"), this);
    auto* const quit = new QPushButton(QStringLiteral("Quitter"), this);
    for (QPushButton* const button : {play, editor, options, keybindings, quit}) {
        button->setFont(optionFont);
        button->setCursor(Qt::PointingHandCursor);
        button->setFlat(true);
    }

    connect(play, &QPushButton::clicked, this, &MainMenu::playRequested);
    connect(editor, &QPushButton::clicked, this, &MainMenu::editorRequested);
    connect(options, &QPushButton::clicked, this, &MainMenu::optionsRequested);
    connect(keybindings, &QPushButton::clicked, this, &MainMenu::keybindingsRequested);
    connect(quit, &QPushButton::clicked, this, &MainMenu::quitRequested);

    // Colonne alignée à gauche, avec marge, titre en haut puis les options (comme l'ancien menu).
    auto* const layout = new QVBoxLayout(this);
    layout->setContentsMargins(72, 64, 40, 40);
    layout->setSpacing(4);
    layout->addWidget(title);
    layout->addSpacing(40);
    layout->addWidget(play);
    layout->addWidget(editor);
    layout->addWidget(options);
    layout->addWidget(keybindings);
    layout->addWidget(quit);
    layout->addStretch();
}

}  // namespace editor
