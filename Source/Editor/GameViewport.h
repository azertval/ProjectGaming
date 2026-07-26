#pragma once

#include <chrono>
#include <memory>
#include <optional>

#include <QWindow>

#include "Core/Time/FixedTimestep.h"
#include "HMI/Game/GameSession.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"

/**
 * @file Editor/GameViewport.h
 * @brief Surface native embarquant le rendu Direct3D 11, la boucle de jeu et une session de jeu (LOT-34).
 */

namespace hmi {
class GraphicsDevice;
class SpriteBatch;
class TextureAtlas;
}  // namespace hmi

namespace editor {

/**
 * @brief Fenêtre native (`QWindow`) où Direct3D 11 présente, embarquée dans Qt via
 *        `QWidget::createWindowContainer`.
 *
 * Le `HWND` natif (`winId()`) alimente `hmi::GraphicsDevice`. La **boucle** est pilotée par Qt
 * (`QEvent::UpdateRequest`) et rejoue le **pas fixe** (`core::FixedTimestep`) avec la discipline
 * d'entrées du `LOT-33` (déterminisme `EX-NFR-002`). Les entrées clavier/souris Qt et la manette
 * (`hmi::GamepadPoller`) alimentent un `hmi::InputState`.
 *
 * À la TACHE-04, le viewport **charge et joue un niveau** via `hmi::GameSession` (la même logique
 * que l'écran de jeu historique, sans duplication), rendu par le `SpriteRenderer`. À ce stade, un
 * niveau de démonstration est chargé pour valider le rendu/jouabilité ; l'édition d'un niveau dans
 * le viewport arrive au LOT-35.
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
    /// Construit, au premier affichage, le `GraphicsDevice` puis les ressources de rendu
    /// (`SpriteBatch`, `TextureAtlas`) et charge la session de jeu.
    void ensureResources();

    /// Un tick de boucle : mesure le temps, sonde la manette, avance le pas fixe, rend une frame.
    void tick();

    /// Efface, dessine la session de jeu (si chargée), présente.
    void renderFrame();

    /// Reporte la position courante de la souris (@p event) dans l'`InputState`, en pixels physiques.
    void updateMousePosition(const QMouseEvent* event);

    /// Dimensions de la surface en **pixels physiques** (taille logique × ratio DPI).
    [[nodiscard]] int pixelWidth() const;
    [[nodiscard]] int pixelHeight() const;

    using Clock = std::chrono::steady_clock;

    std::unique_ptr<hmi::GraphicsDevice> _graphics;
    std::unique_ptr<hmi::SpriteBatch> _spriteBatch;
    std::unique_ptr<hmi::TextureAtlas> _atlas;
    hmi::GameKeyBindings _gameBindings;
    hmi::GamepadBindings _gamepadBindings;
    hmi::InputState _input;
    hmi::GamepadPoller _gamepad;
    core::FixedTimestep _timestep;
    Clock::time_point _previousFrame;
    bool _loopStarted = false;
    /// Session de jeu du niveau chargé — déclarée après les ressources qu'elle référence
    /// (`_spriteBatch`, `_atlas`, bindings), donc détruite avant elles.
    std::optional<hmi::GameSession> _session;
};

}  // namespace editor
