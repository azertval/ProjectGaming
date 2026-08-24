// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_level_fingerprint.cpp
 * @brief Tests unitaires de `aisolver::computeLevelFingerprint` (LOT-ANNEXE-17, TACHE-01,
 * `EX-IA-018`).
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "AiSolver/Replay/LevelFingerprint.h"

/**
 * @brief Deux appels sur la meme chaine produisent la meme empreinte (determinisme de la fonction
 * elle-meme).
 * \castest{Empreintes identiques pour un contenu identique.<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `computeLevelFingerprint` deux fois sur la meme chaine.<br/>
 * \tattendu Les deux empreintes sont strictement egales.}
 */
TEST(LevelFingerprintTest, EmpreintesIdentiquesPourContenuIdentique) {
    const std::string content = R"({"width": 10, "height": 8, "tiles": []})";
    EXPECT_EQ(aisolver::computeLevelFingerprint(content), aisolver::computeLevelFingerprint(content));
}

/**
 * @brief Un changement d'un seul caractere change l'empreinte (test de non-trivialite, pas une
 * preuve d'absence de collision).
 * \castest{Empreintes differentes pour un contenu different.<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `computeLevelFingerprint` sur deux chaines ne differant que d'un caractere.<br/>
 * \tattendu Les deux empreintes different.}
 */
TEST(LevelFingerprintTest, EmpreintesDifferentesPourContenuDifferent) {
    const std::string a = R"({"width": 10, "height": 8, "tiles": []})";
    const std::string b = R"({"width": 11, "height": 8, "tiles": []})";
    EXPECT_NE(aisolver::computeLevelFingerprint(a), aisolver::computeLevelFingerprint(b));
}

/**
 * @brief La chaine vide est un cas valide (aucune exception), empreinte fixe egale au biais
 * initial de FNV-1a.
 * \castest{Chaine vide : cas limite valide.<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. `computeLevelFingerprint` sur une chaine vide.<br/>
 * \tattendu Aucune exception ; le resultat est deterministe (identique a un second appel).}
 */
TEST(LevelFingerprintTest, ChaineVideEstUnCasLimiteValide) {
    EXPECT_EQ(aisolver::computeLevelFingerprint(""), aisolver::computeLevelFingerprint(""));
}

/**
 * @brief L'empreinte calculee par cette implementation C++ sur un fichier de niveau reel committe
 * correspond exactement a celle calculee par la reimplementation Python de `LOT-ANNEXE-20`
 * (`scripts/check_ai_replays.py`, fonction `fnv1a_64`) sur le meme fichier -- valeur de reference
 * partagee entre les deux implementations, verifiee manuellement en appelant `fnv1a_64` depuis un
 * interprete Python sur le contenu binaire brut du meme fichier de niveau.
 * \castest{Empreinte C++ identique a la reimplementation Python (LOT-ANNEXE-20).<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Lecture brute de `Source/Elements/Levels/demo-deplacement.json`.<br/>2.
 * `computeLevelFingerprint` sur son contenu.<br/>
 * \tattendu Le resultat vaut exactement `12567232183729497359`, la meme valeur que celle produite
 * par `fnv1a_64` (Python) sur le meme fichier.}
 */
TEST(LevelFingerprintTest, EmpreinteIdentiqueALaReimplementationPythonDuGardeFouCi) {
    std::ifstream file(std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "demo-deplacement.json",
                       std::ios::binary);
    ASSERT_TRUE(file);
    std::ostringstream contents;
    contents << file.rdbuf();

    constexpr std::uint64_t kExpectedFingerprint = 12567232183729497359ULL;
    EXPECT_EQ(aisolver::computeLevelFingerprint(contents.str()), kExpectedFingerprint);
}
