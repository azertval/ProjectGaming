/**
 * @file test_options_model.cpp
 * @brief Tests unitaires de la logique du menu d'options : navigation et actions.
 */

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Interface/MenuModel.h"
#include "HMI/Interface/OptionsModel.h"
#include "HMI/Localization/Localization.h"

namespace {

/// Construit un catalogue français minimal pour les libellés des options.
hmi::Localization frenchCatalog() {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"options.vsync_on", "V-Sync : Active"},
                                          {"options.vsync_off", "V-Sync : Desactive"},
                                          {"options.retour", "Retour"}});
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

/// Point situé à l'intérieur du libellé de l'option @p index (même mise en page que MenuModel).
int optionPointX() {
    return static_cast<int>(hmi::MenuModel::MARGIN_X) + 5;
}
int optionPointY(int index) {
    return static_cast<int>(hmi::MenuModel::optionTop(index)) + 5;
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
    EXPECT_EQ(options.optionLabel(0), "V-Sync : Active");
    EXPECT_EQ(options.optionLabel(1), "Retour");
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

    const std::optional<hmi::OptionsAction> action = options.update(keyPress(hmi::Key::Enter));
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::ToggleVSync);
}

/**
 * @brief Flèche bas puis valider (Retour) renvoie l'action Back.
 * \castest{<b>Flèche bas puis valider (Retour) renvoie l'action Back.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Flèche bas puis valider (Retour) renvoie l'action Back.
 * }
 */
TEST(OptionsModelTest, FlecheBasPuisValiderRenvoieBack) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    const std::optional<hmi::OptionsAction> none = options.update(keyPress(hmi::Key::Down));
    EXPECT_FALSE(none.has_value());
    EXPECT_EQ(options.selectedIndex(), 1);

    const std::optional<hmi::OptionsAction> action = options.update(keyPress(hmi::Key::Enter));
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::Back);
}

/**
 * @brief La flèche haut depuis la première option boucle sur la dernière (Retour).
 * \castest{<b>La flèche haut depuis la première option boucle sur la dernière (Retour).</b><br/>
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

    (void)options.update(keyPress(hmi::Key::Up));
    EXPECT_EQ(options.selectedIndex(), 1);
}

/**
 * @brief Le survol d'une option à la souris la sélectionne ; un clic la valide.
 * \castest{<b>Le survol d'une option à la souris la sélectionne ; un clic la valide.</b><br/>
 * \tcat Unitaire · Options Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le survol d'une option à la souris la sélectionne ; un clic la valide.
 * }
 */
TEST(OptionsModelTest, ClicSourisValideRetour) {
    hmi::Localization catalog = frenchCatalog();
    hmi::OptionsModel options(catalog, /*vsyncEnabled=*/true);

    const std::optional<hmi::OptionsAction> action =
        options.update(mouseClick(optionPointX(), optionPointY(1)));
    EXPECT_EQ(options.selectedIndex(), 1);
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::OptionsAction::Back);
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

    const std::optional<hmi::OptionsAction> action = options.update(mouseClick(5, 5));
    EXPECT_FALSE(action.has_value());
}
