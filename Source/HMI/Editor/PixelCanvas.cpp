#include "HMI/Editor/PixelCanvas.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPixmap>
#include <QRectF>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <cstring>

#include "HMI/Editor/PixelPalette.h"
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

// Correspondance outil -> type d'operation d'historique (LOT-54 TACHE-02/TACHE-06). L'outil
// pipette ne mute jamais l'image (aucune region non vide n'est jamais poussee pour lui) : la
// valeur de repli n'est donc jamais observee pour lui. L'outil Selection ne mute l'image QUE
// lorsqu'il deplace une selection existante -- c'est alors toujours un deplacement de region.
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
        case PixelTool::Selection:
            return PixelOperationKind::Move;
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
    // Depart d'un document neuf (ouverture/creation, TACHE-05) : tout l'etat propre au document
    // precedent est reinitialise ensemble, pas seulement l'image -- un nom d'asset ou un historique
    // laisse derriere induirait la barre d'etat et l'annulation en erreur.
    _image = std::move(image);
    _assetName.clear();
    _dirty = false;
    _history = PixelHistory();
    _gestureActive = false;
    _lastGesturePixel.reset();
    _selection = PixelRegion{};
    _clipboard = PixelClipboard{};
    _selectionMoveActive = false;
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
        _dirty = true;
        update();
        emit imageChanged();
        emit historyChanged();
    }
}

void PixelCanvas::redo() {
    if (_history.redo(_image)) {
        _dirty = true;
        update();
        emit imageChanged();
        emit historyChanged();
    }
}

void PixelCanvas::jumpHistoryTo(std::size_t index) {
    if (_history.jumpTo(_image, index)) {
        _dirty = true;
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
            touched = setPixel(_image, x, y, effectivePaintColor());
            break;
        case PixelTool::Eraser:
            touched = erasePixel(_image, x, y);
            break;
        case PixelTool::Fill:
            touched = floodFill(_image, x, y, effectivePaintColor());
            break;
        case PixelTool::Eyedropper:
            // Ne mute jamais l'image : prelevement seul, aucune entree d'historique.
            if (const std::optional<std::uint32_t> picked = pickColor(_image, x, y)) {
                _currentColor = *picked;
            }
            return;
        case PixelTool::Selection: {
            // Premier point du geste : decide s'il deplace la selection existante (clic dedans) ou
            // en definit une nouvelle (clic dehors) -- ne mute jamais l'image a ce point precis
            // (un deplacement nul serait un no-op couteux et polluerait l'historique).
            const bool insideSelection =
                !_selection.empty() && x >= _selection.minX && x <= _selection.maxX &&
                y >= _selection.minY && y <= _selection.maxY;
            _selectionMoveActive = insideSelection;
            if (!insideSelection) {
                _selectionDragAnchor = std::make_pair(x, y);
                _selection = PixelRegion{x, y, x, y};
                update();
            }
            return;
        }
    }
    _gestureRegion = unionPixelRegion(_gestureRegion, touched);
}

void PixelCanvas::continueToolTo(int x, int y) {
    const auto [lastX, lastY] = _lastGesturePixel.value_or(std::make_pair(x, y));
    PixelRegion touched;
    switch (_activeTool) {
        case PixelTool::Brush:
            touched = drawLine(_image, lastX, lastY, x, y, effectivePaintColor());
            break;
        case PixelTool::Eraser:
            touched = eraseLine(_image, lastX, lastY, x, y);
            break;
        case PixelTool::Fill:
        case PixelTool::Eyedropper:
            // Geste ponctuel : le pot de peinture et la pipette ignorent le glisser.
            return;
        case PixelTool::Selection:
            if (_selectionMoveActive) {
                const int dx = x - lastX;
                const int dy = y - lastY;
                touched = moveRegion(_image, _selection, dx, dy);
                _selection = PixelRegion{_selection.minX + dx, _selection.minY + dy,
                                         _selection.maxX + dx, _selection.maxY + dy};
            } else {
                _selection = PixelRegion{
                    std::min(_selectionDragAnchor.first, x), std::min(_selectionDragAnchor.second, y),
                    std::max(_selectionDragAnchor.first, x), std::max(_selectionDragAnchor.second, y)};
                update();
                return;  // definir une selection ne mute jamais l'image : aucune region touchee.
            }
            break;
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
        _dirty = true;
        emit imageChanged();
        emit historyChanged();
    }
    _gestureActive = false;
    _lastGesturePixel.reset();
}

std::uint32_t PixelCanvas::effectivePaintColor() const noexcept {
    if (!_paletteConstrained) {
        return _currentColor;
    }
    return nearestPaletteColor(_currentColor, _paletteColors);
}

PixelRegion PixelCanvas::effectiveRegion() const {
    if (!_selection.empty()) {
        return _selection;
    }
    if (_image.width <= 0 || _image.height <= 0) {
        return {};
    }
    return PixelRegion{0, 0, _image.width - 1, _image.height - 1};
}

void PixelCanvas::commitRegionMutation(PixelOperationKind kind, const DecodedImage& beforeSnapshot,
                                       const PixelRegion& touched) {
    if (touched.empty()) {
        return;
    }
    const std::vector<std::uint32_t> before = readRegion(beforeSnapshot, touched);
    const std::vector<std::uint32_t> after = readRegion(_image, touched);
    _history.push(kind, touched, before, after);
    _dirty = true;
    update();
    emit imageChanged();
    emit historyChanged();
}

void PixelCanvas::applyRegionOperation(PixelOperationKind kind,
                                       PixelRegion (*operation)(DecodedImage&, const PixelRegion&)) {
    const PixelRegion region = effectiveRegion();
    if (region.empty()) {
        return;
    }
    const DecodedImage beforeSnapshot = _image;
    const PixelRegion touched = operation(_image, region);
    commitRegionMutation(kind, beforeSnapshot, touched);
}

void PixelCanvas::applyFlipHorizontal() {
    applyRegionOperation(PixelOperationKind::FlipHorizontal, &flipHorizontal);
}

void PixelCanvas::applyFlipVertical() {
    applyRegionOperation(PixelOperationKind::FlipVertical, &flipVertical);
}

void PixelCanvas::applyRotateClockwise() {
    applyRegionOperation(PixelOperationKind::RotateClockwise, &rotateClockwise);
}

void PixelCanvas::applyRotateCounterClockwise() {
    applyRegionOperation(PixelOperationKind::RotateCounterClockwise, &rotateCounterClockwise);
}

void PixelCanvas::copy() {
    const PixelRegion region = effectiveRegion();
    _clipboard = copyRegion(_image, region);
}

void PixelCanvas::paste() {
    if (_clipboard.empty()) {
        return;
    }
    const int x = _hoveredPixel ? _hoveredPixel->first : 0;
    const int y = _hoveredPixel ? _hoveredPixel->second : 0;
    const DecodedImage beforeSnapshot = _image;
    const PixelRegion touched = pasteClipboard(_image, _clipboard, x, y);
    if (touched.empty()) {
        return;
    }
    _selection = touched;  // le contenu colle devient la selection : ajustable immediatement.
    commitRegionMutation(PixelOperationKind::Paste, beforeSnapshot, touched);
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

    // Region selectionnee (TACHE-06) : reste visible pendant qu'on la deplace, tracee depuis les
    // jetons (portee invariante, comme le reste de la surface de peinture) sans masquer les pixels
    // du bord (contour seul, jamais rempli).
    if (!_selection.empty()) {
        const double zoomReal = static_cast<double>(_view.zoom) * scale;
        const double rectX = (_selection.minX - _view.panX) * zoomReal;
        const double rectY = (_selection.minY - _view.panY) * zoomReal;
        const double rectW = _selection.width() * zoomReal;
        const double rectH = _selection.height() * zoomReal;
        painter.setPen(QPen(toQColor(identityTokens().color.accent), std::max(1.0, scale)));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(rectX, rectY, rectW, rectH));
    }

    pixmap.setDevicePixelRatio(scale);
    return pixmap;
}

void PixelCanvas::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.drawPixmap(0, 0, renderPixmap());
}

}  // namespace hmi
