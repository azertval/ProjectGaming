#pragma once

#include <QWidget>

/**
 * @file Editor/MainMenu.h
 * @brief Menu principal de l'application Qt (LOT-38).
 */

namespace editor {

/**
 * @brief Menu principal : point d'entrée de l'application Qt (`EX-IHM-040`).
 *
 * Remplace le `MenuScreen` « maison ». N'émet que des **intentions** (jouer, éditer, options,
 * quitter) — la navigation est appliquée par `MainWindow`, comme l'ancien couple
 * `IScreen`/`ScreenManager` mais sans cette infrastructure.
 */
class MainMenu : public QWidget {
    Q_OBJECT

public:
    explicit MainMenu(QWidget* parent = nullptr);

signals:
    void playRequested();
    void editorRequested();
    void optionsRequested();
    void quitRequested();
};

}  // namespace editor
