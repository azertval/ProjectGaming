// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/PropertiesPanel.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QString>
#include <algorithm>

#include "Core/Levels/LevelDraft.h"
#include "HMI/Localization/Localization.h"
#include "ui_PropertiesPanel.h"

namespace hmi {

namespace {

// Valeur sentinelle des spinbox de capacite : -1 signifie "s'en remettre au reglage du moteur",
// affichee "Par defaut" via specialValueText (meme patron que les spinbox de cadrage camera).
constexpr int CAPABILITY_DEFAULT = -1;

[[nodiscard]] std::optional<int> capabilityFromSpin(int value) {
    return value == CAPABILITY_DEFAULT ? std::nullopt : std::make_optional(value);
}

[[nodiscard]] int spinFromCapability(const std::optional<int>& capability) {
    return capability.value_or(CAPABILITY_DEFAULT);
}

}  // namespace

PropertiesPanel::PropertiesPanel(QWidget* parent) : QWidget(parent), _ui(new Ui::PropertiesPanel) {
    _ui->setupUi(this);
    _ui->platformModeCombo->addItem(QString{});  // aller-retour
    _ui->platformModeCombo->addItem(QString{});  // boucle
    _ui->moverAxisCombo->addItem(QString{});     // horizontal
    _ui->moverAxisCombo->addItem(QString{});     // vertical
    connectFields();
}

PropertiesPanel::~PropertiesPanel() {
    delete _ui;
}

void PropertiesPanel::connectFields() {
    // editingFinished, jamais valueChanged : taper "120" ne doit produire qu'UNE mutation, donc un
    // seul pas d'annulation -- sinon chaque frappe en empilerait un (1, 12, 120).
    const auto guard = [this](auto&& emitter) {
        return [this, emitter]() {
            if (_updating || !_target) {
                return;
            }
            emitter(*_target);
        };
    };

    connect(_ui->platformSpeedSpin, &QDoubleSpinBox::editingFinished, this,
            guard([this](core::GridPosition target) {
                emit platformSpeedChanged(target,
                                          static_cast<float>(_ui->platformSpeedSpin->value()));
            }));
    connect(_ui->platformPhaseSpin, &QSpinBox::editingFinished, this,
            guard([this](core::GridPosition target) {
                emit platformPhaseChanged(target, _ui->platformPhaseSpin->value());
            }));
    // Une liste deroulante change en un seul geste : currentIndexChanged n'a pas le probleme de
    // frappe intermediaire des champs numeriques.
    connect(_ui->platformModeCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (_updating || !_target) {
            return;
        }
        emit platformModeChanged(
            *_target, index == 1 ? core::PlatformPathMode::Loop : core::PlatformPathMode::PingPong);
    });

    // Axe et portee d'un danger mobile voyagent ENSEMBLE : setMoverConfig remplace la
    // configuration entiere, envoyer l'un sans l'autre ecraserait le second avec sa valeur par
    // defaut.
    const auto emitMover = [this]() {
        if (_updating || !_target) {
            return;
        }
        emit moverConfigChanged(*_target,
                                _ui->moverAxisCombo->currentIndex() == 1
                                    ? core::DangerMoverAxis::Vertical
                                    : core::DangerMoverAxis::Horizontal,
                                _ui->moverRangeSpin->value());
    };
    connect(_ui->moverAxisCombo, &QComboBox::currentIndexChanged, this, emitMover);
    connect(_ui->moverRangeSpin, &QSpinBox::editingFinished, this, emitMover);

    // Meme raison pour les trois champs d'un danger temporise.
    const auto emitBlink = [this]() {
        if (_updating || !_target) {
            return;
        }
        emit blinkConfigChanged(*_target, _ui->blinkPeriodSpin->value(),
                                _ui->blinkPhaseSpin->value(), _ui->blinkActiveSpin->value());
    };
    connect(_ui->blinkPeriodSpin, &QSpinBox::editingFinished, this, emitBlink);
    connect(_ui->blinkPhaseSpin, &QSpinBox::editingFinished, this, emitBlink);
    connect(_ui->blinkActiveSpin, &QSpinBox::editingFinished, this, emitBlink);

    // Regles du tableau : independantes de la selection, donc sans garde sur _target.
    connect(_ui->jumpBudgetSpin, &QSpinBox::editingFinished, this, [this]() {
        if (!_updating) {
            emit jumpBudgetChanged(_ui->jumpBudgetSpin->value());
        }
    });
    connect(_ui->dashBudgetSpin, &QSpinBox::editingFinished, this, [this]() {
        if (!_updating) {
            emit dashBudgetChanged(_ui->dashBudgetSpin->value());
        }
    });
    connect(_ui->airJumpsSpin, &QSpinBox::editingFinished, this, [this]() {
        if (!_updating) {
            emit airJumpsChanged(capabilityFromSpin(_ui->airJumpsSpin->value()));
        }
    });
    connect(_ui->dashChargesSpin, &QSpinBox::editingFinished, this, [this]() {
        if (!_updating) {
            emit dashChargesChanged(capabilityFromSpin(_ui->dashChargesSpin->value()));
        }
    });
}

void PropertiesPanel::refresh(const core::LevelDraft& draft, std::optional<PathSelection> selection,
                              std::optional<core::GridPosition> blinkCell) {
    _updating = true;  // les setValue ci-dessous ne doivent pas reboucler en mutations
    _target.reset();

    if (selection && selection->kind == PathTargetKind::Platform &&
        selection->index < draft.platformConfigs().size()) {
        const core::MovingPlatformConfig& config = draft.platformConfigs()[selection->index];
        _target = config.startPosition;
        _ui->selectionStack->setCurrentIndex(PagePlatform);
        _ui->platformSpeedSpin->setValue(static_cast<double>(config.speed));
        _ui->platformPhaseSpin->setValue(config.phase);
        _ui->platformModeCombo->setCurrentIndex(config.mode == core::PlatformPathMode::Loop ? 1
                                                                                            : 0);
        _ui->platformWaypointsLabel->setText(
            _loc == nullptr ? QString::number(static_cast<int>(config.waypoints.size()))
                            : QString::fromStdString(_loc->text("properties.platform_waypoints"))
                                  .arg(static_cast<int>(config.waypoints.size())));
    } else if (selection && selection->kind == PathTargetKind::Mover &&
               selection->index < draft.moverConfigs().size()) {
        const core::DangerMoverConfig& config = draft.moverConfigs()[selection->index];
        _target = config.startPosition;
        _ui->selectionStack->setCurrentIndex(PageMover);
        _ui->moverAxisCombo->setCurrentIndex(config.axis == core::DangerMoverAxis::Vertical ? 1
                                                                                            : 0);
        _ui->moverRangeSpin->setValue(config.range);
    } else if (blinkCell) {
        // Un danger temporise n'a pas de trajectoire, donc pas de PathSelection : il est designe
        // par sa case. Sans configuration explicite, on affiche les valeurs de conception par
        // defaut, celles que LevelLoader appliquera au rechargement.
        const auto found = std::find_if(draft.blinkConfigs().begin(), draft.blinkConfigs().end(),
                                        [cell = *blinkCell](const core::DangerBlinkConfig& config) {
                                            return config.position == cell;
                                        });
        const core::DangerBlinkConfig config =
            found != draft.blinkConfigs().end() ? *found
                                                : core::DangerBlinkConfig{.position = *blinkCell};
        _target = *blinkCell;
        _ui->selectionStack->setCurrentIndex(PageBlink);
        _ui->blinkPeriodSpin->setValue(config.period);
        _ui->blinkPhaseSpin->setValue(config.phase);
        _ui->blinkActiveSpin->setValue(config.activeDuration);
    } else {
        _ui->selectionStack->setCurrentIndex(PageEmpty);
    }

    _ui->jumpBudgetSpin->setValue(draft.jumpBudget());
    _ui->dashBudgetSpin->setValue(draft.dashBudget());
    _ui->airJumpsSpin->setValue(spinFromCapability(draft.airJumps()));
    _ui->dashChargesSpin->setValue(spinFromCapability(draft.dashCharges()));
    _updating = false;
}

void PropertiesPanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    const auto text = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };
    _ui->selectionGroup->setTitle(text("properties.selection_group"));
    _ui->emptyLabel->setText(text("properties.no_selection"));
    _ui->platformSpeedLabel->setText(text("properties.platform_speed"));
    _ui->platformPhaseLabel->setText(text("properties.platform_phase"));
    _ui->platformModeLabel->setText(text("properties.platform_mode"));
    _ui->moverAxisLabel->setText(text("properties.mover_axis"));
    _ui->moverRangeLabel->setText(text("properties.mover_range"));
    _ui->blinkPeriodLabel->setText(text("properties.blink_period"));
    _ui->blinkPhaseLabel->setText(text("properties.blink_phase"));
    _ui->blinkActiveLabel->setText(text("properties.blink_active"));
    _ui->budgetGroup->setTitle(text("properties.budget_group"));
    _ui->jumpBudgetLabel->setText(text("properties.jump_budget"));
    _ui->dashBudgetLabel->setText(text("properties.dash_budget"));
    _ui->capabilityGroup->setTitle(text("properties.capability_group"));
    _ui->airJumpsLabel->setText(text("properties.air_jumps"));
    _ui->dashChargesLabel->setText(text("properties.dash_charges"));
    _ui->jumpBudgetSpin->setSpecialValueText(text("properties.unlimited"));
    _ui->dashBudgetSpin->setSpecialValueText(text("properties.unlimited"));
    _ui->airJumpsSpin->setSpecialValueText(text("properties.engine_default"));
    _ui->dashChargesSpin->setSpecialValueText(text("properties.engine_default"));

    // Les listes deroulantes sont repeuplees sans emettre : retraduire ne doit jamais passer pour
    // un changement de valeur de l'utilisateur.
    {
        const QSignalBlocker blockMode(_ui->platformModeCombo);
        _ui->platformModeCombo->setItemText(0, text("properties.mode_pingpong"));
        _ui->platformModeCombo->setItemText(1, text("properties.mode_loop"));
    }
    {
        const QSignalBlocker blockAxis(_ui->moverAxisCombo);
        _ui->moverAxisCombo->setItemText(0, text("properties.axis_horizontal"));
        _ui->moverAxisCombo->setItemText(1, text("properties.axis_vertical"));
    }
}

}  // namespace hmi
