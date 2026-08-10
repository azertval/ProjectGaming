#include "HMI/Interface/OptionsPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QString>
#include <QTabWidget>
#include <utility>

#include "HMI/Game/GameViewport.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/EditorKeybindingsWidget.h"
#include "HMI/Interface/GamepadBindingsWidget.h"
#include "HMI/Interface/KeybindingsWidget.h"
#include "HMI/Localization/Localization.h"
#include "ui_OptionsPage.h"

namespace hmi {

OptionsPage::OptionsPage(GameViewport* viewport, std::filesystem::path keybindingsPath,
                         QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::OptionsPage>()) {
    setObjectName(QStringLiteral("OptionsPage"));  // ciblé par le thème (theme.qss)
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);

    // Marges de la mise en page, depuis l'echelle d'espacement des jetons (LOT-56 TACHE-03) --
    // remplace les nombres jusqu'ici figes dans OptionsPage.ui.
    const SpacingTokens& spacing = identityTokens().spacing;
    _ui->verticalLayout->setContentsMargins(spacing.extraLarge * 3, spacing.extraLarge * 2,
                                            spacing.extraLarge * 3, spacing.extraLarge * 2);

    // Onglet Vidéo : V-Sync et plein écran fonctionnels (résolution/FPS présents mais désactivés).
    _ui->vsyncCheck->setChecked(viewport->vsyncEnabled());
    connect(_ui->vsyncCheck, &QCheckBox::toggled, viewport,
            [viewport](bool on) { viewport->setVSync(on); });
    connect(_ui->fullscreenCheck, &QCheckBox::toggled, this,
            [this](bool on) { emit fullscreenRequested(on); });

    // Onglet Général (mise en page dans OptionsPage.ui) : langue (index 0 = fr, 1 = en) et logs.
    connect(_ui->languageCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        emit languageChanged(index == 0 ? QStringLiteral("fr") : QStringLiteral("en"));
    });
    connect(_ui->saveLogsButton, &QPushButton::clicked, this, &OptionsPage::saveLogsRequested);

    // Onglets à contenu **dynamique** (une ligne par action) : générés en code (exception admise).
    _keyboard = new KeybindingsWidget(viewport->gameBindings(), keybindingsPath, this);
    _keyboardTabIndex = _ui->tabWidget->addTab(_keyboard, QString());
    // Onglet Éditeur (LOT-57 TACHE-04) : remappage des touches d'éditeur, jusqu'ici définies et
    // jamais exposées à l'écran. bindingsChanged fait resynchroniser les raccourcis effectifs des
    // actions (menu/barre d'outils), sans quoi un remappage ici resterait invisible ailleurs.
    _editorKeyboard =
        new EditorKeybindingsWidget(viewport->editorBindings(), keybindingsPath, this);
    _editorKeyboardTabIndex = _ui->tabWidget->addTab(_editorKeyboard, QString());
    connect(_editorKeyboard, &EditorKeybindingsWidget::bindingsChanged, this,
            &OptionsPage::editorBindingsChanged);
    _gamepad =
        new GamepadBindingsWidget(viewport->gamepadBindings(), std::move(keybindingsPath), this);
    _gamepadTabIndex = _ui->tabWidget->addTab(_gamepad, QString());

    connect(_ui->backButton, &QPushButton::clicked, this, &OptionsPage::backRequested);
}

OptionsPage::~OptionsPage() = default;

void OptionsPage::retranslateUi(const Localization& loc) {
    const auto t = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };

    _ui->optionsTitle->setText(t("options.title"));

    // Onglets et libellés de l'onglet Vidéo.
    _ui->tabWidget->setTabText(0, t("tab.video"));
    _ui->tabWidget->setTabText(1, t("tab.audio"));
    _ui->tabWidget->setTabText(2, t("tab.general"));
    _ui->vsyncLabel->setText(t("options.vsync"));
    _ui->vsyncCheck->setText(t("options.enabled"));
    _ui->displayLabel->setText(t("options.display"));
    _ui->fullscreenCheck->setText(t("options.fullscreen"));
    _ui->resolutionLabel->setText(t("options.resolution"));
    _ui->resolutionCombo->setItemText(0, t("options.resolution_auto"));
    _ui->fpsLabel->setText(t("options.fps_limit"));
    _ui->fpsCombo->setItemText(0, t("options.unlimited"));
    _ui->audioLabel->setText(t("options.audio_soon"));
    _ui->backButton->setText(t("options.back"));

    // Onglet Général.
    _ui->languageLabel->setText(t("options.language"));
    _ui->languageCombo->setItemText(0, t("language.fr"));
    _ui->languageCombo->setItemText(1, t("language.en"));
    _ui->logsLabel->setText(t("options.logs"));
    _ui->saveLogsButton->setText(t("options.save_logs"));

    // Aligne la sélection de langue sur la langue active, sans réémettre languageChanged.
    const int activeIndex = loc.activeLanguage() == "en" ? 1 : 0;
    if (activeIndex != _ui->languageCombo->currentIndex()) {
        const QSignalBlocker blocker(_ui->languageCombo);
        _ui->languageCombo->setCurrentIndex(activeIndex);
    }

    // Onglets de remappage (contenu dynamique).
    _ui->tabWidget->setTabText(_keyboardTabIndex, t("tab.keyboard"));
    _ui->tabWidget->setTabText(_editorKeyboardTabIndex, t("tab.editor"));
    _ui->tabWidget->setTabText(_gamepadTabIndex, t("tab.gamepad"));
    _keyboard->retranslateUi(loc);
    _editorKeyboard->retranslateUi(loc);
    _gamepad->retranslateUi(loc);
}

}  // namespace hmi
