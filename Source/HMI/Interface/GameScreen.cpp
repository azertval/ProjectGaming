#include "HMI/Interface/GameScreen.h"

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
#include "Core/Physics/PlayerSpawn.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/PlayerInputMapper.h"
#include "HMI/Interface/RenderContext.h"

namespace hmi {

// Construit l'ecran et charge le premier niveau de la sequence.
GameScreen::GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                       int viewportHeight, std::vector<std::filesystem::path> levels)
    : _atlas(atlas),
      _camera(viewportWidth, viewportHeight),
      _renderer(batch, atlas),
      _sequence(LevelSequence(std::move(levels))) {
    if (_sequence->empty()) {
        _loadError = "Aucun niveau a charger.";  // robustesse : sequence vide -> etat neutre
        HMI_LOG_WARNING(_loadError);
        return;
    }
    loadLevel(_sequence->current());
}

// Construit l'ecran pour un niveau unique deja en memoire (essai immediat de l'editeur, LOT-15) :
// pas de sequence/fichier, la sortie termine l'essai au lieu d'enchainer (voir update()).
GameScreen::GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                       int viewportHeight, core::Level level)
    : _atlas(atlas), _camera(viewportWidth, viewportHeight), _renderer(batch, atlas) {
    loadLevel(std::move(level));
}

// Charge le niveau path depuis un fichier, puis delegue a loadLevel(core::Level) ; echec
// recuperable (EX-NFR-040) : _level reste vide, _loadError est renseigne.
void GameScreen::loadLevel(const std::filesystem::path& path) {
    core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
    if (!result.ok()) {
        _world = core::World{};  // repart d'un monde vierge (etat neutre)
        _level.reset();
        _loadError = result.error;
        HMI_LOG_WARNING("Echec du chargement du niveau : " + result.error);
        return;
    }
    loadLevel(std::move(*result.level));
}

// (Re)construit la scene pour un niveau deja charge et valide : monde neuf + grille + personnage
// a l'entree. Coeur commun aux deux constructeurs et aux rechargements (echec, niveau suivant).
void GameScreen::loadLevel(core::Level level) {
    _world = core::World{};  // repart d'un monde vierge (aucune entite du niveau precedent)
    _loadError.clear();

    _level = std::move(level);  // conserve le niveau pour la simulation et le reset
    const core::Level& levelRef = *_level;  // level est deplace : plus lu au-dela de cette ligne
    _levelWidth = levelRef.tileMap().width();
    _levelHeight = levelRef.tileMap().height();
    _camera.setCenter(core::Vector2{static_cast<float>(_levelWidth) * 0.5f,
                                    static_cast<float>(_levelHeight) * 0.5f});
    // La correspondance type -> region d'atlas (rendu) est injectee dans la projection pure.
    core::buildLevelScene(_world, levelRef,
                          [this](core::TileType type) { return regionForTile(type, _atlas); });
    // Mecanismes : etat interrupteurs/portes + grille de collision (portes fermees = solides).
    _mechanisms.emplace(levelRef);
    // Repere l'entite-tuile de chaque porte (avant le spawn du perso) pour le retour visuel d'etat.
    _doorEntities.clear();
    for (const core::Mechanism& mechanism : _mechanisms->mechanisms()) {
        core::Entity doorEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found &&
                    static_cast<int>(transform.position.x) == mechanism.doorPosition.column &&
                    static_cast<int>(transform.position.y) == mechanism.doorPosition.row) {
                    doorEntity = entity;
                    found = true;
                }
            });
        _doorEntities.push_back(doorEntity);
    }
    spawnPlayer(levelRef.entry());
    HMI_LOG_INFO("Niveau charge : " + levelRef.name() + " (" + std::to_string(_levelWidth) + "x" +
                 std::to_string(_levelHeight) + ")");
}

// Fait apparaitre le personnage humanoide (0,4x0,8), centre dans la tuile d'entree (voir en-tete).
void GameScreen::spawnPlayer(core::GridPosition entry) {
    _player = _world.createEntity();
    const core::Vector2 size = core::playerSize();  // collision ET rendu partagent la meme taille
    _world.addComponent(
        _player, core::Transform{core::playerSpawnPosition(entry.column, entry.row), size, 0.0f});
    _world.addComponent(_player, core::Velocity{});
    _world.addComponent(_player, core::Collider{size});
    // Budget de mouvements du tableau (EX-GP-024) : -1 = illimite si le niveau n'en fixe pas.
    core::Player playerComponent;
    playerComponent.jumpsRemaining = _level->jumpBudget();
    playerComponent.dashesRemaining = _level->dashBudget();
    _world.addComponent(_player, playerComponent);
    // Sprite du personnage : couche haute (dessine par-dessus les tuiles), teinte claire. La
    // taille a l'ecran suit l'echelle du Transform (silhouette humanoide).
    core::Sprite sprite;
    sprite.region = _atlas.tile(1, 1);
    sprite.layer = 100;
    sprite.tint = core::Color{1.0f, 1.0f, 1.0f, 1.0f};
    _world.addComponent(_player, sprite);
}

// Met a jour la teinte des sprites de portes selon leur etat (ouverte attenuee / fermee opaque).
void GameScreen::refreshDoorVisuals() {
    for (std::size_t index = 0; index < _doorEntities.size(); ++index) {
        const core::Entity door = _doorEntities[index];
        if (!_world.hasComponent<core::Sprite>(door)) {
            continue;  // porte non reperee (robustesse) : rien a faire
        }
        const float alpha = _mechanisms->isDoorOpen(index) ? 0.25f : 1.0f;
        _world.getComponent<core::Sprite>(door).tint = core::Color{1.0f, 1.0f, 1.0f, alpha};
    }
}

// Simule le personnage d'un pas fixe (mecanismes + physique), puis statue sur l'issue du niveau.
ScreenTransition GameScreen::update(const InputState& input, float fixedDelta) {
    if (input.keyPressed(Key::Escape)) {
        return ScreenTransition::switchTo(ScreenId::Menu);
    }
    if (!_level) {
        return ScreenTransition::none();  // chargement echoue : rien a simuler
    }

    // 1. Entrees -> intention, 2. physique sur la grille des MECANISMES (portes fermees = solides).
    const core::PlayerInput intent = toPlayerInput(input);
    _physics.update(_world, _mechanisms->collisionMap(), intent, fixedDelta);

    // 3. Boite du personnage apres deplacement.
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, collider.size);

    // 4. Mecanismes : contact des interrupteurs -> etat des portes (pour le pas suivant + visuel).
    _mechanisms->update(box);
    refreshDoorVisuals();

    // 5. Issue du niveau.
    switch (core::evaluateOutcome(box, *_level)) {
        case core::LevelOutcome::Won:
            if (_sequence && _sequence->hasNext()) {
                // Enchaine le niveau suivant : on reste sur l'ecran de jeu (EX-LVL-011).
                _sequence->advance();
                HMI_LOG_INFO("Niveau termine : passage au niveau suivant.");
                loadLevel(_sequence->current());
                return ScreenTransition::none();
            }
            // Dernier niveau de la sequence franchi, ou niveau unique en memoire (essai immediat,
            // LOT-15, pas de sequence) : retour au titre, sans enchainement.
            HMI_LOG_INFO("Niveau/sequence termine(e) : retour au menu.");
            return ScreenTransition::switchTo(ScreenId::Menu);
        case core::LevelOutcome::Lost:
            // Echec : rechargement COMPLET du niveau (perso a l'entree, mecanismes et budget
            // remis) depuis le Level deja en memoire — pas de nouvelle lecture disque, valable
            // aussi bien en mode sequence qu'en mode niveau unique en memoire.
            loadLevel(*_level);
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

    // Zoom pour faire tenir le niveau ENTIER dans la fenetre (LOT-16, EX-REN-013) : entier
    // (nettete pixel art) tant qu'il tient a l'echelle x1, fractionnaire au-dela pour qu'aucune
    // zone ne reste hors champ — la camera cadre le niveau, elle ne suit pas le personnage.
    const float zoom =
        Camera2D::fitZoom(static_cast<float>(context.viewportWidth),
                          static_cast<float>(context.viewportHeight),
                          static_cast<float>(_levelWidth), static_cast<float>(_levelHeight), 0.92f);
    _camera.setZoom(zoom);

    _renderer.render(_world, _camera);
}

}  // namespace hmi
