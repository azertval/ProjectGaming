#include "HMI/Graphics/DraftRenderer.h"

#include <algorithm>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/LinkGeometry.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/RoomGrid.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"
#include "HMI/Graphics/TileAutotile.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"

namespace hmi {

namespace {
// Ordre de dessin des aides d'edition **a l'interieur** du calque EditorOverlay : la grille sous
// les liens, les liens sous l'apercu de selection. Ces valeurs jouent le role de
// `core::Sprite::layer` pour des primitives qui ne viennent d'aucune entite.
constexpr std::int32_t OVERLAY_ORDER_GRID = 0;
constexpr std::int32_t OVERLAY_ORDER_LINKS = 1;
constexpr std::int32_t OVERLAY_ORDER_HIGHLIGHT = 2;
}  // namespace

DraftRenderer::DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache)
    : _batch(batch), _atlas(atlas), _cache(cache) {}

void DraftRenderer::render(
    const core::LevelDraft& draft, const Camera2D& camera, bool showGrid,
    const std::optional<std::pair<core::GridPosition, core::GridPosition>>& highlight,
    const LinkOverlayState& linkOverlay, RenderMode mode) {
    if (_dirty) {
        rebuild(draft);
        _dirty = false;
    }

    // Une seule scene pour toute l'image : l'ordre visuel est porte par les calques, plus par
    // l'ordre des appels de dessin (LOT-40). L'ordre de composition ci-dessous reste celui d'avant
    // le lot -- a calque et texture egaux, le tri stable le preserve tel quel.
    _scene.clear();
    _scene.setVisibleBounds(camera.visibleBounds());
    composeWorldSprites(_scene, _world, mode, sceneTextures(_atlas, _cache, _skins, _skinSet), 1.0f);
    if (showGrid) {
        composeGrid(draft);
    }
    composeLinks(draft, linkOverlay);
    if (highlight) {
        composeHighlight(highlight->first, highlight->second);
    }
    _scene.sort();
    submitComposedScene(_batch, camera.projectionMatrix(), _scene);
}

// Compose le voile d'apercu d'une zone (outil Rectangle/Selection) sur le calque d'edition.
void DraftRenderer::composeHighlight(const core::GridPosition& minimum,
                                     const core::GridPosition& maximum) {
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    const core::AtlasRegion solid = _atlas.tile(0, 0);
    SpriteQuad quad;
    quad.x = static_cast<float>(minimum.column);
    quad.y = static_cast<float>(minimum.row);
    quad.width = static_cast<float>(maximum.column - minimum.column + 1);
    quad.height = static_cast<float>(maximum.row - minimum.row + 1);
    quad.u0 = static_cast<float>(solid.x) / atlasWidth;
    quad.v0 = static_cast<float>(solid.y) / atlasHeight;
    quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
    quad.r = 0.3f;
    quad.g = 0.7f;
    quad.b = 1.0f;
    quad.a = 0.28f;  // voile bleu semi-transparent (apercu rectangle/selection)
    _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureView(), OVERLAY_ORDER_HIGHLIGHT,
                     quad);
}

// Compose la grille de repere (frontieres de cases + de salles) sur le calque d'edition.
void DraftRenderer::composeGrid(const core::LevelDraft& draft) {
    const int width = draft.tileMap().width();
    const int height = draft.tileMap().height();
    const core::AtlasRegion solid =
        _atlas.tile(0, 0);  // region opaque unie (teintee pour la ligne)
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());

    // Fabrique un quad plein (UV de la region opaque) a une position/taille et teinte donnees.
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
    const auto add = [&](const SpriteQuad& quad) {
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureView(), OVERLAY_ORDER_GRID,
                         quad);
    };

    // Grille de cases : lignes fines, faible alpha (repere de placement, EX-EDIT-023).
    constexpr float LINE = 0.035f;  // epaisseur en unites monde (fraction de case)
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    for (int column = 0; column <= width; ++column) {
        add(lineQuad(static_cast<float>(column) - LINE * 0.5f, 0.0f, LINE, h, 1.0f, 1.0f, 1.0f,
                     0.18f));
    }
    for (int row = 0; row <= height; ++row) {
        add(lineQuad(0.0f, static_cast<float>(row) - LINE * 0.5f, w, LINE, 1.0f, 1.0f, 1.0f,
                     0.18f));
    }

    // Frontieres de salles (RoomGrid, LOT-32) : plus epaisses, teinte ambre.
    constexpr float ROOM_LINE = 0.09f;
    for (int column = 0; column * RoomGrid::ROOM_WIDTH_TILES <= width; ++column) {
        const float x = static_cast<float>(std::min(column * RoomGrid::ROOM_WIDTH_TILES, width));
        add(lineQuad(x - ROOM_LINE * 0.5f, 0.0f, ROOM_LINE, h, 1.0f, 0.85f, 0.3f, 0.5f));
    }
    for (int row = 0; row * RoomGrid::ROOM_HEIGHT_TILES <= height; ++row) {
        const float y = static_cast<float>(std::min(row * RoomGrid::ROOM_HEIGHT_TILES, height));
        add(lineQuad(0.0f, y - ROOM_LINE * 0.5f, w, ROOM_LINE, 1.0f, 0.85f, 0.3f, 0.5f));
    }
}

// Compose les liens de mecanismes (fleches declencheur -> cible) sur le calque d'edition.
void DraftRenderer::composeLinks(const core::LevelDraft& draft, const LinkOverlayState& overlay) {
    const std::vector<LinkRow> rows = buildLinkRows(draft);
    if (rows.empty() && !overlay.pendingLink) {
        return;  // rien a dessiner (ni liaison, ni geste de creation en cours).
    }

    // Regroupe les liens par declencheur (fan-out anti-superposition, LinkGeometry::linkSegment) :
    // index et nombre total de liens partageant le meme declencheur, pour chaque ligne.
    std::vector<int> fanIndex(rows.size(), 0);
    std::vector<int> fanCount(rows.size(), 1);
    for (std::size_t i = 0; i < rows.size(); ++i) {
        int index = 0;
        int count = 0;
        for (std::size_t j = 0; j < rows.size(); ++j) {
            if (rows[j].trigger == rows[i].trigger) {
                if (j < i) {
                    ++index;
                }
                ++count;
            }
        }
        fanIndex[i] = index;
        fanCount[i] = count;
    }

    const core::AtlasRegion solid = _atlas.tile(0, 0);  // region opaque unie (teintee).
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    const float u0 = static_cast<float>(solid.x) / atlasWidth;
    const float v0 = static_cast<float>(solid.y) / atlasHeight;
    const float u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    const float v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;

    // Trait epais entre deux points monde, meme region de texture (unie) que le reste de l'overlay.
    const auto segment = [&](core::Vector2 a, core::Vector2 b, float thickness, float r, float g,
                             float bl, float alpha) {
        LineQuad quad;
        quad.ax = a.x;
        quad.ay = a.y;
        quad.bx = b.x;
        quad.by = b.y;
        quad.thickness = thickness;
        quad.u0 = u0;
        quad.v0 = v0;
        quad.u1 = u1;
        quad.v1 = v1;
        quad.r = r;
        quad.g = g;
        quad.b = bl;
        quad.a = alpha;
        return quad;
    };
    const auto addLine = [&](const LineQuad& quad) {
        _scene.addLine(RenderLayer::EditorOverlay, _atlas.textureView(), OVERLAY_ORDER_LINKS, quad);
    };

    constexpr float LINE_THICKNESS = 0.045f;
    constexpr float HIGHLIGHT_THICKNESS = 0.08f;
    constexpr float PENDING_THICKNESS = 0.03f;

    // Case en attente (premier clic de l'outil Lien) : voile plein pour la signaler.
    if (overlay.pendingLink) {
        SpriteQuad pendingQuad;
        pendingQuad.x = static_cast<float>(overlay.pendingLink->column);
        pendingQuad.y = static_cast<float>(overlay.pendingLink->row);
        pendingQuad.width = 1.0f;
        pendingQuad.height = 1.0f;
        pendingQuad.u0 = u0;
        pendingQuad.v0 = v0;
        pendingQuad.u1 = u1;
        pendingQuad.v1 = v1;
        pendingQuad.r = 1.0f;
        pendingQuad.g = 0.9f;
        pendingQuad.b = 0.2f;
        pendingQuad.a = 0.35f;
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureView(), OVERLAY_ORDER_LINKS,
                         pendingQuad);

        // Trait provisoire vers la case survolee, si distincte (retour visuel du geste en cours).
        if (overlay.hoveredCell && *overlay.hoveredCell != *overlay.pendingLink) {
            const core::Vector2 from{static_cast<float>(overlay.pendingLink->column) + 0.5f,
                                     static_cast<float>(overlay.pendingLink->row) + 0.5f};
            const core::Vector2 to{static_cast<float>(overlay.hoveredCell->column) + 0.5f,
                                   static_cast<float>(overlay.hoveredCell->row) + 0.5f};
            addLine(segment(from, to, PENDING_THICKNESS, 1.0f, 0.9f, 0.2f, 0.7f));
        }
    }

    for (std::size_t i = 0; i < rows.size(); ++i) {
        const LinkRow& row = rows[i];
        const bool selected = overlay.selectedLink.has_value() &&
                              overlay.selectedLink->first == row.trigger &&
                              overlay.selectedLink->second == row.target;
        const bool hovered =
            overlay.hoveredCell.has_value() &&
            (*overlay.hoveredCell == row.trigger || *overlay.hoveredCell == row.target);
        const bool highlighted = selected || hovered;

        const LinkSegment line = linkSegment(row.trigger, row.target, fanIndex[i], fanCount[i]);
        const bool mechanism = row.kind == LinkKind::Mechanism;
        const float r = mechanism ? 0.35f : 1.0f;
        const float g = mechanism ? 0.85f : 0.45f;
        const float b = mechanism ? 1.0f : 0.3f;
        const float alpha = highlighted ? 0.95f : 0.65f;
        const float thickness = highlighted ? HIGHLIGHT_THICKNESS : LINE_THICKNESS;

        addLine(segment(line.a, line.b, thickness, r, g, b, alpha));
        const ArrowHead head = arrowHead(line.a, line.b);
        addLine(segment(line.b, head.left, thickness, r, g, b, alpha));
        addLine(segment(line.b, head.right, thickness, r, g, b, alpha));
    }
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
            _world.addComponent(entity,
                                core::Transform{core::Vector2{static_cast<float>(column) + margin,
                                                              static_cast<float>(row) + margin},
                                                core::Vector2{scale, scale}, 0.0f});
            core::Sprite sprite;
            sprite.region = regionForTile(type);
            sprite.tint = core::Color{1.0f, 1.0f, 1.0f, 1.0f};
            // Aucun calque a fixer : une entite sans `RenderLayerTag` est dessinee sur
            // RenderLayer::Tile (hmi::DEFAULT_RENDER_LAYER), et `core::Sprite::layer` garde sa
            // valeur par defaut -- le tri fin entre tuiles n'a pas lieu d'etre.
            _world.addComponent(entity, sprite);
            // Marque d'habillage (LOT-42), identique a celle posee en jeu : c'est ce qui fait que
            // le canevas de l'editeur montre exactement ce que le joueur verra.
            _world.addComponent(entity, TileSkinTag{type, solidNeighborMask(map, column, row)});
        }
    }
}

}  // namespace hmi
