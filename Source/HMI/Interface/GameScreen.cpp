#include "HMI/Interface/GameScreen.h"

#include <utility>

#include "Core/Ecs/Components/Sprite.h"  // core::Color
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/RenderContext.h"

namespace hmi {

// Construit l'ecran et charge le premier niveau de la sequence.
GameScreen::GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                       int viewportHeight, std::vector<std::filesystem::path> levels,
                       const GameKeyBindings& gameBindings, const GamepadBindings& gamepadBindings)
    : _batch(batch),
      _atlas(atlas),
      _gameBindings(gameBindings),
      _gamepadBindings(gamepadBindings),
      _viewportWidth(viewportWidth),
      _viewportHeight(viewportHeight),
      _sequence(LevelSequence(std::move(levels))) {
    if (_sequence->empty()) {
        _loadError = "Aucun niveau a charger.";  // robustesse : sequence vide -> etat neutre
        HMI_LOG_WARNING(_loadError);
        return;
    }
    loadCurrentSequenceLevel();
}

// Construit l'ecran pour un niveau unique deja en memoire (essai immediat de l'editeur) : pas de
// sequence/fichier, la sortie termine l'essai au lieu d'enchainer (voir update()).
GameScreen::GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                       int viewportHeight, core::Level level, const GameKeyBindings& gameBindings,
                       const GamepadBindings& gamepadBindings)
    : _batch(batch),
      _atlas(atlas),
      _gameBindings(gameBindings),
      _gamepadBindings(gamepadBindings),
      _viewportWidth(viewportWidth),
      _viewportHeight(viewportHeight) {
    _session.emplace(_batch, _atlas, _viewportWidth, _viewportHeight, std::move(level), _gameBindings,
                     _gamepadBindings);
}

// Charge le niveau courant de la sequence depuis un fichier et (re)cree la session ; echec
// recuperable (EX-NFR-040) : _session reste vide, _loadError est renseigne.
void GameScreen::loadCurrentSequenceLevel() {
    core::LevelLoadResult result = core::LevelLoader::loadFromFile(_sequence->current());
    if (!result.ok()) {
        _session.reset();  // etat neutre (aucune scene a simuler/rendre)
        _loadError = result.error;
        HMI_LOG_WARNING("Echec du chargement du niveau : " + result.error);
        return;
    }
    _loadError.clear();
    _session.emplace(_batch, _atlas, _viewportWidth, _viewportHeight, std::move(*result.level),
                     _gameBindings, _gamepadBindings);
}

// Simule le niveau courant d'un pas fixe (delegue a GameSession), puis gere l'enchainement de
// sequence / le retour au menu selon l'issue.
ScreenTransition GameScreen::update(const InputState& input, float fixedDelta) {
    if (input.keyPressed(Key::Escape)) {
        return ScreenTransition::switchTo(ScreenId::Menu);
    }
    if (!_session) {
        return ScreenTransition::none();  // chargement echoue : rien a simuler
    }

    switch (_session->update(input, fixedDelta)) {
        case core::LevelOutcome::Won:
            if (_sequence && _sequence->hasNext()) {
                // Enchaine le niveau suivant : on reste sur l'ecran de jeu (EX-LVL-011).
                _sequence->advance();
                HMI_LOG_INFO("Niveau termine : passage au niveau suivant.");
                loadCurrentSequenceLevel();
                return ScreenTransition::none();
            }
            // Dernier niveau de la sequence franchi, ou niveau unique en memoire (essai immediat,
            // pas de sequence) : retour au titre, sans enchainement.
            HMI_LOG_INFO("Niveau/sequence termine(e) : retour au menu.");
            return ScreenTransition::switchTo(ScreenId::Menu);
        case core::LevelOutcome::Lost:
            // Echec : GameSession a deja recharge le niveau courant (perso a l'entree, etat remis).
            break;
        case core::LevelOutcome::Playing:
            break;
    }
    return ScreenTransition::none();
}

// Dessine le niveau charge (delegue a GameSession), ou un etat neutre si le chargement a echoue.
void GameScreen::render(RenderContext& context) {
    if (!_session) {
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

    _session->render(context.viewportWidth, context.viewportHeight, context.interpolationAlpha);
}

}  // namespace hmi
