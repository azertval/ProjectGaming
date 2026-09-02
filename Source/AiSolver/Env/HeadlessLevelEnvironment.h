// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "AiSolver/Env/Reward.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/BlockController.h"
#include "Core/Gameplay/DangerController.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Gameplay/PlatformController.h"
#include "Core/Gameplay/SinkingBlockController.h"
#include "Core/Gameplay/VolatileBlockController.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlayerInput.h"

/**
 * @file AiSolver/Env/HeadlessLevelEnvironment.h
 * @brief Environnement de simulation headless : jouer un niveau sans fenêtre ni GPU
 * (`LOT-ANNEXE-05`).
 */

namespace aisolver {

/**
 * @brief Retour d'un appel à `HeadlessLevelEnvironment::step` : issue, boîte et état complet du
 *        personnage à l'instant de ce pas.
 *
 * La position du personnage est implicite dans `playerBox` (pas de champ séparé) : boîte, vitesse
 * et état (`grounded`/`wallDirection`/timers/budgets, `core::Player`) suffisent à décrire
 * intégralement l'état simulé du personnage pour ce pas (`EX-IA-005`).
 */
struct StepObservation {
    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    core::Aabb playerBox;
    core::Player playerState;
    core::Velocity playerVelocity;
    int stepIndex = 0;
};

/**
 * @brief Configuration d'un `HeadlessLevelEnvironment` : budget de pas et seuil de blocage, tous
 *        deux **dérivés du niveau** par défaut.
 *
 * Une constante globale ne peut convenir aux deux bouts du catalogue : `demo-wall-jump.json` se
 * termine en quelques centaines de pas, `demo-final.json` en demande près de `4 000`. Les deux
 * valeurs valent donc `0` = « dérive-la du niveau chargé » (`StepBudget.h`) ; une valeur explicite
 * non nulle reste respectée telle quelle — c'est ce dont se servent le harnais de benchmark et les
 * tests qui veulent un budget fixé.
 */
struct EnvironmentConfig {
    /// Nombre maximal de pas simulables entre deux `reset` ; `0` = dérivé du niveau
    /// (`estimateStepBudget`).
    int maxSteps = 0;
    /// Nombre de pas consécutifs sans progression au-delà duquel l'épisode est jugé bloqué ;
    /// `0` = dérivé du budget (`stuckThresholdForBudget`).
    int stuckThreshold = 0;
};

/**
 * @brief Joue **un seul niveau**, pas à pas, sans aucun état de rendu — enveloppe de simulation
 * pure derrière une API `reset`/`step` (`EX-IA-005`).
 *
 * Réplique **indépendante mais testée fidèle** (garde de non-régression pas-à-pas permanente en CI,
 * `Source/Test/Systeme/test_parcours_complet.cpp`) de l'orchestration de simulation déjà établie
 * par `hmi::GameSession::update` et sa reproduction headless historique `playLevel()` — voir
 * `Documentation/Lot-Annexe/LOT-ANNEXE-05-environnement-simulation-headless/epic.md` pour la
 * décision de cadrage qui explique pourquoi ce n'est pas un partage de code. N'ajoute à l'entité
 * joueur que les quatre composants nécessaires à la physique (`core::Transform`, `core::Velocity`,
 * `core::Collider`, `core::Player`) : ni `Sprite` ni `Animation`, `HeadlessLevelEnvironment` ne
 * rendant jamais rien.
 *
 * Rejouable des millions de fois : `reset` reconstruit intégralement `core::World` (pas de purge
 * sélective), et n'acquiert aucune ressource qu'un second `reset` ne pourrait libérer (pas de
 * `init()`/`cleanup()` séparés).
 */
class HeadlessLevelEnvironment {
public:
    explicit HeadlessLevelEnvironment(EnvironmentConfig config = {});

    /**
     * @brief Charge un niveau et fait apparaître le personnage à l'entrée.
     *
     * Reconstruit `core::World` à un monde vierge, ainsi que les contrôleurs de simulation
     * (`core::CharacterPhysicsSystem`, `core::BlockController`, `core::MechanismController`,
     * `core::DangerController`, `core::PlatformController`) pour le niveau chargé. Remet à zéro les
     * compteurs de pas et de progression (`EX-NFR-002` : un `reset` répété sur le même chemin
     * reproduit un état strictement identique).
     * @param levelPath Chemin du fichier de niveau JSON.
     * @return `false` (état neutre, rien de chargé) si le chargement échoue — jamais d'exception
     *         (`EX-NFR-040`).
     */
    [[nodiscard]] bool reset(const std::filesystem::path& levelPath);

    /**
     * @brief Avance la simulation d'exactement un pas fixe (`1/60 s`).
     *
     * Reproduit l'ordre de composition exact de `hmi::GameSession::update`/`playLevel()` :
     * plateformes → blocs (poussée/chute) → physique du personnage → sweep boîte-boîte des blocs à
     * taille réduite (`EX-GP-005`) → mécanismes (état + grille) → dangers (état) →
     * `core::evaluateOutcome`. Un appel sans `reset` réussi préalable, ou au-delà du budget de pas
     * (`EnvironmentConfig::maxSteps`), est une erreur de programmation (`PROJECTGAMING_ASSERT`),
     * pas un cas récupérable.
     * @param input Entrées du personnage pour ce pas.
     * @return Issue, boîte et état complet du personnage à l'issue de ce pas.
     */
    [[nodiscard]] StepObservation step(const core::PlayerInput& input);

    /// @return `true` si un niveau est chargé et simulable.
    [[nodiscard]] bool loaded() const;

    /// @return Message d'erreur si le chargement a échoué, chaîne vide sinon.
    [[nodiscard]] const std::string& loadError() const;

    /// @return Le niveau chargé (nom, dimensions, sortie…), en lecture seule.
    [[nodiscard]] const core::Level& level() const;

    /// @return Le contrôleur de mécanismes du niveau chargé, en lecture seule (`LOT-ANNEXE-06` :
    /// interroge l'état porte ouverte/fermée sans dupliquer la simulation).
    [[nodiscard]] const core::MechanismController& mechanisms() const noexcept;

    /// @return Le contrôleur de dangers du niveau chargé, en lecture seule (`LOT-ANNEXE-06` :
    /// interroge l'état danger mobile/temporisé actif sans dupliquer la simulation).
    [[nodiscard]] const core::DangerController& dangers() const noexcept;

    /// @return Le contrôleur de blocs poussables du niveau chargé, en lecture seule (positions
    /// **courantes** : celles du fichier ne valent qu'au premier pas, et une observation qui les
    /// lirait décrirait un monde périmé dès la première poussée).
    [[nodiscard]] const core::BlockController& blocks() const noexcept;

    /// @return Le contrôleur de plateformes mobiles du niveau chargé, en lecture seule (position
    /// continue courante, même raison que `blocks()`).
    [[nodiscard]] const core::PlatformController& platforms() const noexcept;

    /// @return La grille de collision **composée** du dernier pas : portes du
    /// `MechanismController`, blocs volatils disparus retirés, blocs descendants reportés à leur
    /// case courante et blocs poussables à leur position courante. Celle-là même que la physique
    /// vient de résoudre (le report des blocs descendants, non solides, n'y change rien d'autre que
    /// ce que l'observation y **voit**). Mémorisée plutôt que recomposée : `step()` la construit
    /// déjà.
    [[nodiscard]] const core::TileMap& collisionMap() const noexcept;

    /// @return Le champ de distances vers l'objectif immédiat pour l'état courant
    /// (`buildObjectiveDistanceField`), reconstruit seulement quand il cesse d'être valide. Une
    /// seule instance pour tous les consommateurs — récompense, détection de blocage et
    /// observation lisent le **même** champ au lieu de le recalculer chacun.
    [[nodiscard]] const GridDistanceField& objectiveField() const noexcept;

    /// @return `true` une fois `stepBudget()` pas simulés depuis le dernier `reset`.
    [[nodiscard]] bool budgetExhausted() const noexcept;

    /// @return Le budget de pas **résolu** du niveau chargé (valeur explicite de
    /// `EnvironmentConfig::maxSteps`, ou `estimateStepBudget(level())` si elle valait `0`).
    [[nodiscard]] int stepBudget() const noexcept;

    /// @return Le seuil de blocage **résolu** du niveau chargé, à passer à `classifyEpisode`.
    [[nodiscard]] int stuckThreshold() const noexcept;

    /// @return Nombre de pas consécutifs sans amélioration de `bestObjectiveDistance()` — matière
    /// première pour la détection de blocage, jamais interprétée ici (`LOT-ANNEXE-08`).
    [[nodiscard]] int stepsSinceProgress() const noexcept;

    /// @return Plus petite distance de plus court chemin (en cases) observée depuis le dernier
    /// `reset` entre la case du personnage et l'objectif immédiat.
    ///
    /// Mesurée sur le **même champ que la récompense** (`EX-IA-023`), et non à vol d'oiseau vers
    /// la sortie : un niveau dont la solution s'éloigne de la sortie — clé à l'opposé, salle
    /// annexe — verrait sinon son record atteint dès les premiers pas et jamais rebattu, et tout
    /// épisode serait déclaré bloqué peu après.
    ///
    /// Remise à la distance courante à chaque changement de l'ensemble des objectifs : le nouvel
    /// objectif est presque toujours plus loin que celui qui vient d'être atteint, et un record
    /// conservé d'un objectif à l'autre ne pourrait plus jamais être battu.
    [[nodiscard]] int bestObjectiveDistance() const noexcept;

private:
    /// Boîtes des dangers actifs à cet instant (mobile/temporisé/commuté), même logique que
    /// `hmi::GameSession::collectActiveDangerBoxes` — les dangers statiques sont déjà résolus par
    /// `core::evaluateOutcome` à partir du niveau, seuls les dangers à état doivent être assemblés
    /// par l'appelant.
    [[nodiscard]] std::vector<core::Aabb> collectActiveDangerBoxes() const;

    /// Met à jour `_bestObjectiveDistance`/`_stepsSinceProgress` d'après la boîte du personnage à
    /// l'issue de ce pas (TACHE-03).
    void updateProgress(const core::Aabb& playerBox);

    /// Recompose `_collision` (portes + blocs) et rafraîchit le champ d'objectif d'après l'état
    /// courant ; appelée par `reset()` et à la fin de chaque `step()`.
    /// @param playerBox Boîte du personnage à cet instant, pour ré-amorcer le record de
    ///        progression si l'ensemble des objectifs vient de changer.
    void refreshCollisionAndObjective(const core::Aabb& playerBox);

    EnvironmentConfig _config;
    core::World _world;
    std::optional<core::Level> _level;
    std::optional<core::CharacterPhysicsSystem> _physics;
    std::optional<core::BlockController> _blocks;
    std::optional<core::MechanismController> _mechanisms;
    std::optional<core::DangerController> _dangers;
    std::optional<core::PlatformController> _platforms;
    /// Blocs volatils (`EX-GP-028`/`EX-GP-029`) et descendants (`EX-GP-027`), `LOT-74` :
    /// presents ici pour la meme raison que `_platforms` -- cette orchestration doit rester
    /// fidele pas a pas a `hmi::GameSession::update`, garde-fou `FideliteParPas`.
    std::optional<core::VolatileBlockController> _volatileBlocks;
    std::optional<core::SinkingBlockController> _sinkingBlocks;
    /// Echantillons des supports mobiles du pas : plateformes puis blocs descendants.
    std::vector<core::PlatformSample> _supportSamples;
    /// Grille de collision composée du dernier pas (portes + blocs volatils, descendants et
    /// poussables), voir `collisionMap()`.
    std::optional<core::TileMap> _collision;
    ObjectiveDistanceFieldCache _objectiveCache;
    core::Entity _player{};
    std::string _loadError;
    int _stepIndex = 0;
    /// Budget et seuil résolus au chargement, jamais relus depuis `_config` ailleurs.
    int _stepBudget = 0;
    int _stuckThreshold = 0;
    int _bestObjectiveDistance = 0;
    int _stepsSinceProgress = 0;
};

}  // namespace aisolver
