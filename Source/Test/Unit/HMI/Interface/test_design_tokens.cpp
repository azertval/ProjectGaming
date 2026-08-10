/**
 * @file test_design_tokens.cpp
 * @brief Tests unitaires des jetons de design de l'IHM (`LOT-56` TACHE-01, `EX-IHM-050`,
 *        `EX-IHM-051`).
 */

#include <gtest/gtest.h>

#include "HMI/Interface/DesignTokens.h"

/**
 * @brief Chaque couleur produit une chaîne hexadécimale stable et valide (six chiffres, préfixe
 *        `#`), sans alpha.
 * \castest{<b>La conversion en couleur CSS hexadecimale est stable et bien formee.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Convertir une couleur connue en chaine hexadecimale.<br/>2. Reconvertir la meme
 * couleur.<br/>
 * \tattendu La chaine fait sept caracteres, commence par '#', et les deux conversions sont
 * identiques (fonction pure).
 * }
 */
TEST(DesignTokensTest, ConversionHexadecimaleStableEtBienFormee) {
    constexpr hmi::DesignColor color{0x1a, 0x2b, 0x3c, 255};
    const std::string first = hmi::toCssColor(color);
    const std::string second = hmi::toCssColor(color);
    EXPECT_EQ(first, second);
    ASSERT_EQ(first.size(), 7u);
    EXPECT_EQ(first, "#1a2b3c");
}

/**
 * @brief La conversion `rgba()` porte les quatre composantes sur 0-255, y compris une opacité
 *        partielle.
 * \castest{<b>La conversion en couleur CSS rgba() porte les quatre composantes.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Convertir une couleur avec alpha partiel.<br/>
 * \tattendu La chaine produite contient les quatre valeurs entieres attendues.
 * }
 */
TEST(DesignTokensTest, ConversionRgbaPorteLesQuatreComposantes) {
    constexpr hmi::DesignColor color{0, 128, 255, 64};
    EXPECT_EQ(hmi::toCssRgba(color), "rgba(0, 128, 255, 64)");
}

/**
 * @brief La couleur d'effacement du viewport dérive du **même** jeton que le fond de la portée
 *        correspondante, en mode édition comme en mode jeu : c'est le seul garde-fou contre la
 *        réapparition de la couture entre le canevas et les widgets.
 * \castest{<b>La couleur d'effacement du viewport suit le jeton de fond de la bonne
 * portee.</b><br/> \tcat Unitaire · Jetons de design<br/> \tcrit Critique<br/> \tetapes 1. Demander
 * la couleur d'effacement en mode edition, puis en mode jeu.<br/> \tattendu En edition elle vaut le
 * fond du chassis variable ; en jeu, le fond de l'identite invariante.
 * }
 */
TEST(DesignTokensTest, CouleurViewportSuitLaPorteeDuMode) {
    EXPECT_EQ(hmi::viewportClearColor(/*editorMode=*/true, hmi::editorDarkTokens()),
              hmi::editorDarkTokens().color.background);
    EXPECT_EQ(hmi::viewportClearColor(/*editorMode=*/true, hmi::editorLightTokens()),
              hmi::editorLightTokens().color.background);
    EXPECT_EQ(hmi::viewportClearColor(/*editorMode=*/false, hmi::editorDarkTokens()),
              hmi::identityTokens().color.background);
}

/**
 * @brief Les deux portées d'habillage (identité invariante, châssis variable) partagent
 *        exactement les mêmes échelles d'espacement, de typographie et de tailles — seules les
 *        couleurs diffèrent. Sans cette symétrie, TACHE-06 découvrirait une divergence trop tard.
 * \castest{<b>Les deux portees partagent les memes echelles et tailles.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Comparer espacement, typographie et tailles des deux jeux de jetons.<br/>
 * \tattendu Les trois echelles sont identiques entre les deux portees.
 * }
 */
TEST(DesignTokensTest, LesDeuxPorteesPartagentLesMemesEchelles) {
    EXPECT_EQ(hmi::identityTokens().spacing, hmi::editorDarkTokens().spacing);
    EXPECT_EQ(hmi::identityTokens().typography, hmi::editorDarkTokens().typography);
    EXPECT_EQ(hmi::identityTokens().size, hmi::editorDarkTokens().size);
    // Les deux themes d'editeur (LOT-56 TACHE-06) partagent aussi ces echelles entre eux.
    EXPECT_EQ(hmi::editorDarkTokens().spacing, hmi::editorLightTokens().spacing);
    EXPECT_EQ(hmi::editorDarkTokens().typography, hmi::editorLightTokens().typography);
    EXPECT_EQ(hmi::editorDarkTokens().size, hmi::editorLightTokens().size);
}

/**
 * @brief Les deux portées restent visuellement distinctes en couleur (sans quoi le découpage en
 *        deux jeux de jetons n'aurait pas de sens), et aucune n'utilise de couleurs dégénérées.
 * \castest{<b>Les couleurs des deux portees sont definies et distinctes.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Comparer le fond des deux portees.<br/>2. Verifier que le texte contraste avec le
 * fond dans chaque portee.<br/>
 * \tattendu Les fonds different ; dans chaque portee, le texte n'est pas egal au fond.
 * }
 */
TEST(DesignTokensTest, CouleursDesDeuxPorteesDistinctesEtCoherentes) {
    EXPECT_FALSE(hmi::identityTokens().color.background ==
                 hmi::editorDarkTokens().color.background);
    EXPECT_FALSE(hmi::identityTokens().color.text == hmi::identityTokens().color.background);
    EXPECT_FALSE(hmi::editorDarkTokens().color.text == hmi::editorDarkTokens().color.background);
}

/**
 * @brief Les deux widgets de remappage (clavier, manette) doivent exposer la même largeur
 *        minimale : c'est la grandeur que TACHE-01 unifie via `SizeTokens::controlMinWidth`.
 * \castest{<b>Une seule grandeur de largeur minimale existe pour les deux widgets de
 * remappage.</b><br/> \tcat Unitaire · Jetons de design<br/> \tcrit Mineur<br/> \tetapes 1. Lire
 * `controlMinWidth` dans les jetons du chassis d'edition.<br/> \tattendu La valeur est strictement
 * positive (utilisee telle quelle par les deux widgets).
 * }
 */
TEST(DesignTokensTest, AucunDoublonDeLargeurMinimale) {
    EXPECT_GT(hmi::editorDarkTokens().size.controlMinWidth, 0);
}

/**
 * @brief Pour chaque thème d'éditeur (sombre, clair), le rapport de contraste entre le texte et
 *        son fond, et entre le texte atténué et son fond, dépasse un seuil de lisibilité fixé —
 *        garde-fou contre un thème clair livré illisible (`LOT-56` TACHE-06).
 * \castest{<b>Chaque theme d'editeur satisfait un seuil de contraste texte/fond.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer le contraste texte/fond et texte-attenue/fond pour les deux themes.<br/>
 * \tattendu Le texte principal depasse 4.5:1, le texte attenue depasse 3:1, dans les deux themes.
 * }
 */
TEST(DesignTokensTest, ChaqueThemeSatisfaitLeSeuilDeContraste) {
    constexpr double MIN_TEXT_CONTRAST = 4.5;
    constexpr double MIN_MUTED_TEXT_CONTRAST = 3.0;
    for (const hmi::DesignTokens* tokens : {&hmi::editorDarkTokens(), &hmi::editorLightTokens()}) {
        EXPECT_GE(hmi::contrastRatio(tokens->color.text, tokens->color.background),
                  MIN_TEXT_CONTRAST);
        EXPECT_GE(hmi::contrastRatio(tokens->color.textMuted, tokens->color.background),
                  MIN_MUTED_TEXT_CONTRAST);
    }
}

/**
 * @brief Le rapport de contraste est symétrique et vaut 1 pour deux couleurs identiques (aucun
 *        contraste) : garde-fou de la formule elle-même.
 * \castest{<b>Le rapport de contraste est symetrique et vaut 1 sans difference de couleur.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Calculer le contraste d'une couleur avec elle-meme, puis dans les deux sens entre
 * deux couleurs distinctes.<br/>
 * \tattendu Le premier resultat vaut 1 ; les deux sens du second sont egaux.
 * }
 */
TEST(DesignTokensTest, ContrasteSymetriqueEtUnitairePourUneMemeCouleur) {
    constexpr hmi::DesignColor color{0x42, 0x88, 0xcc};
    EXPECT_NEAR(hmi::contrastRatio(color, color), 1.0, 1e-9);
    EXPECT_NEAR(hmi::contrastRatio(hmi::DesignColor{0, 0, 0}, hmi::DesignColor{255, 255, 255}),
                hmi::contrastRatio(hmi::DesignColor{255, 255, 255}, hmi::DesignColor{0, 0, 0}),
                1e-9);
}
