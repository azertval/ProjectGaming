#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/SpriteRenderer.h"
#include "HMI/Interface/IScreen.h"
#include "HMI/Interface/LevelSequence.h"

/**
 * @file HMI/Interface/GameScreen.h
 * @brief Écran de jeu : joue une séquence de niveaux, avec enchaînement à la réussite.
 */

namespace hmi {

class SpriteBatch;
class TextureAtlas;

/**
 * @brief Écran de jeu jouant une **séquence de niveaux** chargés depuis des fichiers.
 *
 * Chaque niveau (`core::LevelLoader`) est projeté en **entités ECS** rendues par le
 * `SpriteRenderer` en lecture seule (`EX-ARCH-012`) ; un échec de chargement est **récupérable**
 * (`EX-NFR-040`, état neutre au lieu de planter). Un **personnage** apparaît à l'entrée et est
 * jouable : la physique (déplacement, gravité, saut, collisions) s'applique à chaque pas fixe.
 * Atteindre la **sortie** (`EX-GP-030`) **enchaîne le niveau suivant** de la séquence ; après le
 * **dernier**, retour au menu/titre (`EX-LVL-010`, `EX-LVL-011`). Toucher un **danger** ou tomber
 * **redémarre** le niveau courant à l'entrée (`EX-GP-031`, `EX-GP-032`). La caméra reste **fixe**
 * et cadre le tableau ; **Échap** revient au menu. Le comportement des mécanismes relève d'un lot
 * ultérieur.
 */
class GameScreen : public IScreen {
public:
    /**
     * @brief Construit l'écran et charge le premier niveau de la séquence.
     * @param batch          Lot de sprites partagé (rendu).
     * @param atlas          Atlas de tuiles fournissant les régions de sprites.
     * @param viewportWidth  Largeur initiale de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur initiale de la surface de rendu, en pixels.
     * @param levels         Liste **ordonnée** des chemins de niveaux à enchaîner.
     */
    GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth, int viewportHeight,
               std::vector<std::filesystem::path> levels);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    /// Charge le niveau @p path : reconstruit la scène (monde ECS + personnage à l'entrée).
    void loadLevel(const std::filesystem::path& path);

    /// Fait apparaître le personnage (Player + Transform + Velocity + Collider + Sprite) à
    /// l'entrée.
    void spawnPlayer(core::GridPosition entry);

    /// Réinitialise le personnage à l'entrée du niveau courant (vitesse nulle) après un échec.
    void resetPlayer();

    const TextureAtlas& _atlas;  ///< Atlas conservé pour reconstruire la scène à chaque niveau.
    core::World _world;
    Camera2D _camera;
    SpriteRenderer _renderer;
    LevelSequence _sequence;            ///< Progression : niveaux ordonnés + indice courant.
    std::optional<core::Level> _level;  ///< Niveau courant chargé (simulation, reset).
    core::CharacterPhysicsSystem _physics;
    core::Entity _player{};  ///< Entité du personnage jouable (valide si `_level`).
    int _levelWidth = 0;
    int _levelHeight = 0;
    std::string _loadError;  ///< Vide si le niveau est chargé ; message d'erreur sinon.
};

}  // namespace hmi
