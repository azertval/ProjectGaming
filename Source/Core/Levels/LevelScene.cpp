// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/LevelScene.h"

#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/DangerGeometry.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace core {

// Peuple un World d'une entite (Transform + Sprite) par tuile non vide du niveau, puis d'une
// entite par decor libre (EX-DEC-001, LOT-49).
void buildLevelScene(World& world, const Level& level,
                     const std::function<AtlasRegion(TileType)>& regionForTile,
                     const std::function<void(Entity, TileType, int, int)>& onTileEntity,
                     const std::function<void(Entity, const Decor&, std::size_t)>& onDecorEntity) {
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
            // `layer` garde sa valeur par defaut : il n'ordonne que les sprites d'un **meme**
            // calque de rendu, et toutes les tuiles partagent le leur. Le calque lui-meme est une
            // notion de presentation (hmi::RenderLayer, LOT-40) que `Core` ignore (EX-NFR-011).
            world.addComponent(entity, sprite);

            // La presentation attache ici ses propres composants (habillage) : Core fournit le
            // point d'accroche, sans rien savoir de ce qui s'y greffe.
            if (onTileEntity) {
                onTileEntity(entity, type, column, row);
            }
        }
    }

    // Decors libres (EX-DEC-001, LOT-49) : une entite par decor, position/echelle/rotation en
    // unites monde flottantes, jamais calees sur la grille (contrairement aux tuiles ci-dessus).
    // L'ordre du vecteur alimente Sprite::layer (tri fin intra-calque, TACHE-02).
    const std::vector<Decor>& decors = level.decors();
    for (std::size_t index = 0; index < decors.size(); ++index) {
        const Decor& decor = decors[index];
        const Entity entity = world.createEntity();

        Transform transform;
        transform.position = decor.position;
        transform.scale = decor.scale;
        transform.rotation = decor.rotation;
        world.addComponent(entity, transform);

        Sprite sprite;
        sprite.layer = static_cast<std::int32_t>(index);
        world.addComponent(entity, sprite);

        if (onDecorEntity) {
            onDecorEntity(entity, decor, index);
        }
    }
}

}  // namespace core
