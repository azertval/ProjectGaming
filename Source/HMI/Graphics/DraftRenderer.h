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
 * @brief État d'affichage des liens de mécanismes (LOT-37), fourni par le viewport à chaque rendu.
 *
 * Purement informatif pour `DraftRenderer` : ne modifie jamais le brouillon, seulement la
 * présentation (surbrillance, trait provisoire).
 */
struct LinkOverlayState {
    /// Case survolée par la souris (surbrillance des liens incidents ; extrémité du trait
    /// provisoire si `pendingLink` est posé).
    std::optional<core::GridPosition> hoveredCell;
    /// Case du déclencheur/de la cible en attente d'appariement (outil « Lien », premier clic).
    std::optional<core::GridPosition> pendingLink;
    /// Liaison sélectionnée dans le panneau « Liens » (déclencheur, cible) : mise en surbrillance.
    std::optional<std::pair<core::GridPosition, core::GridPosition>> selectedLink;
};

/**
 * @brief Dessine la grille d'un `core::LevelDraft` en cours d'édition.
 *
 * Réutilise le pipeline de rendu du jeu : chaque tuile non vide du brouillon devient une entité
 * (`Transform` + `Sprite`) d'un `core::World` interne, composée par `hmi::composeWorldSprites`
 * (lecture seule, `EX-ARCH-012`). Le `core::World` n'est **reconstruit** que lorsque le brouillon
 * change (`invalidate()`), pas à chaque frame. Les blocs à taille réduite sont dessinés à leur
 * échelle réelle (`core::tileVisualScale`), comme en jeu — cohérence visuelle stricte.
 *
 * Depuis `LOT-40`, **toutes** les primitives d'une image (tuiles, grille de repère, liens de
 * mécanismes, aperçu de sélection) sont composées dans une seule `hmi::ComposedScene` puis
 * soumises en bloc : les aides d'édition portent le calque `RenderLayer::EditorOverlay`, qui les
 * place au-dessus de tout le reste par construction plutôt que par l'ordre des appels de dessin.
 * La liste obtenue est donc inspectable sans GPU (`EX-NFR-004`) et soumise au culling
 * (`EX-NFR-005`).
 */
class DraftRenderer {
public:
    DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas);

    /// Rend le brouillon avec la caméra donnée (reconstruit la scène si invalidée). Si @p showGrid,
    /// superpose la grille de repère (frontières de cases + frontières de salles) — aide au
    /// placement, équivalent de la bascule `F10` de l'éditeur historique (`EX-EDIT-023`). Si
    /// @p highlight est présent, met en surbrillance la zone (bornes min/max incluses) — aperçu de
    /// l'outil Rectangle/Sélection. @p linkOverlay pilote l'affichage des liens de mécanismes
    /// (flèches, trait provisoire, surbrillance — `EX-IHM-030`).
    void render(const core::LevelDraft& draft, const Camera2D& camera, bool showGrid,
                const std::optional<std::pair<core::GridPosition, core::GridPosition>>& highlight,
                const LinkOverlayState& linkOverlay);

    /// Marque la scène comme périmée : elle sera reconstruite au prochain `render` (à appeler après
    /// toute mutation du brouillon — peinture, undo/redo, chargement, redimensionnement).
    void invalidate() noexcept {
        _dirty = true;
    }

    /// @return La scène composée à la dernière image (primitives soumises et compteurs).
    [[nodiscard]] const ComposedScene& lastScene() const noexcept {
        return _scene;
    }

private:
    void rebuild(const core::LevelDraft& draft);
    /// Compose la grille de repère (frontières de cases + de salles) sur le calque d'édition.
    void composeGrid(const core::LevelDraft& draft);
    /// Compose les liens de mécanismes (flèches déclencheur → cible) sur le calque d'édition.
    void composeLinks(const core::LevelDraft& draft, const LinkOverlayState& overlay);
    /// Compose le voile d'aperçu d'une zone (outil Rectangle/Sélection) sur le calque d'édition.
    void composeHighlight(const core::GridPosition& minimum, const core::GridPosition& maximum);

    SpriteBatch& _batch;
    const TextureAtlas& _atlas;
    ComposedScene _scene;
    core::World _world;
    bool _dirty = true;
};

}  // namespace hmi
