#pragma once

#include <QWidget>
#include <memory>

/**
 * @file HMI/Interface/MainMenu.h
 * @brief Menu principal de l'application Qt (LOT-38). Mise en page dans `MainMenu.ui` (Qt
 * Designer).
 */

namespace Ui {
class MainMenu;
}

namespace hmi {

class Localization;

/**
 * @brief Menu principal : point d'entrée de l'application Qt (`EX-IHM-040`).
 *
 * La **mise en page** est décrite dans `MainMenu.ui` (éditable dans Qt Designer, compilée par
 * `uic`) ; ce code ne porte que la **logique** (émission des intentions). Le **thème** vient de
 * `resources/theme.qss`. N'émet que des intentions (jouer, éditer, options, quitter) — la
 * navigation est appliquée par `MainWindow`.
 */
class MainMenu : public QWidget {
    Q_OBJECT

public:
    explicit MainMenu(QWidget* parent = nullptr);
    ~MainMenu() override;

    /// Applique la langue active aux libellés (titre et boutons).
    void retranslateUi(const Localization& loc);

signals:
    void playRequested();
    void editorRequested();
    void optionsRequested();
    void quitRequested();

private:
    std::unique_ptr<Ui::MainMenu> _ui;
};

}  // namespace hmi
