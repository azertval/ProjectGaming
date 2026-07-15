#include "HMI/Graphics/SpriteRenderer.h"

#include <algorithm>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Entity.h"
#include "Core/Ecs/World.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

/**
 * @brief Construit le rendu de sprites.
 * @param batch Pipeline de quads texturés utilisé pour dessiner (non possédé).
 * @param atlas Atlas fournissant la texture et ses dimensions (non possédé).
 */
SpriteRenderer::SpriteRenderer(SpriteBatch& batch, const TextureAtlas& atlas)
    : _batch(&batch), _atlas(&atlas) {}

/**
 * @brief Dessine toutes les entités affichables du monde, vues par la caméra.
 * @param world  Monde dont on lit les composants `Transform` et `Sprite`.
 * @param camera Caméra fournissant la projection monde → écran.
 */
void SpriteRenderer::render(core::World& world, const Camera2D& camera) {
    const float atlasWidth = static_cast<float>(_atlas->width());
    const float atlasHeight = static_cast<float>(_atlas->height());

    _items.clear();

    // Lecture seule de l'ECS : les composants sont pris par référence constante.
    world.view<core::Transform, core::Sprite>().each(
        [&](core::Entity, const core::Transform& transform, const core::Sprite& sprite) {
            const core::AtlasRegion& region = sprite.region;

            // Taille du sprite en unités monde : la région (en pixels) ramenée à l'échelle
            // du monde (16 px/unité), multipliée par l'échelle du Transform. Le zoom est
            // appliqué plus tard par la projection de la caméra.
            const float worldWidth =
                static_cast<float>(region.width) / Camera2D::PIXELS_PER_UNIT * transform.scale.x;
            const float worldHeight =
                static_cast<float>(region.height) / Camera2D::PIXELS_PER_UNIT * transform.scale.y;

            SpriteQuad quad;
            quad.x = transform.position.x;
            quad.y = transform.position.y;
            quad.width = worldWidth;
            quad.height = worldHeight;
            // Coordonnées de texture normalisées à partir de la région en pixels.
            quad.u0 = static_cast<float>(region.x) / atlasWidth;
            quad.v0 = static_cast<float>(region.y) / atlasHeight;
            quad.u1 = static_cast<float>(region.x + region.width) / atlasWidth;
            quad.v1 = static_cast<float>(region.y + region.height) / atlasHeight;
            quad.r = sprite.tint.r;
            quad.g = sprite.tint.g;
            quad.b = sprite.tint.b;
            quad.a = sprite.tint.a;

            _items.push_back(LayeredQuad{sprite.layer, quad});
        });

    // Tri **stable** par couche : les couches basses (fond) sont dessinées avant les hautes
    // (entités, interface) ; l'ordre reste déterministe à couche égale (EX-REN-014).
    std::stable_sort(
        _items.begin(), _items.end(),
        [](const LayeredQuad& lhs, const LayeredQuad& rhs) { return lhs.layer < rhs.layer; });

    _batch->begin(camera.projectionMatrix(), _atlas->textureView());
    for (const LayeredQuad& item : _items) {
        _batch->draw(item.quad);
    }
    _batch->end();
}

}  // namespace hmi
