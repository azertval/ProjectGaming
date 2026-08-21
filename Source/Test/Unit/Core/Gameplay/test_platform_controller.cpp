// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_platform_controller.cpp
 * @brief Tests unitaires de la plateforme mobile (`core::PlatformController`, `EX-GP-026`).
 */

#include <optional>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Gameplay/PlatformController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace {

// Niveau minimal 8x8 avec une seule plateforme mobile en (1,1), aller-retour vers (endColumn,
// endRow), a vitesse et dephasage donnes.
core::Level makeLevelWithPlatform(int endColumn, int endRow, float speed = 2.0f, int phase = 0) {
    core::TileMap map(8, 8);
    map.setTile(1, 1, core::TileType::MovingPlatform);
    std::vector<core::MovingPlatformConfig> platformConfigs{
        core::MovingPlatformConfig{.startPosition = core::GridPosition{1, 1},
                                   .waypoints = {core::GridPosition{endColumn, endRow}},
                                   .speed = speed,
                                   .phase = phase}};
    return core::Level("plateforme", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{7, 7}, {}, -1, -1, {}, {}, {}, std::nullopt, std::nullopt,
                       {}, std::move(platformConfigs));
}

// Niveau minimal 8x8 avec une seule plateforme mobile en (1,1) suivant la route donnee (points
// APRES le depart), dans le mode et a la vitesse donnes.
core::Level makeLevelWithPath(std::vector<core::GridPosition> waypoints,
                              core::PlatformPathMode mode, float speed = 2.0f, int phase = 0) {
    core::TileMap map(8, 8);
    map.setTile(1, 1, core::TileType::MovingPlatform);
    std::vector<core::MovingPlatformConfig> platformConfigs{
        core::MovingPlatformConfig{.startPosition = core::GridPosition{1, 1},
                                   .waypoints = std::move(waypoints),
                                   .mode = mode,
                                   .speed = speed,
                                   .phase = phase}};
    return core::Level("plateforme", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{7, 7}, {}, -1, -1, {}, {}, {}, std::nullopt, std::nullopt,
                       {}, std::move(platformConfigs));
}

core::Aabb boxAt(int col, int row) {
    return core::Aabb::fromTopLeftSize(
        core::Vector2{static_cast<float>(col), static_cast<float>(row)}, core::Vector2{1.0f, 1.0f});
}

}  // namespace

/**
 * @brief Au chargement (aucun pas fixe écoulé), une plateforme est à sa position de départ.
 * \castest{<b>Au chargement, une plateforme est à sa position de départ.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Au chargement, une plateforme est à sa position de départ.
 * }
 */
TEST(PlatformControllerTest, DemarreASaPositionDeDepart) {
    const core::PlatformController controller(makeLevelWithPlatform(3, 1));
    ASSERT_EQ(controller.count(), 1u);
    const core::Aabb box = controller.boxAt(0);
    EXPECT_FLOAT_EQ(box.min.x, 1.0f);
    EXPECT_FLOAT_EQ(box.min.y, 1.0f);
    EXPECT_FLOAT_EQ(box.max.x, 2.0f);
    EXPECT_FLOAT_EQ(box.max.y, 2.0f);
}

/**
 * @brief Une plateforme horizontale fait un aller-retour déterministe entre ses deux points, à
 *        vitesse constante (`EX-GP-026`, `EX-NFR-002`).
 * \castest{<b>Une plateforme horizontale fait un aller-retour déterministe.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La plateforme atteint le point d'arrivée, revient au départ après un cycle complet.
 * }
 */
TEST(PlatformControllerTest, AllerRetourHorizontalDeterministe) {
    core::PlatformController controller(makeLevelWithPlatform(3, 1));  // distance 2, vitesse 2/s

    const auto advance = [&](int steps) {
        for (int i = 0; i < steps; ++i) {
            controller.update();
        }
    };

    advance(30);  // 0.5s : a mi-chemin de l'aller
    EXPECT_NEAR(controller.boxAt(0).min.x, 2.0f, 1e-4f);

    advance(30);  // 1s : au bout de l'aller
    EXPECT_NEAR(controller.boxAt(0).min.x, 3.0f, 1e-4f);

    advance(30);  // 1.5s : a mi-chemin du retour
    EXPECT_NEAR(controller.boxAt(0).min.x, 2.0f, 1e-4f);

    advance(30);  // 2s : cycle complet, de retour au depart
    EXPECT_NEAR(controller.boxAt(0).min.x, 1.0f, 1e-4f);

    // La ligne ne bouge jamais sur un parcours purement horizontal.
    EXPECT_FLOAT_EQ(controller.boxAt(0).min.y, 1.0f);
}

/**
 * @brief Une plateforme verticale déplace sa ligne, jamais sa colonne.
 * \castest{<b>Une plateforme verticale déplace sa ligne, jamais sa colonne.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La plateforme verticale déplace sa ligne, jamais sa colonne.
 * }
 */
TEST(PlatformControllerTest, AllerRetourVerticalDeterministe) {
    core::PlatformController controller(makeLevelWithPlatform(1, 3));  // distance 2, vitesse 2/s
    for (int i = 0; i < 60; ++i) {                                     // 1s : au bout de l'aller
        controller.update();
    }
    const core::Aabb box = controller.boxAt(0);
    EXPECT_FLOAT_EQ(box.min.x, 1.0f);
    EXPECT_NEAR(box.min.y, 3.0f, 1e-4f);
}

/**
 * @brief Un déphasage décale la position d'une plateforme dans son cycle (`EX-GP-053`, patron
 *        repris).
 * \castest{<b>Un déphasage décale la position d'une plateforme dans son cycle.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Deux plateformes identiques mais déphasées ne sont pas à la même position au même pas.
 * }
 */
TEST(PlatformControllerTest, DephasageDecaleLaPosition) {
    core::PlatformController withoutPhase(makeLevelWithPlatform(3, 1, 2.0f, /*phase=*/0));
    core::PlatformController withPhase(makeLevelWithPlatform(3, 1, 2.0f, /*phase=*/30));

    // Meme pas (0) : la plateforme dephasee de 30 pas est deja a mi-chemin, l'autre au depart.
    EXPECT_NEAR(withoutPhase.boxAt(0).min.x, 1.0f, 1e-4f);
    EXPECT_NEAR(withPhase.boxAt(0).min.x, 2.0f, 1e-4f);
}

/**
 * @brief Deux exécutions de la même séquence de pas donnent exactement les mêmes positions —
 *        déterminisme (`EX-NFR-002`), garantie centrale de la plateforme mobile.
 * \castest{<b>Deux exécutions de la même séquence donnent les mêmes positions.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les deux contrôleurs, avancés du même nombre de pas, sont à la position identique.
 * }
 */
TEST(PlatformControllerTest, DeterminismeSurRejeu) {
    core::PlatformController first(makeLevelWithPlatform(4, 3));
    core::PlatformController second(makeLevelWithPlatform(4, 3));

    for (int i = 0; i < 137; ++i) {  // nombre de pas arbitraire, non multiple du cycle
        first.update();
        second.update();
        ASSERT_FLOAT_EQ(first.boxAt(0).min.x, second.boxAt(0).min.x) << "pas " << i;
        ASSERT_FLOAT_EQ(first.boxAt(0).min.y, second.boxAt(0).min.y) << "pas " << i;
    }
}

/**
 * @brief `deltaAt`/`previousBoxAt` donnent le déplacement exact survenu au dernier pas — ce que
 *        consomme le portage du personnage et des blocs.
 * \castest{<b>deltaAt/previousBoxAt donnent le déplacement exact du dernier pas.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu boxAt - previousBoxAt correspond exactement à deltaAt.
 * }
 */
TEST(PlatformControllerTest, DeltaCorrespondAuDeplacementDuDernierPas) {
    core::PlatformController controller(makeLevelWithPlatform(3, 1));
    controller.update();
    const core::Vector2 delta = controller.deltaAt(0);
    const core::Aabb current = controller.boxAt(0);
    const core::Aabb previous = controller.previousBoxAt(0);
    EXPECT_NEAR(delta.x, current.min.x - previous.min.x, 1e-6f);
    EXPECT_NEAR(delta.y, current.min.y - previous.min.y, 1e-6f);
    EXPECT_GT(delta.x, 0.0f);  // la plateforme avance vers (3,1) au premier pas
}

/**
 * @brief Un parcours de distance nulle (les deux points coïncident) immobilise la plateforme.
 * \castest{<b>Un parcours de distance nulle immobilise la plateforme.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La plateforme reste à sa position de départ, quel que soit le nombre de pas.
 * }
 */
TEST(PlatformControllerTest, ParcoursNulImmobiliseLaPlateforme) {
    core::PlatformController controller(makeLevelWithPlatform(1, 1));  // meme point que le depart
    for (int i = 0; i < 100; ++i) {
        controller.update();
    }
    const core::Aabb box = controller.boxAt(0);
    EXPECT_FLOAT_EQ(box.min.x, 1.0f);
    EXPECT_FLOAT_EQ(box.min.y, 1.0f);
}

/**
 * @brief `restsOnTopOfPlatform` détecte une boîte posée sur le dessus d'une plateforme, et refuse
 *        un simple contact latéral ou l'absence de chevauchement.
 * \castest{<b>restsOnTopOfPlatform distingue un appui sur le dessus d'un contact latéral.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Vrai posé dessus, faux à côté ou trop loin.
 * }
 */
TEST(PlatformControllerTest, RestsOnTopOfPlatformDistingueLAppuiDuContact) {
    const core::Aabb platform = boxAt(2, 2);
    EXPECT_TRUE(core::restsOnTopOfPlatform(boxAt(2, 1), platform));  // pose juste dessus
    EXPECT_FALSE(
        core::restsOnTopOfPlatform(boxAt(4, 1), platform));  // aucun chevauchement horizontal
    EXPECT_FALSE(
        core::restsOnTopOfPlatform(boxAt(2, 2), platform));  // a cote (meme ligne), pas dessus
}

/**
 * @brief `isSquishedByPlatform` détecte un personnage porté embarqué dans une tuile solide
 *        (écrasement plafond, `EX-GP-026`), et ne signale rien en espace libre.
 * \castest{<b>isSquishedByPlatform détecte l'écrasement contre un plafond.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Vrai quand la boîte portée chevauche une tuile solide, faux sinon.
 * }
 */
TEST(PlatformControllerTest, IsSquishedByPlatformDetecteLEcrasement) {
    core::TileMap map(6, 6);
    map.setTile(2, 0, core::TileType::Solid);  // "plafond" au-dessus de la zone testee

    EXPECT_TRUE(core::isSquishedByPlatform(boxAt(2, 0), map));   // chevauche le plafond
    EXPECT_FALSE(core::isSquishedByPlatform(boxAt(2, 2), map));  // espace libre, loin du plafond
}

/**
 * @brief Une route à trois points est parcourue segment par segment à vitesse constante, puis
 *        refaite à l'envers (aller-retour généralisé, `EX-GP-054`).
 * \castest{<b>Une route à trois points est parcourue puis refaite à l'envers.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Poser une route en L (1,1) vers (3,1) puis (3,4), a vitesse 2 cases/s.<br/>2.
 * Avancer pas a pas et relever la position aux instants remarquables de l'aller puis du
 * retour.<br/>
 * \tattendu La plateforme suit chaque segment a vitesse constante, atteint le dernier point au
 * bout de l'aller, puis revient au depart en repassant par le coin.
 * }
 */
TEST(PlatformControllerTest, RouteATroisPointsEstParcourueEnAllerRetour) {
    // Route en L : 2 cases vers la droite puis 3 cases vers le bas, soit 5 cases d'aller.
    core::PlatformController controller(makeLevelWithPath(
        {core::GridPosition{3, 1}, core::GridPosition{3, 4}}, core::PlatformPathMode::PingPong));

    const auto advance = [&](int steps) {
        for (int i = 0; i < steps; ++i) {
            controller.update();
        }
    };

    advance(30);  // 0,5 s a 2 cases/s = 1 case : encore sur le premier segment
    EXPECT_NEAR(controller.boxAt(0).min.x, 2.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 1.0f, 1e-4f);

    advance(30);  // 1 s = 2 cases : pile sur le coin, le waypoint intermediaire
    EXPECT_NEAR(controller.boxAt(0).min.x, 3.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 1.0f, 1e-4f);

    advance(30);  // 1,5 s = 3 cases : une case apres le coin, sur le second segment
    EXPECT_NEAR(controller.boxAt(0).min.x, 3.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 2.0f, 1e-4f);

    advance(60);  // 2,5 s = 5 cases : bout de l'aller
    EXPECT_NEAR(controller.boxAt(0).min.x, 3.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 4.0f, 1e-4f);

    advance(90);  // 4 s = 8 cases : 3 cases de retour, de nouveau sur le coin
    EXPECT_NEAR(controller.boxAt(0).min.x, 3.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 1.0f, 1e-4f);

    advance(60);  // 5 s = 10 cases = un cycle complet : retour exact au depart
    EXPECT_NEAR(controller.boxAt(0).min.x, 1.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 1.0f, 1e-4f);
}

/**
 * @brief En mode circuit fermé, la plateforme revient au départ par le segment de fermeture sans
 *        jamais rebrousser chemin (`EX-GP-054`).
 * \castest{<b>Un circuit fermé se parcourt toujours dans le même sens.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Poser un carre (1,1), (3,1), (3,3), (1,3) en mode boucle.<br/>2. Avancer d'un cycle
 * complet en relevant la position sur le segment de fermeture.<br/>
 * \tattendu La plateforme emprunte le segment de fermeture puis retrouve exactement sa position de
 * depart apres un cycle, sans repasser a l'envers par les points intermediaires.
 * }
 */
TEST(PlatformControllerTest, CircuitFermeNeRebrousseJamaisChemin) {
    // Carre de 2 cases de cote : 2 + 2 + 2 pour l'aller, + 2 de fermeture = perimetre 8 cases.
    core::PlatformController controller(makeLevelWithPath(
        {core::GridPosition{3, 1}, core::GridPosition{3, 3}, core::GridPosition{1, 3}},
        core::PlatformPathMode::Loop));

    const auto advance = [&](int steps) {
        for (int i = 0; i < steps; ++i) {
            controller.update();
        }
    };

    advance(90);  // 1,5 s a 2 cases/s = 3 cases : un segment et demi, sur le cote droit
    EXPECT_NEAR(controller.boxAt(0).min.x, 3.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 2.0f, 1e-4f);

    advance(90);  // 3 s = 6 cases : dernier point du carre atteint
    EXPECT_NEAR(controller.boxAt(0).min.x, 1.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 3.0f, 1e-4f);

    advance(30);  // 3,5 s = 7 cases : EN PLEIN sur le segment de fermeture, pas un retour arriere
    EXPECT_NEAR(controller.boxAt(0).min.x, 1.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 2.0f, 1e-4f);

    advance(30);  // 4 s = 8 cases = perimetre complet : retour exact au depart
    EXPECT_NEAR(controller.boxAt(0).min.x, 1.0f, 1e-4f);
    EXPECT_NEAR(controller.boxAt(0).min.y, 1.0f, 1e-4f);
}

/**
 * @brief Une route dégénérée (aucun point, ou points confondus avec le départ) laisse la
 *        plateforme immobile plutôt que de diviser par une longueur nulle (`EX-NFR-040`).
 * \castest{<b>Une route dégénérée laisse la plateforme immobile.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une plateforme sans waypoint, puis une autre dont tous les waypoints
 * valent la position de depart.<br/>2. Avancer de plusieurs pas fixes.<br/>
 * \tattendu Les deux restent a leur position de depart, sans division par zero.
 * }
 */
TEST(PlatformControllerTest, RouteDegenereeLaissePlateformeImmobile) {
    const auto run = [](std::vector<core::GridPosition> waypoints) {
        core::PlatformController controller(
            makeLevelWithPath(std::move(waypoints), core::PlatformPathMode::PingPong));
        for (int i = 0; i < 120; ++i) {
            controller.update();
        }
        return controller.boxAt(0);
    };

    const core::Aabb sansWaypoint = run({});
    EXPECT_FLOAT_EQ(sansWaypoint.min.x, 1.0f);
    EXPECT_FLOAT_EQ(sansWaypoint.min.y, 1.0f);

    // Deux points confondus avec le depart : la route existe mais sa longueur est nulle.
    const core::Aabb pointsConfondus = run({core::GridPosition{1, 1}, core::GridPosition{1, 1}});
    EXPECT_FLOAT_EQ(pointsConfondus.min.x, 1.0f);
    EXPECT_FLOAT_EQ(pointsConfondus.min.y, 1.0f);
}

/**
 * @brief Un point dupliqué au milieu d'une route ne produit ni division par zéro ni saut : le
 *        segment de longueur nulle est simplement traversé (`EX-NFR-040`).
 * \castest{<b>Un point dupliqué au milieu d'une route est traversé sans incident.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser une route (1,1), (3,1), (3,1), (5,1) avec un point duplique.<br/>2. Relever la
 * position de part et d'autre du point duplique.<br/>
 * \tattendu La plateforme progresse a vitesse constante, comme si le doublon n'existait pas.
 * }
 */
TEST(PlatformControllerTest, PointDupliqueNeCassePasLeParcours) {
    core::PlatformController controller(makeLevelWithPath(
        {core::GridPosition{3, 1}, core::GridPosition{3, 1}, core::GridPosition{5, 1}},
        core::PlatformPathMode::PingPong));

    for (int i = 0; i < 60; ++i) {
        controller.update();
    }
    // 1 s a 2 cases/s = 2 cases : pile sur le point duplique.
    EXPECT_NEAR(controller.boxAt(0).min.x, 3.0f, 1e-4f);

    for (int i = 0; i < 30; ++i) {
        controller.update();
    }
    // 1,5 s = 3 cases : la progression a continue normalement au-dela du doublon.
    EXPECT_NEAR(controller.boxAt(0).min.x, 4.0f, 1e-4f);
}

/**
 * @brief La position reste exacte après des millions de pas fixes : le parcours est une fonction
 *        pure du numéro de pas, calculée sans perte de précision (`EX-NFR-002`).
 * \castest{<b>La position ne dérive pas après des millions de pas fixes.</b><br/>
 * \tcat Unitaire · Platform Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Poser une plateforme dont le cycle dure un nombre entier de pas.<br/>2. Comparer la
 * position apres un tres grand nombre de cycles entiers a celle du meme reste de cycle.<br/>
 * \tattendu Les deux positions coincident : aucune derive accumulee, contrairement a un calcul
 * mene en simple precision qui perdrait le bit de poids faible au-dela de ~16,7 millions de pas.
 * }
 */
TEST(PlatformControllerTest, AucuneDeriveApresDesMillionsDePas) {
    // Aller-retour de 2 cases a 2 cases/s : cycle = 4 cases = 2 s = 120 pas fixes, exactement.
    const core::Level level =
        makeLevelWithPath({core::GridPosition{3, 1}}, core::PlatformPathMode::PingPong);
    constexpr int TOTAL_STEPS = 20000000;  // bien au-dela des ~16,7 M ou un compteur float saute
    constexpr int CYCLE_STEPS = 120;

    core::PlatformController reference(level);
    for (int i = 0; i < TOTAL_STEPS % CYCLE_STEPS; ++i) {
        reference.update();
    }

    core::PlatformController longRun(level);
    for (int i = 0; i < TOTAL_STEPS; ++i) {
        longRun.update();
    }

    EXPECT_NEAR(longRun.boxAt(0).min.x, reference.boxAt(0).min.x, 1e-4f);
    EXPECT_NEAR(longRun.boxAt(0).min.y, reference.boxAt(0).min.y, 1e-4f);
}
