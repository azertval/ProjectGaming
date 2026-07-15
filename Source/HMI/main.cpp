/**
 * @file main.cpp
 * @brief Point d'entrée : assemble fenêtre, Direct3D 11, simulation ECS et rendu 2D.
 */

#include <chrono>
#include <exception>
#include <memory>
#include <string>

#include "Core/Diagnostics/ConsoleLogSink.h"
#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/MovementSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Math/Vector2.h"
#include "Core/Time/FixedTimestep.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/GraphicsDevice.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/SpriteRenderer.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/Window.h"

namespace {

/**
 * @brief Construit une scène de démonstration codée en dur dans l'ECS.
 *
 * Une grille de tuiles de fond (couche 0) et un sprite mobile au-dessus (couche 1, tuile
 * partiellement transparente) doté d'une vitesse : le `MovementSystem` le fait dériver, ce
 * qui montre la simulation à pas fixe et le rendu par couches.
 *
 * @param world Monde recevant les entités de la scène.
 * @param atlas Atlas fournissant les régions de tuiles.
 */
void buildDemoScene(core::World& world, const hmi::TextureAtlas& atlas) {
    constexpr int columns = 12;
    constexpr int rows = 8;
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const core::Entity tile = world.createEntity();
            core::Transform transform;
            transform.position = core::Vector2{static_cast<float>(column), static_cast<float>(row)};
            world.addComponent(tile, transform);

            core::Sprite sprite;
            sprite.region = atlas.tile(column % hmi::TextureAtlas::TILES_PER_SIDE,
                                       row % hmi::TextureAtlas::TILES_PER_SIDE);
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
        atlas.tile(hmi::TextureAtlas::TILES_PER_SIDE - 1, hmi::TextureAtlas::TILES_PER_SIDE - 1);
    moverSprite.layer = 1;
    world.addComponent(mover, moverSprite);

    world.addComponent(mover, core::Velocity{core::Vector2{1.0f, 0.6f}});
}

}  // namespace

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main() {
    // Configure le journaliseur global avec une sortie console dès le démarrage.
    core::defaultLogger().addSink(std::make_unique<core::ConsoleLogSink>());

    try {
        HMI_LOG_INFO("Demarrage de ProjectGaming");

        hmi::Window window(L"ProjectGaming", 1280, 720);
        HMI_LOG_INFO("Fenetre creee");

        hmi::GraphicsDevice graphics(window.handle(), window.clientWidth(), window.clientHeight());
        HMI_LOG_INFO("Direct3D 11 initialise");

        // Rendu 2D : pipeline de quads, atlas procédural, caméra et rendu des sprites.
        hmi::SpriteBatch spriteBatch(graphics.device(), graphics.context());
        hmi::TextureAtlas atlas(graphics.device());
        hmi::Camera2D camera(window.clientWidth(), window.clientHeight());
        camera.setCenter(core::Vector2{6.0f, 4.0f});
        camera.setZoom(4.0f);
        hmi::SpriteRenderer spriteRenderer(spriteBatch, atlas);

        // Simulation : un monde ECS et le système de mouvement, cadencés à pas fixe.
        core::World world;
        world.addSystem(std::make_unique<core::MovementSystem>());
        buildDemoScene(world, atlas);
        HMI_LOG_INFO("Scene de demonstration prete");

        core::FixedTimestep timestep;
        const float fixedDelta = timestep.fixedDeltaSeconds();

        using Clock = std::chrono::steady_clock;
        Clock::time_point previous = Clock::now();

        while (!window.shouldClose()) {
            window.pumpMessages();

            // Répercute un éventuel redimensionnement sur la swap chain et la caméra.
            int resizedWidth = 0;
            int resizedHeight = 0;
            if (window.consumeResize(resizedWidth, resizedHeight)) {
                graphics.resize(resizedWidth, resizedHeight);
                camera.setViewportSize(resizedWidth, resizedHeight);
            }

            // Mesure du temps réel écoulé, découpé en pas fixes déterministes.
            const Clock::time_point now = Clock::now();
            const float elapsedSeconds = std::chrono::duration<float>(now - previous).count();
            previous = now;

            // Simulation : le monde avance d'un nombre entier de pas fixes.
            const int steps = timestep.advance(elapsedSeconds);
            for (int step = 0; step < steps; ++step) {
                world.update(fixedDelta);
            }

            // Rendu découplé : lecture seule de l'état simulé.
            graphics.clear(0.10f, 0.12f, 0.16f, 1.0f);
            spriteRenderer.render(world, camera);
            graphics.present();
        }

        HMI_LOG_INFO("Arret propre");
        return 0;
    } catch (const std::exception& error) {
        HMI_LOG_ERROR(std::string("Erreur fatale : ") + error.what());
        return 1;
    }
}
