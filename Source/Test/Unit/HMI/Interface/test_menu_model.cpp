/**
 * @file test_menu_model.cpp
 * @brief Tests unitaires de la logique du menu : navigation clavier/souris et transitions.
 */

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Interface/IScreen.h"
#include "HMI/Interface/MenuModel.h"
#include "HMI/Localization/Localization.h"

namespace {

/// Construit un catalogue français minimal pour les libellés du menu.
hmi::Localization frenchCatalog() {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"menu.titre", "ProjectGaming"},
                                          {"menu.jouer", "Charger niveau"},
                                          {"menu.mode_edition", "Mode Edition"},
                                          {"menu.quitter", "Quitter"}});
    return localization;
}

/// @return Un état d'entrées où @p key vient d'être pressée (front montant).
hmi::InputState keyPress(hmi::Key key) {
    hmi::InputState input;
    input.beginFrame();
    input.onKeyDown(key);
    return input;
}

/// @return Un état d'entrées avec la souris en (@p x, @p y).
hmi::InputState mouseAt(int x, int y) {
    hmi::InputState input;
    input.beginFrame();
    input.onMouseMove(x, y);
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

/// Point situé à l'intérieur du libellé de l'option @p index.
int optionPointX() {
    return static_cast<int>(hmi::MenuModel::MARGIN_X) + 5;
}
int optionPointY(int index) {
    return static_cast<int>(hmi::MenuModel::optionTop(index)) + 5;
}

}  // namespace

/// À l'ouverture, la première option (Charger niveau) est sélectionnée.
TEST(MenuModelTest, SelectionParDefaut) {
    hmi::Localization catalog = frenchCatalog();
    hmi::MenuModel menu(catalog);

    EXPECT_EQ(menu.selectedIndex(), 0);
    EXPECT_EQ(menu.optionLabel(0), "Charger niveau");
    EXPECT_EQ(menu.title(), "ProjectGaming");
}

/// Valider par défaut (Entrée) bascule vers l'écran de jeu.
TEST(MenuModelTest, ValiderChargeNiveau) {
    hmi::Localization catalog = frenchCatalog();
    hmi::MenuModel menu(catalog);

    const hmi::ScreenTransition transition = menu.update(keyPress(hmi::Key::Enter));
    EXPECT_EQ(transition.kind, hmi::ScreenTransition::Kind::Switch);
    EXPECT_EQ(transition.target, hmi::ScreenId::Game);
}

/// La flèche bas déplace la sélection ; valider mène alors au Mode Edition.
TEST(MenuModelTest, FlecheBasPuisValider) {
    hmi::Localization catalog = frenchCatalog();
    hmi::MenuModel menu(catalog);

    const hmi::ScreenTransition none = menu.update(keyPress(hmi::Key::Down));
    EXPECT_EQ(none.kind, hmi::ScreenTransition::Kind::None);
    EXPECT_EQ(menu.selectedIndex(), 1);

    const hmi::ScreenTransition transition = menu.update(keyPress(hmi::Key::Enter));
    EXPECT_EQ(transition.kind, hmi::ScreenTransition::Kind::Switch);
    EXPECT_EQ(transition.target, hmi::ScreenId::Editor);
}

/// La flèche haut depuis la première option boucle sur la dernière (Quitter).
TEST(MenuModelTest, FlecheHautBoucleSurQuitter) {
    hmi::Localization catalog = frenchCatalog();
    hmi::MenuModel menu(catalog);

    (void)menu.update(keyPress(hmi::Key::Up));
    EXPECT_EQ(menu.selectedIndex(), 2);

    const hmi::ScreenTransition transition = menu.update(keyPress(hmi::Key::Enter));
    EXPECT_EQ(transition.kind, hmi::ScreenTransition::Kind::Quit);
}

/// Trois flèches bas ramènent à la première option (bouclage déterministe).
TEST(MenuModelTest, BouclageBasComplet) {
    hmi::Localization catalog = frenchCatalog();
    hmi::MenuModel menu(catalog);

    (void)menu.update(keyPress(hmi::Key::Down));
    EXPECT_EQ(menu.selectedIndex(), 1);
    (void)menu.update(keyPress(hmi::Key::Down));
    EXPECT_EQ(menu.selectedIndex(), 2);
    (void)menu.update(keyPress(hmi::Key::Down));
    EXPECT_EQ(menu.selectedIndex(), 0);
}

/// Le survol d'une option à la souris la sélectionne.
TEST(MenuModelTest, SurvolSourisSelectionne) {
    hmi::Localization catalog = frenchCatalog();
    hmi::MenuModel menu(catalog);

    const hmi::ScreenTransition transition = menu.update(mouseAt(optionPointX(), optionPointY(1)));
    EXPECT_EQ(transition.kind, hmi::ScreenTransition::Kind::None);
    EXPECT_EQ(menu.selectedIndex(), 1);
}

/// Un clic gauche sur une option la valide.
TEST(MenuModelTest, ClicValideOption) {
    hmi::Localization catalog = frenchCatalog();
    hmi::MenuModel menu(catalog);

    const hmi::ScreenTransition transition =
        menu.update(mouseClick(optionPointX(), optionPointY(2)));
    EXPECT_EQ(menu.selectedIndex(), 2);
    EXPECT_EQ(transition.kind, hmi::ScreenTransition::Kind::Quit);
}

/// Un clic hors de toute option ne valide rien.
TEST(MenuModelTest, ClicHorsOptionNeValidePas) {
    hmi::Localization catalog = frenchCatalog();
    hmi::MenuModel menu(catalog);

    const hmi::ScreenTransition transition = menu.update(mouseClick(5, 5));
    EXPECT_EQ(transition.kind, hmi::ScreenTransition::Kind::None);
}
