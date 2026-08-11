/**
 * @file test_progression.cpp
 * @brief Tests unitaires de la progression de partie persistée (LOT-59 TACHE-05, EX-LVL-014).
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "HMI/Game/Progression.h"

namespace {

// Fournit un dossier temporaire vierge par test (créé/supprimé automatiquement) -- même patron que
// LevelFileOps (Source/Test/Unit/HMI/Editor/test_level_file_operations.cpp).
class ProgressionDir : public ::testing::Test {
protected:
    std::filesystem::path dir;
    std::filesystem::path path;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("pg_progression_" + std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        std::filesystem::create_directories(dir);
        path = dir / "progression.json";
    }
    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(dir, error);
    }
};

}  // namespace

/**
 * @brief Une progression neuve (construction par défaut) n'a ni séquence, ni tableau atteint, ni
 * tableau terminé.
 * \castest{<b>Une progression neuve est vide.</b><br/>
 * \tcat Unitaire · Progression<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une `Progression` par défaut.<br/>2. Vérifier ses champs.<br/>
 * \tattendu Séquence et tableau atteint vides, aucun tableau terminé.
 * }
 */
TEST(ProgressionTest, ProgressionNeuveEstVide) {
    const hmi::Progression progression;
    EXPECT_TRUE(progression.sequenceId().empty());
    EXPECT_TRUE(progression.currentLevel().empty());
    EXPECT_TRUE(progression.completedLevels().empty());
}

/**
 * @brief Marquer un tableau terminé deux fois (rejouer puis re-terminer) ne change pas l'ensemble
 * -- couvre « marqué une seule fois » sans logique dédiée (`std::set`).
 * \castest{<b>Marquer un tableau terminé deux fois est sans effet.</b><br/>
 * \tcat Unitaire · Progression<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Marquer un tableau terminé.<br/>2. Le marquer une seconde fois.<br/>3. Vérifier la
 * taille de l'ensemble.<br/>
 * \tattendu L'ensemble ne contient qu'une seule entrée pour ce tableau.
 * }
 */
TEST(ProgressionTest, MarquerDeuxFoisLeMemeTableauEstSansEffet) {
    hmi::Progression progression;
    progression.markCompleted("demo-saut.json");
    progression.markCompleted("demo-saut.json");
    EXPECT_EQ(progression.completedLevels().size(), 1U);
    EXPECT_TRUE(progression.isCompleted("demo-saut.json"));
    EXPECT_FALSE(progression.isCompleted("demo-dash.json"));
}

/**
 * @brief `reset()` efface toute la progression, retour à l'état de construction (« Nouvelle
 * partie »).
 * \castest{<b>reset() efface toute la progression.</b><br/>
 * \tcat Unitaire · Progression<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une progression avec du contenu.<br/>2. Appeler `reset()`.<br/>3.
 * Vérifier qu'elle est redevenue vide.<br/>
 * \tattendu Séquence, tableau atteint et tableaux terminés sont tous vidés.
 * }
 */
TEST(ProgressionTest, ResetEffaceToutLaProgression) {
    hmi::Progression progression;
    progression.setSequenceId("sequence-demo.json");
    progression.setCurrentLevel("demo-saut.json");
    progression.markCompleted("demo-deplacement.json");

    progression.reset();

    EXPECT_TRUE(progression.sequenceId().empty());
    EXPECT_TRUE(progression.currentLevel().empty());
    EXPECT_TRUE(progression.completedLevels().empty());
}

/**
 * @brief Aller-retour : sauvegarder une progression puis la relire produit exactement la même.
 * \castest{<b>Sauvegarder puis relire une progression produit exactement la même.</b><br/>
 * \tcat Unitaire · Progression<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une progression avec plusieurs tableaux terminés.<br/>2. La
 * sauvegarder.<br/>3. La relire depuis le même fichier.<br/>
 * \tattendu La progression relue a exactement la même séquence, le même tableau atteint et le
 * même ensemble de tableaux terminés.
 * }
 */
TEST_F(ProgressionDir, AllerRetourProduitExactementLaMemeProgression) {
    hmi::Progression original;
    original.setSequenceId("sequence-demo.json");
    original.setCurrentLevel("demo-dash.json");
    original.markCompleted("demo-deplacement.json");
    original.markCompleted("demo-saut.json");
    original.markCompleted("demo-double-saut.json");

    ASSERT_TRUE(original.save(path));
    const hmi::Progression reloaded = hmi::Progression::load(path);

    EXPECT_EQ(reloaded.sequenceId(), original.sequenceId());
    EXPECT_EQ(reloaded.currentLevel(), original.currentLevel());
    EXPECT_EQ(reloaded.completedLevels(), original.completedLevels());
}

/**
 * @brief Un fichier de progression absent donne une partie neuve, sans erreur.
 * \castest{<b>Un fichier de progression absent donne une partie neuve.</b><br/>
 * \tcat Unitaire · Progression<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger depuis un chemin qui n'existe pas.<br/>2. Vérifier le résultat.<br/>
 * \tattendu La progression chargée est vide (équivalente à une construction par défaut).
 * }
 */
TEST_F(ProgressionDir, FichierAbsentDonneUnePartieNeuve) {
    const hmi::Progression progression = hmi::Progression::load(path);
    EXPECT_TRUE(progression.sequenceId().empty());
    EXPECT_TRUE(progression.currentLevel().empty());
    EXPECT_TRUE(progression.completedLevels().empty());
}

/**
 * @brief Un fichier de progression vide donne une partie neuve, sans exception.
 * \castest{<b>Un fichier de progression vide donne une partie neuve.</b><br/>
 * \tcat Unitaire · Progression<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Écrire un fichier vide.<br/>2. Le charger.<br/>
 * \tattendu La progression chargée est vide, aucune exception.
 * }
 */
TEST_F(ProgressionDir, FichierVideDonneUnePartieNeuve) {
    std::ofstream(path).close();
    const hmi::Progression progression = hmi::Progression::load(path);
    EXPECT_TRUE(progression.sequenceId().empty());
    EXPECT_TRUE(progression.completedLevels().empty());
}

/**
 * @brief Un fichier de progression malformé (JSON invalide) donne une partie neuve, sans
 * exception qui remonte.
 * \castest{<b>Un fichier de progression malformé donne une partie neuve.</b><br/>
 * \tcat Unitaire · Progression<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Écrire du JSON invalide.<br/>2. Le charger.<br/>
 * \tattendu La progression chargée est vide, aucune exception.
 * }
 */
TEST_F(ProgressionDir, FichierMalformeDonneUnePartieNeuve) {
    std::ofstream output(path);
    output << "{ pas du json";
    output.close();

    const hmi::Progression progression = hmi::Progression::load(path);
    EXPECT_TRUE(progression.sequenceId().empty());
    EXPECT_TRUE(progression.completedLevels().empty());
}

/**
 * @brief Une entrée de `completedLevels` d'un type inattendu (pas une chaîne) est ignorée, le
 * reste de la progression restant valide -- jamais de plantage sur une entrée inconnue.
 * \castest{<b>Une entrée inconnue de completedLevels est ignorée sans invalider le reste.</b><br/>
 * \tcat Unitaire · Progression<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Écrire un fichier dont `completedLevels` contient une entrée numérique.<br/>2. Le
 * charger.<br/>
 * \tattendu L'entrée invalide est ignorée ; les entrées valides sont conservées.
 * }
 */
TEST_F(ProgressionDir, EntreeInconnueDansCompletedLevelsEstIgnoree) {
    std::ofstream output(path);
    output << R"({
      "sequenceId": "sequence-demo.json",
      "currentLevel": "demo-saut.json",
      "completedLevels": ["demo-deplacement.json", 42, "demo-saut.json"]
    })";
    output.close();

    const hmi::Progression progression = hmi::Progression::load(path);
    EXPECT_EQ(progression.sequenceId(), "sequence-demo.json");
    EXPECT_EQ(progression.completedLevels().size(), 2U);
    EXPECT_TRUE(progression.isCompleted("demo-deplacement.json"));
    EXPECT_TRUE(progression.isCompleted("demo-saut.json"));
}

/**
 * @brief Un tableau terminé puis retiré de la séquence n'invalide pas le reste de la progression
 * -- le stockage par nom ne dépend d'aucune liste de niveaux existants.
 * \castest{<b>Un tableau retiré de la séquence n'invalide pas le reste de la progression.</b><br/>
 * \tcat Unitaire · Progression<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Marquer trois tableaux terminés.<br/>2. Simuler le retrait d'un tableau de la
 * séquence (aucune action sur la progression : elle ne connaît pas la séquence).<br/>3. Vérifier
 * que les deux autres restent marqués terminés.<br/>
 * \tattendu Les tableaux non retirés restent terminés, indépendamment de la séquence.
 * }
 */
TEST(ProgressionTest, TableauRetireDeLaSequenceNInvalidePasLeReste) {
    hmi::Progression progression;
    progression.markCompleted("demo-deplacement.json");
    progression.markCompleted("demo-saut.json");
    progression.markCompleted("demo-dash.json");

    // "demo-saut.json" retiré de la séquence (TACHE-04) : la progression ne réagit à rien, elle
    // ne connaît que des noms, jamais la séquence elle-même.
    EXPECT_TRUE(progression.isCompleted("demo-deplacement.json"));
    EXPECT_TRUE(progression.isCompleted("demo-dash.json"));
    EXPECT_EQ(progression.completedLevels().size(), 3U);
}
