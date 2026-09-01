// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/ScreenPageHost.h"

#include <QSizePolicy>
#include <QWidget>

namespace hmi {

ScreenPageHost::ScreenPageHost(QWidget* page, QWidget* parent) : QScrollArea(parent) {
    setObjectName(QStringLiteral("ScreenPageHost"));

    // La page suit la largeur du conteneur : le defilement reste VERTICAL dans le cas courant, une
    // page qui se re-agence en largeur n'ayant aucune raison d'imposer une barre horizontale.
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Contribution NULLE a la taille minimale de la fenetre (EX-IHM-080). `Ignored` est la seule
    // politique qui le dise vraiment : elle ecarte a la fois la taille ideale et la taille minimale
    // de la page du calcul du parent. Une simple QScrollArea propage encore un plancher.
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    // Le fond appartient a la PAGE, jamais a l'enveloppe : les ecrans de la portee identite posent
    // le leur par `objectName` dans le theme, et une enveloppe opaque le recouvrirait d'un aplat
    // gris. `widgetResizable` garantissant que la page occupe tout le viewport, aucun vide n'est a
    // peindre.
    viewport()->setAutoFillBackground(false);
    setAutoFillBackground(false);

    if (page != nullptr) {
        setWidget(page);
    }
}

QWidget* ScreenPageHost::page() const {
    return widget();
}

}  // namespace hmi
