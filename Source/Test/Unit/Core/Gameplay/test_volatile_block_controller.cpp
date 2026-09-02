// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_volatile_block_controller.cpp
 * @brief Tests unitaires des blocs fragile (`EX-GP-028`) et éphémère (`EX-GP-029`).
 */

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Gameplay/VolatileBlockController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace {

// Niveau 6x4 : sol plein en rangee 3, et une dalle @p type en (2, 3) a la place du sol. Le
// personnage marche donc dessus, a hauteur de la rangee 2.
core::Level makeLevel(core::TileType type) {
    core::TileMap map(6, 4);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 3, core::TileType::Solid);
    }
    map.setTile(2, 3, type);
    return core::Level("volatil", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{5, 2}, std::vector<core::Mechanism>{});
}

// Boite 1x1 posee sur la case (column, row).
core::Aabb boxAt(int column, int row) {
    return core::Aabb::fromTopLeftSize(
        core::Vector2{static_cast<float>(column), static_cast<float>(row)},
        core::Vector2{1.0f, 1.0f});
}

// Boite qui REPOSE sur la dalle (2, 3) : juste au-dessus d'elle, en (2, 2).
core::Aabb restingOnTile() {
    return boxAt(2, 2);
}

// Boite loin de la dalle.
core::Aabb farAway() {
    return boxAt(5, 2);
}

// Niveau 6x6 : sol plein en rangee 3 et une dalle @p below en (2, 4), donc SOUS le sol, hors de
// portee de qui que ce soit qui pietine la rangee 3.
core::Level makeLevelWithTileBelowFloor(core::TileType below) {
    core::TileMap map(6, 6);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 3, core::TileType::Solid);
    }
    map.setTile(2, 4, below);
    return core::Level("sous-le-sol", std::move(map), core::GridPosition{0, 2},
                       core::GridPosition{5, 2}, std::vector<core::Mechanism>{});
}

}  // namespace

/**
 * @brief Le ground pound, et lui seul, brise un bloc fragile.
 * \castest{<b>Un ground pound par le dessus brise un bloc fragile.</b><br/>
 * \tcat Unitaire · Volatile Block Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Poser une dalle fragile en (2, 3).<br/>2. Passer un pas avec le personnage au-dessus
 * et `groundPounding` vrai.<br/>
 * \tattendu La dalle a disparu, elle est signalee dans les disparitions du pas, et sa case n'est
 * plus solide dans la grille de collision resolue.}
 */
TEST(VolatileBlockControllerTest, LeGroundPoundBriseUnBlocFragile) {
    core::Level level = makeLevel(core::TileType::FragileBlock);
    core::VolatileBlockController controller(level);
    ASSERT_EQ(controller.count(), 1u);

    controller.update(restingOnTile(), true);

    EXPECT_TRUE(controller.isGoneAt(0));
    ASSERT_EQ(controller.blocksGoneThisStep().size(), 1u);
    EXPECT_EQ(controller.blocksGoneThisStep()[0], (core::GridPosition{2, 3}));
    EXPECT_FALSE(controller.collisionMap(level.tileMap()).isSolid(2, 3));
}

/**
 * @brief Aucun autre geste ne brise un bloc fragile : c'est la garantie centrale de `EX-GP-028`.
 * \castest{<b>Sans ground pound, un bloc fragile survit a tout contact.</b><br/>
 * \tcat Unitaire · Volatile Block Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Passer 600 pas avec le personnage POSE sur la dalle, `groundPounding` faux.<br/>2.
 * Verifier l'etat de la dalle.<br/>
 * \tattendu La dalle est intacte et sa case reste solide. Marcher, sauter, atterrir ou dasher
 * dessus ne la brise pas -- seul le ground pound le fait, et c'est deliberé (un dash vertical reste
 * un dash tant qu'une charge existe, EX-GP-058).}
 */
TEST(VolatileBlockControllerTest, SansGroundPoundLeBlocFragileSurvit) {
    core::Level level = makeLevel(core::TileType::FragileBlock);
    core::VolatileBlockController controller(level);

    for (int step = 0; step < 600; ++step) {
        controller.update(restingOnTile(), false);
    }

    EXPECT_FALSE(controller.isGoneAt(0));
    EXPECT_TRUE(controller.collisionMap(level.tileMap()).isSolid(2, 3));
}

/**
 * @brief Un ground pound qui passe a cote ne brise rien : le contact doit venir du dessus.
 * \castest{<b>Un ground pound decale lateralement ne brise pas un bloc fragile.</b><br/>
 * \tcat Unitaire · Volatile Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Passer un pas avec `groundPounding` vrai, mais le personnage sur une autre
 * colonne.<br/>
 * \tattendu La dalle est intacte.}
 */
TEST(VolatileBlockControllerTest, UnGroundPoundACoteNeBrisePas) {
    core::Level level = makeLevel(core::TileType::FragileBlock);
    core::VolatileBlockController controller(level);

    controller.update(farAway(), true);

    EXPECT_FALSE(controller.isGoneAt(0));
}

/**
 * @brief Un bloc ephemere reste solide tant qu'on est dessus, quelle que soit la duree.
 * \castest{<b>Un bloc ephemere ne disparait pas sous les pieds du personnage.</b><br/>
 * \tcat Unitaire · Volatile Block Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Rester pose sur la dalle pendant 600 pas, tres au-dela du delai de
 * disparition.<br/>
 * \tattendu La dalle est intacte : c'est le DEPART qui declenche le compte a rebours, jamais le
 * simple ecoulement du temps.}
 */
TEST(VolatileBlockControllerTest, LeBlocEphemereResteTantQuOnEstDessus) {
    core::Level level = makeLevel(core::TileType::VanishingBlock);
    core::VolatileBlockController controller(level);

    for (int step = 0; step < 600; ++step) {
        controller.update(restingOnTile(), false);
    }

    EXPECT_FALSE(controller.isGoneAt(0));
    EXPECT_TRUE(controller.collisionMap(level.tileMap()).isSolid(2, 3));
}

/**
 * @brief Le depart declenche un compte a rebours d'une duree exacte, en pas fixes.
 * \castest{<b>Un bloc ephemere disparait exactement N pas apres le depart du personnage.</b><br/>
 * \tcat Unitaire · Volatile Block Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Se poser sur la dalle, puis la quitter.<br/>2. Compter les pas jusqu'a la
 * disparition.<br/>
 * \tattendu La dalle est encore la au pas `VANISH_DELAY_STEPS - 1` et a disparu au pas
 * `VANISH_DELAY_STEPS` (delai exprime en pas fixes, EX-NFR-002).}
 */
TEST(VolatileBlockControllerTest, LeBlocEphemereDisparaitApresLeDelaiExact) {
    core::Level level = makeLevel(core::TileType::VanishingBlock);
    core::VolatileBlockController controller(level);

    controller.update(restingOnTile(), false);  // pose dessus
    controller.update(farAway(), false);        // front de depart : le compte a rebours demarre

    for (int step = 1; step < core::VolatileBlockController::VANISH_DELAY_STEPS; ++step) {
        controller.update(farAway(), false);
        EXPECT_FALSE(controller.isGoneAt(0)) << "disparu trop tot, au pas " << step;
    }
    controller.update(farAway(), false);
    EXPECT_TRUE(controller.isGoneAt(0));
    EXPECT_FALSE(controller.collisionMap(level.tileMap()).isSolid(2, 3));
}

/**
 * @brief Passer SOUS un bloc ephemere ne l'arme pas : le declencheur est un front de portage.
 * \castest{<b>Passer sous un bloc ephemere ne declenche pas sa disparition.</b><br/>
 * \tcat Unitaire · Volatile Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer le personnage sous la dalle, puis l'eloigner.<br/>2. Avancer largement au-dela
 * du delai.<br/>
 * \tattendu La dalle est intacte : seul le fait d'avoir REPOSE dessus puis d'en etre parti compte,
 * jamais un contact quelconque -- a la difference du bloc descendant (EX-GP-027).}
 */
TEST(VolatileBlockControllerTest, PasserSousUnBlocEphemereNeLArmePas) {
    core::TileMap map(6, 6);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 5, core::TileType::Solid);
    }
    map.setTile(2, 1, core::TileType::VanishingBlock);  // en hauteur : on peut passer dessous
    core::Level level("dessous", std::move(map), core::GridPosition{0, 4}, core::GridPosition{5, 4},
                      std::vector<core::Mechanism>{});
    core::VolatileBlockController controller(level);

    controller.update(boxAt(2, 2), false);  // juste SOUS la dalle
    for (int step = 0; step < 600; ++step) {
        controller.update(boxAt(5, 4), false);
    }

    EXPECT_FALSE(controller.isGoneAt(0));
}

/**
 * @brief Revenir sur un bloc ephemere pendant son compte a rebours ne l'annule pas.
 * \castest{<b>Un retour pendant le compte a rebours n'annule pas la disparition.</b><br/>
 * \tcat Unitaire · Volatile Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Se poser sur la dalle, la quitter, puis y revenir immediatement.<br/>2. Avancer
 * au-dela du delai.<br/>
 * \tattendu La dalle disparait quand meme : la decision de cadrage retient l'aller simple, coherent
 * avec un bloc qui s'effrite.}
 */
TEST(VolatileBlockControllerTest, RevenirNAnnulePasLaDisparition) {
    core::Level level = makeLevel(core::TileType::VanishingBlock);
    core::VolatileBlockController controller(level);

    controller.update(restingOnTile(), false);
    controller.update(farAway(), false);  // depart : compte a rebours arme
    for (int step = 0; step < core::VolatileBlockController::VANISH_DELAY_STEPS + 5; ++step) {
        controller.update(restingOnTile(), false);  // on revient, et on reste
    }

    EXPECT_TRUE(controller.isGoneAt(0));
}

/**
 * @brief La disparition ne touche jamais la carte du niveau, seulement la grille de collision.
 * \castest{<b>Un bloc disparu laisse la carte du niveau inchangee.</b><br/>
 * \tcat Unitaire · Volatile Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Briser une dalle fragile au ground pound.<br/>2. Relire la carte du `Level`.<br/>
 * \tattendu La tuile du `Level` est toujours `FragileBlock` -- c'est la reconstruction du
 * controleur au rechargement du tableau qui remet tout a neuf, sans code de remise a zero dedie.}
 */
TEST(VolatileBlockControllerTest, LaCarteDuNiveauResteImmuable) {
    core::Level level = makeLevel(core::TileType::FragileBlock);
    core::VolatileBlockController controller(level);

    controller.update(restingOnTile(), true);
    ASSERT_TRUE(controller.isGoneAt(0));

    EXPECT_EQ(level.tileMap().tile(2, 3), core::TileType::FragileBlock);

    core::VolatileBlockController reloaded(level);
    EXPECT_FALSE(reloaded.isGoneAt(0)) << "un rechargement doit remettre le tableau a neuf";
}

/**
 * @brief Un ground pound ne traverse pas le sol : la portee verticale reste sous une case entiere.
 * \castest{<b>Un ground pound sur le sol ne brise pas la dalle fragile situee dessous.</b><br/>
 * 	cat Unitaire · Volatile Block Controller<br/>
 * 	crit Bloquant<br/>
 * 	etapes 1. Poser un sol plein en rangee 3 et une dalle fragile en (2, 4), donc de l'autre cote
 * du sol.<br/>2. Passer un pas avec le personnage pose sur le sol, `groundPounding` vrai.<br/>
 * 	attendu La dalle est intacte : la portee du ground pound (`POUND_REACH_CELLS`) est
 * strictement inferieure a une case, sans quoi des pieds poses en rangee R atteindraient un bloc en
 * R+1 a travers la matiere.}
 */
TEST(VolatileBlockControllerTest, UnGroundPoundNeTraversePasLeSol) {
    core::Level level = makeLevelWithTileBelowFloor(core::TileType::FragileBlock);
    core::VolatileBlockController controller(level);
    ASSERT_EQ(controller.count(), 1u);

    controller.update(boxAt(2, 2), true);  // pieds en rangee 3, sur le sol plein

    EXPECT_FALSE(controller.isGoneAt(0));
    EXPECT_TRUE(controller.blocksGoneThisStep().empty());
}

/**
 * @brief Deux dalles fragiles empilees se brisent une par une, jamais les deux d'un coup.
 * \castest{<b>Un ground pound sur une pile de dalles fragiles ne brise que celle du
 * dessus.</b><br/>
 * 	cat Unitaire · Volatile Block Controller<br/>
 * 	crit Majeur<br/>
 * 	etapes 1. Empiler deux dalles fragiles en (2, 3) et (2, 4).<br/>2. Passer un pas avec le
 * personnage pose sur celle du dessus, `groundPounding` vrai.<br/>
 * 	attendu Seule la dalle du dessus a disparu ; celle du dessous est intacte et sa case reste
 * solide -- il faudra un second ground pound, une fois retombe dessus, pour l'atteindre.}
 */
TEST(VolatileBlockControllerTest, UnGroundPoundNeBriseQuUneDalleDUnePile) {
    core::TileMap map(6, 6);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 5, core::TileType::Solid);
    }
    map.setTile(2, 3, core::TileType::FragileBlock);
    map.setTile(2, 4, core::TileType::FragileBlock);
    core::Level level("pile", std::move(map), core::GridPosition{0, 4}, core::GridPosition{5, 4},
                      std::vector<core::Mechanism>{});
    core::VolatileBlockController controller(level);
    ASSERT_EQ(controller.count(), 2u);

    controller.update(boxAt(2, 2), true);  // pieds en rangee 3 : sur la dalle du dessus

    EXPECT_TRUE(controller.isGoneAt(0));
    EXPECT_FALSE(controller.isGoneAt(1));
    ASSERT_EQ(controller.blocksGoneThisStep().size(), 1u);
    EXPECT_EQ(controller.blocksGoneThisStep()[0], (core::GridPosition{2, 3}));
    EXPECT_TRUE(controller.collisionMap(level.tileMap()).isSolid(2, 4));
}

/**
 * @brief La portee reduite couvre encore le pas de chute impose par le ground pound.
 * \castest{<b>Un ground pound brise la dalle alors que les pieds sont encore une demi-case
 * au-dessus.</b><br/>
 * 	cat Unitaire · Volatile Block Controller<br/>
 * 	crit Bloquant<br/>
 * 	etapes 1. Passer un pas avec `groundPounding` vrai et une boite dont les pieds sont a 0,5 case
 * au-dessus de la dalle -- exactement le deplacement d'un pas fixe a la vitesse de chute
 * imposee.<br/>
 * 	attendu La dalle est brisee : la boite examinee est celle d'AVANT le pas, la portee doit donc
 * couvrir la demi-case parcourue pendant le pas.}
 */
TEST(VolatileBlockControllerTest, LeGroundPoundBriseDepuisUneDemiCaseAuDessus) {
    core::Level level = makeLevel(core::TileType::FragileBlock);
    core::VolatileBlockController controller(level);

    const core::Aabb aboveByHalfCell =
        core::Aabb::fromTopLeftSize(core::Vector2{2.0f, 1.5f}, core::Vector2{1.0f, 1.0f});
    controller.update(aboveByHalfCell, true);

    EXPECT_TRUE(controller.isGoneAt(0));
}
