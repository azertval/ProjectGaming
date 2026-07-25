/**
 * @file test_level.cpp
 * @brief Tests unitaires du modèle de niveau : TileMap et Level.
 */

#include <gtest/gtest.h>

#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

/**
 * @brief Une grille neuve a les bonnes dimensions et n'est composée que de cases Empty.
 * \castest{<b>Une grille neuve a les bonnes dimensions et n'est composée que de cases
 * Empty.</b><br/>
 * \tcat Unitaire · Tile Map<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une grille neuve a les bonnes dimensions et n'est composée que de cases Empty.
 * }
 */
TEST(TileMapTest, GrilleNeuveVideAuxBonnesDimensions) {
    const core::TileMap map(4, 3);

    EXPECT_EQ(map.width(), 4);
    EXPECT_EQ(map.height(), 3);
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            EXPECT_EQ(map.tile(column, row), core::TileType::Empty);
        }
    }
}

/**
 * @brief Écrire puis lire une tuile restitue le type posé.
 * \castest{<b>Écrire puis lire une tuile restitue le type posé.</b><br/>
 * \tcat Unitaire · Tile Map<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Écrire puis lire une tuile restitue le type posé.
 * }
 */
TEST(TileMapTest, EcritureLectureDUneTuile) {
    core::TileMap map(4, 3);
    map.setTile(2, 1, core::TileType::Solid);

    EXPECT_EQ(map.tile(2, 1), core::TileType::Solid);
    EXPECT_EQ(map.tile(0, 0), core::TileType::Empty);  // les autres restent vides
}

/**
 * @brief Les bornes de la grille sont correctement détectées.
 * \castest{<b>Les bornes de la grille sont correctement détectées.</b><br/>
 * \tcat Unitaire · Tile Map<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les bornes de la grille sont correctement détectées.
 * }
 */
TEST(TileMapTest, Bornes) {
    const core::TileMap map(4, 3);

    EXPECT_TRUE(map.inBounds(0, 0));
    EXPECT_TRUE(map.inBounds(3, 2));
    EXPECT_FALSE(map.inBounds(4, 2));
    EXPECT_FALSE(map.inBounds(3, 3));
    EXPECT_FALSE(map.inBounds(-1, 0));
}

/**
 * @brief Seules les tuiles solides bloquent statiquement.
 * \castest{<b>Seules les tuiles solides bloquent statiquement.</b><br/>
 * \tcat Unitaire · Tile Map<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Seules les tuiles solides bloquent statiquement.
 * }
 */
TEST(TileMapTest, Solidite) {
    core::TileMap map(3, 1);
    map.setTile(0, 0, core::TileType::Solid);
    map.setTile(1, 0, core::TileType::Danger);
    // (2, 0) reste Empty

    EXPECT_TRUE(map.isSolid(0, 0));
    EXPECT_FALSE(map.isSolid(1, 0));
    EXPECT_FALSE(map.isSolid(2, 0));
}

/**
 * @brief `isSolid(TileType)` : vrai seulement pour Solid.
 * \castest{<b>`isSolid(TileType)` : vrai seulement pour Solid.</b><br/>
 * \tcat Unitaire · Tile Map<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `isSolid(TileType)` : vrai seulement pour Solid.
 * }
 */
TEST(TileMapTest, IsSolidParType) {
    EXPECT_TRUE(core::isSolid(core::TileType::Solid));
    EXPECT_FALSE(core::isSolid(core::TileType::Empty));
    EXPECT_FALSE(core::isSolid(core::TileType::Door));
    EXPECT_FALSE(core::isSolid(core::TileType::Danger));
}

/**
 * @brief Les sept nouvelles variantes de danger ne sont jamais solides, comme le danger classique
 * (`EX-GP-050` à `EX-GP-053`).
 * \castest{<b>Les sept nouvelles variantes de danger ne sont jamais solides.</b><br/>
 * \tcat Unitaire · Tile Map<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les sept nouvelles variantes de danger ne sont jamais solides.
 * }
 */
TEST(TileMapTest, IsSolidFauxPourLesDangersAvances) {
    EXPECT_FALSE(core::isSolid(core::TileType::DangerUp));
    EXPECT_FALSE(core::isSolid(core::TileType::DangerDown));
    EXPECT_FALSE(core::isSolid(core::TileType::DangerLeft));
    EXPECT_FALSE(core::isSolid(core::TileType::DangerRight));
    EXPECT_FALSE(core::isSolid(core::TileType::DangerMover));
    EXPECT_FALSE(core::isSolid(core::TileType::DangerSwitched));
    EXPECT_FALSE(core::isSolid(core::TileType::DangerBlink));
}

/**
 * @brief Un Level restitue ses composantes (nom, grille, entrée/sortie, mécanismes).
 * \castest{<b>Un Level restitue ses composantes (nom, grille, entrée/sortie, mécanismes).</b><br/>
 * \tcat Unitaire · Level<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un Level restitue ses composantes (nom, grille, entrée/sortie, mécanismes).
 * }
 */
TEST(LevelTest, RestitueSesComposantes) {
    core::TileMap map(5, 4);
    map.setTile(1, 1, core::TileType::Entry);
    map.setTile(3, 2, core::TileType::Exit);

    const std::vector<core::Mechanism> mechanisms = {
        {core::GridPosition{2, 1}, core::GridPosition{4, 2}},
    };
    const core::Level level("Tutoriel", std::move(map), core::GridPosition{1, 1},
                            core::GridPosition{3, 2}, mechanisms);

    EXPECT_EQ(level.name(), "Tutoriel");
    EXPECT_EQ(level.tileMap().width(), 5);
    EXPECT_EQ(level.entry(), (core::GridPosition{1, 1}));
    EXPECT_EQ(level.exit(), (core::GridPosition{3, 2}));
    ASSERT_EQ(level.mechanisms().size(), 1u);
    EXPECT_EQ(level.mechanisms().front().switchPosition, (core::GridPosition{2, 1}));
    EXPECT_EQ(level.mechanisms().front().doorPosition, (core::GridPosition{4, 2}));
}

/**
 * @brief Un Level restitue ses liaisons de danger commuté et ses configurations de danger mobile/
 * temporisé (`EX-GP-051`/`EX-GP-052`/`EX-GP-053`).
 * \castest{<b>Un Level restitue ses liaisons et configurations de dangers avancés.</b><br/>
 * \tcat Unitaire · Level<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un Level restitue ses liaisons de danger commuté et ses configurations de danger
 * mobile/temporisé.
 * }
 */
TEST(LevelTest, RestitueSesDangersAvances) {
    core::TileMap map(5, 4);
    map.setTile(1, 1, core::TileType::Entry);
    map.setTile(3, 2, core::TileType::Exit);

    const std::vector<core::DangerLink> dangerLinks = {
        {core::GridPosition{0, 0}, core::GridPosition{4, 0}},
    };
    const std::vector<core::DangerMoverConfig> moverConfigs = {
        {core::GridPosition{2, 2}, core::DangerMoverAxis::Vertical, 3},
    };
    const std::vector<core::DangerBlinkConfig> blinkConfigs = {
        {core::GridPosition{4, 3}, 90, 10, 30},
    };
    const core::Level level("Avance", std::move(map), core::GridPosition{1, 1},
                            core::GridPosition{3, 2}, {}, -1, -1, dangerLinks, moverConfigs,
                            blinkConfigs);

    ASSERT_EQ(level.dangerLinks().size(), 1u);
    EXPECT_EQ(level.dangerLinks().front().triggerPosition, (core::GridPosition{0, 0}));
    EXPECT_EQ(level.dangerLinks().front().dangerPosition, (core::GridPosition{4, 0}));

    ASSERT_EQ(level.moverConfigs().size(), 1u);
    EXPECT_EQ(level.moverConfigs().front().axis, core::DangerMoverAxis::Vertical);
    EXPECT_EQ(level.moverConfigs().front().range, 3);

    ASSERT_EQ(level.blinkConfigs().size(), 1u);
    EXPECT_EQ(level.blinkConfigs().front().period, 90);
    EXPECT_EQ(level.blinkConfigs().front().phase, 10);
    EXPECT_EQ(level.blinkConfigs().front().activeDuration, 30);
}
