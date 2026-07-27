#include "Editor/KeybindingsDialog.h"

#include <array>
#include <utility>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "Editor/QtKeyMap.h"
#include "HMI/Input/KeyName.h"

namespace editor {

namespace {

// Libellés français des actions de jeu (même ordre que `hmi::GameAction`).
constexpr std::array<const char*, hmi::GAME_ACTION_COUNT> ACTION_LABELS{
    "Aller à gauche", "Aller à droite", "Viser haut", "Viser bas", "Sauter", "Dash"};

[[nodiscard]] hmi::GameAction actionAt(int index) {
    return static_cast<hmi::GameAction>(index);
}

}  // namespace

KeybindingsDialog::KeybindingsDialog(hmi::GameKeyBindings& bindings, std::filesystem::path savePath,
                                     QWidget* parent)
    : QDialog(parent), _bindings(bindings), _savePath(std::move(savePath)) {
    setWindowTitle(QStringLiteral("Touches — Jeu"));

    auto* const form = new QFormLayout();
    for (int index = 0; index < hmi::GAME_ACTION_COUNT; ++index) {
        auto* const button = new QPushButton(this);
        button->setMinimumWidth(120);
        connect(button, &QPushButton::clicked, this, [this, index] {
            _capturing = index;
            _buttons[static_cast<std::size_t>(index)]->setText(QStringLiteral("Appuyez…"));
            setFocus();  // capter la prochaine touche via keyPressEvent
        });
        _buttons[static_cast<std::size_t>(index)] = button;
        form->addRow(QString::fromUtf8(ACTION_LABELS[static_cast<std::size_t>(index)]), button);
    }

    auto* const close = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(close, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(close, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto* const layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(new QLabel(
        QStringLiteral("Cliquez une action puis appuyez sur la nouvelle touche (Échap annule)."),
        this));
    layout->addWidget(close);

    refresh();
}

void KeybindingsDialog::refresh() {
    for (int index = 0; index < hmi::GAME_ACTION_COUNT; ++index) {
        _buttons[static_cast<std::size_t>(index)]->setText(
            QString::fromStdString(hmi::keyDisplayName(_bindings.key(actionAt(index)))));
    }
}

void KeybindingsDialog::keyPressEvent(QKeyEvent* event) {
    if (_capturing < 0) {
        QDialog::keyPressEvent(event);
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        _capturing = -1;  // annule la capture, restitue l'affichage
        refresh();
        return;
    }
    if (const std::optional<hmi::Key> key = qtKeyToHmiKey(event->key())) {
        _bindings.setKey(actionAt(_capturing), *key);
        static_cast<void>(_bindings.save(_savePath));  // persistance (échec non bloquant)
        _capturing = -1;
        refresh();
    }
}

}  // namespace editor
