#pragma once

#include <memory>

#include <QWindow>

/**
 * @file Editor/GameViewport.h
 * @brief Surface native embarquant le rendu Direct3D 11 dans l'application Qt (LOT-34).
 */

namespace hmi {
class GraphicsDevice;
}

namespace editor {

/**
 * @brief Fenêtre native (`QWindow`) dans laquelle Direct3D 11 présente, embarquée dans l'interface
 *        Qt via `QWidget::createWindowContainer`.
 *
 * Le `HWND` natif de la fenêtre (`winId()`) est fourni à `hmi::GraphicsDevice`, qui y crée sa swap
 * chain et y présente. Le device est construit **paresseusement** au premier `exposeEvent` (quand le
 * handle natif est valide), et la swap chain suit la taille de la fenêtre en **pixels physiques**
 * (DPI). À cette tâche (LOT-34 TACHE-02), le rendu se limite à un effacement + présentation ; la
 * boucle de jeu et les entrées arrivent à la TACHE-03, le rendu d'un niveau à la TACHE-04.
 */
class GameViewport : public QWindow {
public:
    explicit GameViewport(QWindow* parent = nullptr);
    ~GameViewport() override;

    GameViewport(const GameViewport&) = delete;
    GameViewport& operator=(const GameViewport&) = delete;

protected:
    void exposeEvent(QExposeEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    /// Construit le `GraphicsDevice` à partir du `HWND` natif, au premier affichage.
    void ensureDevice();

    /// Efface et présente une frame (rendu minimal de la TACHE-02).
    void renderFrame();

    /// Dimensions de la surface en **pixels physiques** (taille logique × ratio DPI).
    [[nodiscard]] int pixelWidth() const;
    [[nodiscard]] int pixelHeight() const;

    std::unique_ptr<hmi::GraphicsDevice> _graphics;
};

}  // namespace editor
