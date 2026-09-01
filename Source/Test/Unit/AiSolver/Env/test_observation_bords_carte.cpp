// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_observation_bords_carte.cpp
 * @brief Stabilité dimensionnelle de l'observation près des bords de carte (LOT-ANNEXE-06,
 *        TACHE-04).
 */

#include <cstddef>
#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/MechanismStateEncoder.h"
#include "AiSolver/Env/TileWindowEncoder.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/TileMap.h"

namespace {

std::filesystem::path levelPath(const char* file) {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
}

std::vector<std::size_t> expectedTileShape(int radius) {
    const auto size = static_cast<std::size_t>(2 * radius + 1);
    return {static_cast<std::size_t>(aisolver::TileWindowEncoder::CHANNEL_COUNT), size, size};
}

std::vector<std::size_t> expectedMechanismShape(int radius) {
    const auto size = static_cast<std::size_t>(2 * radius + 1);
    return {static_cast<std::size_t>(aisolver::MechanismStateEncoder::CHANNEL_COUNT), size, size};
}

}  // namespace

/**
 * @brief Chaque coin/bord d'un niveau reel produit la forme exacte attendue, a plusieurs rayons.
 * \castest{<b>La forme de l'observation reste exacte a chaque coin/bord d'un niveau reel, quel que
 * soit le rayon.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Charger `demo-deplacement.json`.<br/>2. Pour chaque coin (`(0,0)`,
 * `(width-1,0)`, `(0,height-1)`, `(width-1,height-1)`) et chaque rayon (`0, 1, 3, 5`), encoder la
 * fenetre de tuiles et l'etat des mecanismes.<br/>
 * \tattendu La forme du tenseur categoriel est exactement `(33, 2r+1, 2r+1)`, celle du tenseur de
 * mecanisme `(2, 2r+1, 2r+1)`, y compris quand `2r+1` depasse la dimension de la carte.}
 */
TEST(ObservationBordsCarteTest, FormeExacteAChaqueCoinQuelQueSoitLeRayon) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromFile(levelPath("demo-deplacement.json"));
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;
    const core::TileMap& tiles = level.tileMap();

    core::MechanismController mechanisms(level);
    core::DangerController dangers(level);
    core::PlatformController platforms(level);

    const std::vector<core::GridPosition> corners = {
        core::GridPosition{0, 0},
        core::GridPosition{tiles.width() - 1, 0},
        core::GridPosition{0, tiles.height() - 1},
        core::GridPosition{tiles.width() - 1, tiles.height() - 1},
    };
    const std::vector<int> radii = {0, 1, 3, 5};

    for (const core::GridPosition& corner : corners) {
        for (int radius : radii) {
            const aisolver::TileWindowEncoder tileEncoder(radius);
            const aisolver::Tensor<float> tileTensor = tileEncoder.encode(tiles, corner);
            EXPECT_EQ(tileTensor.shape(), expectedTileShape(radius))
                << "coin (" << corner.column << ", " << corner.row << "), radius=" << radius;

            const aisolver::MechanismStateEncoder mechanismEncoder;
            const aisolver::Tensor<float> mechanismTensor =
                mechanismEncoder.encode(mechanisms, dangers, platforms, level, corner, radius);
            EXPECT_EQ(mechanismTensor.shape(), expectedMechanismShape(radius))
                << "coin (" << corner.column << ", " << corner.row << "), radius=" << radius;
        }
    }
}

/**
 * @brief Une carte 1x1 avec `radius = 3` produit un tenseur `(33, 7, 7)` : une seule case reelle,
 * les 48 autres en vecteur nul.
 * \castest{<b>Carte 1x1 : une seule case reelle, les 48 autres en vecteur nul.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une `TileMap` 1x1 (`TileType::Solid`).<br/>2. Encoder avec
 * `radius = 3`.<br/>
 * \tattendu La forme est `(33, 7, 7)` ; la case centrale (3, 3) encode `Solid`, les 48 autres cases
 * ont leurs 33 canaux a `0.0f`.}
 */
TEST(ObservationBordsCarteTest, CarteUnicaseAvecRayonLargeProduitDesCasesNullesAutourDuCentre) {
    core::TileMap tiles(1, 1);
    tiles.setTile(0, 0, core::TileType::Solid);

    const aisolver::TileWindowEncoder encoder(3);
    const aisolver::Tensor<float> encoded = encoder.encode(tiles, core::GridPosition{0, 0});
    ASSERT_EQ(encoded.shape(), (std::vector<std::size_t>{33, 7, 7}));

    std::size_t nonZeroCells = 0;
    for (std::size_t row = 0; row < 7; ++row) {
        for (std::size_t column = 0; column < 7; ++column) {
            bool cellNonZero = false;
            for (std::size_t channel = 0; channel < 33; ++channel) {
                if (encoded.at({channel, row, column}) != 0.0f) {
                    cellNonZero = true;
                }
            }
            if (cellNonZero) {
                ++nonZeroCells;
                EXPECT_EQ(row, 3u);
                EXPECT_EQ(column, 3u);
                EXPECT_FLOAT_EQ(
                    encoded.at({static_cast<std::size_t>(core::TileType::Solid), row, column}),
                    1.0f);
            }
        }
    }
    EXPECT_EQ(nonZeroCells, 1u);
}
