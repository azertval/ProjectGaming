// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScrollArea>

class QWidget;

/**
 * @file HMI/Interface/ScreenPageHost.h
 * @brief Enveloppe défilante de toute page d'écran (`LOT-73`, `EX-IHM-080`).
 */

namespace hmi {

/**
 * @brief Conteneur défilant par lequel passe **toute** page ajoutée à la pile d'écrans.
 *
 * Un écran de l'application ne doit jamais contraindre la taille de la fenêtre : il s'y adapte, au
 * besoin en défilant (`EX-IHM-080`). Sans cette garantie, la taille minimale d'un écran dense
 * remonte jusqu'à la fenêtre — et `QStackedWidget::minimumSizeHint` étant le **maximum sur toutes
 * ses pages, y compris celles qu'on ne regarde pas**, un seul écran trop dense fixe le plancher de
 * la fenêtre entière, que Windows refuse ensuite de retailler.
 *
 * Le défaut s'est produit trois fois, corrigé deux fois écran par écran. C'est pourquoi la garantie
 * vit **ici**, sur le chemin d'ajout commun, et non dans chaque fichier `.ui` : un écran ajouté
 * plus tard en hérite sans que personne ait à y penser, là où une convention à réappliquer se
 * reperd au premier ajout.
 *
 * Deux mécanismes, complémentaires :
 * - une `QScrollArea` redimensionnable, qui fait **défiler** ce qui ne tient pas au lieu de le
 *   rogner ;
 * - une politique de taille `Ignored`, qui fait contribuer **zéro** à la taille minimale de la
 *   fenêtre. La `QScrollArea` seule ne suffirait pas : elle propage encore un plancher, petit mais
 *   non nul, et surtout rien n'empêcherait un futur appelant de poser une taille minimale sur la
 *   page elle-même.
 */
class ScreenPageHost : public QScrollArea {
    Q_OBJECT

public:
    /**
     * @brief Enveloppe @p page et en prend possession.
     * @param page   Page d'écran à héberger. L'enveloppe en devient parente.
     * @param parent Parent Qt de l'enveloppe.
     */
    explicit ScreenPageHost(QWidget* page, QWidget* parent = nullptr);

    /// @return La page hébergée, telle que passée à la construction.
    [[nodiscard]] QWidget* page() const;
};

}  // namespace hmi
