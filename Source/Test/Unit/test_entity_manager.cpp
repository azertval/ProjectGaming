/**
 * @file test_entity_manager.cpp
 * @brief Tests unitaires du gestionnaire de cycle de vie des entités.
 */

#include <gtest/gtest.h>

#include "Core/Ecs/Entity.h"
#include "Core/Ecs/EntityManager.h"

/// `create` renvoie des entités vivantes et distinctes.
TEST(EntityManagerTest, CreeEntitesVivantesEtDistinctes) {
    core::EntityManager manager;
    const core::Entity a = manager.create();
    const core::Entity b = manager.create();

    EXPECT_TRUE(manager.isAlive(a));
    EXPECT_TRUE(manager.isAlive(b));
    EXPECT_NE(a, b);
    EXPECT_EQ(manager.aliveCount(), 2u);
}

/// Après destruction, l'ancien handle n'est plus vivant.
TEST(EntityManagerTest, DestructionInvalideLeHandle) {
    core::EntityManager manager;
    const core::Entity entity = manager.create();

    manager.destroy(entity);

    EXPECT_FALSE(manager.isAlive(entity));
    EXPECT_EQ(manager.aliveCount(), 0u);
}

/// Un index recyclé produit une génération différente : l'ancien handle reste
/// invalide, le nouveau est valide.
TEST(EntityManagerTest, RecyclageChangeLaGeneration) {
    core::EntityManager manager;
    const core::Entity first = manager.create();
    manager.destroy(first);

    const core::Entity recycled = manager.create();

    // Même index recyclé, mais génération distincte.
    EXPECT_EQ(recycled.index, first.index);
    EXPECT_NE(recycled.generation, first.generation);
    // L'ancien handle reste périmé, le nouveau est vivant.
    EXPECT_FALSE(manager.isAlive(first));
    EXPECT_TRUE(manager.isAlive(recycled));
}

/// Détruire un handle périmé est sans effet (idempotent) et ne touche pas
/// l'entité vivante qui occupe désormais le même index.
TEST(EntityManagerTest, DestructionHandlePerimeSansEffet) {
    core::EntityManager manager;
    const core::Entity stale = manager.create();
    manager.destroy(stale);
    const core::Entity live = manager.create();  // recycle l'index de `stale`

    manager.destroy(stale);  // handle périmé : ne doit rien faire

    EXPECT_TRUE(manager.isAlive(live));
    EXPECT_EQ(manager.aliveCount(), 1u);
}

/// L'entité invalide conventionnelle n'est jamais vivante.
TEST(EntityManagerTest, EntiteInvalideJamaisVivante) {
    core::EntityManager manager;
    EXPECT_FALSE(manager.isAlive(core::INVALID_ENTITY));
    const core::Entity created = manager.create();
    EXPECT_TRUE(manager.isAlive(created));
    EXPECT_FALSE(manager.isAlive(core::INVALID_ENTITY));
}

/// Le handle invalide se compare comme tel.
TEST(EntityManagerTest, EgaliteHandleInvalide) {
    EXPECT_EQ(core::INVALID_ENTITY, core::INVALID_ENTITY);
    EXPECT_EQ(core::INVALID_ENTITY.index, core::Entity::INVALID_INDEX);
}
