#pragma once

#include <filesystem>
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
#include "Core/Physics/Aabb.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/RoomGrid.h"
#include "HMI/Graphics/SpriteRenderer.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"
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
 * **redémarre** le niveau courant à l'entrée (`EX-GP-031`, `EX-GP-032`). La caméra cadre la
 * **salle** (`RoomGrid`, `LOT-32`) contenant le personnage, au zoom pixel art natif, et bascule
 * **nettement** sur la salle voisine quand le personnage en franchit la frontière — un niveau qui
 * tient dans une seule salle se comporte comme un cadrage « niveau entier » classique (`LOT-16`,
 * `EX-REN-013`). **Échap** revient au menu. Les mécanismes interrupteur/porte et plaque de
 * pression (`core::MechanismController`) sont résolus chaque pas fixe et pris en compte par la
 * carte de collision et le rendu des portes. Les blocs poussables (`core::BlockController`,
 * `EX-GP-022`) sont résolus **avant** la physique du personnage (poussée), leur position courante
 * complétant la carte de collision au même titre que les portes. Les dangers mobile, commuté et
 * temporisé (`core::DangerController`, `core::MechanismController::isDangerActive`, `EX-GP-051`/
 * `052`/`053`) sont résolus après la physique, leurs boîtes actuellement mortelles assemblées
 * (`collectActiveDangerBoxes`) avant l'évaluation de fin de niveau.
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
     * @param gameBindings    Association action de jeu -> touche clavier courante (`EX-CTRL-012`,
     *                        référence conservée, doit survivre à l'écran).
     * @param gamepadBindings Association action de jeu -> bouton manette courant (`EX-CTRL-002`,
     *                        référence conservée, doit survivre à l'écran).
     */
    GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth, int viewportHeight,
               std::vector<std::filesystem::path> levels, const GameKeyBindings& gameBindings,
               const GamepadBindings& gamepadBindings);

    /**
     * @brief Construit l'écran pour un **niveau unique déjà en mémoire**, sans fichier ni
     *        séquence (essai immédiat de l'éditeur, LOT-15 `EX-EDIT-008`).
     *
     * Atteindre la sortie termine l'essai (retour au menu, pas d'enchaînement) ; un échec
     * (danger/chute) redémarre ce même niveau, à l'identique du mode séquence.
     * @param batch          Lot de sprites partagé (rendu).
     * @param atlas          Atlas de tuiles fournissant les régions de sprites.
     * @param viewportWidth  Largeur initiale de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur initiale de la surface de rendu, en pixels.
     * @param level          Niveau déjà validé à jouer.
     * @param gameBindings    Association action de jeu -> touche clavier courante (`EX-CTRL-012`,
     *                        référence conservée, doit survivre à l'écran).
     * @param gamepadBindings Association action de jeu -> bouton manette courant (`EX-CTRL-002`,
     *                        référence conservée, doit survivre à l'écran).
     */
    GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth, int viewportHeight,
               core::Level level, const GameKeyBindings& gameBindings,
               const GamepadBindings& gamepadBindings);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    /// Charge le niveau @p path depuis un fichier, puis délègue à `loadLevel(core::Level)`.
    /// Échec récupérable (`EX-NFR-040`) : `_level` reste vide, `_loadError` est renseigné.
    void loadLevel(const std::filesystem::path& path);

    /// Reconstruit la scène (monde ECS + personnage à l'entrée) pour @p level, déjà chargé et
    /// validé — cœur commun aux deux constructeurs (fichier ou niveau en mémoire) et aux
    /// rechargements (échec, niveau suivant).
    void loadLevel(core::Level level);

    /// Fait apparaître le personnage (Player + Transform + Velocity + Collider + Sprite) à
    /// l'entrée.
    void spawnPlayer(core::GridPosition entry);

    /// Recopie la position courante de chaque entité mobile (personnage, dangers mobiles, blocs)
    /// vers son `hmi::PreviousPosition`, au début de chaque pas fixe — base de l'interpolation de
    /// rendu (`EX-ARCH-031`, voir `render()` et `hmi::SpriteRenderer`).
    void snapshotPreviousPositions();

    /// Met à jour la teinte des sprites de portes selon leur état (ouverte atténuée / fermée
    /// opaque).
    void refreshDoorVisuals();

    /// Repositionne les sprites des blocs poussables sur leur position courante (`_blocks`).
    void refreshBlockVisuals();

    /// Repositionne les sprites des dangers mobiles sur leur position courante (`_dangers`,
    /// `EX-GP-051`) — sans quoi la tuile resterait visuellement figée à sa position de départ,
    /// alors que sa boîte mortelle, elle, se déplace bien (`collectActiveDangerBoxes`).
    void refreshDangerVisuals();

    /// Teinte les dangers commuté/temporisé selon leur état actif/inactif courant
    /// (`EX-GP-052`/`EX-GP-053`) — même principe que `refreshDoorVisuals` (alpha atténué =
    /// inactif/inoffensif, opaque = actif/mortel) : sans ce retour visuel, l'activation ne se
    /// verrait jamais, alors qu'elle affecte bien `collectActiveDangerBoxes`.
    void refreshDangerStateVisuals();

    /// Assemble les boîtes **actuellement mortelles** des dangers à état (mobile/commuté/
    /// temporisé, `EX-GP-051`/`052`/`053`), à passer à `core::evaluateOutcome` — cf. en-tête de
    /// `core::LevelOutcome.h` (cette composition vit en `HMI` car `Core/Levels` ne connaît pas les
    /// contrôleurs de `Core/Gameplay` qui portent cet état).
    [[nodiscard]] std::vector<core::Aabb> collectActiveDangerBoxes() const;

    /// Met à jour la région d'atlas du sprite du personnage depuis son état d'animation
    /// courant (`core::Animation`) — appelé à chaque frame de rendu, pas seulement au spawn.
    void refreshPlayerSprite();

    /// Centre la caméra sur le rectangle de la salle @p roomIndex (`RoomGrid`, `LOT-32`) —
    /// coupure nette : appelé uniquement au chargement et quand la salle courante change
    /// (`updateCurrentRoom`), jamais à chaque frame.
    void centerCameraOnRoom(core::GridPosition roomIndex);

    /// Détermine la salle contenant le personnage ; si elle diffère de `_currentRoomIndex`,
    /// recentre la caméra dessus (`centerCameraOnRoom`). Appelé chaque pas fixe, après résolution
    /// de la physique (`EX-REN-015`).
    void updateCurrentRoom();

    const TextureAtlas& _atlas;  ///< Atlas conservé pour reconstruire la scène à chaque niveau.
    const GameKeyBindings& _gameBindings;     ///< Touches clavier courantes (`EX-CTRL-012`).
    const GamepadBindings& _gamepadBindings;  ///< Boutons manette courants (`EX-CTRL-002`).
    core::World _world;
    Camera2D _camera;
    SpriteRenderer _renderer;
    /// Progression : niveaux ordonnés + indice courant ; absente en mode niveau unique en
    /// mémoire (essai immédiat de l'éditeur), auquel cas atteindre la sortie termine sans
    /// enchaîner (cf. `update()`).
    std::optional<LevelSequence> _sequence;
    std::optional<core::Level> _level;  ///< Niveau courant chargé (simulation, reset).
    std::optional<core::MechanismController>
        _mechanisms;                          ///< Interrupteurs/portes du niveau courant.
    std::vector<core::Entity> _doorEntities;  ///< Entités-tuiles des portes (retour visuel d'état).
    std::optional<core::BlockController> _blocks;  ///< Blocs poussables du niveau courant.
    std::optional<core::DangerController>
        _dangers;  ///< Dangers mobile/temporisé du niveau courant (`EX-GP-051`/`EX-GP-053`).
    std::vector<core::Entity>
        _moverEntities;  ///< Entités-tuiles des dangers mobiles (même ordre que
                         ///< `_dangers->moverBox(index)`), repositionnées chaque pas
                         ///< (`refreshDangerVisuals`).
    std::vector<core::Entity>
        _dangerSwitchedEntities;  ///< Entités-tuiles des dangers commutés (même ordre que
                                  ///< `_level->dangerLinks()`), teinte rafraîchie chaque pas
                                  ///< (`refreshDangerStateVisuals`).
    std::vector<core::Entity>
        _dangerBlinkEntities;  ///< Entités-tuiles des dangers temporisés (même ordre que
                               ///< `_level->blinkConfigs()`), teinte rafraîchie chaque pas
                               ///< (`refreshDangerStateVisuals`).
    std::vector<core::Entity> _blockEntities;  ///< Entités-tuiles des blocs (même ordre que
                                               ///< `_blocks->positions()`).
    core::CharacterPhysicsSystem _physics;
    core::AnimationSystem _animation;
    core::Entity _player{};  ///< Entité du personnage jouable (valide si `_level`).
    int _levelWidth = 0;
    int _levelHeight = 0;
    std::optional<RoomGrid> _roomGrid;       ///< Partition en salles du niveau courant (`LOT-32`).
    core::GridPosition _currentRoomIndex{};  ///< Salle actuellement cadrée par la caméra.
    std::string _loadError;  ///< Vide si le niveau est chargé ; message d'erreur sinon.
};

}  // namespace hmi
