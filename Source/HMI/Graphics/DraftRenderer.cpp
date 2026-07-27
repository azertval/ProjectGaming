#include "HMI/Graphics/DraftRenderer.h"

#include <algorithm>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/RoomGrid.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"

namespace hmi {

DraftRenderer::DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas)
    : _batch(batch), _atlas(atlas), _renderer(batch, atlas) {}

void DraftRenderer::render(const core::LevelDraft& draft, const Camera2D& camera, bool showGrid) {
    if (_dirty) {
        rebuild(draft);
        _dirty = false;
    }
    _renderer.render(_world, camera, 1.0f);
    if (showGrid) {
        drawGrid(draft, camera);
    }
}

void DraftRenderer::drawGrid(const core::LevelDraft& draft, const Camera2D& camera) {
    const int width = draft.tileMap().width();
    const int height = draft.tileMap().height();
    const core::AtlasRegion solid = _atlas.tile(0, 0);  // région opaque unie (teintée pour la ligne)
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());

    // Fabrique un quad plein (UV de la région opaque) à une position/taille et teinte données.
    const auto lineQuad = [&](float x, float y, float w, float h, float r, float g, float b,
                              float a) {
        SpriteQuad quad;
        quad.x = x;
        quad.y = y;
        quad.width = w;
        quad.height = h;
        quad.u0 = static_cast<float>(solid.x) / atlasWidth;
        quad.v0 = static_cast<float>(solid.y) / atlasHeight;
        quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
        quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
        quad.r = r;
        quad.g = g;
        quad.b = b;
        quad.a = a;
        return quad;
    };

    _batch.begin(camera.projectionMatrix(), _atlas.textureView());

    // Grille de cases : lignes fines, faible alpha (repère de placement, EX-EDIT-023).
    constexpr float LINE = 0.035f;  // épaisseur en unités monde (fraction de case)
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    for (int column = 0; column <= width; ++column) {
        _batch.draw(lineQuad(static_cast<float>(column) - LINE * 0.5f, 0.0f, LINE, h, 1.0f, 1.0f,
                             1.0f, 0.18f));
    }
    for (int row = 0; row <= height; ++row) {
        _batch.draw(lineQuad(0.0f, static_cast<float>(row) - LINE * 0.5f, w, LINE, 1.0f, 1.0f, 1.0f,
                             0.18f));
    }

    // Frontières de salles (RoomGrid, LOT-32) : plus épaisses, teinte ambre.
    constexpr float ROOM_LINE = 0.09f;
    for (int column = 0; column * RoomGrid::ROOM_WIDTH_TILES <= width; ++column) {
        const float x = static_cast<float>(std::min(column * RoomGrid::ROOM_WIDTH_TILES, width));
        _batch.draw(lineQuad(x - ROOM_LINE * 0.5f, 0.0f, ROOM_LINE, h, 1.0f, 0.85f, 0.3f, 0.5f));
    }
    for (int row = 0; row * RoomGrid::ROOM_HEIGHT_TILES <= height; ++row) {
        const float y = static_cast<float>(std::min(row * RoomGrid::ROOM_HEIGHT_TILES, height));
        _batch.draw(lineQuad(0.0f, y - ROOM_LINE * 0.5f, w, ROOM_LINE, 1.0f, 0.85f, 0.3f, 0.5f));
    }

    _batch.end();
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
