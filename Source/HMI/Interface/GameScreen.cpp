#include "HMI/Interface/GameScreen.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/LevelScene.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlayerInput.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/PlayerInputMapper.h"
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

// Construit l'ecran et charge le premier niveau de la sequence.
GameScreen::GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                       int viewportHeight, std::vector<std::filesystem::path> levels)
    : _atlas(atlas),
      _camera(viewportWidth, viewportHeight),
      _renderer(batch, atlas),
      _sequence(std::move(levels)) {
    if (_sequence.empty()) {
        _loadError = "Aucun niveau a charger.";  // robustesse : sequence vide -> etat neutre
        HMI_LOG_WARNING(_loadError);
        return;
    }
    loadLevel(_sequence.current());
}

// (Re)construit la scene pour un niveau : monde neuf + grille + personnage a l'entree.
void GameScreen::loadLevel(const std::filesystem::path& path) {
    _world = core::World{};  // repart d'un monde vierge (aucune entite du niveau precedent)
    _loadError.clear();

    core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
    if (!result.ok()) {
        // Echec recuperable : on retient le message et on affichera un etat neutre.
        _level.reset();
        _loadError = result.error;
        HMI_LOG_WARNING("Echec du chargement du niveau : " + result.error);
        return;
    }

    _level = std::move(result.level);  // conserve le niveau pour la simulation et le reset
    const core::Level& level = *_level;
    _levelWidth = level.tileMap().width();
    _levelHeight = level.tileMap().height();
    _camera.setCenter(core::Vector2{static_cast<float>(_levelWidth) * 0.5f,
                                    static_cast<float>(_levelHeight) * 0.5f});
    // La correspondance type -> region d'atlas (rendu) est injectee dans la projection pure.
    core::buildLevelScene(_world, level,
                          [this](core::TileType type) { return regionForTile(type, _atlas); });
    spawnPlayer(level.entry());
    HMI_LOG_INFO("Niveau charge : " + level.name() + " (" + std::to_string(_levelWidth) + "x" +
                 std::to_string(_levelHeight) + ")");
}

// Fait apparaitre le personnage a l'entree du niveau (voir en-tete).
void GameScreen::spawnPlayer(core::GridPosition entry) {
    _player = _world.createEntity();
    _world.addComponent(_player, core::Transform{core::Vector2{static_cast<float>(entry.column),
                                                               static_cast<float>(entry.row)},
                                                 core::Vector2{1.0f, 1.0f}, 0.0f});
    _world.addComponent(_player, core::Velocity{});
    _world.addComponent(_player, core::Collider{core::Vector2{1.0f, 1.0f}});
    _world.addComponent(_player, core::Player{});
    // Sprite du personnage : couche haute (dessine par-dessus les tuiles), teinte claire.
    core::Sprite sprite;
    sprite.region = _atlas.tile(1, 1);
    sprite.layer = 100;
    sprite.tint = core::Color{1.0f, 1.0f, 1.0f, 1.0f};
    _world.addComponent(_player, sprite);
}

// Remet le personnage a l'entree, immobile (apres un echec).
void GameScreen::resetPlayer() {
    const core::GridPosition entry = _level->entry();
    _world.getComponent<core::Transform>(_player).position =
        core::Vector2{static_cast<float>(entry.column), static_cast<float>(entry.row)};
    _world.getComponent<core::Velocity>(_player).value = core::Vector2{0.0f, 0.0f};
    _world.getComponent<core::Player>(_player).grounded = false;
}

// Simule le personnage d'un pas fixe, puis statue sur l'issue du niveau.
ScreenTransition GameScreen::update(const InputState& input, float fixedDelta) {
    if (input.keyPressed(Key::Escape)) {
        return ScreenTransition::switchTo(ScreenId::Menu);
    }
    if (!_level) {
        return ScreenTransition::none();  // chargement echoue : rien a simuler
    }

    // 1. Entrees -> intention (action logique), 2. physique au pas fixe.
    const core::PlayerInput intent = toPlayerInput(input);
    _physics.update(_world, _level->tileMap(), intent, fixedDelta);

    // 3. Issue du niveau depuis la boite du personnage.
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, collider.size);
    switch (core::evaluateOutcome(box, *_level)) {
        case core::LevelOutcome::Won:
            if (_sequence.hasNext()) {
                // Enchaine le niveau suivant : on reste sur l'ecran de jeu (EX-LVL-011).
                _sequence.advance();
                HMI_LOG_INFO("Niveau termine : passage au niveau suivant.");
                loadLevel(_sequence.current());
                return ScreenTransition::none();
            }
            // Dernier niveau franchi : retour au titre.
            HMI_LOG_INFO("Sequence terminee : retour au menu.");
            return ScreenTransition::switchTo(ScreenId::Menu);
        case core::LevelOutcome::Lost:
            resetPlayer();  // echec : on redemarre le personnage a l'entree
            break;
        case core::LevelOutcome::Playing:
            break;
    }
    return ScreenTransition::none();
}

// Dessine le niveau charge, ou un etat neutre si le chargement a echoue.
void GameScreen::render(RenderContext& context) {
    if (!_loadError.empty()) {
        // Etat d'erreur : message centre a l'ecran.
        const char* message = "Niveau indisponible";
        constexpr float scale = 4.0f;
        const float x =
            (static_cast<float>(context.viewportWidth) - context.font.textWidth(message, scale)) *
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
