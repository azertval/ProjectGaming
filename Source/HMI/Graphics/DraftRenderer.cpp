#include "HMI/Graphics/DraftRenderer.h"

#include <algorithm>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Gameplay/PlatformPath.h"
#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/Decor.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/DecorGeometry.h"
#include "HMI/Editor/LinkGeometry.h"
#include "HMI/Editor/PathGeometry.h"
#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/DecorVisuals.h"
#include "HMI/Graphics/FollowCamera.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/Parallax.h"
#include "HMI/Graphics/RoomGrid.h"
#include "HMI/Graphics/ShadowRenderer.h"
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
// Previsualisation du cadrage de camera (LOT-64) : juste au-dessus de la grille de repere, sous
// tout le reste -- un grand rectangle qui ne doit jamais masquer un lien ou une poignee.
constexpr std::int32_t OVERLAY_ORDER_CAMERA_FRAMING = 1;
constexpr std::int32_t OVERLAY_ORDER_LINKS = 2;
constexpr std::int32_t OVERLAY_ORDER_HIGHLIGHT = 3;
constexpr std::int32_t OVERLAY_ORDER_TEXTURE_OVERRIDES = 4;
// Cadre de selection : contour double couleur (sombre puis clair, dessine par-dessus) pour rester
// lisible sur tout fond (LOT-50 TACHE-03), puis les poignees, encore au-dessus (memes deux tons).
constexpr std::int32_t OVERLAY_ORDER_DECOR_OUTLINE_DARK = 5;
constexpr std::int32_t OVERLAY_ORDER_DECOR_OUTLINE_BRIGHT = 6;
constexpr std::int32_t OVERLAY_ORDER_DECOR_HANDLE_DARK = 7;
constexpr std::int32_t OVERLAY_ORDER_DECOR_HANDLE_BRIGHT = 8;
// Parcours de plateforme mobile (LOT-63) : au-dessus des poignees de décor, dernier calque
// d'édition -- un repère de placement, jamais masqué par une sélection en cours.
constexpr std::int32_t OVERLAY_ORDER_PLATFORM_PATH = 9;
// Course d'un danger mobile (LOT-67) : juste au-dessus du parcours des plateformes, teinte
// distincte -- les deux peuvent se croiser sans qu'on confonde le sur quoi on marche et le qui
// tue. Les poignees de parcours reutilisent les ordres des poignees de decors : jamais les deux
// outils actifs en meme temps, donc aucun risque de superposition.
constexpr std::int32_t OVERLAY_ORDER_MOVER_PATH = 10;
}  // namespace

DraftRenderer::DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache)
    : _batch(batch), _atlas(atlas), _cache(cache) {}

void DraftRenderer::render(
    const core::LevelDraft& draft, const Camera2D& camera, bool showGrid,
    const std::optional<std::pair<core::GridPosition, core::GridPosition>>& highlight,
    const LinkOverlayState& linkOverlay, RenderMode mode, bool showTextureOverrides,
    float deltaSeconds, const DecorOverlayState& decorOverlay, const LayerVisibility& visibility,
    const PathOverlayState& pathOverlay) {
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
    // Calque Fond (LOT-51) : hors de composeWorldSprites (le fond ne vient pas de l'ECS), gate donc
    // ici l'appel entier plutot que de filtrer une primitive deja composee.
    if (visibility.visible(RenderLayer::Background)) {
        composeBackground(_scene, resolveBackgroundTexture(draft.background(), _cache),
                          draft.tileMap().width(), draft.tileMap().height(), mode);
    }
    const SceneTextures textures =
        sceneTextures(_atlas, _cache, _skins, _skinSet, draft.textureOverrides(), _tileAnimations,
                      draft.decors());
    // Calque Ombre (LOT-55) : meme raison de gate a l'appel que Fond ci-dessus -- aucune simulation
    // de mecanisme dans l'editeur (jamais de porte fermee/ouverte a suivre), donc pas de grille de
    // collision a transmettre.
    if (visibility.visible(RenderLayer::Shadow)) {
        composeShadows(_scene, _world, mode, textures, 1.0f);
    }
    composeWorldSprites(_scene, _world, mode, textures, 1.0f, &camera, visibility);
    if (showGrid) {
        composeGrid(draft, decorOverlay.snapToGrid);
    }
    composeCameraFraming(draft);
    composeLinks(draft, linkOverlay);
    composeMovingPlatformPaths(draft, pathOverlay);
    composeDangerMoverPaths(draft, pathOverlay);
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

    // Frontieres de salles (RoomGrid, LOT-32), a la taille RESOLUE du niveau (LOT-64 : reglable,
    // valeurs par defaut sinon -- jamais les seules constantes desormais, memes remarque que
    // composeCameraFraming ci-dessous pour le mode "par salle") : plus epaisses, teinte ambre.
    const core::CameraFramingConfig& framing = draft.cameraFraming();
    const int roomWidthTiles = framing.roomWidthTiles.value_or(core::kDefaultRoomWidthTiles);
    const int roomHeightTiles = framing.roomHeightTiles.value_or(core::kDefaultRoomHeightTiles);
    constexpr float ROOM_LINE = 0.09f;
    const float roomLineAlpha = accentuate ? 0.75f : 0.5f;
    for (int column = 0; column * roomWidthTiles <= width; ++column) {
        const float x = static_cast<float>(std::min(column * roomWidthTiles, width));
        add(lineQuad(x - ROOM_LINE * 0.5f, 0.0f, ROOM_LINE, h, 1.0f, 0.85f, 0.3f, roomLineAlpha));
    }
    for (int row = 0; row * roomHeightTiles <= height; ++row) {
        const float y = static_cast<float>(std::min(row * roomHeightTiles, height));
        add(lineQuad(0.0f, y - ROOM_LINE * 0.5f, w, ROOM_LINE, 1.0f, 0.85f, 0.3f, roomLineAlpha));
    }
}

// Compose la previsualisation du cadrage de camera du niveau (EX-EDIT-028, LOT-64, voir en-tete) :
// composee INCONDITIONNELLEMENT (comme composeLinks/composeDecorSelection), pas derriere le
// bascule F10 -- ce n'est pas une aide de placement mais une information sur ce que montrera la
// camera en jeu.
void DraftRenderer::composeCameraFraming(const core::LevelDraft& draft) {
    const int width = draft.tileMap().width();
    const int height = draft.tileMap().height();
    const core::AtlasRegion solid = _atlas.tile(0, 0);
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());

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
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureView(),
                         OVERLAY_ORDER_CAMERA_FRAMING, quad);
    };
    // Rectangle CREUX (quatre bords) : plus lisible qu'un voile plein sur une grande zone, et
    // distinguable de l'ambre de la grille de salles (teinte cyan).
    const auto strokeRect = [&](float x, float y, float w, float h, float thickness, float r,
                                float g, float b, float a) {
        add(lineQuad(x - thickness * 0.5f, y - thickness * 0.5f, w + thickness, thickness, r, g, b,
                     a));
        add(lineQuad(x - thickness * 0.5f, y + h - thickness * 0.5f, w + thickness, thickness, r, g,
                     b, a));
        add(lineQuad(x - thickness * 0.5f, y - thickness * 0.5f, thickness, h + thickness, r, g, b,
                     a));
        add(lineQuad(x + w - thickness * 0.5f, y - thickness * 0.5f, thickness, h + thickness, r, g,
                     b, a));
    };

    constexpr float FRAME_THICKNESS = 0.12f;
    constexpr float CYAN_R = 0.35f;
    constexpr float CYAN_G = 0.85f;
    constexpr float CYAN_B = 1.0f;

    const core::CameraFramingConfig& framing = draft.cameraFraming();
    switch (framing.mode) {
        case core::CameraFramingMode::WholeLevel:
            strokeRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height),
                       FRAME_THICKNESS, CYAN_R, CYAN_G, CYAN_B, 0.6f);
            break;
        case core::CameraFramingMode::PerRoom: {
            if (!framing.zones.empty()) {
                // Zones dessinees a la main (EX-LVL-007) : remplacent entierement la grille
                // automatique -- un rectangle par zone, dans l'ordre de la liste (la premiere
                // zone qui contient une position gagne en cas de chevauchement, hmi::
                // activeCameraZoneIndex, non visible ici : la previsualisation montre simplement
                // les rectangles tels que dessines).
                for (const core::CameraZone& zone : framing.zones) {
                    strokeRect(static_cast<float>(zone.x), static_cast<float>(zone.y),
                               static_cast<float>(zone.width), static_cast<float>(zone.height),
                               FRAME_THICKNESS, CYAN_R, CYAN_G, CYAN_B, 0.5f);
                }
                break;
            }
            // Reutilise hmi::RoomGrid (LOT-32) a la taille resolue -- jamais une seconde
            // implementation du decoupage en salles (tache-03).
            const int roomWidthTiles =
                framing.roomWidthTiles.value_or(core::kDefaultRoomWidthTiles);
            const int roomHeightTiles =
                framing.roomHeightTiles.value_or(core::kDefaultRoomHeightTiles);
            const RoomGrid rooms(width, height, roomWidthTiles, roomHeightTiles);
            for (int row = 0; row < rooms.rows(); ++row) {
                for (int column = 0; column < rooms.columns(); ++column) {
                    const RoomBounds bounds = rooms.roomBounds(core::GridPosition{column, row});
                    strokeRect(static_cast<float>(bounds.column), static_cast<float>(bounds.row),
                               static_cast<float>(bounds.width), static_cast<float>(bounds.height),
                               FRAME_THICKNESS, CYAN_R, CYAN_G, CYAN_B, 0.5f);
                }
            }
            break;
        }
        case core::CameraFramingMode::Follow: {
            const std::optional<core::GridPosition> entry = draft.entry();
            if (!entry) {
                break;  // pas d'entree posee : rien de significatif a previsualiser (tache-03).
            }
            const float viewWidth = static_cast<float>(core::kDefaultRoomWidthTiles);
            const float viewHeight = static_cast<float>(core::kDefaultRoomHeightTiles);
            // Meme regle de bornage/centrage qu'hmi::advanceFollowCamera (FollowCamera.cpp) : un
            // axe plus etroit que le cadrage centre plutot que borne -- previsualisation fidele au
            // comportement reel, pas une approximation.
            const auto clampCenter = [](float center, float levelSize, float viewSize) {
                if (levelSize <= viewSize) {
                    return levelSize * 0.5f;
                }
                return std::clamp(center, viewSize * 0.5f, levelSize - viewSize * 0.5f);
            };
            const float centerX = clampCenter(static_cast<float>(entry->column) + 0.5f,
                                              static_cast<float>(width), viewWidth);
            const float centerY = clampCenter(static_cast<float>(entry->row) + 0.5f,
                                              static_cast<float>(height), viewHeight);
            strokeRect(centerX - viewWidth * 0.5f, centerY - viewHeight * 0.5f, viewWidth,
                       viewHeight, FRAME_THICKNESS, CYAN_R, CYAN_G, CYAN_B, 0.6f);
            // Zone morte materialisee (tache-03) : plus fine, meme teinte, plus opaque (repere
            // secondaire a l'interieur du rectangle visible).
            strokeRect(centerX - FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS,
                       centerY - FOLLOW_DEAD_ZONE_HALF_HEIGHT_UNITS,
                       FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS * 2.0f,
                       FOLLOW_DEAD_ZONE_HALF_HEIGHT_UNITS * 2.0f, FRAME_THICKNESS * 0.6f, CYAN_R,
                       CYAN_G, CYAN_B, 0.85f);
            break;
        }
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

// Materialise le parcours de chaque plateforme mobile (LOT-63, EX-GP-026) : un trait fin entre
// son point de depart et son second point, teinte azur (meme famille que la couleur procedurale
// de MovingPlatform, TileVisuals.cpp), avec une pointe de fleche au second point -- lisible sans
// se confondre avec les liens de mecanismes (bleu/orange, composeLinks ci-dessus).
void DraftRenderer::composeMovingPlatformPaths(const core::LevelDraft& draft,
                                               const PathOverlayState& pathOverlay) {
    if (draft.platformConfigs().empty()) {
        return;
    }
    // Apercu du geste en cours (LOT-67) : applique sur une COPIE locale de la configuration visee,
    // jamais sur le brouillon -- le glisser ne devient une mutation qu'au relachement.
    std::vector<core::MovingPlatformConfig> configs = draft.platformConfigs();
    if (pathOverlay.preview && pathOverlay.preview->target.kind == PathTargetKind::Platform &&
        pathOverlay.preview->target.index < configs.size()) {
        const PathGestureAction& preview = *pathOverlay.preview;
        std::vector<core::GridPosition>& waypoints = configs[preview.target.index].waypoints;
        if (preview.kind == PathGestureActionKind::MoveWaypoint &&
            preview.waypointIndex < waypoints.size()) {
            waypoints[preview.waypointIndex] = preview.position;
        } else if (preview.kind == PathGestureActionKind::InsertWaypoint &&
                   preview.waypointIndex <= waypoints.size()) {
            waypoints.insert(waypoints.begin() + static_cast<std::ptrdiff_t>(preview.waypointIndex),
                             preview.position);
        }
    }

    const core::AtlasRegion solid = _atlas.tile(0, 0);  // region opaque unie (teintee).
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    const float u0 = static_cast<float>(solid.x) / atlasWidth;
    const float v0 = static_cast<float>(solid.y) / atlasHeight;
    const float u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    const float v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;

    constexpr float THICKNESS = 0.04f;
    constexpr float R = 0.0f;
    constexpr float G = 0.6f;
    constexpr float BLUE = 1.0f;
    constexpr float ALPHA = 0.6f;

    const auto addLine = [&](core::Vector2 a, core::Vector2 b) {
        LineQuad quad;
        quad.ax = a.x;
        quad.ay = a.y;
        quad.bx = b.x;
        quad.by = b.y;
        quad.thickness = THICKNESS;
        quad.u0 = u0;
        quad.v0 = v0;
        quad.u1 = u1;
        quad.v1 = v1;
        quad.r = R;
        quad.g = G;
        quad.b = BLUE;
        quad.a = ALPHA;
        _scene.addLine(RenderLayer::EditorOverlay, _atlas.textureView(),
                       OVERLAY_ORDER_PLATFORM_PATH, quad);
    };

    for (const core::MovingPlatformConfig& config : configs) {
        // Meme polyligne que celle reellement parcourue par le gameplay (core::PlatformPath) :
        // aucune reimplementation parallele de la trajectoire cote rendu. Les points sont des
        // coins haut-gauche de case, recentres ici pour relier les centres.
        const std::vector<core::Vector2> points = core::platformPathPoints(config);
        for (std::size_t index = 1; index < points.size(); ++index) {
            const core::Vector2 from{points[index - 1].x + 0.5f, points[index - 1].y + 0.5f};
            const core::Vector2 to{points[index].x + 0.5f, points[index].y + 0.5f};
            if (from == to) {
                continue;  // segment nul (point duplique) : rien a materialiser.
            }
            addLine(from, to);
            // Une pointe par segment : elle donne le SENS de parcours, indispensable des que la
            // route depasse deux points (et seule facon de distinguer un circuit ferme a l'oeil).
            const ArrowHead head = arrowHead(from, to);
            addLine(to, head.left);
            addLine(to, head.right);
        }
    }

    // Poignees du parcours SELECTIONNE uniquement : les afficher sur tous les parcours saturerait
    // le canevas des qu'un niveau en compte plusieurs.
    if (pathOverlay.selected && pathOverlay.selected->kind == PathTargetKind::Platform &&
        pathOverlay.selected->index < configs.size()) {
        composePathHandles(pathHandleLayout(configs[pathOverlay.selected->index],
                                            pathOverlay.worldUnitsPerScreenPixel));
    }
}

// Poignees d'un parcours : carres double ton (liseré sombre puis coeur clair), exactement le
// patron de composeDecorSelection -- lisible sur tout fond (EX-EDIT-030). Les milieux de segment
// se distinguent par leur couleur : ils AJOUTENT un point, les autres en deplacent un.
// Rectangle plein d'une aide d'edition, texture par la region unie de l'atlas puis teinte. Etait
// une lambda locale a composeDecorSelection ; extrait en methode quand les poignees de parcours
// (LOT-67) en ont eu besoin a leur tour -- une seule definition plutot que deux copies vouees a
// diverger au premier ajustement.
SpriteQuad DraftRenderer::solidOverlayQuad(const core::Rect& rect, float r, float g, float b,
                                           float a) const {
    const core::AtlasRegion solid = _atlas.tile(0, 0);
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    SpriteQuad quad;
    quad.x = rect.position.x;
    quad.y = rect.position.y;
    quad.width = rect.size.x;
    quad.height = rect.size.y;
    quad.u0 = static_cast<float>(solid.x) / atlasWidth;
    quad.v0 = static_cast<float>(solid.y) / atlasHeight;
    quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
    quad.r = r;
    quad.g = g;
    quad.b = b;
    quad.a = a;
    return quad;
}

void DraftRenderer::composePathHandles(const std::vector<PathHandle>& handles) {
    for (const PathHandle& handle : handles) {
        constexpr float OUTSET = 0.015f;
        _scene.addSprite(
            RenderLayer::EditorOverlay, _atlas.textureView(), OVERLAY_ORDER_DECOR_HANDLE_DARK,
            solidOverlayQuad(core::Rect{core::Vector2{handle.rect.position.x - OUTSET,
                                                      handle.rect.position.y - OUTSET},
                                        core::Vector2{handle.rect.size.x + (OUTSET * 2.0f),
                                                      handle.rect.size.y + (OUTSET * 2.0f)}},
                             0.02f, 0.05f, 0.08f, 0.95f));
        // L'amorce d'une route vide se dessine comme un milieu de segment : dans les deux cas, la
        // poignee designe un point A CREER, pas un point acquis (LOT-68).
        const bool midpoint =
            handle.kind == PathHandleKind::Midpoint || handle.kind == PathHandleKind::Origin;
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureView(),
                         OVERLAY_ORDER_DECOR_HANDLE_BRIGHT,
                         solidOverlayQuad(handle.rect, midpoint ? 0.4f : 0.25f, 0.95f,
                                          midpoint ? 0.35f : 1.0f, 1.0f));
    }
}

// Materialise la course aller-retour de chaque danger mobile (LOT-67, EX-GP-051) : un trait de sa
// case de depart a son extremite, teinte rouge-orangee (famille danger, distincte de l'azur des
// plateformes et du bleu/orange des liens), avec une pointe a CHAQUE bout -- la course est un
// aller-retour, pas un sens unique.
void DraftRenderer::composeDangerMoverPaths(const core::LevelDraft& draft,
                                            const PathOverlayState& pathOverlay) {
    if (draft.moverConfigs().empty()) {
        return;
    }
    const core::AtlasRegion solid = _atlas.tile(0, 0);
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());

    constexpr float THICKNESS = 0.04f;
    const auto addLine = [&](core::Vector2 a, core::Vector2 b) {
        LineQuad quad;
        quad.ax = a.x;
        quad.ay = a.y;
        quad.bx = b.x;
        quad.by = b.y;
        quad.thickness = THICKNESS;
        quad.u0 = static_cast<float>(solid.x) / atlasWidth;
        quad.v0 = static_cast<float>(solid.y) / atlasHeight;
        quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
        quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
        quad.r = 1.0f;
        quad.g = 0.35f;
        quad.b = 0.2f;
        quad.a = 0.6f;
        _scene.addLine(RenderLayer::EditorOverlay, _atlas.textureView(), OVERLAY_ORDER_MOVER_PATH,
                       quad);
    };

    for (const core::DangerMoverConfig& config : draft.moverConfigs()) {
        const core::Vector2 from{static_cast<float>(config.startPosition.column) + 0.5f,
                                 static_cast<float>(config.startPosition.row) + 0.5f};
        const core::Vector2 to{from.x + (config.axis == core::DangerMoverAxis::Horizontal
                                             ? static_cast<float>(config.range)
                                             : 0.0f),
                               from.y + (config.axis == core::DangerMoverAxis::Vertical
                                             ? static_cast<float>(config.range)
                                             : 0.0f)};
        if (from == to) {
            continue;  // portee nulle : danger immobile, rien a materialiser.
        }
        addLine(from, to);
        const ArrowHead forward = arrowHead(from, to);
        addLine(to, forward.left);
        addLine(to, forward.right);
        const ArrowHead backward = arrowHead(to, from);
        addLine(from, backward.left);
        addLine(from, backward.right);
    }

    if (pathOverlay.selected && pathOverlay.selected->kind == PathTargetKind::Mover &&
        pathOverlay.selected->index < draft.moverConfigs().size()) {
        composePathHandles({moverHandleLayout(draft.moverConfigs()[pathOverlay.selected->index],
                                              pathOverlay.worldUnitsPerScreenPixel)});
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
        pixelSize =
            core::Vector2{static_cast<float>(loaded->width), static_cast<float>(loaded->height)};
    }
    const core::Rect bounds = decorWorldBounds(decor, pixelSize);
    const float worldUnitsPerScreenPixel = 1.0f / (Camera2D::PIXELS_PER_UNIT * camera.zoom());
    const DecorHandleLayout handles =
        decorHandleLayout(bounds, worldUnitsPerScreenPixel, decor.rotation);

    const core::AtlasRegion solid = _atlas.tile(0, 0);
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    const float solidU0 = static_cast<float>(solid.x) / atlasWidth;
    const float solidV0 = static_cast<float>(solid.y) / atlasHeight;
    const float solidU1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    const float solidV1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
    const auto solidQuad = [this](const core::Rect& rect, float r, float g, float b, float a) {
        return solidOverlayQuad(rect, r, g, b, a);
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
            solidQuad(
                core::Rect{core::Vector2{rect.position.x - OUTSET, rect.position.y - OUTSET},
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
            _world.addComponent(entity,
                                TileSkinTag{type, solidNeighborMask(map, column, row),
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
