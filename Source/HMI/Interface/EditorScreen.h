#pragma once

#include <optional>

#include "Core/Levels/LevelDraft.h"
#include "HMI/Editor/TilePalette.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Interface/IScreen.h"

/**
 * @file HMI/Interface/EditorScreen.h
 * @brief Écran « Mode Édition » : éditeur de niveau intégré (LOT-14).
 */

namespace hmi {

class SpriteBatch;
class TextureAtlas;

/**
 * @brief Éditeur de niveau intégré : grille peignable à la souris depuis une palette de tuiles.
 *
 * Réutilise le rendu Direct3D 11 (`SpriteBatch`, `TextureAtlas`, `Camera2D`) et le modèle
 * d'édition de `Core` (`core::LevelDraft`, LOT-14 TACHE-01) — aucune duplication de la logique de
 * niveau (`EX-EDIT-010`, `EX-EDIT-030`, `EX-EDIT-031`). Un clic sur la palette change le type de
 * tuile actif ; un clic (ou glisser-clic) sur la grille peint ce type à la position visée
 * (`EX-EDIT-002`, y compris l'entrée/la sortie, dont l'unicité est gérée par `LevelDraft`).
 *
 * **Liaison de mécanismes** (`EX-EDIT-003`) : `Maj` + clic sur une tuile `Switch` puis, `Maj`
 * toujours enfoncé, sur une tuile `Door` (ou l'inverse) les lie ; répéter la même paire la délie
 * (bascule). **Redimensionnement** (`EX-EDIT-005`) : les flèches ↑/↓ réduisent/agrandissent la
 * hauteur, ←/→ la largeur. **Annuler/refaire** (`EX-EDIT-005`) : `Ctrl+Z`/`Ctrl+Y`, délégués à
 * `core::LevelDraft::undo`/`redo`. **Échap** revient au menu.
 */
class EditorScreen : public IScreen {
public:
    /**
     * @brief Construit l'éditeur avec un brouillon de niveau vierge.
     * @param batch          Lot de sprites partagé (rendu).
     * @param atlas          Atlas de tuiles fournissant les régions de sprites.
     * @param viewportWidth  Largeur initiale de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur initiale de la surface de rendu, en pixels.
     */
    EditorScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                int viewportHeight);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    /// Convertit une position souris (pixels écran) en case de grille, si elle est dans les
    /// bornes du brouillon courant.
    [[nodiscard]] std::optional<core::GridPosition> hoveredCell(float mouseX, float mouseY) const;

    /// Traite un clic Maj+souris pour la liaison de mécanismes (voir la doc de la classe).
    void handleLinkClick(float mouseX, float mouseY);

    /// Dessine la grille du brouillon (tuiles non vides), les liaisons de mécanismes, la
    /// sélection de liaison en attente et la case survolée en surbrillance.
    void renderGrid(RenderContext& context);

    /// Dessine la palette (couleurs des types + surbrillance de la sélection).
    void renderPalette(RenderContext& context);

    const TextureAtlas& _atlas;
    core::LevelDraft _draft;
    Camera2D _camera;
    TilePalette _palette;
    bool _paintingDrag = false;  ///< Le clic gauche en cours peint-il la grille (vs. la palette) ?
    float _mouseX = 0.0f;        ///< Dernière position souris connue (pour le rendu du survol).
    float _mouseY = 0.0f;
    /// Première case (Switch ou Door) choisie pour une liaison, en attente de sa contrepartie.
    std::optional<core::GridPosition> _pendingLink;
};

}  // namespace hmi
