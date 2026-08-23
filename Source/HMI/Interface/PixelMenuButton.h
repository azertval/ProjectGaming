// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPushButton>

/**
 * @file HMI/Interface/PixelMenuButton.h
 * @brief Entrée de menu des écrans du jeu, portant le curseur de focus (`LOT-68`, `EX-IHM-071`).
 *
 * Une feuille de style ne peut pas **ajouter de contenu** : elle sait changer une teinte, pas
 * dessiner une marque. Or la teinte seule ne suffit pas à dire où l'on en est quand on navigue à
 * la manette, faute de pointeur — et elle ne dit rien du tout à qui distingue mal les couleurs.
 * D'où ce bouton, qui peint un curseur devant l'entrée active.
 *
 * **Promu dans les `.ui`** : la mise en page reste déclarative, le tracé seul est en C++. Tout le
 * reste de l'habillage (couleur, taille, graisse, marges) continue de venir de `theme.qss`.
 */

namespace hmi {

class PixelMenuButton : public QPushButton {
    Q_OBJECT

public:
    explicit PixelMenuButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

    /// Le curseur suit le **survol** autant que le focus clavier : sans cela, promener la souris
    /// sur le menu changerait la couleur d'une entrée sans y amener la marque, et les deux
    /// indications se contrediraient.
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
};

}  // namespace hmi
