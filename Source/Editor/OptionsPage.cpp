#include "Editor/OptionsPage.h"

#include <utility>

#include <QCheckBox>
#include <QPushButton>
#include <QTabWidget>

#include "Editor/GameViewport.h"
#include "Editor/GamepadBindingsWidget.h"
#include "Editor/KeybindingsWidget.h"
#include "ui_OptionsPage.h"

namespace editor {

OptionsPage::OptionsPage(GameViewport* viewport, std::filesystem::path keybindingsPath,
                         QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::OptionsPage>()) {
    setObjectName(QStringLiteral("OptionsPage"));  // ciblé par le thème (theme.qss)
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);

    // Onglet Vidéo : V-Sync et plein écran fonctionnels (résolution/FPS présents mais désactivés).
    _ui->vsyncCheck->setChecked(viewport->vsyncEnabled());
    connect(_ui->vsyncCheck, &QCheckBox::toggled, viewport,
            [viewport](bool on) { viewport->setVSync(on); });
    connect(_ui->fullscreenCheck, &QCheckBox::toggled, this,
            [this](bool on) { emit fullscreenRequested(on); });

    // Onglets à contenu généré (ajoutés en code) : remappage clavier et manette.
    _ui->tabWidget->addTab(new KeybindingsWidget(viewport->gameBindings(), keybindingsPath, this),
                           QStringLiteral("Commande clavier"));
    _ui->tabWidget->addTab(
        new GamepadBindingsWidget(viewport->gamepadBindings(), std::move(keybindingsPath), this),
        QStringLiteral("Commande manette"));

    connect(_ui->backButton, &QPushButton::clicked, this, &OptionsPage::backRequested);
}

OptionsPage::~OptionsPage() = default;

}  // namespace editor
