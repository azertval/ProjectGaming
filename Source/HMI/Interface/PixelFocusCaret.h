// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

/**
 * @file HMI/Interface/PixelFocusCaret.h
 * @brief Marque explicite de focus des écrans du jeu (`LOT-68`, `EX-IHM-071`), pour les contrôles
 *        qui ne la peignent pas eux-mêmes.
 */

namespace hmi {

/**
 * @brief Curseur de focus flottant, posé à gauche du contrôle focalisé de l'écran hôte.
 *
 * `EX-IHM-071` demande que l'élément focalisé d'un écran du jeu soit signalé par une **marque
 * explicite**, et non par la seule teinte : la navigation à la manette repose entièrement sur le
 * parcours de focus, qu'une nuance de couleur rend difficile à suivre — et impossible pour un
 * joueur qui distingue mal les couleurs. Une feuille de style ne sachant pas ajouter de contenu,
 * cette marque est nécessairement peinte.
 *
 * `PixelMenuButton` la peint lui-même, dans sa propre gouttière. Ce widget couvre l'autre cas :
 * les contrôles **ordinaires** de Qt (boutons d'action, listes, onglets, champs numériques), qu'on
 * ne peut pas tous sous-classer. Il se place **à côté** du contrôle focalisé plutôt que dedans,
 * dans le repère de l'écran hôte, et se hisse au-dessus de ses frères.
 *
 * Transparent aux événements de souris : il ne s'interpose jamais entre le joueur et le contrôle
 * qu'il désigne.
 *
 * Le suivi du focus reste à la charge de l'hôte (`follow`), qui seul sait quels contrôles lui
 * appartiennent — un `QApplication::focusChanged` global désignerait aussi ceux des autres écrans.
 */
class PixelFocusCaret : public QWidget {
    Q_OBJECT

public:
    /// @param host Écran suivi ; le curseur en devient un enfant, et vit dans son repère.
    explicit PixelFocusCaret(QWidget* host);

    /**
     * @brief Place le curseur devant @p focused, ou le masque si le focus a quitté l'hôte.
     * @param focused Contrôle actuellement focalisé (peut être nul, ou hors de l'hôte).
     */
    void follow(QWidget* focused);

protected:
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace hmi
