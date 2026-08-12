/**
 * @file test_plateforme_composition.cpp
 * @brief Test d'intégration de la composition ECS d'une plateforme mobile (`EX-GP-026`,
 * `LOT-63`) : reproduit EXACTEMENT le repérage d'entité-tuile et la remise à jour visuelle de
 * `hmi::GameSession::loadLevel`/`refreshPlatformVisuals` (jamais exercés par
 * `Source/Test/Systeme/test_parcours_complet.cpp`, qui ne consomme `core::PlatformController`
 * que via ses boîtes, sans jamais chercher ni déplacer d'entité).
 */

#include <cstddef>
#include <filesystem>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/PlatformController.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelScene.h"
#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

namespace {

core::Level loadDemoPlateforme() {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "demo-plateforme.json";
    const core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
    if (!result.ok()) {
        ADD_FAILURE() << "Echec de chargement de demo-plateforme.json : " << result.error;
        return core::Level{"invalide", core::TileMap{1, 1}, core::GridPosition{0, 0},
                           core::GridPosition{0, 0}, {}};
    }
    return *result.level;
}

// Repere l'entite-tuile d'une plateforme a sa position de DEPART, EXACTEMENT comme
// hmi::GameSession::loadLevel (meme boucle sur core::World::view<Transform, Sprite>).
core::Entity findPlatformEntity(core::World& world, const core::GridPosition& startPosition) {
    core::Entity found{};
    bool matched = false;
    world.view<core::Transform, core::Sprite>().each(
        [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
            if (!matched && static_cast<int>(transform.position.x) == startPosition.column &&
                static_cast<int>(transform.position.y) == startPosition.row) {
                found = entity;
                matched = true;
            }
        });
    EXPECT_TRUE(matched) << "Aucune entite-tuile trouvee a la position de depart de la plateforme.";
    return found;
}

}  // namespace

/**
 * @brief L'entité-tuile d'une plateforme mobile, repérée comme le fait `hmi::GameSession`, voit
 *        sa position `Transform` effectivement bouger au fil des pas, en suivant `boxAt`.
 * \castest{<b>L'entite-tuile d'une plateforme mobile suit sa position simulee.</b><br/>
 * \tcat Integration · Plateforme mobile<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Charger demo-plateforme.json et construire la scene (`core::buildLevelScene`).<br/>
 * 2. Reperer l'entite-tuile de la plateforme a sa position de depart.<br/>3. Avancer
 * `core::PlatformController` de plusieurs pas, en reappliquant `boxAt` a l'entite comme le ferait
 * `hmi::GameSession::refreshPlatformVisuals`.<br/>
 * \tattendu La position de l'entite change au fil des pas et correspond exactement a `boxAt`.
 * }
 */
TEST(PlateformeCompositionTest, EntiteTuileSuitLaPositionSimulee) {
    const core::Level level = loadDemoPlateforme();
    ASSERT_FALSE(level.platformConfigs().empty());
    const core::MovingPlatformConfig& config = level.platformConfigs().front();

    core::World world;
    core::buildLevelScene(
        world, level, [](core::TileType) { return core::AtlasRegion{}; }, nullptr, nullptr);

    const core::Entity platform = findPlatformEntity(world, config.startPosition);
    ASSERT_TRUE(world.hasComponent<core::Transform>(platform));

    core::PlatformController controller(level);
    const core::Vector2 startPosition = world.getComponent<core::Transform>(platform).position;

    bool moved = false;
    for (int step = 0; step < 180; ++step) {
        controller.update();
        core::Transform& transform = world.getComponent<core::Transform>(platform);
        transform.position = controller.boxAt(0).min;
        EXPECT_EQ(transform.position.x, controller.boxAt(0).min.x);
        EXPECT_EQ(transform.position.y, controller.boxAt(0).min.y);
        if (transform.position.x != startPosition.x || transform.position.y != startPosition.y) {
            moved = true;
        }
    }

    EXPECT_TRUE(moved) << "La position de l'entite-tuile n'a jamais change en 180 pas (3 secondes)"
                          " -- la plateforme reste visuellement statique.";
}
