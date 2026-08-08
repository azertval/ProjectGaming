/**
 * @file test_game_hud.cpp
 * @brief Tests unitaires du choix de contenu de l'affichage tête haute (LOT-52 TACHE-03,
 *        EX-IHM-003). Fonction pure, sans GPU.
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Player.h"
#include "HMI/Game/GameHud.h"
#include "HMI/Localization/Localization.h"

namespace {

hmi::Localization testLocalization() {
    hmi::Localization localization;
    localization.setDefaultCatalog(
        "fr", {{"hud.jumps_remaining", "Sauts : %1"}, {"hud.dashes_remaining", "Dashs : %1"}});
    return localization;
}

}  // namespace

/**
 * @brief Un niveau avec des budgets finis affiche les deux compteurs, puis le nom du tableau.
 * \castest{<b>Un niveau avec des budgets finis affiche les deux compteurs et le nom du
 * tableau.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les lignes « Sauts : 3 », « Dashs : 1 » et le nom du tableau sont produites, dans cet
 * ordre.
 * }
 */
TEST(GameHudTest, BudgetsFinisAffichentLesDeuxCompteurs) {
    core::Player player;
    player.jumpsRemaining = 3;
    player.dashesRemaining = 1;

    const std::vector<std::string> lines =
        hmi::gameHudLines(player, "Salle des epreuves", testLocalization());

    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "Sauts : 3");
    EXPECT_EQ(lines[1], "Dashs : 1");
    EXPECT_EQ(lines[2], "Salle des epreuves");
}

/**
 * @brief Un niveau sans budget (illimité aux deux mouvements) n'affiche aucun compteur, seulement
 *        le nom du tableau.
 * \castest{<b>Un niveau sans budget n'affiche aucun compteur.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Seul le nom du tableau est produit.
 * }
 */
TEST(GameHudTest, NiveauSansBudgetNAfficheAucunCompteur) {
    core::Player player;  // jumpsRemaining/dashesRemaining par defaut : -1 (illimite)

    const std::vector<std::string> lines = hmi::gameHudLines(player, "Promenade", testLocalization());

    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "Promenade");
}

/**
 * @brief Un budget partiellement défini (sauts limités, dashs illimités) n'affiche qu'un seul
 *        compteur.
 * \castest{<b>Un budget partiellement défini n'affiche qu'un seul compteur.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Seul le compteur de sauts est produit, avant le nom du tableau.
 * }
 */
TEST(GameHudTest, BudgetPartielNAfficheQuUnCompteur) {
    core::Player player;
    player.jumpsRemaining = 2;
    player.dashesRemaining = -1;

    const std::vector<std::string> lines = hmi::gameHudLines(player, "Chute libre", testLocalization());

    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "Sauts : 2");
    EXPECT_EQ(lines[1], "Chute libre");
}

/**
 * @brief Le compteur affiché suit immédiatement la décroissance du budget (après un saut).
 * \castest{<b>Le compteur affiché suit immédiatement la décroissance du budget.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer les lignes avec 3 sauts restants, puis avec 2 (après un saut simulé).<br/>
 * 2. Comparer les deux résultats.<br/>
 * \tattendu Le compteur passe de « Sauts : 3 » à « Sauts : 2 ».
 * }
 */
TEST(GameHudTest, CompteurSuitLaDecroissanceDuBudget) {
    core::Player player;
    player.jumpsRemaining = 3;
    const hmi::Localization localization = testLocalization();

    ASSERT_EQ(hmi::gameHudLines(player, "Niveau", localization)[0], "Sauts : 3");

    --player.jumpsRemaining;  // meme decompte que core::CharacterPhysicsSystem apres un saut

    EXPECT_EQ(hmi::gameHudLines(player, "Niveau", localization)[0], "Sauts : 2");
}

/**
 * @brief Chaque clé de traduction utilisée par le HUD existe, traduite, dans les deux catalogues
 *        livrés (français et anglais).
 * \castest{<b>Les clés de traduction du HUD existent dans les deux catalogues livrés.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang puis en.lang depuis les catalogues livrés.<br/>2. Résoudre les clés
 * du HUD.<br/>
 * \tattendu Les deux clés résolvent vers un texte traduit, distinct de la clé elle-même.
 * }
 */
TEST(GameHudTest, ClesDeTraductionExistentDansLesDeuxCatalogues) {
    const std::filesystem::path directory(PROJECTGAMING_LOCALIZATION_DIR);
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(directory);
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;
        EXPECT_NE(localization.text("hud.jumps_remaining"), "hud.jumps_remaining") << language;
        EXPECT_NE(localization.text("hud.dashes_remaining"), "hud.dashes_remaining") << language;
    }
}
