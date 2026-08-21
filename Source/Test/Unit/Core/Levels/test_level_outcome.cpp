// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_level_outcome.cpp
 * @brief Tests unitaires de l'évaluation d'issue de niveau (`evaluateOutcome`).
 */

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace {

// Boîte 1×1 dont le coin haut-gauche est en (x, y).
core::Aabb unitBox(float x, float y) {
    return core::Aabb::fromTopLeftSize(core::Vector2{x, y}, core::Vector2{1.0f, 1.0f});
}

// Niveau 10×10 avec une sortie en @p exit et un éventuel danger posé en (dangerCol, dangerRow).
core::Level makeLevel(core::GridPosition exit, int dangerCol = -1, int dangerRow = -1) {
    core::TileMap map(10, 10);
    if (dangerCol >= 0) {
        map.setTile(dangerCol, dangerRow, core::TileType::Danger);
    }
    return core::Level("test", std::move(map), core::GridPosition{0, 0}, exit, {});
}

}  // namespace

/**
 * @brief En zone libre (ni sortie, ni danger, dans les limites) : partie en cours.
 * \castest{<b>En zone libre (ni sortie, ni danger, dans les limites) : partie en cours.</b><br/>
 * \tcat Unitaire · Level Outcome<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu En zone libre (ni sortie, ni danger, dans les limites) : partie en cours.
 * }
 */
TEST(LevelOutcomeTest, ZoneLibreEstEnCours) {
    const core::Level level = makeLevel(core::GridPosition{8, 8}, 5, 5);
    EXPECT_EQ(core::evaluateOutcome(unitBox(1.0f, 1.0f), level), core::LevelOutcome::Playing);
}

/**
 * @brief La boîte recouvrant la case de sortie : niveau gagné.
 * \castest{<b>La boîte recouvrant la case de sortie : niveau gagné.</b><br/>
 * \tcat Unitaire · Level Outcome<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La boîte recouvrant la case de sortie : niveau gagné.
 * }
 */
TEST(LevelOutcomeTest, SurLaSortieEstGagne) {
    const core::Level level = makeLevel(core::GridPosition{8, 8});
    EXPECT_EQ(core::evaluateOutcome(unitBox(8.0f, 8.0f), level), core::LevelOutcome::Won);
}

/**
 * @brief La boîte recouvrant une tuile Danger : niveau perdu.
 * \castest{<b>La boîte recouvrant une tuile Danger : niveau perdu.</b><br/>
 * \tcat Unitaire · Level Outcome<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La boîte recouvrant une tuile Danger : niveau perdu.
 * }
 */
TEST(LevelOutcomeTest, SurUnDangerEstPerdu) {
    const core::Level level = makeLevel(core::GridPosition{8, 8}, 5, 5);
    EXPECT_EQ(core::evaluateOutcome(unitBox(5.0f, 5.0f), level), core::LevelOutcome::Lost);
}

/**
 * @brief La boîte sous la limite basse (chute dans le vide) : niveau perdu.
 * \castest{<b>La boîte sous la limite basse (chute dans le vide) : niveau perdu.</b><br/>
 * \tcat Unitaire · Level Outcome<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La boîte sous la limite basse (chute dans le vide) : niveau perdu.
 * }
 */
TEST(LevelOutcomeTest, SousLaLimiteBasseEstPerdu) {
    const core::Level level = makeLevel(core::GridPosition{8, 8});
    EXPECT_EQ(core::evaluateOutcome(unitBox(2.0f, 10.0f), level), core::LevelOutcome::Lost);
}

/**
 * @brief Recouvrement simultané sortie + danger : l'échec est prioritaire (déterminisme).
 * \castest{<b>Recouvrement simultané sortie + danger : l'échec est prioritaire
 * (déterminisme).</b><br/>
 * \tcat Unitaire · Level Outcome<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Recouvrement simultané sortie + danger : l'échec est prioritaire (déterminisme).
 * }
 */
TEST(LevelOutcomeTest, EchecPrioritaireSurSucces) {
    // Sortie ET danger sur la même case (5,5) : la boîte recouvre les deux.
    const core::Level level = makeLevel(core::GridPosition{5, 5}, 5, 5);
    EXPECT_EQ(core::evaluateOutcome(unitBox(5.0f, 5.0f), level), core::LevelOutcome::Lost);
}

/**
 * @brief Un danger directionnel (`DangerRight`) ne provoque l'échec que sur la bande de son bord
 * désigné, pas sur le reste de la case (`EX-GP-050`).
 * \castest{<b>Un danger directionnel ne provoque l'échec que sur la bande de son bord
 * désigné.</b><br/>
 * \tcat Unitaire · Level Outcome<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger directionnel ne provoque l'échec que sur la bande de son bord désigné.
 * }
 */
TEST(LevelOutcomeTest, DangerDirectionnelPerduSeulementSurSaBande) {
    core::TileMap map(10, 10);
    map.setTile(5, 5, core::TileType::DangerRight);  // bande mortelle : x in [5.75, 6.0]
    const core::Level level("test", std::move(map), core::GridPosition{0, 0},
                            core::GridPosition{8, 8}, {});

    // Boîte étroite (0,2 de large) posée avant la bande : survit.
    const core::Aabb safeBox =
        core::Aabb::fromTopLeftSize(core::Vector2{5.0f, 5.0f}, core::Vector2{0.2f, 1.0f});
    EXPECT_EQ(core::evaluateOutcome(safeBox, level), core::LevelOutcome::Playing);

    // Même largeur, posée sur la bande : perdu.
    const core::Aabb dangerBox =
        core::Aabb::fromTopLeftSize(core::Vector2{5.8f, 5.0f}, core::Vector2{0.2f, 1.0f});
    EXPECT_EQ(core::evaluateOutcome(dangerBox, level), core::LevelOutcome::Lost);
}

/**
 * @brief Une boîte supplémentaire (`extraDangerBoxes`) provoque l'échec au même titre qu'une
 * tuile de danger statique (`EX-GP-051`/`052`/`053`, assemblées par l'appelant).
 * \castest{<b>Une boîte supplémentaire fournie par l'appelant provoque l'échec.</b><br/>
 * \tcat Unitaire · Level Outcome<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une boîte supplémentaire fournie par l'appelant provoque l'échec.
 * }
 */
TEST(LevelOutcomeTest, BoiteSupplementaireProvoqueLEchec) {
    const core::Level level = makeLevel(core::GridPosition{8, 8});
    const std::vector<core::Aabb> extraDangerBoxes{unitBox(3.0f, 3.0f)};

    EXPECT_EQ(core::evaluateOutcome(unitBox(1.0f, 1.0f), level, extraDangerBoxes),
              core::LevelOutcome::Playing);  // loin de la boîte supplémentaire
    EXPECT_EQ(core::evaluateOutcome(unitBox(3.0f, 3.0f), level, extraDangerBoxes),
              core::LevelOutcome::Lost);  // recouvre la boîte supplémentaire
}

/**
 * @brief Sans boîte supplémentaire (paramètre par défaut), le comportement est inchangé par
 * rapport à avant l'introduction des dangers avancés (non-régression).
 * \castest{<b>Sans boîte supplémentaire, le comportement est inchangé.</b><br/>
 * \tcat Unitaire · Level Outcome<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans boîte supplémentaire, le comportement est inchangé.
 * }
 */
TEST(LevelOutcomeTest, SansBoiteSupplementaireComportementInchange) {
    const core::Level level = makeLevel(core::GridPosition{8, 8}, 5, 5);
    EXPECT_EQ(core::evaluateOutcome(unitBox(5.0f, 5.0f), level), core::LevelOutcome::Lost);
    EXPECT_EQ(core::evaluateOutcome(unitBox(5.0f, 5.0f), level, {}), core::LevelOutcome::Lost);
}
