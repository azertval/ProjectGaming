#pragma once

#include <optional>
#include <utility>

#include "Core/Ecs/World.h"
#include "Core/Levels/GridPosition.h"
#include "HMI/Graphics/SpriteRenderer.h"

/**
 * @file HMI/Graphics/DraftRenderer.h
 * @brief Rendu d'un brouillon d'édition (`core::LevelDraft`) dans le viewport (LOT-35).
 */

namespace core {
class LevelDraft;
}

namespace hmi {

class SpriteBatch;
class TextureAtlas;
class Camera2D;

/**
 * @brief Dessine la grille d'un `core::LevelDraft` en cours d'édition.
 *
 * Réutilise le pipeline de rendu du jeu : chaque tuile non vide du brouillon devient une entité
 * (`Transform` + `Sprite`) d'un `core::World` interne, rendue par `hmi::SpriteRenderer` (lecture
 * seule, `EX-ARCH-012`). La scène n'est **reconstruite** que lorsque le brouillon change
 * (`invalidate()`), pas à chaque frame. Les blocs à taille réduite sont dessinés à leur échelle
 * réelle (`core::tileVisualScale`), comme en jeu — cohérence visuelle stricte.
 */
class DraftRenderer {
public:
    DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas);

    /// Rend le brouillon avec la caméra donnée (reconstruit la scène si invalidée). Si @p showGrid,
    /// superpose la grille de repère (frontières de cases + frontières de salles) — aide au
    /// placement, équivalent de la bascule `F10` de l'éditeur historique (`EX-EDIT-023`). Si
    /// @p highlight est présent, met en surbrillance la zone (bornes min/max incluses) — aperçu de
    /// l'outil Rectangle/Sélection.
    void render(const core::LevelDraft& draft, const Camera2D& camera, bool showGrid,
                const std::optional<std::pair<core::GridPosition, core::GridPosition>>& highlight);

    /// Marque la scène comme périmée : elle sera reconstruite au prochain `render` (à appeler après
    /// toute mutation du brouillon — peinture, undo/redo, chargement, redimensionnement).
    void invalidate() noexcept {
        _dirty = true;
    }

private:
    void rebuild(const core::LevelDraft& draft);
    /// Dessine la grille de repère (cases + salles) par-dessus les tuiles.
    void drawGrid(const core::LevelDraft& draft, const Camera2D& camera);

    SpriteBatch& _batch;
    const TextureAtlas& _atlas;
    SpriteRenderer _renderer;
    core::World _world;
    bool _dirty = true;
};

}  // namespace hmi
