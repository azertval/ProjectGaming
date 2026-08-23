// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <memory>

/**
 * @file HMI/Interface/CreditsScreen.h
 * @brief Écran « Crédits » du menu principal (`LOT-60`). Mise en page dans `CreditsScreen.ui`.
 */

namespace Ui {
class CreditsScreen;
}

namespace hmi {

class Localization;

/**
 * @brief Écran de crédits (développement, bruitages) atteint depuis le menu principal.
 *
 * Simple page du `QStackedWidget` (comme `MainMenu`/`OptionsPage`), pas un recouvrement : pas de
 * scène de jeu à laisser visible derrière. N'émet que l'intention de retour.
 */
class CreditsScreen : public QWidget {
    Q_OBJECT

public:
    explicit CreditsScreen(QWidget* parent = nullptr);
    ~CreditsScreen() override;

    /// Applique la langue active aux libellés (titre, intitulés de section, bouton retour) — les
    /// noms d'auteurs et licences restent inchangés d'une langue à l'autre.
    void retranslateUi(const Localization& loc);

    /// Donne le focus clavier au bouton « Retour » (navigation manette/clavier, même patron que
    /// `PauseScreen::focusDefaultAction`) : seul élément interactif de cet écran.
    void focusDefaultAction();

signals:
    void backRequested();

private:
    std::unique_ptr<Ui::CreditsScreen> _ui;
};

}  // namespace hmi
