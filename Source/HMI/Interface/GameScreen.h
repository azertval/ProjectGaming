#pragma once

#include "Core/Ecs/World.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/SpriteRenderer.h"
#include "HMI/Interface/IScreen.h"

/**
 * @file HMI/Interface/GameScreen.h
 * @brief Écran de jeu : scène de démonstration (LOT-05) encapsulée en écran.
 */

namespace hmi {

class SpriteBatch;
class TextureAtlas;

/**
 * @brief Écran de jeu hébergeant la **scène de démonstration** du LOT-05 (niveau provisoire).
 *
 * Reprend la scène auparavant construite dans `main` : un `core::World` avec le
 * `MovementSystem` (simulation à pas fixe, `EX-ARCH-030`) et un rendu par `SpriteRenderer` en
 * **lecture seule** de l'ECS (`EX-ARCH-012`). L'écran possède ses ressources de simulation
 * (RAII) ; **Échap** demande le retour au menu. Le chargement de niveaux depuis fichier est un
 * lot ultérieur — la scène reste codée en dur ici.
 */
class GameScreen : public IScreen {
public:
    /**
     * @brief Construit l'écran de jeu et sa scène de démonstration.
     * @param batch          Lot de sprites partagé (rendu).
     * @param atlas          Atlas de tuiles fournissant les régions de sprites.
     * @param viewportWidth  Largeur initiale de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur initiale de la surface de rendu, en pixels.
     */
    GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
               int viewportHeight);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    core::World _world;
    Camera2D _camera;
    SpriteRenderer _renderer;
};

}  // namespace hmi
