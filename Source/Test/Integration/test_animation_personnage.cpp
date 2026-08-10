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

// Personnage complet (Player + Velocity + Animation), pour les scenarios LOT-48 TACHE-02 qui ont
// besoin de regler dashTimer/wallDirection/velocity.y en plus de grounded/velocity.x.
core::Entity spawnFullCharacter(core::World& world, const core::Player& player,
                                const core::Velocity& velocity) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, player);
    world.addComponent(entity, velocity);
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
    system.update(world,
                  STEP);  // Idle -> Run : consomme le pas de transition (pas d'accumulation).

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_RUN);
    EXPECT_EQ(animation.frameIndex, 0);

    // Durée d'une image de course : 0,1 s (6 pas à 60 Hz). Vérifie le passage 0 -> 1 -> 2 -> 3 ->
    // 0.
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
    system.update(world,
                  STEP);  // Idle -> Run : consomme le pas de transition (pas d'accumulation).
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

/**
 * @brief En l'air et en train de descendre (vitesse verticale positive), le clip est Fall,
 *        distinct de Jump (`LOT-48` TACHE-02).
 * \castest{<b>En l'air et en train de descendre, le clip est Fall.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer le personnage en l'air, vitesse verticale positive (chute).<br/>2. Executer
 * un pas.<br/>
 * \tattendu Le clip resolu est Fall, pas Jump.
 * }
 */
TEST(AnimationPersonnageIntegration, EnChuteEstDistinctDuSaut) {
    core::World world;
    core::Player player;
    player.grounded = false;
    const core::Entity entity =
        spawnFullCharacter(world, player, core::Velocity{core::Vector2{0.0f, 5.0f}});

    core::AnimationSystem system;
    system.update(world, STEP);

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_FALL);
}

/**
 * @brief En l'air et en train de monter (vitesse verticale negative ou nulle, apex compris), le
 *        clip reste Jump.
 * \castest{<b>En l'air en montee ou a l'apex, le clip est Jump.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer le personnage en l'air, vitesse verticale nulle (apex).<br/>2. Executer un
 * pas.<br/>
 * \tattendu Le clip resolu est Jump.
 * }
 */
TEST(AnimationPersonnageIntegration, ApexSuspenduResteEnSaut) {
    core::World world;
    core::Player player;
    player.grounded = false;
    const core::Entity entity =
        spawnFullCharacter(world, player, core::Velocity{core::Vector2{0.0f, 0.0f}});

    core::AnimationSystem system;
    system.update(world, STEP);

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_JUMP);
}

/**
 * @brief Contact mural en l'air (wallDirection non nul, pas au sol) : le clip est WallSlide, quel
 *        que soit le signe de la vitesse verticale.
 * \castest{<b>Contact mural en l'air : le clip est WallSlide.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer le personnage en l'air, au contact d'un mur.<br/>2. Executer un pas.<br/>
 * \tattendu Le clip resolu est WallSlide, pas Fall/Jump.
 * }
 */
TEST(AnimationPersonnageIntegration, ContactMuralEstEnGlissade) {
    core::World world;
    core::Player player;
    player.grounded = false;
    player.wallDirection = -1.0f;
    const core::Entity entity =
        spawnFullCharacter(world, player, core::Velocity{core::Vector2{0.0f, 2.0f}});

    core::AnimationSystem system;
    system.update(world, STEP);

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_WALLSLIDE);
}

/**
 * @brief Le dash est prioritaire sur tout le reste, y compris une chute en cours.
 * \castest{<b>Le dash domine la chute (combinaison ambigue).</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer le personnage en chute, dashTimer actif.<br/>2. Executer un pas.<br/>
 * \tattendu Le clip resolu est Dash.
 * }
 */
TEST(AnimationPersonnageIntegration, DashDomineLaChute) {
    core::World world;
    core::Player player;
    player.grounded = false;
    player.dashTimer = 0.1f;
    const core::Entity entity =
        spawnFullCharacter(world, player, core::Velocity{core::Vector2{4.0f, 5.0f}});

    core::AnimationSystem system;
    system.update(world, STEP);

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_DASH);
}

/**
 * @brief Le dash est prioritaire meme sur un atterrissage en cours.
 * \castest{<b>Le dash domine un atterrissage en cours.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Faire atterrir le personnage (transition Land engagee).<br/>2. Declencher un dash au
 * pas suivant.<br/>
 * \tattendu Le clip resolu bascule sur Dash, interrompant Land.
 * }
 */
TEST(AnimationPersonnageIntegration, DashInterrompLAtterrissage) {
    core::World world;
    core::Player player;
    player.grounded = false;
    core::Velocity velocity{core::Vector2{0.0f, 3.0f}};
    const core::Entity entity = spawnFullCharacter(world, player, velocity);

    core::AnimationSystem system;
    system.update(world, STEP);  // en l'air : Fall.
    ASSERT_EQ(world.getComponent<core::Animation>(entity).clipIndex, core::PLAYER_CLIP_FALL);

    world.getComponent<core::Player>(entity).grounded = true;  // contact au sol.
    system.update(world, STEP);
    ASSERT_EQ(world.getComponent<core::Animation>(entity).clipIndex, core::PLAYER_CLIP_LAND);

    world.getComponent<core::Player>(entity).dashTimer = 0.1f;  // dash au pas suivant.
    system.update(world, STEP);
    EXPECT_EQ(world.getComponent<core::Animation>(entity).clipIndex, core::PLAYER_CLIP_DASH);
}

/**
 * @brief Un contact au sol depuis un clip aerien (Jump/Fall/WallSlide) declenche l'atterrissage
 *        (transition detectee par comparaison avec le clip du pas precedent, comme les
 *        transitions de mecanismes, `LOT-47` TACHE-02), qui enchaine sur Idle ou Run une fois
 *        jouee.
 * \castest{<b>Un contact au sol depuis un clip aerien declenche Land, qui enchaine sur
 * Idle.</b><br/> \tcat Integration · Animation Personnage<br/> \tcrit Critique<br/> \tetapes 1.
 * Faire chuter le personnage (Fall).<br/>2. Le poser au sol, immobile.<br/>3. Executer jusqu'a la
 * fin de la transition.<br/> \tattendu Land se joue une fois puis bascule sur Idle.
 * }
 */
TEST(AnimationPersonnageIntegration, AtterrissageEnchaineSurRepos) {
    core::World world;
    core::Player player;
    player.grounded = false;
    core::Velocity velocity{core::Vector2{0.0f, 3.0f}};
    const core::Entity entity = spawnFullCharacter(world, player, velocity);

    core::AnimationSystem system;
    system.update(world, STEP);  // Fall.
    ASSERT_EQ(world.getComponent<core::Animation>(entity).clipIndex, core::PLAYER_CLIP_FALL);

    world.getComponent<core::Player>(entity).grounded = true;
    world.getComponent<core::Velocity>(entity).value.x = 0.0f;
    system.update(world, STEP);  // Fall -> Land : consomme le pas de transition.
    ASSERT_EQ(world.getComponent<core::Animation>(entity).clipIndex, core::PLAYER_CLIP_LAND);
    ASSERT_EQ(world.getComponent<core::Animation>(entity).frameIndex, 0);

    // Land : 2 images de 0,08 s (0,16 s, 10 pas a 60 Hz) avant bascule sur Idle.
    for (int i = 0; i < 10; ++i) {
        system.update(world, STEP);
    }
    EXPECT_EQ(world.getComponent<core::Animation>(entity).clipIndex, core::PLAYER_CLIP_IDLE);
}

/**
 * @brief Une glissade murale a l'instant du contact au sol declenche l'atterrissage (WallSlide est
 *        un clip aerien, meme priorite que Jump/Fall).
 * \castest{<b>Glissade murale au moment du contact au sol : atterrissage.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer le personnage en glissade murale.<br/>2. Le poser au sol.<br/>3. Executer un
 * pas.<br/>
 * \tattendu Le clip resolu est Land, pas WallSlide ni Idle.
 * }
 */
TEST(AnimationPersonnageIntegration, GlissadeMuraleAuContactDeclencheAtterrissage) {
    core::World world;
    core::Player player;
    player.grounded = false;
    player.wallDirection = 1.0f;
    core::Velocity velocity{core::Vector2{0.0f, 1.0f}};
    const core::Entity entity = spawnFullCharacter(world, player, velocity);

    core::AnimationSystem system;
    system.update(world, STEP);  // WallSlide.
    ASSERT_EQ(world.getComponent<core::Animation>(entity).clipIndex, core::PLAYER_CLIP_WALLSLIDE);

    world.getComponent<core::Player>(entity).grounded = true;
    world.getComponent<core::Player>(entity).wallDirection = 0.0f;  // plus de contact mural au sol.
    system.update(world, STEP);
    EXPECT_EQ(world.getComponent<core::Animation>(entity).clipIndex, core::PLAYER_CLIP_LAND);
}
