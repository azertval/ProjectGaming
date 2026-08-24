// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tile_window_encoder.cpp
 * @brief Tests unitaires de aisolver::TileWindowEncoder (LOT-ANNEXE-06, TACHE-01).
 */

#include <cstddef>

#include <gtest/gtest.h>

#include "AiSolver/Env/TileWindowEncoder.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

namespace {

// Compte de canaux a 1.0f et verifie que le canal actif est bien celui attendu.
void expectOneHot(const aisolver::Tensor<float>& tensor, std::size_t channelCount, std::size_t row,
                  std::size_t column, std::size_t expectedChannel) {
    for (std::size_t channel = 0; channel < channelCount; ++channel) {
        const float value = tensor.at({channel, row, column});
        if (channel == expectedChannel) {
            EXPECT_FLOAT_EQ(value, 1.0f) << "canal attendu " << expectedChannel;
        } else {
            EXPECT_FLOAT_EQ(value, 0.0f) << "canal " << channel << " ne devrait pas etre actif";
        }
    }
}

}  // namespace

/**
 * @brief `radius = 2` produit un tenseur `(29..33, 5, 5)`, `radius = 0` un tenseur `(.., 1, 1)`.
 * \castest{<b>La forme du tenseur depend uniquement du rayon.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire un `TileWindowEncoder(2)` et un `TileWindowEncoder(0)`.<br/>2. Encoder une
 * `TileMap` sur un centre quelconque.<br/>
 * \tattendu La forme est `(channelCount(), 5, 5)` pour `radius = 2`, `(channelCount(), 1, 1)` pour
 * `radius = 0`.}
 */
TEST(TileWindowEncoderTest, FormeDuTenseurSelonLeRayon) {
    const core::TileMap tiles(10, 10);

    const aisolver::TileWindowEncoder radius2(2);
    const aisolver::Tensor<float> encoded2 = radius2.encode(tiles, core::GridPosition{5, 5});
    EXPECT_EQ(encoded2.shape(),
              (std::vector<std::size_t>{static_cast<std::size_t>(radius2.channelCount()), 5, 5}));

    const aisolver::TileWindowEncoder radius0(0);
    const aisolver::Tensor<float> encoded0 = radius0.encode(tiles, core::GridPosition{5, 5});
    EXPECT_EQ(encoded0.shape(),
              (std::vector<std::size_t>{static_cast<std::size_t>(radius0.channelCount()), 1, 1}));
}

/**
 * @brief Chaque case encode exactement un canal a `1.0f`, au bon indice de `core::TileType`.
 * \castest{<b>L'encodage est bien one-hot, au bon indice de canal.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire une `TileMap` 3x3 avec des types connus sur chaque case.<br/>2. Encoder
 * avec `radius = 1`.<br/>
 * \tattendu Pour chaque case, exactement le canal `static_cast<std::size_t>(type)` vaut `1.0f`, les
 * autres `0.0f`.}
 */
TEST(TileWindowEncoderTest, EncodageOneHotAuBonCanal) {
    core::TileMap tiles(3, 3);
    tiles.setTile(0, 0, core::TileType::Solid);
    tiles.setTile(1, 0, core::TileType::Danger);
    tiles.setTile(2, 0, core::TileType::Door);
    tiles.setTile(0, 1, core::TileType::Switch);
    tiles.setTile(1, 1, core::TileType::Empty);
    tiles.setTile(2, 1, core::TileType::PressurePlate);
    tiles.setTile(0, 2, core::TileType::Block);
    tiles.setTile(1, 2, core::TileType::MovingPlatform);
    tiles.setTile(2, 2, core::TileType::Key);

    const aisolver::TileWindowEncoder encoder(1);
    const aisolver::Tensor<float> encoded = encoder.encode(tiles, core::GridPosition{1, 1});
    const auto channelCount = static_cast<std::size_t>(encoder.channelCount());

    expectOneHot(encoded, channelCount, 0, 0, static_cast<std::size_t>(core::TileType::Solid));
    expectOneHot(encoded, channelCount, 0, 1, static_cast<std::size_t>(core::TileType::Danger));
    expectOneHot(encoded, channelCount, 0, 2, static_cast<std::size_t>(core::TileType::Door));
    expectOneHot(encoded, channelCount, 1, 0, static_cast<std::size_t>(core::TileType::Switch));
    expectOneHot(encoded, channelCount, 1, 1, static_cast<std::size_t>(core::TileType::Empty));
    expectOneHot(encoded, channelCount, 1, 2,
                 static_cast<std::size_t>(core::TileType::PressurePlate));
    expectOneHot(encoded, channelCount, 2, 0, static_cast<std::size_t>(core::TileType::Block));
    expectOneHot(encoded, channelCount, 2, 1,
                 static_cast<std::size_t>(core::TileType::MovingPlatform));
    expectOneHot(encoded, channelCount, 2, 2, static_cast<std::size_t>(core::TileType::Key));
}

/**
 * @brief Un centre proche d'un coin produit un tenseur complet, cases hors grille en vecteur nul.
 * \castest{<b>Un centre proche d'un coin produit un tenseur complet, cases hors grille en vecteur
 * nul.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `TileMap` 5x5, centre `(0, 0)`, `radius = 3`.<br/>2. Encoder.<br/>
 * \tattendu La forme reste `(channelCount(), 7, 7)` ; chaque case hors grille a ses
 * `channelCount()` canaux a `0.0f`.}
 */
TEST(TileWindowEncoderTest, BordDeCarteProduitUnTenseurCompletAvecCasesNulles) {
    core::TileMap tiles(5, 5);
    tiles.setTile(0, 0, core::TileType::Solid);

    const aisolver::TileWindowEncoder encoder(3);
    const aisolver::Tensor<float> encoded = encoder.encode(tiles, core::GridPosition{0, 0});
    const auto channelCount = static_cast<std::size_t>(encoder.channelCount());
    ASSERT_EQ(encoded.shape(), (std::vector<std::size_t>{channelCount, 7, 7}));

    // Case du personnage (Solid), au centre de la fenetre (3, 3).
    expectOneHot(encoded, channelCount, 3, 3, static_cast<std::size_t>(core::TileType::Solid));

    // Coin oppose de la fenetre (dc = -3, dr = -3 => window (0, 0)) : hors grille, vecteur nul.
    for (std::size_t channel = 0; channel < channelCount; ++channel) {
        EXPECT_FLOAT_EQ(encoded.at({channel, 0, 0}), 0.0f);
    }
}

/**
 * @brief Deux appels `encode` sur la même `TileMap`/`center` produisent des tenseurs identiques.
 * \castest{<b>`encode` est deterministe (memes entrees, meme tenseur).</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Encoder deux fois la meme `TileMap`/`center`.<br/>
 * \tattendu Les deux tenseurs sont bit-a-bit identiques.}
 */
TEST(TileWindowEncoderTest, EncodageDeterministe) {
    core::TileMap tiles(6, 6);
    tiles.setTile(2, 2, core::TileType::Danger);
    tiles.setTile(3, 3, core::TileType::Door);

    const aisolver::TileWindowEncoder encoder(2);
    const aisolver::Tensor<float> first = encoder.encode(tiles, core::GridPosition{3, 3});
    const aisolver::Tensor<float> second = encoder.encode(tiles, core::GridPosition{3, 3});

    ASSERT_EQ(first.size(), second.size());
    for (std::size_t index = 0; index < first.size(); ++index) {
        EXPECT_EQ(first.data()[index], second.data()[index]);
    }
}
