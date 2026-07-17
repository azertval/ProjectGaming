#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/SpriteRenderer.h"
#include "HMI/Interface/IScreen.h"

/**
 * @file HMI/Interface/GameScreen.h
 * @brief Écran de jeu : charge un niveau depuis un fichier et l'affiche.
 */

namespace hmi {

class SpriteBatch;
class TextureAtlas;

/**
 * @brief Écran de jeu affichant un **niveau chargé depuis un fichier** (`EX-REN-010`).
 *
 * Au chargement, le niveau (`core::LevelLoader`) est projeté en **entités ECS** (une tuile non
 * vide = un sprite) rendues par le `SpriteRenderer` en lecture seule (`EX-ARCH-012`). Un échec
 * de chargement est **récupérable** (`EX-NFR-040`) : l'écran affiche un état neutre au lieu de
 * planter. Un **personnage** apparaît à l'entrée et est jouable : la physique (déplacement,
 * gravité, collisions) s'applique à chaque pas fixe, atteindre la **sortie** revient au menu, et
 * toucher un **danger** ou tomber redémarre le personnage à l'entrée (`EX-GP-030`…`EX-GP-032`).
 * La caméra reste **fixe** et cadre le tableau. **Échap** revient au menu. Le comportement des
 * mécanismes relève d'un lot ultérieur.
 */
class GameScreen : public IScreen {
public:
    /**
     * @brief Construit l'écran et charge le niveau.
     * @param batch          Lot de sprites partagé (rendu).
     * @param atlas          Atlas de tuiles fournissant les régions de sprites.
     * @param viewportWidth  Largeur initiale de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur initiale de la surface de rendu, en pixels.
     * @param levelPath      Chemin du fichier de niveau à charger.
     */
    GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth, int viewportHeight,
               std::filesystem::path levelPath);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    /// Fait apparaître le personnage (Player + Transform + Velocity + Collider + Sprite) à
    /// l'entrée.
    void spawnPlayer(const TextureAtlas& atlas, core::GridPosition entry);

    /// Réinitialise le personnage à l'entrée du niveau (vitesse nulle) après un échec.
    void resetPlayer();

    core::World _world;
    Camera2D _camera;
    SpriteRenderer _renderer;
    std::optional<core::Level> _level;  ///< Niveau chargé, conservé pour la simulation et le reset.
    core::CharacterPhysicsSystem _physics;
    core::Entity _player{};  ///< Entité du personnage jouable (valide si `_level`).
    int _levelWidth = 0;
    int _levelHeight = 0;
    std::string _loadError;  ///< Vide si le niveau est chargé ; message d'erreur sinon.
};

}  // namespace hmi
