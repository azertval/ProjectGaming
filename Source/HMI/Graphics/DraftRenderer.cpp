#include "HMI/Graphics/DraftRenderer.h"

#include "Core/Ecs/Components/Sprite.h"  // core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/TileVisuals.h"

namespace hmi {

DraftRenderer::DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas)
    : _atlas(atlas), _renderer(batch, atlas) {}

void DraftRenderer::render(const core::LevelDraft& draft, const Camera2D& camera) {
    if (_dirty) {
        rebuild(draft);
        _dirty = false;
    }
    _renderer.render(_world, camera, 1.0f);
}

void DraftRenderer::rebuild(const core::LevelDraft& draft) {
    _world = core::World{};  // repart d'une scène vierge
    const core::TileMap& map = draft.tileMap();
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            const core::TileType type = map.tile(column, row);
            if (type == core::TileType::Empty) {
                continue;  // case vide : aucune entité (grille éparse, comme en jeu).
            }
            const core::Entity entity = _world.createEntity();
            // Blocs réduits (EX-GP-005) : centrés dans leur case à leur échelle réelle, comme en
            // jeu — même formule marge/échelle que GameSession (cohérence visuelle stricte).
            const float scale = core::tileVisualScale(type);
            const float margin = (1.0f - scale) * 0.5f;
            _world.addComponent(
                entity,
                core::Transform{core::Vector2{static_cast<float>(column) + margin,
                                              static_cast<float>(row) + margin},
                                core::Vector2{scale, scale}, 0.0f});
            core::Sprite sprite;
            sprite.region = regionForTile(type, _atlas);
            sprite.layer = 0;
            sprite.tint = core::Color{1.0f, 1.0f, 1.0f, 1.0f};
            _world.addComponent(entity, sprite);
        }
    }
}

}  // namespace hmi
