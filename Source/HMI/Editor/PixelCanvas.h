#pragma once

#include <QPointF>
#include <QSize>
#include <QWidget>
#include <cstdint>
#include <optional>
#include <utility>

#include "HMI/Editor/PixelCanvasGeometry.h"
#include "HMI/Editor/PixelHistory.h"
#include "HMI/Editor/PixelOperations.h"
#include "HMI/Graphics/TextureLoader.h"

class QMouseEvent;
class QPaintEvent;
class QPixmap;
class QWheelEvent;

/**
 * @file HMI/Editor/PixelCanvas.h
 * @brief Canevas de l'atelier pixel art : affiche et édite un `hmi::DecodedImage` en mémoire
 *        (`LOT-54` TACHE-03, `EX-EDIT-045`).
 */

namespace hmi {

/**
 * @brief Outil actif du canevas — détermine ce que produisent les gestes de souris.
 *
 * Simple énumération ici : les actions, icônes et barre d'outils qui l'exposent à l'utilisateur
 * arrivent en TACHE-04 (`hmi::EditorActions`), comme un groupe exclusif distinct de celui des
 * outils de niveau.
 */
enum class PixelTool {
    Brush,
    Eraser,
    Fill,
    Eyedropper,
};

/**
 * @brief Widget affichant l'image en cours d'édition et transformant les gestes de souris en
 *        appels aux opérations pures de `hmi::PixelOperations` (TACHE-02).
 *
 * **Zoom entier**, agrandi au plus proche voisin (`EX-ARCH-022`), rendu à la résolution **réelle**
 * de l'écran (`hmi::pixelCanvasRealSize`, réutilise `LOT-56` TACHE-05) : jamais flou à 125 %/150 %
 * d'échelle d'affichage. La **surface de peinture** — fond et damier de transparence sous les zones
 * à alpha nul — appartient à la portée **invariante** des jetons (`hmi::identityTokens()`) : elle
 * ne change jamais avec le thème clair/sombre du châssis d'édition (`LOT-56` TACHE-06), à la
 * différence du châssis qui l'entoure (barre d'outils, panneaux — TACHE-04). Un fond qui change de
 * clarté selon l'heure de la journée fausserait la perception des couleurs posées.
 *
 * Un geste complet (appui, glisser, relâchement) produit **une seule** entrée dans `hmi::
 * PixelHistory`, locale à ce canevas et totalement indépendante de `core::LevelDraft` — annuler un
 * coup de pinceau n'annule jamais une action d'édition de niveau, et réciproquement.
 */
class PixelCanvas : public QWidget {
    Q_OBJECT

public:
    explicit PixelCanvas(QWidget* parent = nullptr);

    /// Remplace l'image éditée et réinitialise l'historique local (ouverture d'un asset, TACHE-05).
    void setImage(DecodedImage image);
    [[nodiscard]] const DecodedImage& image() const noexcept {
        return _image;
    }

    void setActiveTool(PixelTool tool) noexcept {
        _activeTool = tool;
    }
    [[nodiscard]] PixelTool activeTool() const noexcept {
        return _activeTool;
    }

    /// Couleur courante (pinceau, pot de peinture) ; mise à jour par la pipette au clic.
    void setCurrentColor(std::uint32_t color) noexcept {
        _currentColor = color;
    }
    [[nodiscard]] std::uint32_t currentColor() const noexcept {
        return _currentColor;
    }

    [[nodiscard]] const PixelHistory& history() const noexcept {
        return _history;
    }
    [[nodiscard]] bool canUndo() const noexcept {
        return _history.canUndo();
    }
    [[nodiscard]] bool canRedo() const noexcept {
        return _history.canRedo();
    }
    /// @return `true` si l'annulation a eu un effet (déléguée à `hmi::PixelHistory::undo`).
    bool undo();
    /// @return `true` si le rétablissement a eu un effet (déléguée à `hmi::PixelHistory::redo`).
    bool redo();

    [[nodiscard]] const PixelCanvasView& view() const noexcept {
        return _view;
    }
    void zoomIn();
    void zoomOut();

    /// Pixel image sous une position widget donnée (coordonnées **logiques**), ou `std::nullopt`
    /// hors de l'image — alimente le pixel survolé de la barre d'état (TACHE-04).
    [[nodiscard]] std::optional<std::pair<int, int>> imagePixelAt(
        const QPointF& widgetPosition) const;

    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    [[nodiscard]] QPixmap renderPixmap() const;

    /// Applique l'outil actif au pixel `(x, y)` (un seul point : clic, pipette, pot de peinture) et
    /// étend `_gestureRegion` avec la région effectivement modifiée.
    void applyToolAtPoint(int x, int y);
    /// Prolonge le geste courant du dernier point traité jusqu'à `(x, y)` (pinceau/gomme : ligne
    /// sans trou entre deux positions successives, TACHE-02).
    void continueToolTo(int x, int y);
    void beginGesture(int x, int y);
    void endGesture();

    DecodedImage _image;
    PixelHistory _history;
    PixelCanvasView _view;
    PixelTool _activeTool = PixelTool::Brush;
    std::uint32_t _currentColor = 0xFF000000u;  // noir opaque (R8G8B8A8_UNORM, LOT-54 TACHE-01).

    bool _gestureActive = false;
    DecodedImage _gestureBeforeSnapshot;  ///< Copie de `_image` au début du geste courant.
    PixelRegion _gestureRegion;           ///< Union des régions touchées depuis le début du geste.
    std::optional<std::pair<int, int>> _lastGesturePixel;

    bool _panning = false;
    QPointF _panStartWidgetPos;
    PixelCanvasView _panStartView;
};

}  // namespace hmi
