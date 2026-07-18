/**
 * @file test_level_sequence.cpp
 * @brief Tests unitaires de la séquence de niveaux (progression : ordre, enchaînement).
 */

#include <gtest/gtest.h>

#include "HMI/Interface/LevelSequence.h"

/**
 * @brief Une séquence vide est signalée comme telle.
 * \castest{<b>Une séquence vide est signalée comme telle.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une séquence vide est signalée comme telle.
 * }
 */
TEST(LevelSequenceTest, SequenceVide) {
    const hmi::LevelSequence sequence({});
    EXPECT_TRUE(sequence.empty());
    EXPECT_EQ(sequence.size(), 0u);
}

/**
 * @brief La séquence démarre sur le premier niveau, dans l'ordre fourni.
 * \castest{<b>La séquence démarre sur le premier niveau, dans l'ordre fourni.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La séquence démarre sur le premier niveau, dans l'ordre fourni.
 * }
 */
TEST(LevelSequenceTest, DemarreSurLePremier) {
    const hmi::LevelSequence sequence({"a.json", "b.json", "c.json"});
    EXPECT_FALSE(sequence.empty());
    EXPECT_EQ(sequence.size(), 3u);
    EXPECT_EQ(sequence.index(), 0u);
    EXPECT_EQ(sequence.current(), std::filesystem::path("a.json"));
    EXPECT_TRUE(sequence.hasNext());
}

/**
 * @brief `advance` parcourt les niveaux dans l'ordre jusqu'au dernier.
 * \castest{<b>`advance` parcourt les niveaux dans l'ordre jusqu'au dernier.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `advance` parcourt les niveaux dans l'ordre jusqu'au dernier.
 * }
 */
TEST(LevelSequenceTest, AvanceDansLOrdre) {
    hmi::LevelSequence sequence({"a.json", "b.json", "c.json"});

    sequence.advance();
    EXPECT_EQ(sequence.index(), 1u);
    EXPECT_EQ(sequence.current(), std::filesystem::path("b.json"));
    EXPECT_TRUE(sequence.hasNext());

    sequence.advance();
    EXPECT_EQ(sequence.current(), std::filesystem::path("c.json"));
    EXPECT_FALSE(sequence.hasNext());  // dernier niveau
}

/**
 * @brief Au-delà du dernier niveau, `advance` est sans effet (pas de dépassement).
 * \castest{<b>Au-delà du dernier niveau, `advance` est sans effet (pas de dépassement).</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Au-delà du dernier niveau, `advance` est sans effet (pas de dépassement).
 * }
 */
TEST(LevelSequenceTest, NeDepassePasLeDernier) {
    hmi::LevelSequence sequence({"seul.json"});
    EXPECT_FALSE(sequence.hasNext());

    sequence.advance();  // aucun suivant
    EXPECT_EQ(sequence.index(), 0u);
    EXPECT_EQ(sequence.current(), std::filesystem::path("seul.json"));
}
