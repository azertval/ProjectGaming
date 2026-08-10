#pragma once

#include <QPointF>
#include <QSize>
#include <QWidget>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "HMI/Editor/EditContextTarget.h"
#include "HMI/Editor/PixelCanvasGeometry.h"
#include "HMI/Editor/PixelHistory.h"
#include "HMI/Editor/PixelOperations.h"
#include "HMI/Editor/PixelTool.h"
#include "HMI/Graphics/TextureLoader.h"

class QEvent;
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
 *
 * Implémente `hmi::EditContextTarget` (TACHE-04) : `MainWindow` y dispatche Annuler/Refaire/
 * Copier/Coller quand ce canevas est le contexte d'édition actif, exactement comme `GameViewport`
 * pour le niveau — le seuil de dispatch prévu depuis `LOT-57` TACHE-04, sans le réécrire. Copier et
 * coller restent des opérations neutres (aucun presse-papiers) tant que TACHE-06 ne leur donne pas
 * de contenu.
 */
class PixelCanvas : public QWidget, public EditContextTarget {
    Q_OBJECT

public:
    explicit PixelCanvas(QWidget* parent = nullptr);

    /// Remplace l'image éditée et réinitialise l'historique local (ouverture d'un asset, TACHE-05).
    void setImage(DecodedImage image);
    [[nodiscard]] const DecodedImage& image() const noexcept {
        return _image;
    }

    /// Nom de l'asset ouvert (barre d'état, TACHE-04), vide si aucun. `setImage` le remet à vide
    /// (nouveau document) ; l'appelant le renseigne ensuite après une ouverture ou un
    /// enregistrement réussi (TACHE-05).
    void setAssetName(std::string name) {
        _assetName = std::move(name);
    }
    [[nodiscard]] const std::string& assetName() const noexcept {
        return _assetName;
    }
    /// Modifications non enregistrées depuis l'ouverture/le dernier enregistrement (barre d'état
    /// TACHE-04, garde-fou de perte de travail TACHE-05) : un coup de pinceau la marque, un
    /// enregistrement (`markSaved`) la lève.
    [[nodiscard]] bool isDirty() const noexcept {
        return _dirty;
    }
    /// Marque l'image comme enregistrée — appelé après une écriture réussie (TACHE-05). Ne touche
    /// pas à l'historique : annuler après un enregistrement reste possible et remarque « modifié »,
    /// comme dans tout éditeur (revenir en arrière s'écarte du fichier tout juste écrit).
    void markSaved() noexcept {
        _dirty = false;
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

    // hmi::EditContextTarget (TACHE-04) : Annuler/Refaire à cible contextuelle (EX-IHM-062). Copier
    // et coller restent neutres tant que TACHE-06 ne leur donne pas de presse-papiers de région.
    [[nodiscard]] bool canUndo() const override {
        return _history.canUndo();
    }
    void undo() override;
    [[nodiscard]] bool canRedo() const override {
        return _history.canRedo();
    }
    void redo() override;
    /// Revient à l'état immédiatement après l'entrée @p index de `history().appliedEntries()`
    /// (panneau d'historique visuel, TACHE-04) ; sans effet si @p index est hors bornes.
    void jumpHistoryTo(std::size_t index);
    [[nodiscard]] bool canCopy() const override {
        return false;
    }
    void copy() override {}
    [[nodiscard]] bool canPaste() const override {
        return false;
    }
    void paste() override {}

    [[nodiscard]] const PixelCanvasView& view() const noexcept {
        return _view;
    }
    void zoomIn();
    void zoomOut();

    /// Pixel image sous une position widget donnée (coordonnées **logiques**), ou `std::nullopt`
    /// hors de l'image — alimente le pixel survolé de la barre d'état (TACHE-04).
    [[nodiscard]] std::optional<std::pair<int, int>> imagePixelAt(
        const QPointF& widgetPosition) const;
    /// Dernier pixel survolé par la souris à l'intérieur de l'image, ou `std::nullopt` si la souris
    /// n'y est pas (curseur sorti du widget, ou hors de l'image à zoom/décalage donnés).
    [[nodiscard]] std::optional<std::pair<int, int>> hoveredPixel() const noexcept {
        return _hoveredPixel;
    }

    [[nodiscard]] QSize sizeHint() const override;

signals:
    /// Émis après chaque mutation de `image()` (geste, annuler, refaire) — la barre d'état et
    /// l'aperçu live (TACHE-08) s'y abonnent plutôt que d'interroger le canevas à chaque frame.
    void imageChanged();
    /// Émis après chaque changement de l'historique (nouvelle entrée, annuler, refaire) — alimente
    /// le panneau d'historique visuel (`hmi::PixelHistoryPanel`).
    void historyChanged();
    /// Émis à chaque changement du pixel survolé (barre d'état, TACHE-04).
    void hoveredPixelChanged(std::optional<std::pair<int, int>> pixel);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
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
    std::string _assetName;
    bool _dirty = false;
    PixelHistory _history;
    PixelCanvasView _view;
    PixelTool _activeTool = PixelTool::Brush;
    std::uint32_t _currentColor = 0xFF000000u;  // noir opaque (R8G8B8A8_UNORM, LOT-54 TACHE-01).

    bool _gestureActive = false;
    DecodedImage _gestureBeforeSnapshot;  ///< Copie de `_image` au début du geste courant.
    PixelRegion _gestureRegion;           ///< Union des régions touchées depuis le début du geste.
    std::optional<std::pair<int, int>> _lastGesturePixel;
    std::optional<std::pair<int, int>> _hoveredPixel;

    bool _panning = false;
    QPointF _panStartWidgetPos;
    PixelCanvasView _panStartView;
};

}  // namespace hmi
