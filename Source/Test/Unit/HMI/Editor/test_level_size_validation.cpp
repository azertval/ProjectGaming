/**
 * @file test_level_size_validation.cpp
 * @brief Tests unitaires de la validation d'une taille de grille saisie (LOT-16, EX-EDIT-017).
 */

#include <gtest/gtest.h>

#include "HMI/Editor/LevelSizeValidation.h"

/**
 * @brief Un format « largeur x hauteur » valide est analysé correctement.
 * \castest{<b>Un format « largeur x hauteur » valide est analysé correctement.</b><br/>
 * \tcat Unitaire · Level Size Validation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un format « largeur x hauteur » valide est analysé correctement.
 * }
 */
TEST(LevelSizeValidationTest, FormatValideAnalyseCorrectement) {
    const std::optional<std::pair<int, int>> result = hmi::parseLevelSize("40x30");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->first, 40);
    EXPECT_EQ(result->second, 30);
}

/**
 * @brief Les espaces autour du séparateur et une casse différente sont tolérés.
 * \castest{<b>Les espaces autour du séparateur et une casse différente sont tolérés.</b><br/>
 * \tcat Unitaire · Level Size Validation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les espaces autour du séparateur et une casse différente sont tolérés.
 * }
 */
TEST(LevelSizeValidationTest, EspacesEtCasseTolerees) {
    EXPECT_EQ(hmi::parseLevelSize("40 x 30"), (std::make_pair(40, 30)));
    EXPECT_EQ(hmi::parseLevelSize("40X30"), (std::make_pair(40, 30)));
    // Espaces de bord (avant la largeur, apres la hauteur) : chaque dimension est retrimee
    // individuellement, tolere donc aussi les espaces entourant le texte entier.
    EXPECT_EQ(hmi::parseLevelSize("  40x30  "), (std::make_pair(40, 30)));
}

/**
 * @brief Le séparateur `*` est accepté comme alternative à `x`/`X`.
 * \castest{<b>Le séparateur `*` est accepté comme alternative à `x`/`X`.</b><br/>
 * \tcat Unitaire · Level Size Validation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le séparateur `*` est accepté comme alternative à `x`/`X`.
 * }
 */
TEST(LevelSizeValidationTest, SeparateurEtoileAccepte) {
    EXPECT_EQ(hmi::parseLevelSize("40*30"), (std::make_pair(40, 30)));
    EXPECT_EQ(hmi::parseLevelSize("40 * 30"), (std::make_pair(40, 30)));
}

/**
 * @brief Un texte sans séparateur est refusé.
 * \castest{<b>Un texte sans séparateur est refusé.</b><br/>
 * \tcat Unitaire · Level Size Validation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un texte sans séparateur est refusé.
 * }
 */
TEST(LevelSizeValidationTest, SansSeparateurRefuse) {
    EXPECT_FALSE(hmi::parseLevelSize("4030").has_value());
    EXPECT_FALSE(hmi::parseLevelSize("").has_value());
}

/**
 * @brief Une dimension non numérique, nulle, négative ou partiellement numérique est refusée.
 * \castest{<b>Une dimension non numérique, nulle, négative ou partiellement numérique est
 * refusée.</b><br/>
 * \tcat Unitaire · Level Size Validation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une dimension non numérique, nulle, négative ou partiellement numérique est refusée.
 * }
 */
TEST(LevelSizeValidationTest, DimensionInvalideRefusee) {
    EXPECT_FALSE(hmi::parseLevelSize("abcxdef").has_value());
    EXPECT_FALSE(hmi::parseLevelSize("0x30").has_value());
    EXPECT_FALSE(hmi::parseLevelSize("40x0").has_value());
    EXPECT_FALSE(hmi::parseLevelSize("-5x30").has_value());
    EXPECT_FALSE(hmi::parseLevelSize("12abcx30").has_value());
}

/**
 * @brief Une dimension au-delà du plafond est refusée ; le plafond lui-même est accepté.
 * \castest{<b>Une dimension au-delà du plafond est refusée ; le plafond lui-même est
 * accepté.</b><br/>
 * \tcat Unitaire · Level Size Validation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une dimension au-delà du plafond est refusée ; le plafond lui-même est accepté.
 * }
 */
TEST(LevelSizeValidationTest, PlafondRespecte) {
    const std::string atCap =
        std::to_string(hmi::MAX_LEVEL_DIMENSION) + "x" + std::to_string(hmi::MAX_LEVEL_DIMENSION);
    EXPECT_TRUE(hmi::parseLevelSize(atCap).has_value());

    const std::string overCap = std::to_string(hmi::MAX_LEVEL_DIMENSION + 1) + "x10";
    EXPECT_FALSE(hmi::parseLevelSize(overCap).has_value());
}

/**
 * @brief isValidLevelSize reste cohérent avec parseLevelSize.
 * \castest{<b>isValidLevelSize reste cohérent avec parseLevelSize.</b><br/>
 * \tcat Unitaire · Level Size Validation<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu isValidLevelSize reste cohérent avec parseLevelSize.
 * }
 */
TEST(LevelSizeValidationTest, IsValidCoherentAvecParse) {
    EXPECT_TRUE(hmi::isValidLevelSize("14x8"));
    EXPECT_FALSE(hmi::isValidLevelSize("quatorze x huit"));
}
