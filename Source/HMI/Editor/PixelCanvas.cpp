#include "HMI/Editor/PixelCanvas.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <cstring>

#include "HMI/Interface/DesignTokens.h"

namespace hmi {

namespace {

// Convertit des pixels RGBA decodes en QImage. Meme conversion que PalettePanel/
// AssetThumbnailView (LOT-42/LOT-43) : trop petite pour justifier une fonction partagee entre
// widgets independants.
[[nodiscard]] QImage toQImage(const DecodedImage& decoded) {
    QImage image(decoded.width, decoded.height, QImage::Format_RGBA8888);
    for (int y = 0; y < decoded.height; ++y) {
        const std::uint32_t* const row =
            decoded.pixels.data() + static_cast<std::size_t>(y) * decoded.width;
        std::memcpy(image.scanLine(y), row, static_cast<std::size_t>(decoded.width) * 4);
    }
    return image;
}

[[nodiscard]] QColor toQColor(DesignColor color) noexcept {
    return QColor(color.r, color.g, color.b);
}

// Cote d'une case du damier de transparence, en pixels LOGIQUES (espace ecran, TACHE-03 point
// d'attention) : fixe, independant du zoom de l'oeuvre -- sinon il zoome avec le dessin et devient
// illisible aux forts facteurs.
constexpr int CHECKER_LOGICAL_CELL = 6;

// Correspondance outil -> type d'operation d'historique (LOT-54 TACHE-02). L'outil pipette ne
// mute jamais l'image (aucune region non vide n'est jamais poussee pour lui) : la valeur de repli
// n'est donc jamais observee, mais /W4 impose un switch exhaustif.
[[nodiscard]] PixelOperationKind operationKindForTool(PixelTool tool) noexcept {
    switch (tool) {
        case PixelTool::Brush:
            return PixelOperationKind::Brush;
        case PixelTool::Eraser:
            return PixelOperationKind::Eraser;
        case PixelTool::Fill:
            return PixelOperationKind::Fill;
        case PixelTool::Eyedropper:
            return PixelOperationKind::Brush;
    }
    return PixelOperationKind::Brush;
}

}  // namespace

PixelCanvas::PixelCanvas(QWidget* parent) : QWidget(parent) {
    // Suivi de souris SANS bouton presse : necessaire pour que le pixel survole (barre d'etat,
    // TACHE-04) se mette a jour au simple deplacement, pas seulement pendant un geste de dessin.
    setMouseTracking(true);
    // Cible reelle du clavier (raccourcis d'outil) quand l'utilisateur clique dans le canevas --
    // c'est ce changement de focus que MainWindow observe pour reassigner le contexte d'edition
    // actif (TACHE-04, EX-IHM-062).
    setFocusPolicy(Qt::StrongFocus);
}

void PixelCanvas::setImage(DecodedImage image) {
    _image = std::move(image);
    _history = PixelHistory();
    _gestureActive = false;
    _lastGesturePixel.reset();
    if (_hoveredPixel) {
        _hoveredPixel.reset();
        emit hoveredPixelChanged(_hoveredPixel);
    }
    updateGeometry();
    update();
    emit imageChanged();
    emit historyChanged();
}

void PixelCanvas::undo() {
    if (_history.undo(_image)) {
        update();
        emit imageChanged();
        emit historyChanged();
    }
}

void PixelCanvas::redo() {
    if (_history.redo(_image)) {
        update();
        emit imageChanged();
        emit historyChanged();
    }
}

void PixelCanvas::jumpHistoryTo(std::size_t index) {
    if (_history.jumpTo(_image, index)) {
        update();
        emit imageChanged();
        emit historyChanged();
    }
}

void PixelCanvas::zoomIn() {
    _view.zoom = pixelCanvasZoomIn(_view.zoom);
    updateGeometry();
    update();
}

void PixelCanvas::zoomOut() {
    _view.zoom = pixelCanvasZoomOut(_view.zoom);
    updateGeometry();
    update();
}

std::optional<std::pair<int, int>> PixelCanvas::imagePixelAt(const QPointF& widgetPosition) const {
    return screenToImagePixel(_view, _image.width, _image.height, widgetPosition.x(),
                              widgetPosition.y());
}

QSize PixelCanvas::sizeHint() const {
    return QSize(std::max(1, _image.width * _view.zoom), std::max(1, _image.height * _view.zoom));
}

void PixelCanvas::applyToolAtPoint(int x, int y) {
    PixelRegion touched;
    switch (_activeTool) {
        case PixelTool::Brush:
            touched = setPixel(_image, x, y, _currentColor);
            break;
        case PixelTool::Eraser:
            touched = erasePixel(_image, x, y);
            break;
        case PixelTool::Fill:
            touched = floodFill(_image, x, y, _currentColor);
            break;
        case PixelTool::Eyedropper:
            // Ne mute jamais l'image : prelevement seul, aucune entree d'historique.
            if (const std::optional<std::uint32_t> picked = pickColor(_image, x, y)) {
                _currentColor = *picked;
            }
            return;
    }
    _gestureRegion = unionPixelRegion(_gestureRegion, touched);
}

void PixelCanvas::continueToolTo(int x, int y) {
    const auto [lastX, lastY] = _lastGesturePixel.value_or(std::make_pair(x, y));
    PixelRegion touched;
    switch (_activeTool) {
        case PixelTool::Brush:
            touched = drawLine(_image, lastX, lastY, x, y, _currentColor);
            break;
        case PixelTool::Eraser:
            touched = eraseLine(_image, lastX, lastY, x, y);
            break;
        case PixelTool::Fill:
        case PixelTool::Eyedropper:
            // Geste ponctuel : le pot de peinture et la pipette ignorent le glisser.
            return;
    }
    _gestureRegion = unionPixelRegion(_gestureRegion, touched);
}

void PixelCanvas::beginGesture(int x, int y) {
    _gestureActive = true;
    _gestureBeforeSnapshot = _image;
    _gestureRegion = PixelRegion{};
    applyToolAtPoint(x, y);
    _lastGesturePixel = std::make_pair(x, y);
    update();
}

void PixelCanvas::endGesture() {
    if (_gestureActive && !_gestureRegion.empty()) {
        // Un geste complet (appui, glisser, relachement) produit UNE seule entree d'historique,
        // jamais une par deplacement de souris (TACHE-02/TACHE-03) : sinon annuler deviendrait
        // inutilisable et l'apercu live (TACHE-08) invaliderait le cache a chaque pixel.
        const std::vector<std::uint32_t> before = readRegion(_gestureBeforeSnapshot, _gestureRegion);
        const std::vector<std::uint32_t> after = readRegion(_image, _gestureRegion);
        _history.push(operationKindForTool(_activeTool), _gestureRegion, before, after);
        emit imageChanged();
        emit historyChanged();
    }
    _gestureActive = false;
    _lastGesturePixel.reset();
}

void PixelCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        _panning = true;
        _panStartWidgetPos = event->position();
        _panStartView = _view;
        return;
    }
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    const std::optional<std::pair<int, int>> pixel = imagePixelAt(event->position());
    if (!pixel) {
        return;
    }
    beginGesture(pixel->first, pixel->second);
}

void PixelCanvas::mouseMoveEvent(QMouseEvent* event) {
    const std::optional<std::pair<int, int>> pixel = imagePixelAt(event->position());
    if (pixel != _hoveredPixel) {
        _hoveredPixel = pixel;
        emit hoveredPixelChanged(_hoveredPixel);
    }

    if (_panning) {
        const QPointF delta = event->position() - _panStartWidgetPos;
        _view.panX = _panStartView.panX - static_cast<int>(std::lround(delta.x() / _view.zoom));
        _view.panY = _panStartView.panY - static_cast<int>(std::lround(delta.y() / _view.zoom));
        update();
        return;
    }
    if (!_gestureActive || !pixel) {
        return;  // hors image (geste en cours) : ignore, le trait reprend au prochain point dedans.
    }
    continueToolTo(pixel->first, pixel->second);
    _lastGesturePixel = pixel;
    update();
}

void PixelCanvas::leaveEvent(QEvent* /*event*/) {
    if (_hoveredPixel) {
        _hoveredPixel.reset();
        emit hoveredPixelChanged(_hoveredPixel);
    }
}

void PixelCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        _panning = false;
        return;
    }
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    endGesture();
    update();
}

void PixelCanvas::wheelEvent(QWheelEvent* event) {
    const int notches = event->angleDelta().y() / 120;  // 120 = un cran de molette (Qt/Win32).
    for (int i = 0; i < notches; ++i) {
        zoomIn();
    }
    for (int i = 0; i > notches; --i) {
        zoomOut();
    }
}

QPixmap PixelCanvas::renderPixmap() const {
    const qreal scale = devicePixelRatioF();
    const PixelCanvasRealSize real = pixelCanvasRealSize(_image.width, _image.height, _view.zoom,
                                                          scale);
    QPixmap pixmap(std::max(1, real.width), std::max(1, real.height));

    // Surface de peinture (fond + damier) : portee INVARIANTE des jetons (epic.md, decision de
    // cadrage) -- ne suit jamais le theme clair/sombre du chassis d'edition qui l'entoure.
    const QColor checkerA = toQColor(identityTokens().color.surface);
    const QColor checkerB = toQColor(identityTokens().color.surfaceAlt);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.fillRect(0, 0, real.width, real.height, checkerA);

    const int cell = std::max(1, static_cast<int>(std::lround(CHECKER_LOGICAL_CELL * scale)));
    for (int y = 0; y < real.height; y += cell) {
        const bool rowOffset = ((y / cell) % 2) != 0;
        for (int x = rowOffset ? 0 : cell; x < real.width; x += 2 * cell) {
            painter.fillRect(x, y, cell, cell, checkerB);
        }
    }

    // Oeuvre : plus proche voisin, jamais interpolee (EX-ARCH-022) -- SmoothPixmapTransform
    // desactive ci-dessus rend drawImage() rapide (nearest-neighbor) plutot que lisse.
    const QImage art = toQImage(_image);
    painter.drawImage(QRect(0, 0, real.width, real.height), art);

    // Grille de pixels, seulement au-dela du seuil de lisibilite (TACHE-03).
    if (pixelCanvasGridVisible(_view.zoom)) {
        painter.setPen(toQColor(identityTokens().color.border));
        const double zoomReal = static_cast<double>(_view.zoom) * scale;
        for (int column = 0; column <= _image.width; ++column) {
            const int screenX = static_cast<int>(std::lround(column * zoomReal));
            painter.drawLine(screenX, 0, screenX, real.height);
        }
        for (int row = 0; row <= _image.height; ++row) {
            const int screenY = static_cast<int>(std::lround(row * zoomReal));
            painter.drawLine(0, screenY, real.width, screenY);
        }
    }

    pixmap.setDevicePixelRatio(scale);
    return pixmap;
}

void PixelCanvas::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.drawPixmap(0, 0, renderPixmap());
}

}  // namespace hmi
