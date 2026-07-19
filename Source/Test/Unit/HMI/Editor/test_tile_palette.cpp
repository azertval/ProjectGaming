/**
 * @file test_tile_palette.cpp
 * @brief Tests unitaires de la palette de tuiles de l'éditeur (LOT-14, EX-EDIT-002).
 */

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Editor/TilePalette.h"

/**
 * @brief La palette expose au moins une entrée par type de tuile éditable.
 * \castest{<b>La palette expose au moins une entrée par type de tuile éditable.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La palette expose au moins une entrée par type de tuile éditable.
 * }
 */
TEST(TilePaletteTest, ExposeUneEntreeParType) {
    const hmi::TilePalette palette;
    EXPECT_GE(palette.entries().size(), 7u);
}

/**
 * @brief La sélection par défaut est Solid.
 * \castest{<b>La sélection par défaut est Solid.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La sélection par défaut est Solid.
 * }
 */
TEST(TilePaletteTest, SelectionParDefautEstSolid) {
    const hmi::TilePalette palette;
    EXPECT_EQ(palette.selected(), core::TileType::Solid);
}

/**
 * @brief Cliquer une entrée de la palette sélectionne son type et signale le clic consommé.
 * \castest{<b>Cliquer une entrée de la palette sélectionne son type et signale le clic
 * consommé.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Cliquer une entrée de la palette sélectionne son type et signale le clic consommé.
 * }
 */
TEST(TilePaletteTest, ClicSurUneEntreeSelectionneSonType) {
    hmi::TilePalette palette;
    const hmi::TilePalette::Entry& target = palette.entries().back();
    const float centerX = target.x + target.width * 0.5f;
    const float centerY = target.y + target.height * 0.5f;

    const bool consumed = palette.handleClick(centerX, centerY);

    EXPECT_TRUE(consumed);
    EXPECT_EQ(palette.selected(), target.type);
}

/**
 * @brief Cliquer hors de la palette ne change pas la sélection et n'est pas consommé.
 * \castest{<b>Cliquer hors de la palette ne change pas la sélection et n'est pas
 * consommé.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Cliquer hors de la palette ne change pas la sélection et n'est pas consommé.
 * }
 */
TEST(TilePaletteTest, ClicHorsPaletteNonConsomme) {
    hmi::TilePalette palette;
    const core::TileType before = palette.selected();

    const bool consumed = palette.handleClick(-100.0f, -100.0f);

    EXPECT_FALSE(consumed);
    EXPECT_EQ(palette.selected(), before);
}
