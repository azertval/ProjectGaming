#include "HMI/Interface/GameScreen.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelScene.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/RenderContext.h"

namespace hmi {

namespace {

// Region d'atlas (couleur) associee a chaque type de tuile, pour un rendu distinct.
core::AtlasRegion regionForTile(core::TileType type, const TextureAtlas& atlas) {
    switch (type) {
        case core::TileType::Solid:
            return atlas.tile(0, 2);  // gris
        case core::TileType::Danger:
            return atlas.tile(0, 0);  // rouge
        case core::TileType::Entry:
            return atlas.tile(1, 0);  // vert
        case core::TileType::Exit:
            return atlas.tile(2, 0);  // bleu
        case core::TileType::Switch:
            return atlas.tile(3, 0);  // jaune
        case core::TileType::Door:
            return atlas.tile(2, 1);  // orange
        case core::TileType::Empty:
            break;
    }
    return atlas.tile(0, 0);
}

}  // namespace

// Construit l'ecran et charge le niveau.
GameScreen::GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                       int viewportHeight, std::filesystem::path levelPath)
    : _camera(viewportWidth, viewportHeight), _renderer(batch, atlas) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromFile(levelPath);
    if (result.ok()) {
        const core::Level& level = *result.level;
        _levelWidth = level.tileMap().width();
        _levelHeight = level.tileMap().height();
        _camera.setCenter(core::Vector2{static_cast<float>(_levelWidth) * 0.5f,
                                        static_cast<float>(_levelHeight) * 0.5f});
        // La correspondance type -> region d'atlas (rendu) est injectee dans la projection pure.
        core::buildLevelScene(_world, level, [&atlas](core::TileType type) {
            return regionForTile(type, atlas);
        });
        HMI_LOG_INFO("Niveau charge : " + level.name() + " (" + std::to_string(_levelWidth) + "x" +
                     std::to_string(_levelHeight) + ")");
    } else {
        // Echec recuperable : on retient le message et on affichera un etat neutre.
        _loadError = result.error;
        HMI_LOG_WARNING("Echec du chargement du niveau : " + result.error);
    }
}

// Gere le retour au menu (le niveau est statique : aucune simulation ici).
ScreenTransition GameScreen::update(const InputState& input, float /*fixedDelta*/) {
    if (input.keyPressed(Key::Escape)) {
        return ScreenTransition::switchTo(ScreenId::Menu);
    }
    return ScreenTransition::none();
}

// Dessine le niveau charge, ou un etat neutre si le chargement a echoue.
void GameScreen::render(RenderContext& context) {
    if (!_loadError.empty()) {
        // Etat d'erreur : message centre a l'ecran.
        const char* message = "Niveau indisponible";
        constexpr float scale = 4.0f;
        const float x = (static_cast<float>(context.viewportWidth) -
                         context.font.textWidth(message, scale)) *
                        0.5f;
        const float y =
            (static_cast<float>(context.viewportHeight) - context.font.lineHeight(scale)) * 0.5f;
        const DirectX::XMFLOAT4X4 projection =
            BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
        context.spriteBatch.begin(projection, context.font.textureView());
        context.font.drawText(context.spriteBatch, message, x, y, scale,
                              core::Color{0.90f, 0.55f, 0.55f, 1.0f});
        context.spriteBatch.end();
        return;
    }

    _camera.setViewportSize(context.viewportWidth, context.viewportHeight);

    // Zoom pour faire tenir le niveau dans la fenetre, en facteur entier (nettete pixel art).
    const float fitX = static_cast<float>(context.viewportWidth) /
                       (static_cast<float>(_levelWidth) * Camera2D::PIXELS_PER_UNIT);
    const float fitY = static_cast<float>(context.viewportHeight) /
                       (static_cast<float>(_levelHeight) * Camera2D::PIXELS_PER_UNIT);
    const float zoom = std::max(1.0f, std::floor(std::min(fitX, fitY) * 0.92f));
    _camera.setZoom(zoom);

    _renderer.render(_world, _camera);
}

}  // namespace hmi
