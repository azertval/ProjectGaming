#include "Editor/OptionsPage.h"

#include <utility>

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QFont>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QSlider>
#include <QTabWidget>
#include <QVBoxLayout>

#include "Editor/GameViewport.h"
#include "Editor/KeybindingsWidget.h"

namespace editor {

namespace {

// Reprend la palette du menu principal (fond sombre, titre quasi-blanc, accents ambre).
constexpr const char* STYLE = R"(
QLabel#optionsTitle { color: #f2f2ff; }
QTabWidget::pane { border: 1px solid #333a48; background: #1e2531; }
QTabBar::tab { background: #232a36; color: #b3b8c7; padding: 8px 18px; }
QTabBar::tab:selected { color: #ffd133; background: #1e2531; }
QLabel, QCheckBox, QComboBox { color: #cfd3dc; }
QPushButton { color: #b3b8c7; background: transparent; border: 1px solid #333a48;
              border-radius: 3px; padding: 6px 16px; }
QPushButton:hover, QPushButton:focus { color: #ffd133; border-color: #ffd133; }
)";

// Onglet Vidéo : V-Sync (fonctionnel), plein écran (fonctionnel), et réglages standard à venir.
[[nodiscard]] QWidget* makeVideoTab(GameViewport* viewport, OptionsPage* page) {
    auto* const tab = new QWidget(page);
    auto* const form = new QFormLayout(tab);

    auto* const vsync = new QCheckBox(QStringLiteral("Activée"), tab);
    vsync->setChecked(viewport->vsyncEnabled());
    QObject::connect(vsync, &QCheckBox::toggled, viewport,
                     [viewport](bool on) { viewport->setVSync(on); });
    form->addRow(QStringLiteral("Synchronisation verticale"), vsync);

    auto* const fullscreen = new QCheckBox(QStringLiteral("Plein écran"), tab);
    QObject::connect(fullscreen, &QCheckBox::toggled, page,
                     [page](bool on) { emit page->fullscreenRequested(on); });
    form->addRow(QStringLiteral("Affichage"), fullscreen);

    // Réglages vidéo standard supplémentaires (câblage ultérieur) : présents mais désactivés.
    auto* const resolution = new QComboBox(tab);
    resolution->addItems({QStringLiteral("Automatique (fenêtre)"), QStringLiteral("1280 × 720"),
                          QStringLiteral("1920 × 1080")});
    resolution->setEnabled(false);
    form->addRow(QStringLiteral("Résolution"), resolution);

    auto* const fpsCap = new QComboBox(tab);
    fpsCap->addItems({QStringLiteral("Illimité"), QStringLiteral("60"), QStringLiteral("144")});
    fpsCap->setEnabled(false);
    form->addRow(QStringLiteral("Limite d'images/s"), fpsCap);

    return tab;
}

// Onglet Audio : placeholder (curseurs désactivés).
[[nodiscard]] QWidget* makeAudioTab(OptionsPage* page) {
    auto* const tab = new QWidget(page);
    auto* const form = new QFormLayout(tab);
    for (const QString& label :
         {QStringLiteral("Volume général"), QStringLiteral("Musique"), QStringLiteral("Effets")}) {
        auto* const slider = new QSlider(Qt::Horizontal, tab);
        slider->setRange(0, 100);
        slider->setValue(80);
        slider->setEnabled(false);
        form->addRow(label, slider);
    }
    form->addRow(new QLabel(QStringLiteral("(Audio à venir)"), tab));
    return tab;
}

// Onglet Manette : placeholder (remappage manette à venir).
[[nodiscard]] QWidget* makeGamepadTab(OptionsPage* page) {
    auto* const tab = new QWidget(page);
    auto* const layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel(
        QStringLiteral("Remappage de la manette (XInput) — à venir."), tab));
    layout->addStretch();
    return tab;
}

}  // namespace

OptionsPage::OptionsPage(GameViewport* viewport, std::filesystem::path keybindingsPath,
                         QWidget* parent)
    : QWidget(parent) {
    setAutoFillBackground(true);
    QPalette background = palette();
    background.setColor(QPalette::Window, QColor("#1a1f29"));
    setPalette(background);
    setStyleSheet(QString::fromUtf8(STYLE));

    auto* const title = new QLabel(QStringLiteral("Options"), this);
    title->setObjectName(QStringLiteral("optionsTitle"));
    QFont titleFont = title->font();
    titleFont.setPointSize(32);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto* const tabs = new QTabWidget(this);
    tabs->addTab(makeVideoTab(viewport, this), QStringLiteral("Vidéo"));
    tabs->addTab(makeAudioTab(this), QStringLiteral("Audio"));
    tabs->addTab(new KeybindingsWidget(viewport->gameBindings(), std::move(keybindingsPath), this),
                 QStringLiteral("Commande clavier"));
    tabs->addTab(makeGamepadTab(this), QStringLiteral("Commande manette"));

    auto* const back = new QPushButton(QStringLiteral("Retour"), this);
    connect(back, &QPushButton::clicked, this, &OptionsPage::backRequested);

    auto* const bottom = new QHBoxLayout();
    bottom->addWidget(back);
    bottom->addStretch();

    auto* const layout = new QVBoxLayout(this);
    layout->setContentsMargins(72, 48, 72, 40);
    layout->addWidget(title);
    layout->addSpacing(20);
    layout->addWidget(tabs);
    layout->addSpacing(12);
    layout->addLayout(bottom);
}

}  // namespace editor
