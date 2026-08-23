// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/BlockController.h"
#include "Core/Gameplay/DangerController.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Gameplay/PlatformController.h"
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
 * @brief Configuration d'un `HeadlessLevelEnvironment` : budget de pas dur et seuil de progression.
 *
 * `maxSteps` est un budget de **sécurité** (empêcher un run pathologique de tourner indéfiniment),
 * pas un réglage d'entraînement — la politique de détection de blocage réelle, généralement bien
 * plus courte, appartient à l'algorithme consommateur (`LOT-ANNEXE-08`).
 */
struct EnvironmentConfig {
    /// Nombre maximal de pas simulables entre deux `reset` (même valeur par défaut que
    /// `playLevel()` dans `Source/Test/Systeme/test_parcours_complet.cpp`).
    int maxSteps = 3000;
    /// Distance (unités monde) en dessous de laquelle un rapprochement de la sortie n'est pas
    /// considéré comme un progrès.
    float progressEpsilon = 0.05f;
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

    /// @return `true` une fois `EnvironmentConfig::maxSteps` pas simulés depuis le dernier `reset`.
    [[nodiscard]] bool budgetExhausted() const noexcept;

    /// @return Nombre de pas consécutifs sans amélioration significative de `bestDistanceToExit`
    /// (`EnvironmentConfig::progressEpsilon`) — matière première pour la détection de blocage,
    /// jamais interprétée ici (`LOT-ANNEXE-08`).
    [[nodiscard]] int stepsSinceProgress() const noexcept;

    /// @return Plus petite distance (unités monde) observée depuis le dernier `reset` entre le
    /// centre de la boîte du personnage et le centre de la case de sortie.
    [[nodiscard]] float bestDistanceToExit() const noexcept;

private:
    /// Boîtes des dangers actifs à cet instant (mobile/temporisé/commuté), même logique que
    /// `hmi::GameSession::collectActiveDangerBoxes` — les dangers statiques sont déjà résolus par
    /// `core::evaluateOutcome` à partir du niveau, seuls les dangers à état doivent être assemblés
    /// par l'appelant.
    [[nodiscard]] std::vector<core::Aabb> collectActiveDangerBoxes() const;

    /// Met à jour `_bestDistanceToExit`/`_stepsSinceProgress` d'après la boîte du personnage à
    /// l'issue de ce pas (TACHE-03).
    void updateProgress(const core::Aabb& playerBox);

    EnvironmentConfig _config;
    core::World _world;
    std::optional<core::Level> _level;
    std::optional<core::CharacterPhysicsSystem> _physics;
    std::optional<core::BlockController> _blocks;
    std::optional<core::MechanismController> _mechanisms;
    std::optional<core::DangerController> _dangers;
    std::optional<core::PlatformController> _platforms;
    core::Entity _player{};
    std::string _loadError;
    int _stepIndex = 0;
    float _bestDistanceToExit = 0.0f;
    int _stepsSinceProgress = 0;
};

}  // namespace aisolver
