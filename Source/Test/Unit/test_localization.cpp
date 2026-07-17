/**
 * @file test_localization.cpp
 * @brief Tests unitaires du catalogue de traduction : analyse, repli, changement de langue.
 */

#include <filesystem>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "HMI/Localization/Localization.h"

/// L'analyse ignore les lignes vides et les commentaires, et retire les espaces autour de '='.
TEST(LocalizationTest, AnalyseIgnoreCommentairesEtEspaces) {
    const auto strings = hmi::Localization::parseCatalog(
        "# commentaire\n"
        "\n"
        "menu.quitter = Quitter\n"
        "  menu.titre   =   ProjectGaming  \n"
        "# autre commentaire\n");

    ASSERT_EQ(strings.size(), 2u);
    EXPECT_EQ(strings.at("menu.quitter"), "Quitter");
    EXPECT_EQ(strings.at("menu.titre"), "ProjectGaming");
}

/// Seul le premier '=' sépare ; un '=' dans la valeur est conservé.
TEST(LocalizationTest, AnalyseConserveEgalDansLaValeur) {
    const auto strings = hmi::Localization::parseCatalog("expression = a = b + c\n");

    ASSERT_EQ(strings.size(), 1u);
    EXPECT_EQ(strings.at("expression"), "a = b + c");
}

/// Une clé existante est résolue dans la langue active.
TEST(LocalizationTest, CleExistanteResolue) {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}});

    EXPECT_EQ(localization.text("menu.quitter"), "Quitter");
    EXPECT_EQ(localization.activeLanguage(), "fr");
}

/// Une clé inconnue partout est renvoyée telle quelle (repli déterministe, pas de plantage).
TEST(LocalizationTest, CleInconnueRenvoieLaCle) {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}});

    EXPECT_EQ(localization.text("menu.inconnue"), "menu.inconnue");
}

/// Changer de langue résout les valeurs de la nouvelle langue.
TEST(LocalizationTest, ChangementDeLangue) {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}});
    localization.setActiveCatalog("en", {{"menu.quitter", "Quit"}});

    EXPECT_EQ(localization.activeLanguage(), "en");
    EXPECT_EQ(localization.text("menu.quitter"), "Quit");
}

/// Une clé manquante dans la langue active retombe sur la langue par défaut.
TEST(LocalizationTest, RepliSurLangueParDefaut) {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}, {"menu.titre", "Jeu"}});
    localization.setActiveCatalog("en", {{"menu.quitter", "Quit"}});  // pas de menu.titre

    EXPECT_EQ(localization.text("menu.quitter"), "Quit");  // langue active
    EXPECT_EQ(localization.text("menu.titre"), "Jeu");     // repli langue par défaut
}

/// Charger une langue absente échoue proprement et conserve la langue active (récupérable).
TEST(LocalizationTest, LangueAbsenteEstRecuperable) {
    hmi::Localization localization("dossier/inexistant");
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}});

    EXPECT_FALSE(localization.loadLanguage("xx"));
    EXPECT_EQ(localization.activeLanguage(), "fr");
    EXPECT_EQ(localization.text("menu.quitter"), "Quitter");
}

/// Le catalogue français livré (Source/Elements/Localization) se charge et résout ses clés.
TEST(LocalizationTest, CatalogueFrancaisLivreSeCharge) {
    hmi::Localization localization(std::filesystem::path(PROJECTGAMING_LOCALIZATION_DIR));

    ASSERT_TRUE(localization.loadDefaultLanguage("fr"));
    EXPECT_EQ(localization.activeLanguage(), "fr");
    EXPECT_EQ(localization.text("menu.quitter"), "Quitter");
    EXPECT_EQ(localization.text("menu.charger_niveau"), "Charger niveau");
}
