/**
 * @file test_application_theme.cpp
 * @brief Tests unitaires du modèle de feuille de style de l'IHM (`LOT-56` TACHE-02, `EX-IHM-050`,
 *        `EX-IHM-051`).
 */

#include <gtest/gtest.h>

#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_map>

#include "HMI/Interface/StyleSheetTemplate.h"

namespace {

// Chemin (source) du modele reel, pour verifier le fichier livre plutot qu'une chaine de test
// isolee -- coherent avec PROJECTGAMING_ASSETS_DIR etc. (Test/CMakeLists.txt).
[[nodiscard]] std::string readThemeTemplate() {
    std::ifstream file(PROJECTGAMING_THEME_PATH);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace

/**
 * @brief Chaque marqueur `${nom}` présent dans le modèle est remplacé par sa valeur ; le résultat
 *        ne contient plus aucun marqueur.
 * \castest{<b>La substitution remplace tous les marqueurs du modele.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Substituer un modele avec deux marqueurs connus.<br/>
 * \tattendu Le texte produit contient les valeurs substituees et plus aucun marqueur.
 * }
 */
TEST(ApplicationThemeTest, SubstitutionRemplaceTousLesMarqueurs) {
    const std::unordered_map<std::string, std::string> values{{"a", "1"}, {"b", "2"}};
    const hmi::StyleSheetSubstitutionResult result =
        hmi::substituteStyleSheetTemplate("x: ${a}; y: ${b};", values);
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.text, "x: 1; y: 2;");
}

/**
 * @brief Un marqueur absent de la table de substitution est **signalé**, jamais produit
 *        silencieusement avec un trou.
 * \castest{<b>Un marqueur inconnu est signale plutot que produit avec un trou.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Substituer un modele dont un marqueur n'a pas de valeur.<br/>
 * \tattendu Le resultat est en echec et nomme le marqueur manquant.
 * }
 */
TEST(ApplicationThemeTest, MarqueurInconnuEstSignale) {
    const hmi::StyleSheetSubstitutionResult result =
        hmi::substituteStyleSheetTemplate("x: ${inconnu};", {});
    ASSERT_FALSE(result.ok);
    EXPECT_NE(result.error.find("inconnu"), std::string::npos);
}

/**
 * @brief Le modèle réel livré (`Source/Elements/Themes/theme.qss`) ne contient aucune couleur
 *        écrite en dur : toutes passent par un marqueur `${...}`.
 * \castest{<b>Le modele de theme livre ne contient aucune couleur litterale.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire le fichier theme.qss livre.<br/>2. Chercher un motif de couleur hexadecimale.<br/>
 * \tattendu Aucune occurrence en dehors des commentaires n'est trouvee.
 * }
 */
TEST(ApplicationThemeTest, AucuneCouleurLitteraleDansLeModeleReel) {
    const std::string themeText = readThemeTemplate();
    ASSERT_FALSE(themeText.empty()) << "theme.qss introuvable a PROJECTGAMING_THEME_PATH";

    // Retire les commentaires /* ... */ (l'en-tete documente l'historique en exemples de couleurs)
    // avant de chercher un motif de couleur hexadecimale dans les regles elles-memes.
    const std::regex commentPattern(R"(/\*[\s\S]*?\*/)");
    const std::string withoutComments =
        std::regex_replace(themeText, commentPattern, "");

    const std::regex hexColorPattern(R"(#[0-9a-fA-F]{6}\b)");
    EXPECT_FALSE(std::regex_search(withoutComments, hexColorPattern))
        << "couleur hexadecimale litterale trouvee hors commentaire";
}

/**
 * @brief Produire la feuille de style avec deux jeux de valeurs **variables** différents donne
 *        deux résultats dont les règles d'**identité** (`#MainMenu`, `#OptionsPage`) sont
 *        identiques au caractère près : le thème de l'éditeur ne doit jamais faire bouger le menu
 *        principal.
 * \castest{<b>Les regles d'identite sont etanches au theme de l'editeur.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Substituer le modele reel avec deux jeux de valeurs 'editor.*' distincts, memes
 * valeurs 'identity.*'.<br/>2. Extraire les blocs #MainMenu/#OptionsPage des deux resultats.<br/>
 * \tattendu Les deux extraits sont identiques au caractere pres.
 * }
 */
TEST(ApplicationThemeTest, EtancheiteDesPortees) {
    const std::string themeText = readThemeTemplate();
    ASSERT_FALSE(themeText.empty()) << "theme.qss introuvable a PROJECTGAMING_THEME_PATH";

    const std::unordered_map<std::string, std::string> identity{
        {"identity.color.background", "#1a1f29"}, {"identity.color.surface", "#1e2531"},
        {"identity.color.surfaceAlt", "#232a36"},  {"identity.color.border", "#333a48"},
        {"identity.color.text", "#f2f2ff"},        {"identity.color.textMuted", "#b3b8c7"},
        {"identity.color.accent", "#ffd133"},      {"identity.color.accentHover", "#ffdb5c"},
        {"identity.color.error", "#ff5c5c"},       {"tokens.spacing.extraSmall", "4"},
        {"tokens.spacing.small", "8"},              {"tokens.spacing.medium", "12"},
        {"tokens.spacing.large", "16"},             {"tokens.spacing.extraLarge", "24"}};

    auto valuesWithEditor = [&](const std::string& suffix) {
        std::unordered_map<std::string, std::string> values = identity;
        for (const char* role : {"background", "surface", "surfaceAlt", "border", "text",
                                 "textMuted", "accent", "accentHover", "error"}) {
            values[std::string("editor.color.") + role] = std::string("#") + suffix;
        }
        return values;
    };

    const hmi::StyleSheetSubstitutionResult dark =
        hmi::substituteStyleSheetTemplate(themeText, valuesWithEditor("111111"));
    const hmi::StyleSheetSubstitutionResult light =
        hmi::substituteStyleSheetTemplate(themeText, valuesWithEditor("eeeeee"));
    ASSERT_TRUE(dark.ok) << dark.error;
    ASSERT_TRUE(light.ok) << light.error;

    // Extrait la portion "identite" : de la premiere regle #MainMenu jusqu'a la premiere regle du
    // chassis d'edition (marquee par le commentaire de section) -- stable tant que theme.qss garde
    // ses deux sections dans cet ordre.
    const std::string sectionMarker = "Chassis d'edition";
    const std::size_t darkEnd = dark.text.find(sectionMarker);
    const std::size_t lightEnd = light.text.find(sectionMarker);
    ASSERT_NE(darkEnd, std::string::npos);
    ASSERT_NE(lightEnd, std::string::npos);
    EXPECT_EQ(dark.text.substr(0, darkEnd), light.text.substr(0, lightEnd));
}
