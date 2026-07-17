/**
 * @file test_parcours_complet.cpp
 * @brief Test système : jouer toute la séquence de niveaux de bout en bout (« titre → niveaux →
 *        titre »), sur les fichiers livrés, sans la couche GPU.
 *
 * Pour chaque niveau de la séquence, on charge le fichier réel, on fait apparaître le personnage à
 * l'entrée et on rejoue un scénario d'entrées **déterministe** (au pas fixe) jusqu'à la sortie
 * (`Won`). Franchir tous les niveaux dans l'ordre représente le parcours complet du jeu. La couche
 * fenêtre/rendu est exclue (vérifiée visuellement) ; on teste ici la simulation et l'enchaînement.
 */

#include <filesystem>
#include <functional>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlayerInput.h"

namespace {

constexpr float STEP = 1.0f / 60.0f;

// Un niveau de la séquence et le scénario d'entrées (fonction du pas de simulation) qui le
// franchit.
struct ScriptedLevel {
    const char* file;
    std::function<core::PlayerInput(int)> input;
};

// Fait apparaître un personnage 1×1 à la position de grille donnée.
core::Entity spawn(core::World& world, core::GridPosition at) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, core::Transform{core::Vector2{static_cast<float>(at.column),
                                                             static_cast<float>(at.row)},
                                               core::Vector2{1.0f, 1.0f}, 0.0f});
    world.addComponent(entity, core::Velocity{});
    world.addComponent(entity, core::Collider{core::Vector2{1.0f, 1.0f}});
    world.addComponent(entity, core::Player{});
    return entity;
}

// Rejoue un niveau jusqu'à son issue (borne large pour éviter une boucle infinie si ça régresse).
core::LevelOutcome playLevel(const ScriptedLevel& scripted) {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / scripted.file;
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        return core::LevelOutcome::Lost;  // fichier absent/invalide → échec du parcours
    }
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player = spawn(world, level.entry());
    core::CharacterPhysicsSystem system;

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < 5000 && outcome == core::LevelOutcome::Playing; ++step) {
        system.update(world, level.tileMap(), scripted.input(step), STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        outcome = core::evaluateOutcome(
            core::Aabb::fromTopLeftSize(transform.position, collider.size), level);
    }
    return outcome;
}

}  // namespace

/// Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, puis « retour
/// au titre ». Reproduit la boucle titre → niveau 1 → niveau 2 → titre du jeu.
TEST(ParcoursCompletSysteme, FranchitTouteLaSequence) {
    const std::vector<ScriptedLevel> sequence = {
        // Niveau 1 : escalier descendant, franchi en maintenant « droite ».
        {"demo.json", [](int) { return core::PlayerInput{1.0f}; }},
        // Niveau 2 : marche ascendante, franchie en avançant et sautant.
        {"demo2.json",
         [](int) {
             core::PlayerInput in;
             in.moveX = 1.0f;
             in.jumpPressed = true;
             in.jumpHeld = true;
             return in;
         }},
    };

    ASSERT_FALSE(sequence.empty());
    for (const ScriptedLevel& level : sequence) {
        EXPECT_EQ(playLevel(level), core::LevelOutcome::Won) << "niveau : " << level.file;
    }
    // Tous les niveaux franchis dans l'ordre → fin de séquence (retour au titre).
}
