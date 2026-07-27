#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>

#include <QString>
#include <QWindow>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileType.h"
#include "Core/Time/FixedTimestep.h"
#include "HMI/Game/GameSession.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"

/**
 * @file Editor/GameViewport.h
 * @brief Viewport de l'éditeur : rendu Direct3D 11 du brouillon d'édition et de l'essai (LOT-34/35).
 */

namespace hmi {
class GraphicsDevice;
class SpriteBatch;
class TextureAtlas;
class DraftRenderer;
}  // namespace hmi

namespace editor {

/**
 * @brief Fenêtre native (`QWindow`) où Direct3D 11 présente, embarquée dans Qt.
 *
 * **Mode édition** (LOT-35) : affiche le brouillon (`core::LevelDraft`) via `hmi::DraftRenderer`,
 * caméra cadrant le niveau entier. Le clic/glisser gauche **peint** le type de tuile actif
 * (`setActiveTile`, fourni par la palette) à la case survolée (`core::LevelDraft::paintTile`) ;
 * `Ctrl+Z`/`Ctrl+Y` annulent/refont. **Mode essai** (LOT-35 TACHE-04) : une `hmi::GameSession`
 * rejoue le niveau ; sa boucle à pas fixe (`core::FixedTimestep`) et ses entrées (Qt + XInput)
 * reprennent la discipline du `LOT-33`.
 */
class GameViewport : public QWindow {
    Q_OBJECT

public:
    explicit GameViewport(QWindow* parent = nullptr);
    ~GameViewport() override;

    GameViewport(const GameViewport&) = delete;
    GameViewport& operator=(const GameViewport&) = delete;

    /// Définit le type de tuile peint au clic (relié à la sélection de la palette).
    void setActiveTile(core::TileType type) noexcept {
        _activeTile = type;
    }

    /// Enregistre le brouillon (`Ctrl+S`) : valide (`LevelDraft::toLevel`) puis écrit le fichier ;
    /// un brouillon invalide n'écrit rien et rapporte l'erreur (`statusMessage`).
    void save();

    /// Ouvre un fichier de niveau comme brouillon d'édition (remplace le brouillon courant).
    void openLevel(const std::filesystem::path& path);

    /// @return true si le brouillon a des modifications non enregistrées (garde-fou d'ouverture).
    [[nodiscard]] bool isDirty() const noexcept {
        return _dirty;
    }

    /// Lance l'essai immédiat (`P`) sur un brouillon valide ; message d'erreur sinon.
    void startPlaytest();

    /// Redimensionne le niveau en cours d'édition (`EX-EDIT-005`).
    void resizeLevel(int width, int height);

    /// @return true si redimensionner à (@p width, @p height) supprimerait du contenu posé.
    [[nodiscard]] bool wouldResizeDrop(int width, int height) const;

    /// @return Dimensions courantes du niveau, en cases.
    [[nodiscard]] int levelWidth() const;
    [[nodiscard]] int levelHeight() const;

signals:
    /// Message d'état à afficher (enregistrement, essai, erreur de validation…).
    void statusMessage(const QString& message);

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
    void ensureResources();
    void tick();
    void renderFrame();
    void stopPlaytest();  ///< Termine l'essai et restitue l'éditeur (brouillon intact).
    void updateMousePosition(const QMouseEvent* event);

    /// Recale la caméra d'édition pour cadrer le niveau entier dans la surface courante.
    void updateEditCamera();
    /// Case de grille sous la position écran donnée (pixels physiques), si dans les bornes.
    [[nodiscard]] std::optional<core::GridPosition> cellAt(const QMouseEvent* event);
    /// Peint le type actif à la case sous @p event, si valide (invalide la scène rendue).
    void paintAt(const QMouseEvent* event);

    [[nodiscard]] int pixelWidth() const;
    [[nodiscard]] int pixelHeight() const;

    using Clock = std::chrono::steady_clock;

    std::unique_ptr<hmi::GraphicsDevice> _graphics;
    std::unique_ptr<hmi::SpriteBatch> _spriteBatch;
    std::unique_ptr<hmi::TextureAtlas> _atlas;
    std::unique_ptr<hmi::DraftRenderer> _draftRenderer;
    hmi::GameKeyBindings _gameBindings;
    hmi::GamepadBindings _gamepadBindings;
    hmi::InputState _input;
    hmi::GamepadPoller _gamepad;
    core::FixedTimestep _timestep;
    Clock::time_point _previousFrame;
    bool _loopStarted = false;

    core::LevelDraft _draft;            ///< Brouillon en cours d'édition (source de vérité).
    hmi::Camera2D _camera;              ///< Caméra d'édition (cadre le niveau entier).
    core::TileType _activeTile = core::TileType::Solid;  ///< Type peint au clic (palette).
    bool _painting = false;            ///< Un glisser de peinture (bouton gauche) est en cours.
    bool _dirty = false;               ///< Modifications non enregistrées (garde-fou d'ouverture).
    bool _showGrid = true;             ///< Grille de repère (cases + salles) affichée (bascule F10).

    /// Session de jeu de l'essai immédiat ; nulle en mode édition (essai ajouté au LOT-35 TACHE-04).
    std::optional<hmi::GameSession> _session;
};

}  // namespace editor
