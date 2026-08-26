// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/PixelFocusCaret.h"

#include <QPainter>
#include <algorithm>
#include <vector>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/PixelFrameGeometry.h"

namespace hmi {

namespace {

/// Cote du carre englobant du curseur, a l'echelle courante -- meme grandeur que celle employee
/// par `PixelMenuButton`, pour que la marque soit la meme d'un ecran a l'autre.
[[nodiscard]] int caretSide() {
    return identityBaseScale().sectionTitle * identityScale();
}

/// Ecart entre le curseur et le controle qu'il designe.
[[nodiscard]] int caretGap() {
    return identityBaseScale().spaceSmall * identityScale();
}

}  // namespace

PixelFocusCaret::PixelFocusCaret(QWidget* host) : QWidget(host) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    hide();
}

void PixelFocusCaret::follow(QWidget* focused) {
    QWidget* const host = parentWidget();
    // Focus sorti de l'ecran hote (ou pose sur l'hote lui-meme, qui n'est pas un controle) : la
    // marque n'a plus rien a designer.
    if (host == nullptr || focused == nullptr || focused == host || !host->isAncestorOf(focused) ||
        !focused->isVisible()) {
        hide();
        return;
    }

    const int side = caretSide();
    const std::vector<PixelFrameQuad> caret = pixelCaretQuads(side);
    if (caret.empty()) {
        hide();  // echelle trop petite pour un curseur non deforme (voir pixelCaretQuads).
        return;
    }

    // Position du controle dans le repere de l'hote : `focused` peut etre a plusieurs niveaux de
    // profondeur (onglet, disposition imbriquee), d'ou le passage par les coordonnees globales.
    const QPoint topLeft = host->mapFromGlobal(focused->mapToGlobal(QPoint(0, 0)));
    const int x = topLeft.x() - side - caretGap();
    const int y = topLeft.y() + ((focused->height() - side) / 2);

    // Un controle colle au bord gauche ne laisse pas la place : la marque est alors inutilisable,
    // et la masquer vaut mieux que la peindre par-dessus un voisin.
    if (x < 0) {
        hide();
        return;
    }

    setGeometry(x, y, side, side);
    raise();
    show();
    update();
}

void PixelFocusCaret::paintEvent(QPaintEvent* event) {
    (void)event;
    const std::vector<PixelFrameQuad> caret = pixelCaretQuads(caretSide());
    if (caret.empty()) {
        return;
    }

    const DesignColor accent = identityTokens().color.accent;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);  // EX-IHM-053 : aucun bord adouci.
    for (const PixelFrameQuad& quad : caret) {
        painter.fillRect(quad.x, quad.y, quad.width, quad.height,
                         QColor(accent.r, accent.g, accent.b));
    }
}

}  // namespace hmi
