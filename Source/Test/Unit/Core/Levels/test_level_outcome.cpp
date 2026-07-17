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

/// En zone libre (ni sortie, ni danger, dans les limites) : partie en cours.
TEST(LevelOutcomeTest, ZoneLibreEstEnCours) {
    const core::Level level = makeLevel(core::GridPosition{8, 8}, 5, 5);
    EXPECT_EQ(core::evaluateOutcome(unitBox(1.0f, 1.0f), level), core::LevelOutcome::Playing);
}

/// La boîte recouvrant la case de sortie : niveau gagné.
TEST(LevelOutcomeTest, SurLaSortieEstGagne) {
    const core::Level level = makeLevel(core::GridPosition{8, 8});
    EXPECT_EQ(core::evaluateOutcome(unitBox(8.0f, 8.0f), level), core::LevelOutcome::Won);
}

/// La boîte recouvrant une tuile Danger : niveau perdu.
TEST(LevelOutcomeTest, SurUnDangerEstPerdu) {
    const core::Level level = makeLevel(core::GridPosition{8, 8}, 5, 5);
    EXPECT_EQ(core::evaluateOutcome(unitBox(5.0f, 5.0f), level), core::LevelOutcome::Lost);
}

/// La boîte sous la limite basse (chute dans le vide) : niveau perdu.
TEST(LevelOutcomeTest, SousLaLimiteBasseEstPerdu) {
    const core::Level level = makeLevel(core::GridPosition{8, 8});
    EXPECT_EQ(core::evaluateOutcome(unitBox(2.0f, 10.0f), level), core::LevelOutcome::Lost);
}

/// Recouvrement simultané sortie + danger : l'échec est prioritaire (déterminisme).
TEST(LevelOutcomeTest, EchecPrioritaireSurSucces) {
    // Sortie ET danger sur la même case (5,5) : la boîte recouvre les deux.
    const core::Level level = makeLevel(core::GridPosition{5, 5}, 5, 5);
    EXPECT_EQ(core::evaluateOutcome(unitBox(5.0f, 5.0f), level), core::LevelOutcome::Lost);
}
