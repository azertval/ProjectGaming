#include "HMI/Graphics/ComposedScene.h"

#include <algorithm>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Entity.h"
#include "Core/Ecs/World.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/PreviousPosition.h"

namespace hmi {

// Vide la scene (capacite conservee) et remet les compteurs a zero. Le cadrage est conserve.
void ComposedScene::clear() noexcept {
    _quads.clear();
    _textureOrder.clear();
    _considered = 0;
    _culled = 0;
}

// Restreint la composition aux primitives visibles dans un cadrage donne (EX-NFR-005). Le
// rectangle elargi de la marge est calcule une fois ici, et non a chaque primitive testee.
void ComposedScene::setVisibleBounds(const core::Rect& worldBounds) noexcept {
    _visibleBounds = core::Rect{core::Vector2{worldBounds.position.x - CULLING_MARGIN_UNITS,
                                              worldBounds.position.y - CULLING_MARGIN_UNITS},
                                core::Vector2{worldBounds.size.x + 2.0f * CULLING_MARGIN_UNITS,
                                              worldBounds.size.y + 2.0f * CULLING_MARGIN_UNITS}};
}

// Desactive le culling : toutes les primitives ajoutees sont conservees.
void ComposedScene::clearVisibleBounds() noexcept {
    _visibleBounds.reset();
}

// Rectangle reellement utilise pour le culling : le cadrage elargi de la marge.
core::Rect ComposedScene::cullingBounds() const noexcept {
    return _visibleBounds.value_or(core::Rect{});
}

// Rang de premiere apparition d'une texture, ajoutee a la table si elle est nouvelle. Le nombre de
// textures distinctes par image se compte sur les doigts d'une main : une recherche lineaire est
// plus rapide (et bien plus simple) qu'une table de hachage.
int ComposedScene::textureRank(TextureHandle texture) {
    const auto found = std::find(_textureOrder.begin(), _textureOrder.end(), texture);
    if (found != _textureOrder.end()) {
        return static_cast<int>(std::distance(_textureOrder.begin(), found));
    }
    _textureOrder.push_back(texture);
    return static_cast<int>(_textureOrder.size()) - 1;
}

// true si la boite englobante est visible (ou si le culling est desactive).
bool ComposedScene::isVisible(const core::Rect& bounds) const {
    if (!_visibleBounds) {
        return true;
    }
    return _visibleBounds->intersects(bounds);
}

// Ajoute un rectangle texture a la scene, s'il est visible.
// true si la primitive a ete conservee, false si le culling l'a ecartee.
bool ComposedScene::addSprite(RenderLayer layer, TextureHandle texture, std::int32_t sortOrder,
                              const SpriteQuad& quad) {
    ++_considered;
    if (!isVisible(spriteQuadBounds(quad))) {
        ++_culled;
        return false;
    }
    ComposedQuad composed;
    composed.layer = layer;
    composed.texture = texture;
    composed.textureRank = textureRank(texture);
    composed.sortOrder = sortOrder;
    composed.kind = QuadKind::Sprite;
    composed.sprite = quad;
    _quads.push_back(composed);
    return true;
}

// Ajoute un segment epais a la scene, s'il est visible.
// true si la primitive a ete conservee, false si le culling l'a ecartee.
bool ComposedScene::addLine(RenderLayer layer, TextureHandle texture, std::int32_t sortOrder,
                            const LineQuad& quad) {
    ++_considered;
    if (!isVisible(lineQuadBounds(quad))) {
        ++_culled;
        return false;
    }
    ComposedQuad composed;
    composed.layer = layer;
    composed.texture = texture;
    composed.textureRank = textureRank(texture);
    composed.sortOrder = sortOrder;
    composed.kind = QuadKind::Line;
    composed.line = quad;
    _quads.push_back(composed);
    return true;
}

// Ordonne la scene (calque, puis texture, puis sortOrder), de facon stable.
void ComposedScene::sort() {
    // Le calque est **prioritaire** sur la texture : regrouper par texture ne doit jamais faire
    // passer une primitive devant une primitive d'un calque inferieur (EX-REN-014). Le tri stable
    // preserve l'ordre de composition a cle egale, donc le rendu d'avant le lot tant qu'une seule
    // texture et un seul calque sont en jeu.
    std::stable_sort(_quads.begin(), _quads.end(),
                     [](const ComposedQuad& lhs, const ComposedQuad& rhs) {
                         if (lhs.layer != rhs.layer) {
                             return lhs.layer < rhs.layer;
                         }
                         if (lhs.textureRank != rhs.textureRank) {
                             return lhs.textureRank < rhs.textureRank;
                         }
                         return lhs.sortOrder < rhs.sortOrder;
                     });
}

// Le nombre de passes begin/end : groupes contigus de meme texture.
int ComposedScene::batchCount() const noexcept {
    int batches = 0;
    TextureHandle current = nullptr;
    for (std::size_t i = 0; i < _quads.size(); ++i) {
        if (i == 0 || _quads[i].texture != current) {
            ++batches;
            current = _quads[i].texture;
        }
    }
    return batches;
}

// Les compteurs de l'image composee (EX-NFR-005).
SceneStatistics ComposedScene::statistics() const noexcept {
    SceneStatistics stats;
    stats.considered = _considered;
    stats.culled = _culled;
    stats.submitted = static_cast<int>(_quads.size());
    stats.batches = batchCount();
    return stats;
}

// Resume des compteurs d'une image, pour la journalisation de diagnostic (EX-NFR-005).
std::string formatSceneStatistics(const SceneStatistics& statistics) {
    return "Rendu : " + std::to_string(statistics.considered) + " primitive(s) composee(s), " +
           std::to_string(statistics.culled) + " ecartee(s) hors cadrage, " +
           std::to_string(statistics.submitted) + " soumise(s) en " +
           std::to_string(statistics.batches) + " passe(s).";
}

// Boite englobante d'un rectangle texture, en unites monde.
core::Rect spriteQuadBounds(const SpriteQuad& quad) noexcept {
    return core::Rect{core::Vector2{quad.x, quad.y}, core::Vector2{quad.width, quad.height}};
}

// Boite englobante d'un segment epais, en unites monde (extremites elargies d'une demi-epaisseur).
core::Rect lineQuadBounds(const LineQuad& quad) noexcept {
    const float half = quad.thickness * 0.5f;
    const float left = (std::min)(quad.ax, quad.bx) - half;
    const float top = (std::min)(quad.ay, quad.by) - half;
    const float right = (std::max)(quad.ax, quad.bx) + half;
    const float bottom = (std::max)(quad.ay, quad.by) + half;
    return core::Rect{core::Vector2{left, top}, core::Vector2{right - left, bottom - top}};
}

// Compose les entites affichables d'un monde ECS en primitives (lecture seule de l'ECS).
void composeWorldSprites(ComposedScene& scene, core::World& world, RenderMode mode,
                         const SceneTextures& textures, float interpolationAlpha) {
    // Lecture seule de l'ECS : les composants sont pris par reference constante.
    world.view<core::Transform, core::Sprite>().each(
        [&](core::Entity entity, const core::Transform& transform, const core::Sprite& sprite) {
            // Apparence resolue par le point d'appel unique (LOT-41) : c'est ici, a la
            // composition, que le mode de rendu agit -- la scene ECS, elle, ne bouge pas.
            const TileAppearance appearance = resolveTileAppearance(mode, sprite.region);
            const core::AtlasRegion& region = appearance.region;
            const TextureHandle texture = textures.textureFor(appearance.source);
            const float atlasWidth = static_cast<float>(textures.widthFor(appearance.source));
            const float atlasHeight = static_cast<float>(textures.heightFor(appearance.source));

            // Taille du sprite en unites monde : la region (en pixels) ramenee a l'echelle du
            // monde (16 px/unite), multipliee par l'echelle du Transform. Le zoom est applique
            // plus tard par la projection de la camera. Le damier de repli fait exactement une
            // case (MISSING_TEXTURE_SIZE == TILE_SIZE) : la geometrie composee est donc la meme
            // dans les deux modes, seule la texture echantillonnee change.
            const float worldWidth =
                static_cast<float>(region.width) / Camera2D::PIXELS_PER_UNIT * transform.scale.x;
            const float worldHeight =
                static_cast<float>(region.height) / Camera2D::PIXELS_PER_UNIT * transform.scale.y;

            // Position a dessiner : interpolee entre le pas precedent et le pas courant si
            // l'entite porte un `PreviousPosition` (personnage, dangers mobiles, blocs -- mouvement
            // continu), sinon la position courante telle quelle (tuiles fixes). Lire une seconde
            // pool pendant l'iteration de la vue Transform+Sprite est sur : on ne modifie aucune
            // pool, donc la vue reste valide (cf. @ref guide-ecs).
            core::Vector2 position = transform.position;
            if (world.hasComponent<PreviousPosition>(entity)) {
                const core::Vector2 previous = world.getComponent<PreviousPosition>(entity).value;
                position = previous + (transform.position - previous) * interpolationAlpha;
            }

            SpriteQuad quad;
            quad.x = position.x;
            quad.y = position.y;
            quad.width = worldWidth;
            quad.height = worldHeight;
            // Coordonnees de texture normalisees a partir de la region en pixels.
            quad.u0 = static_cast<float>(region.x) / atlasWidth;
            quad.v0 = static_cast<float>(region.y) / atlasHeight;
            quad.u1 = static_cast<float>(region.x + region.width) / atlasWidth;
            quad.v1 = static_cast<float>(region.y + region.height) / atlasHeight;
            quad.r = sprite.tint.r;
            quad.g = sprite.tint.g;
            quad.b = sprite.tint.b;
            quad.a = sprite.tint.a;

            // Calque de presentation : porte par l'entite si elle est taguee, sinon le defaut
            // (tuiles). `core::Sprite::layer` reste le tri fin a l'interieur de ce calque.
            const RenderLayer layer = world.hasComponent<RenderLayerTag>(entity)
                                          ? world.getComponent<RenderLayerTag>(entity).value
                                          : DEFAULT_RENDER_LAYER;
            scene.addSprite(layer, texture, sprite.layer, quad);
        });
}

}  // namespace hmi
