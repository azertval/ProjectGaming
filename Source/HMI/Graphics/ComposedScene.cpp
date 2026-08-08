#include "HMI/Graphics/ComposedScene.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Entity.h"
#include "Core/Ecs/World.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/DecorVisuals.h"
#include "HMI/Graphics/Parallax.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/PreviousPosition.h"
#include "HMI/Graphics/TileSkinTag.h"

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

// Boite englobante d'un rectangle texture, en unites monde -- tient compte de la rotation
// (LOT-50 TACHE-02) : un quad pivote occupe un rectangle englobant plus grand que sa taille
// propre, le culling doit donc le juger sur ce rectangle-la, jamais sur (x, y, width, height) brut
// (un decor pivote pres du bord du cadrage disparaitrait sinon a tort).
core::Rect spriteQuadBounds(const SpriteQuad& quad) noexcept {
    if (quad.rotation == 0.0f) {
        return core::Rect{core::Vector2{quad.x, quad.y}, core::Vector2{quad.width, quad.height}};
    }
    const float halfWidth = quad.width * 0.5f;
    const float halfHeight = quad.height * 0.5f;
    const float centerX = quad.x + halfWidth;
    const float centerY = quad.y + halfHeight;
    const float cosR = std::fabs(std::cos(quad.rotation));
    const float sinR = std::fabs(std::sin(quad.rotation));
    const float extentX = halfWidth * cosR + halfHeight * sinR;
    const float extentY = halfWidth * sinR + halfHeight * cosR;
    return core::Rect{core::Vector2{centerX - extentX, centerY - extentY},
                      core::Vector2{extentX * 2.0f, extentY * 2.0f}};
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
                         const SceneTextures& textures, float interpolationAlpha,
                         const Camera2D* camera, const LayerVisibility& visibility) {
    // Lecture seule de l'ECS : les composants sont pris par reference constante.
    world.view<core::Transform, core::Sprite>().each(
        [&](core::Entity entity, const core::Transform& transform, const core::Sprite& sprite) {
            // Decor libre (EX-DEC-001, LOT-49) : jamais compose en mode Physique (TACHE-02), qui
            // doit rester la lecture nue des collisions -- un decor n'en fait jamais partie
            // (EX-ARCH-012). Sorti tot, avant meme de lire les autres marques de presentation.
            const DecorVisualTag* decorTag = world.hasComponent<DecorVisualTag>(entity)
                                                 ? &world.getComponent<DecorVisualTag>(entity)
                                                 : nullptr;
            if (decorTag != nullptr && mode != RenderMode::Texture) {
                return;
            }

            // Habillage du personnage (LOT-48), en RenderMode::Texture uniquement : resolu a part
            // de hmi::resolveTileAppearance (le personnage n'est pas une tuile, cf. PlayerSpriteTag)
            // -- son quad est decorrelee de la hitbox, contrairement au chemin generique ci-dessous.
            const PlayerSpriteTag* playerTag = world.hasComponent<PlayerSpriteTag>(entity)
                                                   ? &world.getComponent<PlayerSpriteTag>(entity)
                                                   : nullptr;
            const bool usePlayerAppearance = playerTag != nullptr && mode == RenderMode::Texture;

            // Calque de presentation : porte par l'entite si elle est taguee, sinon le defaut
            // (tuiles). Calcule ici, avant toute resolution d'apparence, pour pouvoir ecarter au
            // plus tot une entite de calque masque (LOT-51) -- decor et personnage seulement : une
            // entite "tuile" (aucun RenderLayerTag) est filtree plus finement ci-dessous, par axe
            // skin/surcharge plutot que par ce seul bit de calque (hmi::resolveTileAppearance).
            const RenderLayer layer = world.hasComponent<RenderLayerTag>(entity)
                                          ? world.getComponent<RenderLayerTag>(entity).value
                                          : DEFAULT_RENDER_LAYER;
            const bool isTileEntity = decorTag == nullptr && !usePlayerAppearance;
            if (!isTileEntity && !visibility.visible(layer)) {
                return;
            }

            core::AtlasRegion region;
            TextureHandle texture = nullptr;
            float atlasWidth = 0.0f;
            float atlasHeight = 0.0f;
            float worldWidth = 0.0f;
            float worldHeight = 0.0f;
            core::Vector2 quadOffset{};

            if (decorTag != nullptr) {
                // Apparence resolue par hmi::resolveDecorAppearance (LOT-49 TACHE-02), point
                // d'appel unique symetrique a hmi::resolveTileAppearance pour les tuiles : asset
                // designe si charge, damier de repli a sa taille sinon. L'image est toujours
                // echantillonnee en entier (pas de decoupage en grille, contrairement aux tuiles).
                //
                // Rotation (core::Transform::rotation) : appliquee au quad (LOT-50 TACHE-02) --
                // hmi::SpriteQuad porte une rotation optionnelle (nulle par defaut, cf.
                // HMI/Graphics/Quad.h), tournee autour du centre du quad par
                // hmi::SpriteBatch::draw, meme patron que hmi::LineQuad (LOT-37) pour un quad
                // oriente. Revient sur le choix de LOT-49 TACHE-02 de l'ignorer : necessaire pour
                // que la poignee de rotation de l'editeur (LOT-50) ait un effet visible.
                const DecorAppearance appearance = resolveDecorAppearance(*decorTag, textures);
                texture = appearance.texture;
                region = core::AtlasRegion{0, 0, appearance.pixelWidth, appearance.pixelHeight};
                atlasWidth = static_cast<float>(appearance.pixelWidth);
                atlasHeight = static_cast<float>(appearance.pixelHeight);
                worldWidth = static_cast<float>(appearance.pixelWidth) / Camera2D::PIXELS_PER_UNIT *
                            transform.scale.x;
                worldHeight = static_cast<float>(appearance.pixelHeight) /
                             Camera2D::PIXELS_PER_UNIT * transform.scale.y;
            } else if (usePlayerAppearance) {
                region = playerTag->textureRegion;
                if (playerTag->usesCharacterSheet) {
                    texture = textures.characterSheet;
                    atlasWidth = static_cast<float>(textures.characterSheetWidth);
                    atlasHeight = static_cast<float>(textures.characterSheetHeight);
                } else {
                    texture = textures.atlas;
                    atlasWidth = static_cast<float>(textures.atlasWidth);
                    atlasHeight = static_cast<float>(textures.atlasHeight);
                }
                // Taille du quad independante de la hitbox (TACHE-01) : deja en unites monde,
                // calculee par hmi::computePlayerSpriteQuad -- pas de facteur Transform::scale ici.
                worldWidth = playerTag->quadSize.x;
                worldHeight = playerTag->quadSize.y;
                quadOffset = playerTag->quadOffset;
            } else {
                // Type et voisinage de la tuile, quand l'entite en porte un (LOT-42) : c'est de la
                // que vient le choix du skin. Une entite sans ce composant n'est pas habillable.
                const TileSkinTag* skinTag = world.hasComponent<TileSkinTag>(entity)
                                                 ? &world.getComponent<TileSkinTag>(entity)
                                                 : nullptr;

                // Apparence resolue par le point d'appel unique (LOT-41) : c'est ici, a la
                // composition, que le mode de rendu agit -- la scene ECS, elle, ne bouge pas. Les
                // axes skin/surcharge (LOT-51) sont ceux des bits Tile/Object de `visibility` :
                // absent (std::nullopt), rien n'est compose pour cette entite (calque masque, ou
                // axe isole sans rien a montrer).
                const std::optional<TileAppearance> appearance = resolveTileAppearance(
                    mode, sprite.region, skinTag, textures, visibility.visible(RenderLayer::Tile),
                    visibility.visible(RenderLayer::Object));
                if (!appearance.has_value()) {
                    return;
                }
                region = appearance->region;
                texture = textures.textureFor(*appearance);
                atlasWidth = static_cast<float>(textures.widthFor(*appearance));
                atlasHeight = static_cast<float>(textures.heightFor(*appearance));

                // Taille du sprite en unites monde : la region (en pixels) ramenee a l'echelle du
                // monde (16 px/unite), multipliee par l'echelle du Transform. Le zoom est applique
                // plus tard par la projection de la camera. Le damier de repli fait exactement une
                // case (MISSING_TEXTURE_SIZE == TILE_SIZE) : la geometrie composee est donc la meme
                // dans les deux modes, seule la texture echantillonnee change.
                worldWidth =
                    static_cast<float>(region.width) / Camera2D::PIXELS_PER_UNIT * transform.scale.x;
                worldHeight = static_cast<float>(region.height) / Camera2D::PIXELS_PER_UNIT *
                             transform.scale.y;
            }

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

            // Parallaxe (EX-DEC-006, LOT-49 TACHE-03) : purement visuelle -- ne touche que la
            // position DE RENDU, jamais `transform.position` (la position simulee, EX-ARCH-012).
            // Le culling (scene.addSprite ci-dessous, via spriteQuadBounds) juge donc la position
            // DEJA decalee, comme l'exige la tache (une couche parallaxee n'occupe pas le meme
            // rectangle monde que le niveau).
            if (decorTag != nullptr && camera != nullptr) {
                position = parallaxRenderPosition(position, parallaxFactor(decorTag->layer),
                                                  camera->visibleBounds());
                position = roundToScreenPixel(position,
                                              Camera2D::PIXELS_PER_UNIT * camera->zoom());
            }

            position = position + quadOffset;

            SpriteQuad quad;
            quad.x = position.x;
            quad.y = position.y;
            quad.width = worldWidth;
            quad.height = worldHeight;
            // Rotation du decor (LOT-50) : nulle pour toute autre entite (tuiles, personnage),
            // jamais ecrite ailleurs que par les mutateurs de decors (core::Transform::rotation).
            quad.rotation = transform.rotation;
            // Coordonnees de texture normalisees a partir de la region en pixels.
            quad.u0 = static_cast<float>(region.x) / atlasWidth;
            quad.v0 = static_cast<float>(region.y) / atlasHeight;
            quad.u1 = static_cast<float>(region.x + region.width) / atlasWidth;
            quad.v1 = static_cast<float>(region.y + region.height) / atlasHeight;
            if (usePlayerAppearance && playerTag->flipHorizontal) {
                // Retournement horizontal (TACHE-03) : u0/u1 echanges, ancrage deja symetrique
                // (quadOffset ci-dessus ne depend pas du sens) -- aucun decalage de position.
                std::swap(quad.u0, quad.u1);
            }
            quad.r = sprite.tint.r;
            quad.g = sprite.tint.g;
            quad.b = sprite.tint.b;
            quad.a = sprite.tint.a;

            // `layer` (calcule plus haut) reste le calque de presentation ; `core::Sprite::layer`
            // le tri fin a l'interieur de ce calque.
            scene.addSprite(layer, texture, sprite.layer, quad);
        });
}

}  // namespace hmi
