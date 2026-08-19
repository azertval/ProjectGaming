#pragma once

#include <QWidget>

/**
 * @file HMI/Interface/PixelFrameWidget.h
 * @brief Conteneur peignant un cadre pixel art (`LOT-68`, `EX-IHM-070`).
 *
 * Couche **Qt** au-dessus de `hmi::pixelFrameQuads` : la géométrie décide *quoi* dessiner, cette
 * classe décide *comment* le peindre, en résolvant chaque rôle depuis les jetons de la portée
 * **identité** — jamais depuis ceux du châssis d'édition, qui suivent le thème clair/sombre.
 *
 * **Promu dans les `.ui`** (Qt Designer, `Source/Elements/UI/`) : la mise en page reste
 * déclarative, seul le tracé est en C++. C'est la seule façon de tenir la convention du projet
 * — « la mise en page hors code » — tout en dessinant un cadre qu'aucune feuille de style ne sait
 * produire : une bordure QSS ne peut pas évider ses quatre coins.
 */

namespace hmi {

class PixelFrameWidget : public QWidget {
    Q_OBJECT

public:
    explicit PixelFrameWidget(QWidget* parent = nullptr);

    /// Variante **accentuée** du cadre : biseaux à la couleur d'accent plutôt qu'aux gris de
    /// relief. Réservée aux fenêtres modales (pause, fin de niveau), où elle signale que l'écran
    /// se superpose au jeu au lieu de le remplacer.
    void setAccented(bool accented);
    [[nodiscard]] bool isAccented() const { return _accented; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bool _accented = false;
};

}  // namespace hmi
