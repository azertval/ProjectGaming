#include "HMI/Interface/GameScreen.h"

#include <memory>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/MovementSystem.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/RenderContext.h"

namespace hmi {

namespace {
/// Centre de la caméra sur la scène de démonstration, en unités monde.
const core::Vector2 SCENE_CENTER{6.0f, 4.0f};
/// Zoom (facteur entier pour la netteté pixel art).
constexpr float SCENE_ZOOM = 4.0f;

/**
 * @brief Construit la scène de démonstration codée en dur dans l'ECS.
 *
 * Une grille de tuiles de fond (couche 0) et un sprite mobile au-dessus (couche 1, tuile
 * partiellement transparente) doté d'une vitesse : le `MovementSystem` le fait dériver, ce qui
 * illustre la simulation à pas fixe et le rendu par couches. Repris tel quel de `main` (LOT-05).
 */
void buildDemoScene(core::World& world, const TextureAtlas& atlas) {
    constexpr int columns = 12;
    constexpr int rows = 8;
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const core::Entity tile = world.createEntity();
            core::Transform transform;
            transform.position = core::Vector2{static_cast<float>(column), static_cast<float>(row)};
            world.addComponent(tile, transform);

            core::Sprite sprite;
            sprite.region = atlas.tile(column % TextureAtlas::TILES_PER_SIDE,
                                       row % TextureAtlas::TILES_PER_SIDE);
            sprite.layer = 0;
            world.addComponent(tile, sprite);
        }
    }

    const core::Entity mover = world.createEntity();
    core::Transform moverTransform;
    moverTransform.position = core::Vector2{1.0f, 1.0f};
    world.addComponent(mover, moverTransform);

    core::Sprite moverSprite;
    // Dernière tuile de l'atlas : damier partiellement transparent (test de l'alpha).
    moverSprite.region =
        atlas.tile(TextureAtlas::TILES_PER_SIDE - 1, TextureAtlas::TILES_PER_SIDE - 1);
    moverSprite.layer = 1;
    world.addComponent(mover, moverSprite);

    world.addComponent(mover, core::Velocity{core::Vector2{1.0f, 0.6f}});
}
}  // namespace

/// @brief Construit l'écran de jeu et sa scène de démonstration.
GameScreen::GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                       int viewportHeight)
    : _camera(viewportWidth, viewportHeight), _renderer(batch, atlas) {
    _camera.setCenter(SCENE_CENTER);
    _camera.setZoom(SCENE_ZOOM);

    _world.addSystem(std::make_unique<core::MovementSystem>());
    buildDemoScene(_world, atlas);
    HMI_LOG_TRACE("GameScreen cree (scene de demonstration montee)");
}

/**
 * @brief Avance la simulation d'un pas fixe et gère le retour au menu.
 * @param input      État des entrées de la frame.
 * @param fixedDelta Pas de temps fixe de simulation, en secondes.
 * @return « Basculer vers le menu » sur Échap, sinon « rester ».
 */
ScreenTransition GameScreen::update(const InputState& input, float fixedDelta) {
    if (input.keyPressed(Key::Escape)) {
        return ScreenTransition::switchTo(ScreenId::Menu);
    }
    _world.update(fixedDelta);
    return ScreenTransition::none();
}

/**
 * @brief Dessine la scène en lecture seule de l'ECS.
 * @param context Ressources de rendu partagées.
 */
void GameScreen::render(RenderContext& context) {
    _camera.setViewportSize(context.viewportWidth, context.viewportHeight);
    _renderer.render(_world, _camera);
}

}  // namespace hmi
