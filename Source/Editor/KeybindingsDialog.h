#pragma once

#include <array>
#include <filesystem>

#include <QDialog>

#include "HMI/Input/GameKeyBindings.h"

/**
 * @file Editor/KeybindingsDialog.h
 * @brief Remappage des touches de jeu (`EX-CTRL-012`) en Qt (LOT-38).
 */

class QLabel;
class QPushButton;

namespace editor {

/**
 * @brief Dialogue de remappage des **touches de jeu** (`GameKeyBindings`).
 *
 * Chaque action affiche sa touche courante ; cliquer « Modifier » capture la **prochaine touche
 * pressée** et l'assigne (échange automatique si la touche était déjà prise, `GameKeyBindings`).
 * Toute modification est **persistée** dans le fichier partagé (`keybindings.json`). Le dialogue
 * édite les bindings **en place** : la partie en cours en tient compte immédiatement (mêmes
 * bindings lus par la session de jeu). `Échap` annule une capture en cours.
 */
class KeybindingsDialog : public QDialog {
    Q_OBJECT

public:
    KeybindingsDialog(hmi::GameKeyBindings& bindings, std::filesystem::path savePath,
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
