#include "HMI/Interface/EditorScreen.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/RenderContext.h"

namespace hmi {

namespace {

// Dimensions par defaut d'un brouillon vierge (comparables aux niveaux livres, ex. demo3/demo4).
constexpr int DEFAULT_WIDTH = 14;
constexpr int DEFAULT_HEIGHT = 8;

// Construit un quad texture a partir d'une region d'atlas et d'un rectangle (espace quelconque,
// pilote par la projection active du SpriteBatch), avec une teinte optionnelle.
SpriteQuad quadFor(const core::AtlasRegion& region, float x, float y, float width, float height,
                   const TextureAtlas& atlas, core::Color tint = core::Color{}) {
    const float atlasWidth = static_cast<float>(atlas.width());
    const float atlasHeight = static_cast<float>(atlas.height());
    SpriteQuad quad;
    quad.x = x;
    quad.y = y;
    quad.width = width;
    quad.height = height;
    quad.u0 = static_cast<float>(region.x) / atlasWidth;
    quad.v0 = static_cast<float>(region.y) / atlasHeight;
    quad.u1 = static_cast<float>(region.x + region.width) / atlasWidth;
    quad.v1 = static_cast<float>(region.y + region.height) / atlasHeight;
    quad.r = tint.r;
    quad.g = tint.g;
    quad.b = tint.b;
    quad.a = tint.a;
    return quad;
}

}  // namespace

// Construit l'editeur avec un brouillon de niveau vierge.
EditorScreen::EditorScreen(SpriteBatch& /*batch*/, const TextureAtlas& atlas, int viewportWidth,
                           int viewportHeight)
    : _atlas(atlas),
      _draft(core::LevelDraft::empty("Nouveau niveau", DEFAULT_WIDTH, DEFAULT_HEIGHT)),
      _camera(viewportWidth, viewportHeight) {
    HMI_LOG_TRACE("EditorScreen cree (brouillon vierge " + std::to_string(DEFAULT_WIDTH) + "x" +
                 std::to_string(DEFAULT_HEIGHT) + ")");
}

// Convertit une position souris en case de grille, si elle est dans les bornes du brouillon.
std::optional<core::GridPosition> EditorScreen::hoveredCell(float mouseX, float mouseY) const {
    const core::Vector2 world = _camera.screenToWorld(core::Vector2{mouseX, mouseY});
    const int column = static_cast<int>(std::floor(world.x));
    const int row = static_cast<int>(std::floor(world.y));
    if (!_draft.tileMap().inBounds(column, row)) {
        return std::nullopt;
    }
    return core::GridPosition{column, row};
}

// Clic palette -> selection ; clic/glisser sur la grille -> peinture du type actif ; Echap -> menu.
ScreenTransition EditorScreen::update(const InputState& input, float /*fixedDelta*/) {
    if (input.keyPressed(Key::Escape)) {
        return ScreenTransition::switchTo(ScreenId::Menu);
    }

    _mouseX = static_cast<float>(input.mouseX());
    _mouseY = static_cast<float>(input.mouseY());

    if (input.mouseButtonPressed(MouseButton::Left)) {
        // Le clic initial decide si ce geste peint la grille ou agit sur la palette : la
        // palette, dessinee par-dessus la grille, est prioritaire.
        _paintingDrag = !_palette.handleClick(_mouseX, _mouseY);
    }

    if (_paintingDrag && input.mouseButtonDown(MouseButton::Left)) {
        if (const std::optional<core::GridPosition> cell = hoveredCell(_mouseX, _mouseY)) {
            _draft.paintTile(cell->column, cell->row, _palette.selected());
        }
    }

    return ScreenTransition::none();
}

// Dessine la grille du brouillon (tuiles non vides) et la case survolee en surbrillance.
void EditorScreen::renderGrid(RenderContext& context) {
    const int width = _draft.tileMap().width();
    const int height = _draft.tileMap().height();
    _camera.setCenter(
        core::Vector2{static_cast<float>(width) * 0.5f, static_cast<float>(height) * 0.5f});

    // Zoom pour faire tenir la grille dans la fenetre, en facteur entier (nettete pixel art).
    const float fitX = static_cast<float>(context.viewportWidth) /
                       (static_cast<float>(width) * Camera2D::PIXELS_PER_UNIT);
    const float fitY = static_cast<float>(context.viewportHeight) /
                       (static_cast<float>(height) * Camera2D::PIXELS_PER_UNIT);
    const float zoom = (std::max)(1.0f, std::floor((std::min)(fitX, fitY) * 0.85f));
    _camera.setZoom(zoom);

    context.spriteBatch.begin(_camera.projectionMatrix(), _atlas.textureView());
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            const core::TileType type = _draft.tileMap().tile(column, row);
            if (type == core::TileType::Empty) {
                continue;
            }
            context.spriteBatch.draw(quadFor(regionForTile(type, _atlas),
                                             static_cast<float>(column),
                                             static_cast<float>(row), 1.0f, 1.0f, _atlas));
        }
    }

    // Surbrillance de la case survolee : quad blanc translucide par-dessus la tuile.
    if (const std::optional<core::GridPosition> hovered = hoveredCell(_mouseX, _mouseY)) {
        constexpr core::Color HIGHLIGHT_TINT{1.0f, 1.0f, 1.0f, 0.35f};
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), static_cast<float>(hovered->column),
                                         static_cast<float>(hovered->row), 1.0f, 1.0f, _atlas,
                                         HIGHLIGHT_TINT));
    }
    context.spriteBatch.end();
}

// Dessine la palette : une couleur par type, la selection encadree.
void EditorScreen::renderPalette(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, _atlas.textureView());

    constexpr float BORDER = 3.0f;
    constexpr core::Color SELECTION_TINT{1.0f, 1.0f, 0.4f, 1.0f};
    for (const TilePalette::Entry& entry : _palette.entries()) {
        if (entry.type == _palette.selected()) {
            context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), entry.x - BORDER,
                                             entry.y - BORDER, entry.width + 2.0f * BORDER,
                                             entry.height + 2.0f * BORDER, _atlas,
                                             SELECTION_TINT));
        }
        context.spriteBatch.draw(
            quadFor(regionForTile(entry.type, _atlas), entry.x, entry.y, entry.width,
                   entry.height, _atlas));
    }
    context.spriteBatch.end();
}

// Dessine la grille du niveau en cours d'edition, puis la palette par-dessus.
void EditorScreen::render(RenderContext& context) {
    _camera.setViewportSize(context.viewportWidth, context.viewportHeight);
    renderGrid(context);
    renderPalette(context);
}

}  // namespace hmi
