/**
 * @file test_options_model.cpp
 * @brief Tests unitaires de la logique du menu d'options : navigation, défilement et actions.
 */

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Interface/MenuModel.h"
#include "HMI/Interface/OptionsModel.h"
#include "HMI/Localization/Localization.h"

namespace {

// Hauteur de fenêtre utilisée par la plupart des tests : celle de la fenêtre par défaut du jeu
// (1280x720, cf. Source/HMI/main.cpp) — avec les constantes de MenuModel, seules 4 des 5 options
// tiennent sans défilement (voir OptionsModelTest.VisibleOptionCountA720p ci-dessous).
constexpr float VIEWPORT_HEIGHT = 720.0f;

/// Construit un catalogue français minimal pour les libellés des options.
hmi::Localization frenchCatalog() {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"options.vsync_on", "V-Sync : Active"},
                                          {"options.vsync_off", "V-Sync : Desactive"},
                                          {"options.retour", "Retour"},
                                          {"keybindings.titre_jeu", "Touches de jeu"},
                                          {"keybindings.titre_editeur", "Touches de l'editeur"},
                                          {"keybindings.titre_manette", "Touches de la manette"}});
    return localization;
}

/// @return Un état d'entrées où @p key vient d'être pressée (front montant).
hmi::InputState keyPress(hmi::Key key) {
    hmi::InputState input;
    input.beginFrame();
    input.onKeyDown(key);
    return input;
}

/// @return Un état d'entrées avec la souris en (@p x, @p y) et le bouton gauche cliqué.
hmi::InputState mouseClick(int x, int y) {
    hmi::InputState input;
    input.beginFrame();
    input.onMouseMove(x, y);
    input.onMouseButtonDown(hmi::MouseButton::Left);
    return input;
}

/// Point situé à l'intérieur du libellé affiché au rang visible @p visibleRow (0 = premier rang de
/// la fenêtre visible courante, indépendant du défilement) — même mise en page que MenuModel.
int optionPointX() {
    return static_cast<int>(hmi::MenuModel::MARGIN_X) + 5;
}
int optionPointYForVisibleRow(int visibleRow) {
    return static_cast<int>(hmi::MenuModel::optionTop(visibleRow)) + 5;
}

}  // namespace

/**
 * @brief À l'ouverture, la première option (V-Sync) est sélectionnée et affiche l'état fourni.
 * \castest{<b>À l'ouverture, la première option (V-Sync) est sélectionnée et affiche l'état
 * fourni.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu À l'ouverture, la première option (V-Sync) est sélectionnée et affiche l'état fourni.
 * }
 */
TEST(OptionsModelTest, SelectionParDefautEtLibelleVSync) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    EXPECT_EQ(options.selectedIndex(), 0);
    EXPECT_EQ(options.scrollOffset(), 0);
    EXPECT_EQ(options.optionLabel(0), "V-Sync : Active");
    EXPECT_EQ(options.optionLabel(1), "Touches de jeu");
    EXPECT_EQ(options.optionLabel(2), "Touches de l'editeur");
    EXPECT_EQ(options.optionLabel(3), "Touches de la manette");
    EXPECT_EQ(options.optionLabel(4), "Retour");
}

/**
 * @brief À 720p, seules 4 des 5 options tiennent dans la fenêtre visible sans défilement.
 * \castest{<b>À 720p, seules 4 des 5 options tiennent sans défilement.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu À 720p, `visibleOptionCount` renvoie 4, strictement moins que `OPTION_COUNT` (5).
 * }
 */
TEST(OptionsModelTest, VisibleOptionCountA720p) {
    EXPECT_EQ(hmi::OptionsModel::visibleOptionCount(VIEWPORT_HEIGHT), 4);
    EXPECT_LT(hmi::OptionsModel::visibleOptionCount(VIEWPORT_HEIGHT), hmi::OptionsModel::OPTION_COUNT);
}

/**
 * @brief Valider la première option (V-Sync) renvoie l'action ToggleVSync, sans changer d'écran.
 * \castest{<b>Valider la première option (V-Sync) renvoie l'action ToggleVSync.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Valider la première option (V-Sync) renvoie l'action ToggleVSync.
 * }
 */
TEST(OptionsModelTest, ValiderVSyncRenvoieToggle) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    const std::optional<hmi::OptionsAction> action =
        options.update(keyPress(hmi::Key::Enter), VIEWPORT_HEIGHT);
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::ToggleVSync);
}

/**
 * @brief Quatre flèches bas déplacent la sélection jusqu'à Retour et font défiler la fenêtre pour
 *        la garder visible ; valider ensuite renvoie l'action Back.
 * \castest{<b>Quatre flèches bas puis valider (Retour) renvoie l'action Back, en ayant fait
 * défiler la fenêtre.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Quatre flèches bas puis valider (Retour) renvoie l'action Back.
 * }
 */
TEST(OptionsModelTest, FlecheBasPuisValiderRenvoieBack) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    const std::optional<hmi::OptionsAction> none =
        options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    EXPECT_FALSE(none.has_value());
    EXPECT_EQ(options.selectedIndex(), 4);
    // La selection (index 4) doit rester dans la fenetre visible (4 lignes) : le defilement a du
    // avancer d'au moins une ligne pour l'y maintenir.
    EXPECT_GT(options.scrollOffset(), 0);

    const std::optional<hmi::OptionsAction> action =
        options.update(keyPress(hmi::Key::Enter), VIEWPORT_HEIGHT);
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::Back);
}

/**
 * @brief La flèche haut depuis la première option boucle sur la dernière (Retour) et fait défiler
 *        la fenêtre pour la garder visible.
 * \castest{<b>La flèche haut depuis la première option boucle sur la dernière (Retour), fenêtre
 * défilée en conséquence.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La flèche haut depuis la première option boucle sur la dernière (Retour).
 * }
 */
TEST(OptionsModelTest, FlecheHautBoucleSurRetour) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    (void)options.update(keyPress(hmi::Key::Up), VIEWPORT_HEIGHT);
    EXPECT_EQ(options.selectedIndex(), 4);
    EXPECT_GT(options.scrollOffset(), 0);
}

/**
 * @brief Valider l'option « Touches de jeu » renvoie l'action OpenGameKeybindings.
 * \castest{<b>Valider l'option « Touches de jeu » renvoie l'action OpenGameKeybindings.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Valider l'option « Touches de jeu » renvoie l'action OpenGameKeybindings.
 * }
 */
TEST(OptionsModelTest, ValiderTouchesDeJeuRenvoieOpenGameKeybindings) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    const std::optional<hmi::OptionsAction> action =
        options.update(keyPress(hmi::Key::Enter), VIEWPORT_HEIGHT);
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::OpenGameKeybindings);
}

/**
 * @brief Valider l'option « Touches de l'éditeur » renvoie l'action OpenEditorKeybindings.
 * \castest{<b>Valider l'option « Touches de l'éditeur » renvoie l'action
 * OpenEditorKeybindings.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Valider l'option « Touches de l'éditeur » renvoie l'action OpenEditorKeybindings.
 * }
 */
TEST(OptionsModelTest, ValiderTouchesDeLEditeurRenvoieOpenEditorKeybindings) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    const std::optional<hmi::OptionsAction> action =
        options.update(keyPress(hmi::Key::Enter), VIEWPORT_HEIGHT);
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::OpenEditorKeybindings);
}

/**
 * @brief Valider l'option « Touches de la manette » renvoie l'action OpenGamepadBindings.
 * \castest{<b>Valider l'option « Touches de la manette » renvoie l'action
 * OpenGamepadBindings.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Valider l'option « Touches de la manette » renvoie l'action OpenGamepadBindings.
 * }
 */
TEST(OptionsModelTest, ValiderTouchesDeLaManetteRenvoieOpenGamepadBindings) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    const std::optional<hmi::OptionsAction> action =
        options.update(keyPress(hmi::Key::Enter), VIEWPORT_HEIGHT);
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::OpenGamepadBindings);
}

/**
 * @brief Le survol d'une option visible à la souris la sélectionne ; un clic la valide.
 * \castest{<b>Le survol d'une option visible à la souris la sélectionne ; un clic la
 * valide.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le survol d'une option visible à la souris la sélectionne ; un clic la valide.
 * }
 */
TEST(OptionsModelTest, ClicSourisValideTouchesDeJeu) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    // Rang visible 1 = index 1 ("Touches de jeu") sans defilement (scrollOffset == 0 au depart).
    const std::optional<hmi::OptionsAction> action = options.update(
        mouseClick(optionPointX(), optionPointYForVisibleRow(1)), VIEWPORT_HEIGHT);
    EXPECT_EQ(options.selectedIndex(), 1);
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::OpenGameKeybindings);
}

/**
 * @brief Après défilement (clavier) jusqu'à Retour, un clic à sa position **affichée** (rang
 *        visible, pas l'indice absolu) la valide.
 * \castest{<b>Après défilement jusqu'à Retour, un clic à sa position affichée la valide.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un clic à la position affichée de Retour (après défilement) renvoie l'action Back.
 * }
 */
TEST(OptionsModelTest, ClicApresDefilementValideRetour) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    for (int i = 0; i < 4; ++i) {
        (void)options.update(keyPress(hmi::Key::Down), VIEWPORT_HEIGHT);
    }
    ASSERT_EQ(options.selectedIndex(), 4);
    const int scrollOffset = options.scrollOffset();
    ASSERT_GT(scrollOffset, 0);

    // Rang visible de Retour (index 4) compte tenu du defilement courant.
    const int visibleRow = 4 - scrollOffset;
    const std::optional<hmi::OptionsAction> action = options.update(
        mouseClick(optionPointX(), optionPointYForVisibleRow(visibleRow)), VIEWPORT_HEIGHT);
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::Back);
}

/**
 * @brief La molette défile la fenêtre visible sans changer la sélection courante.
 * \castest{<b>La molette défile la fenêtre visible sans changer la sélection courante.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La molette change le défilement, jamais la sélection.
 * }
 */
TEST(OptionsModelTest, MoletteDefileSansChangerLaSelection) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    hmi::InputState wheelDown;
    wheelDown.beginFrame();
    wheelDown.onMouseWheel(-120);  // un cran vers l'arriere : defile vers le bas (LevelPicker).

    (void)options.update(wheelDown, VIEWPORT_HEIGHT);
    EXPECT_EQ(options.selectedIndex(), 0);  // selection inchangee
    EXPECT_EQ(options.scrollOffset(), 1);   // fenetre defilee d'une ligne
}

/**
 * @brief `setVSyncEnabled` resynchronise le libellé affiché après une bascule.
 * \castest{<b>`setVSyncEnabled` resynchronise le libellé affiché après une bascule.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `setVSyncEnabled` resynchronise le libellé affiché après une bascule.
 * }
 */
TEST(OptionsModelTest, SetVSyncEnabledChangeLeLibelle) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);
    ASSERT_EQ(options.optionLabel(0), "V-Sync : Active");

    options.setVSyncEnabled(false);
    EXPECT_EQ(options.optionLabel(0), "V-Sync : Desactive");
}

/**
 * @brief Un clic hors de toute option ne valide rien.
 * \castest{<b>Un clic hors de toute option ne valide rien.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un clic hors de toute option ne valide rien.
 * }
 */
TEST(OptionsModelTest, ClicHorsOptionNeValidePas) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    const std::optional<hmi::OptionsAction> action = options.update(mouseClick(5, 5), VIEWPORT_HEIGHT);
    EXPECT_FALSE(action.has_value());
}
