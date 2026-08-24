// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_level_fingerprint.cpp
 * @brief Tests unitaires de `aisolver::computeLevelFingerprint` (LOT-ANNEXE-17, TACHE-01,
 * `EX-IA-018`).
 */

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
