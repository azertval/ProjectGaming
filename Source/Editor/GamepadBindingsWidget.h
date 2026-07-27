#pragma once

#include <array>
#include <filesystem>

#include <QWidget>

#include "HMI/Input/GameKeyBindings.h"     // GAME_ACTION_COUNT
#include "HMI/Input/GamepadBindings.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"

/**
 * @file Editor/GamepadBindingsWidget.h
 * @brief Remappage des boutons de manette (`EX-CTRL-002`), en widget (onglet Options, LOT-38).
 */

class QPushButton;
class QTimer;

namespace editor {

/**
 * @brief Widget de remappage des **boutons manette** (`GamepadBindings`), onglet « Commande
 *        manette » de la page Options.
 *
 * Chaque action affiche son bouton courant ; cliquer une action **sonde XInput** (via
 * `hmi::GamepadPoller`, dans un `QTimer`) jusqu'au prochain bouton pressé, puis l'assigne (échange
 * auto). Persisté dans `keybindings.json`. La partie en cours en tient compte immédiatement.
 * La page Options n'étant affichée que hors jeu (viewport masqué, boucle en pause), aucun sondage
 * XInput concurrent.
 */
class GamepadBindingsWidget : public QWidget {
    Q_OBJECT

public:
    GamepadBindingsWidget(hmi::GamepadBindings& bindings, std::filesystem::path savePath,
                          QWidget* parent = nullptr);

private:
    void startCapture(int index);
    void onCaptureTick();
    void refresh();

    hmi::GamepadBindings& _bindings;
    std::filesystem::path _savePath;
    int _capturing = -1;
    hmi::GamepadPoller _poller;
    hmi::InputState _input;
    QTimer* _timer;
    std::array<QPushButton*, hmi::GAME_ACTION_COUNT> _buttons;
};

}  // namespace editor
