/**
 * @file test_gamepad_bindings_model.cpp
 * @brief Tests unitaires de la logique du sous-menu « Touches de la manette » (`EX-CTRL-002`,
 *        `EX-CTRL-012`, `LOT-30`).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Interface/GamepadBindingsModel.h"
#include "HMI/Localization/Localization.h"

namespace {

/// Construit un catalogue français minimal pour les libellés du sous-menu.
hmi::Localization frenchCatalog() {
    hmi::Localization localization;
    localization.setDefaultCatalog(
        "fr", {{"keybindings.action.gauche", "Aller a gauche"},
              {"keybindings.action.droite", "Aller a droite"},
              {"keybindings.action.haut", "Viser haut"},
              {"keybindings.action.bas", "Viser bas"},
              {"keybindings.action.sauter", "Sauter"},
              {"keybindings.action.dash", "Dash"},
              {"keybindings.reinitialiser", "Reinitialiser"},
              {"keybindings.appuyez_bouton", "Appuyez sur un bouton..."},
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

/// @return Un état d'entrées où @p button (piste manette brute) vient d'être pressé, manette
///         déclarée connectée.
hmi::InputState gamepadButtonPress(hmi::GamepadButton button) {
    hmi::InputState input;
    input.setGamepadConnected(true);
    input.beginFrame();
    input.onGamepadButtonDown(button);
    return input;
}

/// @return Un chemin de fichier temporaire dédié à un test (jamais lu : la persistance elle-même
///         est testée par GamepadBindingsTest, pas ici).
std::filesystem::path scratchPath() {
    return std::filesystem::temp_directory_path() /
          "projectgaming_test_gamepad_bindings_ui.json";
}

}  // namespace

/**
 * @brief À l'ouverture, la première ligne (Aller à gauche) est sélectionnée, hors capture.
 * \castest{<b>À l'ouverture, la première ligne est sélectionnée, hors capture.</b><br/>
 * \tcat Unitaire · Gamepad Bindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu À l'ouverture, la première ligne est sélectionnée, hors capture.
 * }
 */
TEST(GamepadBindingsModelTest, SelectionParDefaut) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GamepadBindings bindings;
    hmi::GamepadBindingsModel model(catalog, bindings, scratchPath());

    EXPECT_EQ(model.selectedIndex(), 0);
    EXPECT_FALSE(model.isCapturing());
    EXPECT_EQ(model.rowLabel(0), "Aller a gauche");
    EXPECT_EQ(model.rowLabel(hmi::GamepadBindingsModel::RESET_ROW), "Reinitialiser");
    EXPECT_EQ(model.rowLabel(hmi::GamepadBindingsModel::BACK_ROW), "Retour");
}

/**
 * @brief Sans manette connectée, confirmer une ligne d'action n'entre pas en capture.
 * \castest{<b>Sans manette connectée, confirmer une ligne d'action n'entre pas en
 * capture.</b><br/>
 * \tcat Unitaire · Gamepad Bindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans manette connectée, confirmer une ligne d'action n'entre pas en capture.
 * }
 */
TEST(GamepadBindingsModelTest, SansManetteConnecteeNEntrePasEnCapture) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GamepadBindings bindings;
    hmi::GamepadBindingsModel model(catalog, bindings, scratchPath());

    const std::optional<hmi::GamepadBindingsAction> action =
        model.update(keyPress(hmi::Key::Enter));  // pas de manette connectee (InputState par defaut)
    EXPECT_FALSE(action.has_value());
    EXPECT_FALSE(model.isCapturing());
}

/**
 * @brief Manette connectée, confirmer une ligne d'action entre en capture.
 * \castest{<b>Manette connectée, confirmer une ligne d'action entre en capture.</b><br/>
 * \tcat Unitaire · Gamepad Bindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Manette connectée, confirmer une ligne d'action entre en capture.
 * }
 */
TEST(GamepadBindingsModelTest, ManetteConnecteeConfirmerEntreEnCapture) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GamepadBindings bindings;
    hmi::GamepadBindingsModel model(catalog, bindings, scratchPath());

    hmi::InputState connectedEnter;
    connectedEnter.setGamepadConnected(true);
    connectedEnter.beginFrame();
    connectedEnter.onKeyDown(hmi::Key::Enter);

    const std::optional<hmi::GamepadBindingsAction> action = model.update(connectedEnter);
    EXPECT_FALSE(action.has_value());
    EXPECT_TRUE(model.isCapturing());
}

/**
 * @brief Un bouton manette pressé pendant la capture lie l'action, sauvegarde, et renvoie
 *        Rebound.
 * \castest{<b>Un bouton pressé pendant la capture lie l'action et renvoie Rebound.</b><br/>
 * \tcat Unitaire · Gamepad Bindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bouton pressé pendant la capture lie l'action et renvoie Rebound.
 * }
 */
TEST(GamepadBindingsModelTest, BoutonCaptureLieLActionEtRenvoieRebound) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GamepadBindings bindings;
    const std::filesystem::path path = scratchPath();
    std::filesystem::remove(path);
    hmi::GamepadBindingsModel model(catalog, bindings, path);

    hmi::InputState connectedEnter;
    connectedEnter.setGamepadConnected(true);
    connectedEnter.beginFrame();
    connectedEnter.onKeyDown(hmi::Key::Enter);
    (void)model.update(connectedEnter);  // entre en capture (ligne 0 : Aller a gauche)

    const std::optional<hmi::GamepadBindingsAction> action =
        model.update(gamepadButtonPress(hmi::GamepadButton::X));

    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::GamepadBindingsAction::Rebound);
    EXPECT_FALSE(model.isCapturing());
    EXPECT_EQ(bindings.button(hmi::GameAction::MoveLeft), hmi::GamepadButton::X);

    std::filesystem::remove(path);
}

/**
 * @brief `Échap` pendant la capture l'annule sans modifier le binding.
 * \castest{<b>`Échap` pendant la capture l'annule sans modifier le binding.</b><br/>
 * \tcat Unitaire · Gamepad Bindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `Échap` pendant la capture l'annule sans modifier le binding.
 * }
 */
TEST(GamepadBindingsModelTest, EchapPendantCaptureAnnule) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GamepadBindings bindings;
    hmi::GamepadBindingsModel model(catalog, bindings, scratchPath());

    hmi::InputState connectedEnter;
    connectedEnter.setGamepadConnected(true);
    connectedEnter.beginFrame();
    connectedEnter.onKeyDown(hmi::Key::Enter);
    (void)model.update(connectedEnter);
    ASSERT_TRUE(model.isCapturing());

    const std::optional<hmi::GamepadBindingsAction> action =
        model.update(keyPress(hmi::Key::Escape));

    EXPECT_FALSE(action.has_value());
    EXPECT_FALSE(model.isCapturing());
    EXPECT_EQ(bindings.button(hmi::GameAction::MoveLeft), hmi::GamepadButton::Left);  // inchange
}

/**
 * @brief Confirmer « Réinitialiser » restaure les défauts et renvoie Reset.
 * \castest{<b>Confirmer « Réinitialiser » restaure les défauts et renvoie Reset.</b><br/>
 * \tcat Unitaire · Gamepad Bindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Confirmer « Réinitialiser » restaure les défauts et renvoie Reset.
 * }
 */
TEST(GamepadBindingsModelTest, ConfirmerReinitialiserRestaureLesDefauts) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GamepadBindings bindings;
    const std::filesystem::path path = scratchPath();
    std::filesystem::remove(path);
    hmi::GamepadBindingsModel model(catalog, bindings, path);

    bindings.setKey(hmi::GameAction::MoveLeft, hmi::GamepadButton::X);
    for (int i = 0; i < hmi::GamepadBindingsModel::RESET_ROW; ++i) {
        (void)model.update(keyPress(hmi::Key::Down));
    }
    const std::optional<hmi::GamepadBindingsAction> action =
        model.update(keyPress(hmi::Key::Enter));

    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::GamepadBindingsAction::Reset);
    EXPECT_EQ(bindings.button(hmi::GameAction::MoveLeft), hmi::GamepadButton::Left);

    std::filesystem::remove(path);
}

/**
 * @brief Confirmer « Retour » renvoie l'action Back.
 * \castest{<b>Confirmer « Retour » renvoie l'action Back.</b><br/>
 * \tcat Unitaire · Gamepad Bindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Confirmer « Retour » renvoie l'action Back.
 * }
 */
TEST(GamepadBindingsModelTest, ConfirmerRetourRenvoieBack) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GamepadBindings bindings;
    hmi::GamepadBindingsModel model(catalog, bindings, scratchPath());

    for (int i = 0; i < hmi::GamepadBindingsModel::BACK_ROW; ++i) {
        (void)model.update(keyPress(hmi::Key::Down));
    }
    const std::optional<hmi::GamepadBindingsAction> action =
        model.update(keyPress(hmi::Key::Enter));

    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::GamepadBindingsAction::Back);
}
