/**
 * @file test_application_theme.cpp
 * @brief Tests unitaires du thème de l'IHM : modèle de feuille de style (`LOT-56` TACHE-02,
 *        `EX-IHM-050`, `EX-IHM-051`) et police/typographie (TACHE-03, `EX-IHM-052`).
 */

#include <gtest/gtest.h>

#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_map>

#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/FontResolution.h"
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
        {"tokens.spacing.large", "16"},             {"tokens.spacing.extraLarge", "24"},
        {"tokens.typography.screenTitle.pointSize", "32"},
        {"tokens.typography.sectionTitle.pointSize", "16"}};

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

/**
 * @brief La police embarquée est retenue quand elle a pu être enregistrée ; sinon, aucun nom de
 *        famille n'est renvoyé (TACHE-03) -- l'appelant Qt doit alors demander une famille
 *        générique, jamais un second nom codé en dur.
 * \castest{<b>La resolution de police retombe sur une famille generique sans nom code en dur.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre la police embarquee enregistree, puis non enregistree.<br/>
 * \tattendu Le premier cas rend la famille embarquee ; le second ne rend aucun nom de famille.
 * }
 */
TEST(ApplicationThemeTest, ResolutionDePoliceSansNomDeRepliCodeEnDur) {
    const hmi::FontFamilyResolution registered = hmi::resolveFontFamily(true, "Inter");
    EXPECT_TRUE(registered.useEmbeddedFamily);
    EXPECT_EQ(registered.embeddedFamily, "Inter");

    const hmi::FontFamilyResolution missing = hmi::resolveFontFamily(false, "Inter");
    EXPECT_FALSE(missing.useEmbeddedFamily);
    EXPECT_TRUE(missing.embeddedFamily.empty());
}

/**
 * @brief L'échelle typographique produit des tailles strictement positives, et les rôles de titre
 *        et de corps sont ordonnés du plus grand au plus petit : titre d'écran > titre de
 *        section > corps > libellé secondaire.
 * \castest{<b>L'echelle typographique est positive et ordonnee du plus grand au plus petit.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire les tailles de l'echelle typographique des jetons.<br/>
 * \tattendu Toutes sont strictement positives ; titre d'ecran > titre de section > corps > libelle
 * secondaire.
 * }
 */
TEST(ApplicationThemeTest, EchelleTypographiquePositiveEtOrdonnee) {
    const hmi::TypographyTokens& typography = hmi::identityTokens().typography;
    EXPECT_GT(typography.screenTitle.pointSize, 0);
    EXPECT_GT(typography.sectionTitle.pointSize, 0);
    EXPECT_GT(typography.body.pointSize, 0);
    EXPECT_GT(typography.caption.pointSize, 0);
    EXPECT_GT(typography.monospaceBody.pointSize, 0);

    EXPECT_GT(typography.screenTitle.pointSize, typography.sectionTitle.pointSize);
    EXPECT_GT(typography.sectionTitle.pointSize, typography.body.pointSize);
    EXPECT_GT(typography.body.pointSize, typography.caption.pointSize);
}

/**
 * @brief Aucune propriété de police ni de marge figée ne subsiste dans `MainMenu.ui` ou
 *        `OptionsPage.ui` : la typographie et l'espacement viennent des jetons, pas du fichier
 *        `.ui`.
 * \castest{<b>Aucune taille de police ni marge figee ne subsiste dans les fichiers .ui.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire MainMenu.ui et OptionsPage.ui.<br/>2. Chercher une propriete font/margin figee
 * au niveau du widget racine.<br/>
 * \tattendu Aucune des deux proprietes n'apparait dans l'un ou l'autre fichier.
 * }
 */
TEST(ApplicationThemeTest, AucuneTailleDePoliceResiduelleDansLesFichiersUi) {
    for (const char* path : {PROJECTGAMING_MAIN_MENU_UI_PATH, PROJECTGAMING_OPTIONS_PAGE_UI_PATH}) {
        std::ifstream file(path);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        const std::string text = buffer.str();
        ASSERT_FALSE(text.empty()) << "fichier .ui introuvable : " << path;
        EXPECT_EQ(text.find("<property name=\"font\">"), std::string::npos)
            << "propriete de police figee trouvee dans " << path;
        EXPECT_EQ(text.find("Margin"), std::string::npos)
            << "propriete de marge figee trouvee dans " << path;
    }
}
