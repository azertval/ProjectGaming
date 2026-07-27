#pragma once

#include <array>
#include <filesystem>

#include <QWidget>

#include "HMI/Input/GameKeyBindings.h"

/**
 * @file Editor/KeybindingsWidget.h
 * @brief Remappage des touches de jeu (`EX-CTRL-012`), en widget embarquable (onglet Options, LOT-38).
 */

class QPushButton;

namespace editor {

/**
 * @brief Widget de remappage des **touches de jeu** (`GameKeyBindings`), destiné à l'onglet
 *        « Commande clavier » de la page Options.
 *
 * Chaque action affiche sa touche courante ; cliquer une action capture la **prochaine touche
 * pressée** et l'assigne (échange auto si déjà prise). Toute modification est **persistée** dans le
 * fichier partagé (`keybindings.json`) ; la partie en cours en tient compte immédiatement (mêmes
 * bindings lus par la session de jeu). `Échap` annule une capture en cours.
 */
class KeybindingsWidget : public QWidget {
    Q_OBJECT

public:
    KeybindingsWidget(hmi::GameKeyBindings& bindings, std::filesystem::path savePath,
                      QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void refresh();

    hmi::GameKeyBindings& _bindings;
    std::filesystem::path _savePath;
    int _capturing = -1;  ///< Indice de l'action en cours de capture, ou -1.
    std::array<QPushButton*, hmi::GAME_ACTION_COUNT> _buttons;
};

}  // namespace editor
