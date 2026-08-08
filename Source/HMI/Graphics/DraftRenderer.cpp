#include "HMI/Graphics/DraftRenderer.h"

#include <algorithm>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Levels/Decor.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/DecorGeometry.h"
#include "HMI/Editor/LinkGeometry.h"
#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/DecorVisuals.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/Parallax.h"
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
constexpr std::int32_t OVERLAY_ORDER_TEXTURE_OVERRIDES = 3;
// Cadre de selection : contour double couleur (sombre puis clair, dessine par-dessus) pour rester
// lisible sur tout fond (LOT-50 TACHE-03), puis les poignees, encore au-dessus (memes deux tons).
constexpr std::int32_t OVERLAY_ORDER_DECOR_OUTLINE_DARK = 4;
constexpr std::int32_t OVERLAY_ORDER_DECOR_OUTLINE_BRIGHT = 5;
constexpr std::int32_t OVERLAY_ORDER_DECOR_HANDLE_DARK = 6;
constexpr std::int32_t OVERLAY_ORDER_DECOR_HANDLE_BRIGHT = 7;
}  // namespace

DraftRenderer::DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache)
    : _batch(batch), _atlas(atlas), _cache(cache) {}

void DraftRenderer::render(
    const core::LevelDraft& draft, const Camera2D& camera, bool showGrid,
    const std::optional<std::pair<core::GridPosition, core::GridPosition>>& highlight,
    const LinkOverlayState& linkOverlay, RenderMode mode, bool showTextureOverrides,
    float deltaSeconds, const DecorOverlayState& decorOverlay) {
    if (_dirty) {
        rebuild(draft);
        _dirty = false;
    }

    // Apercu du geste de decor en cours (LOT-50 TACHE-02/03) : applique directement a l'entite
    // deja construite, sans reconstruire toute la scene a chaque image glissee -- _draft, lui,
    // n'est jamais touche tant que le geste n'est pas valide (hmi::GameViewport::endDecorGesture).
    if (decorOverlay.preview && decorOverlay.preview->index < _decorEntities.size()) {
        const DecorGestureAction& preview = *decorOverlay.preview;
        core::Transform& transform =
            _world.getComponent<core::Transform>(_decorEntities[preview.index]);
        switch (preview.kind) {
            case DecorGestureActionKind::Move:
                transform.position = preview.position;
                break;
            case DecorGestureActionKind::Resize:
                transform.position = preview.position;
                transform.scale = preview.scale;
                break;
            case DecorGestureActionKind::Rotate:
                transform.rotation = preview.rotation;
                break;
            case DecorGestureActionKind::None:
                break;
        }
    }

    // Apercu des tuiles animees (LOT-46 TACHE-05), en temps reel : meme mecanisme que
    // GameSession, mais sans exigence de determinisme (EX-NFR-002 ne s'applique qu'en jeu).
    advanceTileAnimations(_skins, _skinSet, _cache, deltaSeconds, _tileAnimations,
                          _warnedExcludedAnimations);

    // Une seule scene pour toute l'image : l'ordre visuel est porte par les calques, plus par
    // l'ordre des appels de dessin (LOT-40). L'ordre de composition ci-dessous reste celui d'avant
    // le lot -- a calque et texture egaux, le tri stable le preserve tel quel.
    _scene.clear();
    _scene.setVisibleBounds(camera.visibleBounds());
    composeBackground(_scene, resolveBackgroundTexture(draft.background(), _cache),
                      draft.tileMap().width(), draft.tileMap().height(), mode);
    composeWorldSprites(_scene, _world, mode,
                        sceneTextures(_atlas, _cache, _skins, _skinSet, draft.textureOverrides(),
                                      _tileAnimations, draft.decors()),
                        1.0f, &camera);
    if (showGrid) {
        composeGrid(draft, decorOverlay.snapToGrid);
    }
    composeLinks(draft, linkOverlay);
    if (showTextureOverrides) {
        composeTextureOverrideMarkers(draft);
    }
    if (highlight) {
        composeHighlight(highlight->first, highlight->second);
    }
    composeDecorSelection(draft, decorOverlay, camera);
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

// Signale les cases portant une surcharge de texture par instance (EX-EDIT-043, LOT-45) : un petit
// marqueur au coin haut-droit de chaque case habillee, visible seulement quand l'outil dedie est
// actif (showTextureOverrides du render()).
void DraftRenderer::composeTextureOverrideMarkers(const core::LevelDraft& draft) {
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    const core::AtlasRegion solid = _atlas.tile(0, 0);
    constexpr float MARKER_SIZE = 0.28f;  // fraction d'une case
    for (const core::TileTextureOverride& override : draft.textureOverrides()) {
        SpriteQuad quad;
        quad.x = static_cast<float>(override.position.column) + 1.0f - MARKER_SIZE;
        quad.y = static_cast<float>(override.position.row);
        quad.width = MARKER_SIZE;
        quad.height = MARKER_SIZE;
        quad.u0 = static_cast<float>(solid.x) / atlasWidth;
        quad.v0 = static_cast<float>(solid.y) / atlasHeight;
        quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
        quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
        quad.r = 1.0f;
        quad.g = 0.85f;
        quad.b = 0.1f;
        quad.a = 0.9f;  // jaune dore, oppose au bleu du voile de selection/rectangle
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureView(),
                         OVERLAY_ORDER_TEXTURE_OVERRIDES, quad);
    }
}

// Compose la grille de repere (frontieres de cases + de salles) sur le calque d'edition.
void DraftRenderer::composeGrid(const core::LevelDraft& draft, bool accentuate) {
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

    // Grille de cases : lignes fines, faible alpha (repere de placement, EX-EDIT-023). Accentuee
    // (alpha double) quand l'aimantation du geste de decors est active (LOT-50 TACHE-03) : sinon
    // l'auteur ne comprend pas pourquoi la position "saute" a la grille.
    constexpr float LINE = 0.035f;  // epaisseur en unites monde (fraction de case)
    const float lineAlpha = accentuate ? 0.4f : 0.18f;
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    for (int column = 0; column <= width; ++column) {
        add(lineQuad(static_cast<float>(column) - LINE * 0.5f, 0.0f, LINE, h, 1.0f, 1.0f, 1.0f,
                     lineAlpha));
    }
    for (int row = 0; row <= height; ++row) {
        add(lineQuad(0.0f, static_cast<float>(row) - LINE * 0.5f, w, LINE, 1.0f, 1.0f, 1.0f,
                     lineAlpha));
    }

    // Frontieres de salles (RoomGrid, LOT-32) : plus epaisses, teinte ambre.
    constexpr float ROOM_LINE = 0.09f;
    const float roomLineAlpha = accentuate ? 0.75f : 0.5f;
    for (int column = 0; column * RoomGrid::ROOM_WIDTH_TILES <= width; ++column) {
        const float x = static_cast<float>(std::min(column * RoomGrid::ROOM_WIDTH_TILES, width));
        add(lineQuad(x - ROOM_LINE * 0.5f, 0.0f, ROOM_LINE, h, 1.0f, 0.85f, 0.3f, roomLineAlpha));
    }
    for (int row = 0; row * RoomGrid::ROOM_HEIGHT_TILES <= height; ++row) {
        const float y = static_cast<float>(std::min(row * RoomGrid::ROOM_HEIGHT_TILES, height));
        add(lineQuad(0.0f, y - ROOM_LINE * 0.5f, w, ROOM_LINE, 1.0f, 0.85f, 0.3f, roomLineAlpha));
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

// Compose le cadre de selection et les poignees du decor selectionne (LOT-50 TACHE-03), a sa
// position d'apercu si un glisser est en cours.
void DraftRenderer::composeDecorSelection(const core::LevelDraft& draft,
                                          const DecorOverlayState& decorOverlay,
                                          const Camera2D& camera) {
    if (!decorOverlay.selectedIndex || *decorOverlay.selectedIndex >= draft.decors().size()) {
        return;
    }

    // Copie locale : l'apercu du geste en cours (TACHE-02) s'applique ici SANS jamais toucher au
    // brouillon, seule la position d'affichage en tient compte. decor.position reste en espace
    // MODELE a ce stade, que ce soit celle du brouillon ou celle de l'apercu (hmi::DecorGesture,
    // TACHE-02, raisonne uniquement en espace modele, EX-ARCH-012) -- jamais l'espace de rendu.
    core::Decor decor = draft.decors()[*decorOverlay.selectedIndex];
    if (decorOverlay.preview && decorOverlay.preview->index == *decorOverlay.selectedIndex) {
        const DecorGestureAction& preview = *decorOverlay.preview;
        switch (preview.kind) {
            case DecorGestureActionKind::Move:
                decor.position = preview.position;
                break;
            case DecorGestureActionKind::Resize:
                decor.position = preview.position;
                decor.scale = preview.scale;
                break;
            case DecorGestureActionKind::Rotate:
                decor.rotation = preview.rotation;
                break;
            case DecorGestureActionKind::None:
                break;
        }
    }

    // Position d'AFFICHAGE : decalee par la parallaxe de la couche du decor, exactement comme le
    // sprite reellement rendu (hmi::composeWorldSprites, LOT-49 TACHE-03) -- appliquee en dernier,
    // decor.position etant toujours en espace modele jusqu'ici (brouillon ou apercu). Sans cette
    // conversion, le cadre de selection se desolidarise visiblement du decor des que sa couche
    // n'est pas la couche de reference (Background/Foreground, facteur != 1.0).
    decor.position = roundToScreenPixel(
        parallaxRenderPosition(decor.position, parallaxFactor(decor.layer), camera.visibleBounds()),
        Camera2D::PIXELS_PER_UNIT * camera.zoom());

    // Dimensions reelles de l'asset (meme repli que hmi::resolveDecorAppearance, LOT-49) : la
    // geometrie partagee (hmi::decorWorldBounds) en depend, comme pour la detection (TACHE-02).
    core::Vector2 pixelSize{static_cast<float>(MISSING_TEXTURE_SIZE),
                            static_cast<float>(MISSING_TEXTURE_SIZE)};
    if (const LoadedTexture* loaded =
            _cache.get(DECORS_SUBDIRECTORY + decor.assetName, AssetFamily::Decor)) {
        pixelSize = core::Vector2{static_cast<float>(loaded->width), static_cast<float>(loaded->height)};
    }
    const core::Rect bounds = decorWorldBounds(decor, pixelSize);
    const float worldUnitsPerScreenPixel = 1.0f / (Camera2D::PIXELS_PER_UNIT * camera.zoom());
    const DecorHandleLayout handles = decorHandleLayout(bounds, worldUnitsPerScreenPixel, decor.rotation);

    const core::AtlasRegion solid = _atlas.tile(0, 0);
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    const float solidU0 = static_cast<float>(solid.x) / atlasWidth;
    const float solidV0 = static_cast<float>(solid.y) / atlasHeight;
    const float solidU1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    const float solidV1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
    const auto solidQuad = [&](const core::Rect& rect, float r, float g, float b, float a) {
        SpriteQuad quad;
        quad.x = rect.position.x;
        quad.y = rect.position.y;
        quad.width = rect.size.x;
        quad.height = rect.size.y;
        quad.u0 = solidU0;
        quad.v0 = solidV0;
        quad.u1 = solidU1;
        quad.v1 = solidV1;
        quad.r = r;
        quad.g = g;
        quad.b = b;
        quad.a = a;
        return quad;
    };
    // Segment epais entre deux points monde, meme region de texture (unie) que solidQuad -- pour
    // un cadre de selection ORIENTE (LOT-50, revision post-livraison de TACHE-03), contrairement a
    // solidQuad qui ne dessine que des rectangles alignes aux axes.
    const auto segment = [&](core::Vector2 a, core::Vector2 b, float thickness, float r, float g,
                             float bl, float alpha, std::int32_t order) {
        LineQuad quad;
        quad.ax = a.x;
        quad.ay = a.y;
        quad.bx = b.x;
        quad.by = b.y;
        quad.thickness = thickness;
        quad.u0 = solidU0;
        quad.v0 = solidV0;
        quad.u1 = solidU1;
        quad.v1 = solidV1;
        quad.r = r;
        quad.g = g;
        quad.b = bl;
        quad.a = alpha;
        _scene.addLine(RenderLayer::EditorOverlay, _atlas.textureView(), order, quad);
    };
    // Cadre de selection : 4 segments joignant les coins du decor -- TOURNES avec lui
    // (hmi::decorRotatedPoint, meme angle que le sprite reellement rendu, LOT-50) plutot que rester
    // droits alors que le decor pivote sous eux. Jamais un voile plein, qui cacherait le decor.
    // Contour double ton (sombre puis cyan clair, EX-EDIT-030) pour rester lisible sur n'importe
    // quel fond -- un simple trait clair disparaitrait sur un fond clair.
    const float halfWidth = bounds.size.x * 0.5f;
    const float halfHeight = bounds.size.y * 0.5f;
    const core::Vector2 topLeftCorner =
        decorRotatedPoint(bounds, core::Vector2{-halfWidth, -halfHeight}, decor.rotation);
    const core::Vector2 topRightCorner =
        decorRotatedPoint(bounds, core::Vector2{halfWidth, -halfHeight}, decor.rotation);
    const core::Vector2 bottomRightCorner =
        decorRotatedPoint(bounds, core::Vector2{halfWidth, halfHeight}, decor.rotation);
    const core::Vector2 bottomLeftCorner =
        decorRotatedPoint(bounds, core::Vector2{-halfWidth, halfHeight}, decor.rotation);
    const auto border = [&](float thickness, float r, float g, float b, float a,
                            std::int32_t order) {
        segment(topLeftCorner, topRightCorner, thickness, r, g, b, a, order);
        segment(topRightCorner, bottomRightCorner, thickness, r, g, b, a, order);
        segment(bottomRightCorner, bottomLeftCorner, thickness, r, g, b, a, order);
        segment(bottomLeftCorner, topLeftCorner, thickness, r, g, b, a, order);
    };
    border(0.07f, 0.02f, 0.05f, 0.08f, 0.9f, OVERLAY_ORDER_DECOR_OUTLINE_DARK);
    border(0.035f, 0.25f, 0.95f, 1.0f, 0.95f, OVERLAY_ORDER_DECOR_OUTLINE_BRIGHT);

    // Poignees : meme principe double ton ; la poignee de rotation en vert pour la distinguer des
    // coins de redimensionnement (EX-EDIT-030, decouvrabilite).
    const auto handle = [&](const core::Rect& rect, float r, float g, float b) {
        constexpr float OUTSET = 0.015f;  // liseré sombre autour de chaque poignée
        _scene.addSprite(
            RenderLayer::EditorOverlay, _atlas.textureView(), OVERLAY_ORDER_DECOR_HANDLE_DARK,
            solidQuad(core::Rect{core::Vector2{rect.position.x - OUTSET, rect.position.y - OUTSET},
                                 core::Vector2{rect.size.x + OUTSET * 2.0f, rect.size.y + OUTSET * 2.0f}},
                      0.02f, 0.05f, 0.08f, 0.95f));
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureView(),
                         OVERLAY_ORDER_DECOR_HANDLE_BRIGHT, solidQuad(rect, r, g, b, 1.0f));
    };
    handle(handles.topLeft, 0.25f, 0.95f, 1.0f);
    handle(handles.topRight, 0.25f, 0.95f, 1.0f);
    handle(handles.bottomLeft, 0.25f, 0.95f, 1.0f);
    handle(handles.bottomRight, 0.25f, 0.95f, 1.0f);
    handle(handles.rotation, 0.4f, 0.95f, 0.35f);
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
            // le canevas de l'editeur montre exactement ce que le joueur verra. La surcharge de
            // texture par instance (EX-EDIT-043, LOT-45) y est resolue une fois, ici, comme le
            // voisinage solide.
            _world.addComponent(
                entity, TileSkinTag{type, solidNeighborMask(map, column, row),
                                    textureOverrideAt(draft.textureOverrides(),
                                                       core::GridPosition{column, row})});
        }
    }

    // Decors libres (EX-DEC-001, LOT-49) : meme construction que core::buildLevelScene (position
    // flottante, jamais calee sur la grille), reproduite ici a la main puisque LevelDraft n'est
    // pas un core::Level -- c'est ce qui garantit que le canevas de l'editeur montre exactement ce
    // que le jeu affichera une fois le brouillon valide.
    const std::vector<core::Decor>& decors = draft.decors();
    _decorEntities.clear();
    _decorEntities.reserve(decors.size());
    for (std::size_t index = 0; index < decors.size(); ++index) {
        const core::Decor& decor = decors[index];
        const core::Entity entity = _world.createEntity();
        _world.addComponent(entity, core::Transform{decor.position, decor.scale, decor.rotation});
        core::Sprite sprite;
        sprite.layer = static_cast<std::int32_t>(index);
        _world.addComponent(entity, sprite);
        _world.addComponent(entity, DecorVisualTag{decor.assetName, decor.layer});
        _world.addComponent(entity, RenderLayerTag{decorRenderLayer(decor.layer)});
        // Meme rang que dans draft.decors() (LOT-50 TACHE-03) : permet d'appliquer l'apercu d'un
        // geste en cours directement a cette entite, sans reconstruire toute la scene.
        _decorEntities.push_back(entity);
    }
}

}  // namespace hmi
