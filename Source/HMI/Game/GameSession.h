#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Core/Ecs/Systems/AnimationSystem.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/BlockController.h"
#include "Core/Gameplay/DangerController.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/Aabb.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/RoomGrid.h"
#include "HMI/Graphics/SpriteRenderer.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"

/**
 * @file HMI/Game/GameSession.h
 * @brief Session de jeu réutilisable : simule et rend **un** niveau, sans dépendance d'écran.
 */

namespace hmi {

class SpriteBatch;
class TextureAtlas;
class TextureCache;
class InputState;

/**
 * @brief Simule et rend **un seul niveau** (déjà validé, en mémoire), indépendamment de toute
 *        infrastructure d'écran.
 *
 * Extraite de l'écran de jeu historique (`LOT-34`) pour être utilisée par l'éditeur Qt (viewport) :
 * toute la logique de jeu (chargement de la scène ECS, apparition du personnage, physique,
 * mécanismes, blocs, dangers, caméra par salle, animation, interpolation) vit ici, une seule fois.
 *
 * `update()` avance d'un **pas fixe** et renvoie l'issue (`core::LevelOutcome`) ; sur un **échec**
 * (danger/chute), le niveau est **rechargé** en interne (personnage à l'entrée, mécanismes/budgets
 * remis) — l'appelant décide quoi faire d'une **réussite** (enchaîner, revenir au menu, etc.).
 * `render()` dessine la scène via le `SpriteRenderer` (lecture seule de l'ECS, `EX-ARCH-012`), avec
 * interpolation (`EX-ARCH-031`). Un échec de chargement est **récupérable** (`EX-NFR-040`) : la
 * session reste dans un état neutre, `loaded()` est faux et `loadError()` renseigné.
 */
class GameSession {
public:
    /**
     * @brief Construit la session et charge la scène du niveau donné.
     * @param batch           Lot de sprites partagé (rendu).
     * @param atlas           Atlas de tuiles fournissant les régions de sprites.
     * @param viewportWidth   Largeur initiale de la surface de rendu, en pixels.
     * @param viewportHeight  Hauteur initiale de la surface de rendu, en pixels.
     * @param cache           Cache de textures, propriétaire du damier de repli du mode Texture
     *                        (référence conservée, doit survivre à la session, `LOT-41`).
     * @param level           Niveau déjà validé à jouer.
     * @param gameBindings    Touches clavier de jeu (référence conservée, doit survivre à la
     * session).
     * @param gamepadBindings Boutons manette de jeu (référence conservée, doit survivre à la
     * session).
     */
    GameSession(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache,
                int viewportWidth, int viewportHeight, core::Level level,
                const GameKeyBindings& gameBindings, const GamepadBindings& gamepadBindings);

    /**
     * @brief Simule le niveau d'un pas fixe (mécanismes, blocs, physique, animation, dangers).
     * @param input      État des entrées de ce pas.
     * @param fixedDelta Durée du pas fixe, en secondes.
     * @return L'issue : `Playing`, `Won` (l'appelant décide), ou `Lost` (niveau déjà rechargé).
     */
    core::LevelOutcome update(const InputState& input, float fixedDelta);

    /// Recharge le niveau courant (personnage à l'entrée, mécanismes et budgets remis).
    void reload();

    /// Dessine la scène du niveau (rien si le chargement a échoué). @p mode choisit le rendu
    /// Physique ou Texture (`EX-REN-046`, `LOT-41`) : bascule purement visuelle, sans effet sur la
    /// simulation ni sur la scène ECS.
    void render(int viewportWidth, int viewportHeight, RenderMode mode, float interpolationAlpha);

    /// @return true si un niveau est chargé et simulable.
    [[nodiscard]] bool loaded() const {
        return _level.has_value();
    }

    /// @return Message d'erreur si le chargement a échoué, chaîne vide sinon.
    [[nodiscard]] const std::string& loadError() const {
        return _loadError;
    }

private:
    void loadLevel(core::Level level);
    void spawnPlayer(core::GridPosition entry);
    void snapshotPreviousPositions();
    void refreshDoorVisuals();
    void refreshBlockVisuals();
    void refreshDangerVisuals();
    void refreshDangerStateVisuals();
    [[nodiscard]] std::vector<core::Aabb> collectActiveDangerBoxes() const;
    void refreshPlayerSprite();
    void centerCameraOnRoom(core::GridPosition roomIndex);
    void updateCurrentRoom();

    const TextureAtlas& _atlas;
    const GameKeyBindings& _gameBindings;
    const GamepadBindings& _gamepadBindings;
    core::World _world;
    Camera2D _camera;
    SpriteRenderer _renderer;
    std::optional<core::Level> _level;
    std::optional<core::MechanismController> _mechanisms;
    std::vector<core::Entity> _doorEntities;
    std::optional<core::BlockController> _blocks;
    std::optional<core::DangerController> _dangers;
    std::vector<core::Entity> _moverEntities;
    std::vector<core::Entity> _dangerSwitchedEntities;
    std::vector<core::Entity> _dangerBlinkEntities;
    std::vector<core::Entity> _blockEntities;
    core::CharacterPhysicsSystem _physics;
    core::AnimationSystem _animation;
    core::Entity _player{};
    int _levelWidth = 0;
    int _levelHeight = 0;
    std::optional<RoomGrid> _roomGrid;
    core::GridPosition _currentRoomIndex{};
    std::string _loadError;
};

}  // namespace hmi
