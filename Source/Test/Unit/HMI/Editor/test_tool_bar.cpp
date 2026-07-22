/**
 * @file test_tool_bar.cpp
 * @brief Tests unitaires de la barre d'outils de l'éditeur (LOT-15, EX-EDIT-014/015).
 */

#include <gtest/gtest.h>

#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/ToolBar.h"

/**
 * @brief La sélection par défaut est l'outil Pinceau.
 * \castest{<b>La sélection par défaut est l'outil Pinceau.</b><br/>
 * \tcat Unitaire · Tool Bar<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La sélection par défaut est l'outil Pinceau.
 * }
 */
TEST(ToolBarTest, SelectionParDefautEstPinceau) {
    const hmi::ToolBar toolBar;
    EXPECT_EQ(toolBar.selected(), hmi::EditorTool::Paint);
}

/**
 * @brief La barre expose une entrée pour chacun des trois outils.
 * \castest{<b>La barre expose une entrée pour chacun des trois outils.</b><br/>
 * \tcat Unitaire · Tool Bar<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La barre expose une entrée pour chacun des trois outils.
 * }
 */
TEST(ToolBarTest, ExposeTroisEntrees) {
    const hmi::ToolBar toolBar;
    EXPECT_EQ(toolBar.entries().size(), 3u);
}

/**
 * @brief Cliquer une entrée de la barre sélectionne son outil et signale le clic consommé.
 * \castest{<b>Cliquer une entrée de la barre sélectionne son outil et signale le clic
 * consommé.</b><br/>
 * \tcat Unitaire · Tool Bar<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Cliquer une entrée de la barre sélectionne son outil et signale le clic consommé.
 * }
 */
TEST(ToolBarTest, ClicSurUneEntreeSelectionneSonOutil) {
    hmi::ToolBar toolBar;
    const hmi::ToolBar::Entry& target = toolBar.entries().back();
    const float centerX = target.x + target.width * 0.5f;
    const float centerY = target.y + target.height * 0.5f;

    const bool consumed = toolBar.handleClick(centerX, centerY);

    EXPECT_TRUE(consumed);
    EXPECT_EQ(toolBar.selected(), target.tool);
}

/**
 * @brief Cliquer hors de la barre ne change pas la sélection et n'est pas consommé.
 * \castest{<b>Cliquer hors de la barre ne change pas la sélection et n'est pas consommé.</b><br/>
 * \tcat Unitaire · Tool Bar<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Cliquer hors de la barre ne change pas la sélection et n'est pas consommé.
 * }
 */
TEST(ToolBarTest, ClicHorsBarreNonConsomme) {
    hmi::ToolBar toolBar;
    const hmi::EditorTool before = toolBar.selected();

    const bool consumed = toolBar.handleClick(-100.0f, -100.0f);

    EXPECT_FALSE(consumed);
    EXPECT_EQ(toolBar.selected(), before);
}

/**
 * @brief selectNext() fait défiler les outils dans l'ordre Pinceau → Rectangle → Sélection →
 *        Pinceau.
 * \castest{<b>selectNext() fait défiler les outils dans l'ordre attendu, en boucle.</b><br/>
 * \tcat Unitaire · Tool Bar<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu selectNext() fait défiler les outils dans l'ordre attendu, en boucle.
 * }
 */
TEST(ToolBarTest, SelectNextDefileEnBoucle) {
    hmi::ToolBar toolBar;
    ASSERT_EQ(toolBar.selected(), hmi::EditorTool::Paint);

    toolBar.selectNext();
    EXPECT_EQ(toolBar.selected(), hmi::EditorTool::Rectangle);

    toolBar.selectNext();
    EXPECT_EQ(toolBar.selected(), hmi::EditorTool::Selection);

    toolBar.selectNext();
    EXPECT_EQ(toolBar.selected(), hmi::EditorTool::Paint);
}
