/**
 * @file test_game_keybindings_model.cpp
 * @brief Tests unitaires de la logique du sous-menu « Touches de jeu » (`EX-CTRL-012`, `LOT-29`).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Interface/GameKeybindingsModel.h"
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
              {"keybindings.appuyez_touche", "Appuyez sur une touche..."},
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

/// @return Un chemin de fichier temporaire dédié à un test (jamais lu : la persistance elle-même
///         est testée par GameKeyBindingsTest, pas ici).
std::filesystem::path scratchPath() {
    return std::filesystem::temp_directory_path() / "projectgaming_test_game_keybindings_ui.json";
}

}  // namespace

/**
 * @brief À l'ouverture, la première ligne (Aller à gauche) est sélectionnée, hors capture.
 * \castest{<b>À l'ouverture, la première ligne (Aller à gauche) est sélectionnée, hors
 * capture.</b><br/>
 * \tcat Unitaire · Game Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu À l'ouverture, la première ligne est sélectionnée, hors capture.
 * }
 */
TEST(GameKeybindingsModelTest, SelectionParDefaut) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GameKeyBindings bindings;
    hmi::GameKeybindingsModel model(catalog, bindings, scratchPath());

    EXPECT_EQ(model.selectedIndex(), 0);
    EXPECT_FALSE(model.isCapturing());
    EXPECT_EQ(model.rowLabel(0), "Aller a gauche");
    EXPECT_EQ(model.rowLabel(hmi::GameKeybindingsModel::RESET_ROW), "Reinitialiser");
    EXPECT_EQ(model.rowLabel(hmi::GameKeybindingsModel::BACK_ROW), "Retour");
}

/**
 * @brief Confirmer une ligne d'action entre en capture, sans modifier le binding immédiatement.
 * \castest{<b>Confirmer une ligne d'action entre en capture, sans modifier le binding
 * immédiatement.</b><br/>
 * \tcat Unitaire · Game Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Confirmer une ligne d'action entre en capture, sans modifier le binding.
 * }
 */
TEST(GameKeybindingsModelTest, ConfirmerUneActionEntreEnCapture) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GameKeyBindings bindings;
    hmi::GameKeybindingsModel model(catalog, bindings, scratchPath());

    const std::optional<hmi::GameKeybindingsAction> action =
        model.update(keyPress(hmi::Key::Enter));
    EXPECT_FALSE(action.has_value());
    EXPECT_TRUE(model.isCapturing());
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::Left);  // inchange
}

/**
 * @brief Une touche pressée pendant la capture lie l'action, sauvegarde, et renvoie Rebound.
 * \castest{<b>Une touche pressée pendant la capture lie l'action, sauvegarde, et renvoie
 * Rebound.</b><br/>
 * \tcat Unitaire · Game Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une touche pressée pendant la capture lie l'action et renvoie Rebound.
 * }
 */
TEST(GameKeybindingsModelTest, ToucheCaptureeLieLActionEtRenvoieRebound) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GameKeyBindings bindings;
    const std::filesystem::path path = scratchPath();
    std::filesystem::remove(path);
    hmi::GameKeybindingsModel model(catalog, bindings, path);

    (void)model.update(keyPress(hmi::Key::Enter));  // entre en capture (ligne 0 : Aller a gauche)
    const std::optional<hmi::GameKeybindingsAction> action =
        model.update(keyPress(hmi::Key::F1));

    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::GameKeybindingsAction::Rebound);
    EXPECT_FALSE(model.isCapturing());
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::F1);

    std::filesystem::remove(path);
}

/**
 * @brief `Échap` pendant la capture l'annule sans modifier le binding.
 * \castest{<b>`Échap` pendant la capture l'annule sans modifier le binding.</b><br/>
 * \tcat Unitaire · Game Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `Échap` pendant la capture l'annule sans modifier le binding.
 * }
 */
TEST(GameKeybindingsModelTest, EchapPendantCaptureAnnule) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GameKeyBindings bindings;
    hmi::GameKeybindingsModel model(catalog, bindings, scratchPath());

    (void)model.update(keyPress(hmi::Key::Enter));
    ASSERT_TRUE(model.isCapturing());
    const std::optional<hmi::GameKeybindingsAction> action =
        model.update(keyPress(hmi::Key::Escape));

    EXPECT_FALSE(action.has_value());
    EXPECT_FALSE(model.isCapturing());
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::Left);  // inchange
}

/**
 * @brief Confirmer « Réinitialiser » restaure les défauts et renvoie Reset.
 * \castest{<b>Confirmer « Réinitialiser » restaure les défauts et renvoie Reset.</b><br/>
 * \tcat Unitaire · Game Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Confirmer « Réinitialiser » restaure les défauts et renvoie Reset.
 * }
 */
TEST(GameKeybindingsModelTest, ConfirmerReinitialiserRestaureLesDefauts) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GameKeyBindings bindings;
    const std::filesystem::path path = scratchPath();
    std::filesystem::remove(path);
    hmi::GameKeybindingsModel model(catalog, bindings, path);

    (void)model.update(keyPress(hmi::Key::Enter));           // capture ligne 0
    (void)model.update(keyPress(hmi::Key::F1));              // Aller a gauche -> F1
    for (int i = 0; i < hmi::GameKeybindingsModel::RESET_ROW; ++i) {
        (void)model.update(keyPress(hmi::Key::Down));
    }
    const std::optional<hmi::GameKeybindingsAction> action =
        model.update(keyPress(hmi::Key::Enter));

    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::GameKeybindingsAction::Reset);
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::Left);  // restaure

    std::filesystem::remove(path);
}

/**
 * @brief Confirmer « Retour » renvoie l'action Back.
 * \castest{<b>Confirmer « Retour » renvoie l'action Back.</b><br/>
 * \tcat Unitaire · Game Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Confirmer « Retour » renvoie l'action Back.
 * }
 */
TEST(GameKeybindingsModelTest, ConfirmerRetourRenvoieBack) {
    hmi::Localization catalog = frenchCatalog();
    hmi::GameKeyBindings bindings;
    hmi::GameKeybindingsModel model(catalog, bindings, scratchPath());

    for (int i = 0; i < hmi::GameKeybindingsModel::BACK_ROW; ++i) {
        (void)model.update(keyPress(hmi::Key::Down));
    }
    const std::optional<hmi::GameKeybindingsAction> action =
        model.update(keyPress(hmi::Key::Enter));

    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::GameKeybindingsAction::Back);
}
