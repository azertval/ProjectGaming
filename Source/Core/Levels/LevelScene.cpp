#include "Core/Levels/LevelScene.h"

#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/DangerGeometry.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace core {

// Peuple un World d'une entite (Transform + Sprite) par tuile non vide du niveau.
void buildLevelScene(World& world, const Level& level,
                     const std::function<AtlasRegion(TileType)>& regionForTile) {
    const TileMap& map = level.tileMap();
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            const TileType type = map.tile(column, row);
            if (type == TileType::Empty) {
                continue;
            }
            const Entity entity = world.createEntity();

            Transform transform;
            transform.position = Vector2{static_cast<float>(column), static_cast<float>(row)};
            if (isDangerTileType(type)) {
                // Aperçu fidèle a la hitbox reelle (bande etroite pour les variantes
                // directionnelles, EX-GP-050 ; case pleine sinon) -- meme principe que
                // core::tileVisualScale pour les blocs reduits (EX-GP-005), mais decalage
                // asymetrique plutot que centre.
                const Aabb box = dangerHitbox(type, column, row);
                transform.position = box.min;
                transform.scale = box.max - box.min;
            }
            world.addComponent(entity, transform);

            Sprite sprite;
            sprite.region = regionForTile(type);
            sprite.layer = 0;
            world.addComponent(entity, sprite);
        }
    }
}

}  // namespace core
