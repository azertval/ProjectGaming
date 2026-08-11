#include "HMI/Interface/LevelSelectScreen.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <Qt>

#include "HMI/Game/Progression.h"
#include "HMI/Localization/Localization.h"
#include "ui_LevelSelectScreen.h"

namespace hmi {

namespace {

// État affiché pour chaque tableau de la séquence -- résolu une fois (setSequenceLevels), jamais
// recalculé au clic : la règle de déverrouillage (hmi::isLevelUnlocked) est la seule autorité,
// mais MainWindow la revalide de toute façon avant de lancer quoi que ce soit.
[[nodiscard]] LevelSelectState resolveState(const Progression& progression,
                                            const std::vector<std::string>& sequence,
                                            const std::string& levelName) {
    if (progression.isCompleted(levelName)) {
        return LevelSelectState::Completed;
    }
    return isLevelUnlocked(progression, sequence, levelName) ? LevelSelectState::Current
                                                             : LevelSelectState::Locked;
}

}  // namespace

LevelSelectScreen::LevelSelectScreen(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::LevelSelectScreen>()) {
    setObjectName(QStringLiteral("LevelSelectScreen"));
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);

    connect(_ui->backButton, &QPushButton::clicked, this, &LevelSelectScreen::backRequested);
    connect(_ui->sequenceList, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        // Un item désactivé (verrouillé) ne devrait pas pouvoir être activé par Qt lui-même, mais
        // le vérifier explicitement ici documente l'invariant et protège d'un futur changement de
        // patron de navigation (EX-IHM-005 : jamais lançable verrouillé, même par la manette).
        if ((item->flags() & Qt::ItemIsEnabled) == 0) {
            return;
        }
        emit sequenceLevelChosen(item->data(Qt::UserRole).toString());
    });
    connect(_ui->personalList, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        emit personalLevelChosen(item->data(Qt::UserRole).toString());
    });

    // autoDefault (LOT-59 TACHE-07, bug réel trouvé en jeu sur PauseScreen, même cause ici) : Qt
    // ne l'active par défaut que pour un bouton dont un ancêtre est un vrai QDialog -- ce widget
    // n'en est pas un, donc Entrée ne déclenchait pas `backButton`, même focus (seule Espace
    // fonctionnait).
    for (QPushButton* const button : findChildren<QPushButton*>()) {
        button->setAutoDefault(true);
    }
}

LevelSelectScreen::~LevelSelectScreen() = default;

void LevelSelectScreen::setSequenceLevels(const std::vector<std::string>& sequence,
                                          const Progression& progression) {
    _sequenceEntries.clear();
    _sequenceEntries.reserve(sequence.size());
    for (const std::string& levelName : sequence) {
        _sequenceEntries.emplace_back(levelName, resolveState(progression, sequence, levelName));
    }
    if (_loc != nullptr) {
        rebuildSequenceList(*_loc);
    }
}

void LevelSelectScreen::setPersonalLevels(const std::vector<std::filesystem::path>& levels) {
    _ui->personalList->clear();
    for (const std::filesystem::path& path : levels) {
        auto* item = new QListWidgetItem(QString::fromStdString(path.filename().string()),
                                         _ui->personalList);
        item->setData(Qt::UserRole, QString::fromStdString(path.string()));
    }
}

void LevelSelectScreen::rebuildSequenceList(const Localization& loc) {
    _ui->sequenceList->clear();
    for (const auto& [levelName, state] : _sequenceEntries) {
        QString label = QString::fromStdString(levelName);
        switch (state) {
            case LevelSelectState::Completed:
                label += QStringLiteral(" ") +
                         QString::fromStdString(loc.text("level_select.completed_suffix"));
                break;
            case LevelSelectState::Current:
                break;  // pas de suffixe : c'est le tableau atteint, jouable par defaut.
            case LevelSelectState::Locked:
                label += QStringLiteral(" ") +
                         QString::fromStdString(loc.text("level_select.locked_suffix"));
                break;
        }
        auto* item = new QListWidgetItem(label, _ui->sequenceList);
        item->setData(Qt::UserRole, QString::fromStdString(levelName));
        if (state == LevelSelectState::Locked) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
    }
}

void LevelSelectScreen::focusDefaultAction() {
    _ui->backButton->setFocus();
}

void LevelSelectScreen::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _ui->titleLabel->setText(QString::fromStdString(loc.text("level_select.title")));
    _ui->backButton->setText(QString::fromStdString(loc.text("level_select.back")));
    _ui->tabs->setTabText(0, QString::fromStdString(loc.text("level_select.tab_sequence")));
    _ui->tabs->setTabText(1, QString::fromStdString(loc.text("level_select.tab_personal")));
    rebuildSequenceList(loc);
}

}  // namespace hmi
