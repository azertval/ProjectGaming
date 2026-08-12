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
    std::vector<core::MovingPlatformConfig> platformConfigs{core::MovingPlatformConfig{
        .startPosition = core::GridPosition{1, 1},
        .endPosition = core::GridPosition{endColumn, endRow},
        .speed = speed,
        .phase = phase}};
    return core::Level("plateforme", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{7, 7}, {}, -1, -1, {}, {}, {}, std::nullopt, std::nullopt,
                       {}, {}, std::move(platformConfigs));
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
    for (int i = 0; i < 60; ++i) {  // 1s : au bout de l'aller
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
    EXPECT_TRUE(core::restsOnTopOfPlatform(boxAt(2, 1), platform));   // pose juste dessus
    EXPECT_FALSE(core::restsOnTopOfPlatform(boxAt(4, 1), platform));  // aucun chevauchement horizontal
    EXPECT_FALSE(core::restsOnTopOfPlatform(boxAt(2, 2), platform));  // a cote (meme ligne), pas dessus
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
