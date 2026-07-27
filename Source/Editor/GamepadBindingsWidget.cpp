#include "Editor/GamepadBindingsWidget.h"

#include <array>
#include <optional>
#include <utility>

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>

#include "HMI/Input/GamepadButtonName.h"

namespace editor {

namespace {

// Libellés français des actions (même ordre que `hmi::GameAction`, partagés avec le clavier).
constexpr std::array<const char*, hmi::GAME_ACTION_COUNT> ACTION_LABELS{
    "Aller à gauche", "Aller à droite", "Viser haut", "Viser bas", "Sauter", "Dash"};

[[nodiscard]] hmi::GameAction actionAt(int index) {
    return static_cast<hmi::GameAction>(index);
}

}  // namespace

GamepadBindingsWidget::GamepadBindingsWidget(hmi::GamepadBindings& bindings,
                                             std::filesystem::path savePath, QWidget* parent)
    : QWidget(parent), _bindings(bindings), _savePath(std::move(savePath)), _timer(new QTimer(this)) {
    _timer->setInterval(30);
    connect(_timer, &QTimer::timeout, this, &GamepadBindingsWidget::onCaptureTick);

    auto* const form = new QFormLayout();
    for (int index = 0; index < hmi::GAME_ACTION_COUNT; ++index) {
        auto* const button = new QPushButton(this);
        button->setMinimumWidth(160);
        connect(button, &QPushButton::clicked, this, [this, index] { startCapture(index); });
        _buttons[static_cast<std::size_t>(index)] = button;
        form->addRow(QString::fromUtf8(ACTION_LABELS[static_cast<std::size_t>(index)]), button);
    }

    auto* const layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(new QLabel(
        QStringLiteral("Cliquez une action puis appuyez sur un bouton de la manette."), this));
    layout->addStretch();

    refresh();
}

void GamepadBindingsWidget::startCapture(int index) {
    _capturing = index;
    _buttons[static_cast<std::size_t>(index)]->setText(QStringLiteral("Appuyez sur un bouton…"));
    // Établit une ligne de base : un bouton déjà maintenu ne sera pas capturé (seul un nouvel appui
    // — front — l'est ensuite, cf. capturedGamepadButton).
    _poller.poll(_input);
    _input.beginFrame();
    _timer->start();
}

void GamepadBindingsWidget::onCaptureTick() {
    _poller.poll(_input);
    if (const std::optional<hmi::GamepadButton> button = hmi::capturedGamepadButton(_input)) {
        _bindings.setKey(actionAt(_capturing), *button);
        static_cast<void>(_bindings.save(_savePath));
        _capturing = -1;
        _timer->stop();
        refresh();
        return;
    }
    _input.beginFrame();  // avance les fronts pour la détection au prochain tick
}

void GamepadBindingsWidget::refresh() {
    for (int index = 0; index < hmi::GAME_ACTION_COUNT; ++index) {
        _buttons[static_cast<std::size_t>(index)]->setText(
            QString::fromStdString(hmi::gamepadButtonDisplayName(_bindings.button(actionAt(index)))));
    }
}

}  // namespace editor
