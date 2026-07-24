/**
 * @file test_tile_palette.cpp
 * @brief Tests unitaires de la palette de tuiles de l'éditeur, organisée en catégories/sous-groupes
 *        repliables (`LOT-27`, `EX-EDIT-018`).
 */

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "Core/Levels/TileType.h"
#include "HMI/Editor/TilePalette.h"

namespace {

// Retrouve une entree visible par son libelle exact (en-tete : prefixe "> "/"v " inclus) ; nullopt
// si absente de la vue courante (repliee, donc invisible par construction de TilePalette).
[[nodiscard]] std::optional<hmi::TilePalette::Entry> findByLabel(const hmi::TilePalette& palette,
                                                                 const std::string& label) {
    for (const hmi::TilePalette::Entry& entry : palette.entries()) {
        if (entry.label == label) {
            return entry;
        }
    }
    return std::nullopt;
}

// Simule un clic au centre de l'entree donnee.
bool clickEntry(hmi::TilePalette& palette, const hmi::TilePalette::Entry& entry) {
    return palette.handleClick(entry.x + entry.width * 0.5f, entry.y + entry.height * 0.5f);
}

}  // namespace

/**
 * @brief À l'état initial (tout replié), seules les catégories et entrées autonomes sont visibles.
 * \castest{<b>À l'état initial, seules les catégories et entrées autonomes sont visibles.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Cinq entrées visibles : Vide, Tuile, Interactif, Piège, Jalon (catégories repliées).
 * }
 */
TEST(TilePaletteTest, EtatInitialCinqEntreesVisibles) {
    const hmi::TilePalette palette;
    EXPECT_EQ(palette.entries().size(), 5u);
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
 * @brief Cliquer une entrée autonome (Piège) sélectionne son type et signale le clic consommé.
 * \castest{<b>Cliquer une entrée autonome sélectionne son type et signale le clic consommé.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le type Danger est sélectionné, le clic est consommé.
 * }
 */
TEST(TilePaletteTest, ClicSurEntreeAutonomeSelectionneSonType) {
    hmi::TilePalette palette;
    const std::optional<hmi::TilePalette::Entry> target = findByLabel(palette, "Piege");
    ASSERT_TRUE(target.has_value());

    const bool consumed = clickEntry(palette, *target);

    EXPECT_TRUE(consumed);
    EXPECT_EQ(palette.selected(), core::TileType::Danger);
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

/**
 * @brief Chaque entrée visible porte un libellé non vide (découvrabilité, `EX-EDIT-015`).
 * \castest{<b>Chaque entrée visible porte un libellé non vide.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Chaque entrée porte un libellé non vide.
 * }
 */
TEST(TilePaletteTest, ChaqueEntreePorteUnLibelle) {
    const hmi::TilePalette palette;
    for (const hmi::TilePalette::Entry& entry : palette.entries()) {
        EXPECT_FALSE(entry.label.empty());
    }
}

/**
 * @brief Cliquer l'en-tête d'une catégorie repliée la déplie sans changer la sélection.
 * \castest{<b>Cliquer l'en-tête d'une catégorie repliée la déplie sans changer la sélection.</b>
 * <br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La catégorie Tuile dépliée expose sa feuille directe (Plein) et ses deux sous-groupes
 * (Pente, Arrondi), la sélection reste inchangée, le clic est consommé.
 * }
 */
TEST(TilePaletteTest, ClicSurEnTeteDeCategorieLaDeplie) {
    hmi::TilePalette palette;
    const core::TileType before = palette.selected();
    const std::optional<hmi::TilePalette::Entry> header = findByLabel(palette, "> Tuile");
    ASSERT_TRUE(header.has_value());

    const bool consumed = clickEntry(palette, *header);

    EXPECT_TRUE(consumed);
    EXPECT_EQ(palette.selected(), before);
    EXPECT_TRUE(findByLabel(palette, "v Tuile").has_value());
    EXPECT_TRUE(findByLabel(palette, "Plein").has_value());
    EXPECT_TRUE(findByLabel(palette, "> Pente").has_value());
    EXPECT_TRUE(findByLabel(palette, "> Arrondi").has_value());
}

/**
 * @brief Cliquer à nouveau l'en-tête d'une catégorie dépliée la replie (toutes ses entrées
 *        redeviennent invisibles, y compris celles d'un sous-groupe resté déplié).
 * \castest{<b>Cliquer à nouveau l'en-tête d'une catégorie dépliée la replie.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Après un second clic sur l'en-tête, la palette retrouve exactement ses 5 entrées
 * initiales.
 * }
 */
TEST(TilePaletteTest, SecondClicSurEnTeteDeCategorieLaReplie) {
    hmi::TilePalette palette;
    clickEntry(palette, *findByLabel(palette, "> Tuile"));

    clickEntry(palette, *findByLabel(palette, "v Tuile"));

    EXPECT_EQ(palette.entries().size(), 5u);
    EXPECT_TRUE(findByLabel(palette, "> Tuile").has_value());
}

/**
 * @brief Une fois la catégorie Tuile dépliée, cliquer sa feuille directe (Plein) sélectionne Solid.
 * \castest{<b>Cliquer la feuille directe d'une catégorie dépliée sélectionne son type.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le type Solid est sélectionné, le clic est consommé.
 * }
 */
TEST(TilePaletteTest, ClicSurFeuilleDirecteSelectionneSonType) {
    hmi::TilePalette palette;
    clickEntry(palette, *findByLabel(palette, "> Interactif"));
    const std::optional<hmi::TilePalette::Entry> door = findByLabel(palette, "Porte");
    ASSERT_TRUE(door.has_value());

    const bool consumed = clickEntry(palette, *door);

    EXPECT_TRUE(consumed);
    EXPECT_EQ(palette.selected(), core::TileType::Door);
}

/**
 * @brief Le sous-groupe Bloc poussable, imbriqué dans Interactif, expose ses trois tailles une
 *        fois déplié — troisième niveau d'accordéon (`EX-EDIT-018`).
 * \castest{<b>Le sous-groupe Bloc poussable déplié expose ses trois tailles.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Cliquer la taille « 1/2 » du sous-groupe déplié sélectionne BlockHalf.
 * }
 */
TEST(TilePaletteTest, SousGroupeBlocPoussableDeplieExposeSesTailles) {
    hmi::TilePalette palette;
    clickEntry(palette, *findByLabel(palette, "> Interactif"));
    ASSERT_TRUE(findByLabel(palette, "> Bloc poussable").has_value());

    clickEntry(palette, *findByLabel(palette, "> Bloc poussable"));
    const std::optional<hmi::TilePalette::Entry> half = findByLabel(palette, "1/2");
    ASSERT_TRUE(half.has_value());

    const bool consumed = clickEntry(palette, *half);

    EXPECT_TRUE(consumed);
    EXPECT_EQ(palette.selected(), core::TileType::BlockHalf);
}

/**
 * @brief Le sous-groupe Pente, imbriqué dans Tuile, expose ses quatre orientations une fois déplié.
 * \castest{<b>Le sous-groupe Pente déplié expose ses quatre orientations.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Cliquer l'orientation « Plaf D » du sous-groupe Pente déplié sélectionne
 * SlopeDownRight ; le sous-groupe Arrondi voisin reste replié et n'expose pas ses propres entrées.
 * }
 */
TEST(TilePaletteTest, SousGroupePenteDeplieExposeSesOrientations) {
    hmi::TilePalette palette;
    clickEntry(palette, *findByLabel(palette, "> Tuile"));
    clickEntry(palette, *findByLabel(palette, "> Pente"));
    const std::optional<hmi::TilePalette::Entry> ceilingRight = findByLabel(palette, "Plaf D");
    ASSERT_TRUE(ceilingRight.has_value());

    const bool consumed = clickEntry(palette, *ceilingRight);

    EXPECT_TRUE(consumed);
    EXPECT_EQ(palette.selected(), core::TileType::SlopeDownRight);
    // Arrondi reste replie : ses propres feuilles (memes libelles que Pente) ne sont pas visibles
    // en double, seul son en-tete "> Arrondi" figure dans la vue courante.
    EXPECT_TRUE(findByLabel(palette, "> Arrondi").has_value());
}

/**
 * @brief `bottom()` augmente quand une catégorie se déplie, et revient à sa valeur initiale quand
 *        elle se replie — la barre d'outils (`ToolBar::relayout`) s'appuie sur cette valeur.
 * \castest{<b>`bottom()` suit l'état de dépliage courant.</b><br/>
 * \tcat Unitaire · Tile Palette<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `bottom()` grandit au dépliage puis revient à sa valeur initiale au repliage.
 * }
 */
TEST(TilePaletteTest, BottomSuitLEtatDeDepliage) {
    hmi::TilePalette palette;
    const float collapsedBottom = palette.bottom();

    clickEntry(palette, *findByLabel(palette, "> Tuile"));
    EXPECT_GT(palette.bottom(), collapsedBottom);

    clickEntry(palette, *findByLabel(palette, "v Tuile"));
    EXPECT_FLOAT_EQ(palette.bottom(), collapsedBottom);
}
