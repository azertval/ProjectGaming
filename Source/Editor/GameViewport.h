#pragma once

#include <chrono>
#include <memory>

#include <QWindow>

#include "Core/Time/FixedTimestep.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"

/**
 * @file Editor/GameViewport.h
 * @brief Surface native embarquant le rendu Direct3D 11 et la boucle de jeu dans Qt (LOT-34).
 */

namespace hmi {
class GraphicsDevice;
}

namespace editor {

/**
 * @brief Fenêtre native (`QWindow`) dans laquelle Direct3D 11 présente, embarquée dans l'interface
 *        Qt via `QWidget::createWindowContainer`.
 *
 * Le `HWND` natif (`winId()`) est fourni à `hmi::GraphicsDevice`. La **boucle** est pilotée par Qt
 * (`QEvent::UpdateRequest` redemandé à chaque frame via `requestUpdate`) et rejoue le **pas fixe**
 * (`core::FixedTimestep`) exactement comme la boucle historique : à chaque tick, la manette est
 * sondée (`hmi::GamepadPoller`), le temps réel écoulé est converti en un nombre entier de pas, et
 * `InputState::beginFrame` est appelé **après chaque pas consommé** (discipline `LOT-33` :
 * déterminisme `EX-NFR-002`, fronts non perdus à haut framerate `EX-CTRL-020`/`021`). Les entrées
 * **clavier/souris** arrivent par les événements Qt et sont traduites vers l'`InputState` ; la perte
 * de focus relâche toutes les entrées (`releaseAll`).
 *
 * À cette tâche (LOT-34 TACHE-03), le rendu reste un effacement + présentation ; le chargement et le
 * rendu d'un niveau jouable arrivent à la TACHE-04.
 */
class GameViewport : public QWindow {
public:
    explicit GameViewport(QWindow* parent = nullptr);
    ~GameViewport() override;

    GameViewport(const GameViewport&) = delete;
    GameViewport& operator=(const GameViewport&) = delete;

protected:
    bool event(QEvent* event) override;
    void exposeEvent(QExposeEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    /// Construit le `GraphicsDevice` à partir du `HWND` natif, au premier affichage.
    void ensureDevice();

    /// Un tick de boucle : mesure le temps, sonde la manette, avance le pas fixe, rend une frame.
    void tick();

    /// Efface et présente une frame.
    void renderFrame();

    /// Reporte la position courante de la souris (@p event) dans l'`InputState`, en pixels
    /// physiques.
    void updateMousePosition(const QMouseEvent* event);

    /// Dimensions de la surface en **pixels physiques** (taille logique × ratio DPI).
    [[nodiscard]] int pixelWidth() const;
    [[nodiscard]] int pixelHeight() const;

    using Clock = std::chrono::steady_clock;

    std::unique_ptr<hmi::GraphicsDevice> _graphics;
    hmi::InputState _input;
    hmi::GamepadPoller _gamepad;
    core::FixedTimestep _timestep;
    Clock::time_point _previousFrame;
    bool _loopStarted = false;
};

}  // namespace editor
