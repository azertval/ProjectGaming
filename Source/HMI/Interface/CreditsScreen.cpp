#include "HMI/Interface/CreditsScreen.h"

#include <QPushButton>

#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"
#include "ui_CreditsScreen.h"

namespace hmi {

CreditsScreen::CreditsScreen(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::CreditsScreen>()) {
    setObjectName(QStringLiteral("CreditsScreen"));  // ciblé par le thème (theme.qss)
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);

    // Marges de la mise en page, depuis l'echelle d'espacement des jetons (LOT-56 TACHE-03) --
    // meme patron que MainMenu/OptionsPage.
    const SpacingTokens& spacing = identityTokens().spacing;
    _ui->verticalLayout->setContentsMargins(spacing.extraLarge * 3, spacing.extraLarge * 2,
                                            spacing.extraLarge * 3, spacing.extraLarge * 2);

    connect(_ui->backButton, &QPushButton::clicked, this, &CreditsScreen::backRequested);
    _ui->backButton->setAutoDefault(true);  // meme piege Entree que MainMenu (LOT-59 TACHE-07).
}

CreditsScreen::~CreditsScreen() = default;

void CreditsScreen::retranslateUi(const Localization& loc) {
    const auto t = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };

    _ui->creditsTitle->setText(t("credits.title"));
    _ui->developmentLabel->setText(t("credits.development"));
    _ui->audioLabel->setText(t("credits.audio"));
    _ui->graphicsLabel->setText(t("credits.graphics"));
    _ui->backButton->setText(t("options.back"));
}

void CreditsScreen::focusDefaultAction() {
    _ui->backButton->setFocus();
}

}  // namespace hmi
