/**
 * @file test_animation_personnage.cpp
 * @brief Tests d'intégration de l'animation du personnage : ECS + système d'animation assemblés.
 *
 * `AnimationSystem` dérive le clip (idle/run/jump) et l'image courante de l'état physique du
 * personnage (`Player::grounded`, `Velocity`). Test de **référence** (`LOT-46` TACHE-04) : la
 * migration vers le moteur générique piloté par données ne doit produire **aucune** différence de
 * comportement — mêmes attentes qu'avant le lot, exprimées avec la nouvelle API (`clipIndex`
 * résolu dans `core::playerClipSet()`, plutôt que l'ancien `enum class core::AnimationClip`).
 */

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/AnimationSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Math/Vector2.h"

namespace {
constexpr float STEP = 1.0f / 60.0f;

core::Entity spawnCharacter(core::World& world, bool grounded, float velocityX) {
    const core::Entity entity = world.createEntity();
    core::Player player;
    player.grounded = grounded;
    world.addComponent(entity, player);
    world.addComponent(entity, core::Velocity{core::Vector2{velocityX, 0.0f}});
    core::Animation animation;
    animation.clips = core::playerClipSet();
    animation.clipIndex = core::PLAYER_CLIP_IDLE;
    world.addComponent(entity, animation);
    return entity;
}
}  // namespace

/**
 * @brief Au sol et immobile, le clip est Idle et l'image alterne 0/1 après chaque durée d'image.
 * \castest{<b>Au sol et immobile, le clip est Idle et l'image alterne 0/1 après chaque durée
 * d'image.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Au sol et immobile, le clip est Idle et l'image alterne 0/1 après chaque durée
 * d'image.
 * }
 */
TEST(AnimationPersonnageIntegration, ImmobileAuSolEstEnRepos) {
    core::World world;
    const core::Entity entity = spawnCharacter(world, /*grounded=*/true, /*velocityX=*/0.0f);

    core::AnimationSystem system;
    system.update(world, STEP);

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_IDLE);
    EXPECT_EQ(animation.frameIndex, 0);

    // 0,5 s à 60 Hz = 30 pas : l'image de repos doit avoir bouclé au moins une fois (2 images).
    for (int i = 0; i < 31; ++i) {
        system.update(world, STEP);
    }
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_IDLE);
    EXPECT_EQ(animation.frameIndex, 1);
}

/**
 * @brief Au sol et en mouvement, le clip est Run et l'image boucle sur les 4 images dans l'ordre.
 * \castest{<b>Au sol et en mouvement, le clip est Run et l'image boucle sur les 4 images dans
 * l'ordre.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Au sol et en mouvement, le clip est Run et l'image boucle sur les 4 images dans
 * l'ordre.
 * }
 */
TEST(AnimationPersonnageIntegration, EnMouvementAuSolCourt) {
    core::World world;
    const core::Entity entity = spawnCharacter(world, /*grounded=*/true, /*velocityX=*/3.0f);

    core::AnimationSystem system;
    system.update(world, STEP);  // Idle -> Run : consomme le pas de transition (pas d'accumulation).

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_RUN);
    EXPECT_EQ(animation.frameIndex, 0);

    // Durée d'une image de course : 0,1 s (6 pas à 60 Hz). Vérifie le passage 0 -> 1 -> 2 -> 3 -> 0.
    for (int expectedFrame : {1, 2, 3, 0}) {
        for (int i = 0; i < 6; ++i) {
            system.update(world, STEP);
        }
        EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_RUN);
        EXPECT_EQ(animation.frameIndex, expectedFrame);
    }
}

/**
 * @brief En l'air, le clip est Jump et l'image reste figée sur 0, quelle que soit la durée.
 * \castest{<b>En l'air, le clip est Jump et l'image reste figée sur 0, quelle que soit la
 * durée.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu En l'air, le clip est Jump et l'image reste figée sur 0, quelle que soit la durée.
 * }
 */
TEST(AnimationPersonnageIntegration, EnLAirEstEnSaut) {
    core::World world;
    const core::Entity entity = spawnCharacter(world, /*grounded=*/false, /*velocityX=*/2.0f);

    core::AnimationSystem system;
    for (int i = 0; i < 120; ++i) {  // 2 secondes : largement de quoi boucler si Jump animait.
        system.update(world, STEP);
    }

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_JUMP);
    EXPECT_EQ(animation.frameIndex, 0);
}

/**
 * @brief Un changement de clip réinitialise immédiatement l'image et le chronomètre à zéro.
 * \castest{<b>Un changement de clip réinitialise immédiatement l'image et le chronomètre à
 * zéro.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un changement de clip réinitialise immédiatement l'image et le chronomètre à zéro.
 * }
 */
TEST(AnimationPersonnageIntegration, ChangementDeClipReinitialiseLImage) {
    core::World world;
    const core::Entity entity = spawnCharacter(world, /*grounded=*/true, /*velocityX=*/3.0f);
    core::Velocity& velocity = world.getComponent<core::Velocity>(entity);

    core::AnimationSystem system;
    system.update(world, STEP);  // Idle -> Run : consomme le pas de transition (pas d'accumulation).
    // Avance l'animation de course jusqu'à une image différente de 0.
    for (int i = 0; i < 6; ++i) {
        system.update(world, STEP);
    }
    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    ASSERT_EQ(animation.clipIndex, core::PLAYER_CLIP_RUN);
    ASSERT_EQ(animation.frameIndex, 1);

    // Décollage : au sol -> en l'air, en cours de course.
    world.getComponent<core::Player>(entity).grounded = false;
    velocity.value.x = 3.0f;
    system.update(world, STEP);

    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_JUMP);
    EXPECT_EQ(animation.frameIndex, 0);
    EXPECT_FLOAT_EQ(animation.elapsed, 0.0f);
}

/**
 * @brief Une entité sans composant Animation n'est pas affectée par le système.
 * \castest{<b>Une entité sans composant Animation n'est pas affectée par le système.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une entité sans composant Animation n'est pas affectée par le système.
 * }
 */
TEST(AnimationPersonnageIntegration, EntiteSansAnimationIgnoree) {
    core::World world;
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, core::Player{});
    world.addComponent(entity, core::Velocity{});

    core::AnimationSystem system;
    for (int i = 0; i < 10; ++i) {
        system.update(world, STEP);
    }

    EXPECT_FALSE(world.hasComponent<core::Animation>(entity));
}

/**
 * @brief Une entité portant Animation mais sans jeu de clips assigné n'est pas affectée
 *        (`EX-NFR-040`) : aucune donnée à progresser, ni plantage.
 * \castest{<b>Une entité Animation sans jeu de clips assigné reste inerte, sans planter.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'animation reste à ses valeurs par défaut, sans exception.
 * }
 */
TEST(AnimationPersonnageIntegration, AnimationSansJeuDeClipsResteInerte) {
    core::World world;
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, core::Player{});
    world.addComponent(entity, core::Velocity{});
    world.addComponent(entity, core::Animation{});  // clips == nullptr

    core::AnimationSystem system;
    for (int i = 0; i < 10; ++i) {
        system.update(world, STEP);
    }

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, 0);
    EXPECT_EQ(animation.frameIndex, 0);
}
