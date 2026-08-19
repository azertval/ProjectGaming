#include "HMI/Interface/PixelMenuButton.h"

#include <algorithm>
#include <vector>

#include <QColor>
#include <QPainter>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/PixelFrameGeometry.h"

namespace hmi {

PixelMenuButton::PixelMenuButton(QWidget* parent) : QPushButton(parent) {
    setFlat(true);
    setCursor(Qt::PointingHandCursor);
}

void PixelMenuButton::enterEvent(QEnterEvent* event) {
    QPushButton::enterEvent(event);
    update();
}

void PixelMenuButton::leaveEvent(QEvent* event) {
    QPushButton::leaveEvent(event);
    update();
}

void PixelMenuButton::paintEvent(QPaintEvent* event) {
    // Le bouton se peint d'abord normalement : couleur, police, marges et etat desactive viennent
    // tous de theme.qss. On n'ajoute que la marque, par-dessus.
    QPushButton::paintEvent(event);

    // Un bouton indisponible ne porte jamais le curseur, meme survole : il indiquerait une action
    // qui n'aura pas lieu.
    if (!isEnabled() || (!hasFocus() && !underMouse())) {
        return;
    }

    const int scale = identityScale();
    // Gouttiere reservee au curseur : exactement la marge gauche que theme.qss donne a ces boutons
    // (identity.space.medium), de sorte que la marque se loge devant le texte sans le decaler.
    const int gutter = identityBaseScale().spaceMedium * scale;
    const int caretSize = identityBaseScale().sectionTitle * scale;
    const std::vector<PixelFrameQuad> caret = pixelCaretQuads(caretSize);
    if (caret.empty()) {
        return;
    }

    // Largeur reelle du dessin : la rangee la plus large (quatre pas sur huit rangees).
    int caretWidth = 0;
    int caretHeight = 0;
    for (const PixelFrameQuad& quad : caret) {
        caretWidth = std::max(caretWidth, quad.x + quad.width);
        caretHeight = std::max(caretHeight, quad.y + quad.height);
    }
    // Centre dans la gouttiere, et verticalement dans le bouton. Un curseur cale en haut
    // « flotterait » au-dessus du texte des que la hauteur de ligne grandit.
    const int originX = std::max(0, (gutter - caretWidth) / 2);
    const int originY = (height() - caretHeight) / 2;

    const QColor accent(identityTokens().color.accent.r, identityTokens().color.accent.g,
                        identityTokens().color.accent.b);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);  // EX-IHM-053 : aucun bord adouci.
    for (const PixelFrameQuad& quad : caret) {
        painter.fillRect(originX + quad.x, originY + quad.y, quad.width, quad.height, accent);
    }
}

}  // namespace hmi
