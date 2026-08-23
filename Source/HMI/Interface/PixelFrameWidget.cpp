// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/PixelFrameWidget.h"

#include <QColor>
#include <QPainter>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/PixelFrameGeometry.h"

namespace hmi {

namespace {

[[nodiscard]] QColor toQColor(DesignColor color) {
    return QColor(color.r, color.g, color.b, color.a);
}

}  // namespace

PixelFrameWidget::PixelFrameWidget(QWidget* parent) : QWidget(parent) {
    // Le cadre est peint ici, pas par la feuille de style : sans fond propre, Qt laisserait
    // transparaitre ce qui se trouve derriere entre les paves.
    setAttribute(Qt::WA_StyledBackground, false);
}

void PixelFrameWidget::setAccented(bool accented) {
    if (_accented == accented) {
        return;
    }
    _accented = accented;
    update();
}

void PixelFrameWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    // Jetons de la portee IDENTITE, jamais ceux du chassis : ces cadres n'existent que sur les
    // ecrans du jeu, dont l'apparence ne suit aucun reglage de theme (EX-IHM-054).
    const ColorTokens& color = identityTokens().color;
    QPainter painter(this);
    // Aucun anticrenelage : le pixel art se rend au plus proche voisin (EX-IHM-053), et un bord
    // adouci trahirait la grille.
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);

    for (const PixelFrameQuad& quad : pixelFrameQuads(width(), height(), identityScale())) {
        QColor fill;
        switch (quad.role) {
            case PixelFrameRole::Fill:
                fill = toQColor(color.surface);
                break;
            case PixelFrameRole::Outline:
                fill = toQColor(color.outline);
                break;
            case PixelFrameRole::BevelLight:
                // Variante accentuee : seul le biseau CLAIR passe a l'accent. Accentuer les deux
                // ferait perdre le relief, qui vient precisement de leur difference.
                fill = _accented ? toQColor(color.accent) : toQColor(color.bevelLight);
                break;
            case PixelFrameRole::BevelDark:
                fill = toQColor(color.bevelDark);
                break;
        }
        painter.fillRect(quad.x, quad.y, quad.width, quad.height, fill);
    }
}

}  // namespace hmi
