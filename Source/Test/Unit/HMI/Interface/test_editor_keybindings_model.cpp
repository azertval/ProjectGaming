/**
 * @file test_editor_keybindings_model.cpp
 * @brief Tests unitaires de la logique du sous-menu « Touches de l'éditeur » (`EX-CTRL-012`,
 *        `LOT-29`).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Interface/EditorKeybindingsModel.h"
#include "HMI/Localization/Localization.h"

namespace {

/// Construit un catalogue français minimal pour les libellés du sous-menu.
hmi::Localization frenchCatalog() {
    hmi::Localization localization;
    localization.setDefaultCatalog(
        "fr", {{"keybindings.action.sauvegarder", "Sauvegarder"},
              {"keybindings.action.annuler", "Annuler"},
              {"keybindings.action.refaire", "Refaire"},
              {"keybindings.action.copier", "Copier"},
              {"keybindings.action.coller", "Coller"},
              {"keybindings.action.test_rapide", "Test rapide"},
              {"keybindings.action.grille", "Grille"},
              {"keybindings.action.aide", "Aide"},
              {"keybindings.action.renommer", "Renommer"},
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
///         est testée par EditorKeyBindingsTest, pas ici).
std::filesystem::path scratchPath() {
    return std::filesystem::temp_directory_path() /
          "projectgaming_test_editor_keybindings_ui.json";
}

}  // namespace

/**
 * @brief À l'ouverture, la première ligne (Sauvegarder) est sélectionnée, hors capture.
 * \castest{<b>À l'ouverture, la première ligne (Sauvegarder) est sélectionnée, hors
 * capture.</b><br/>
 * \tcat Unitaire · Editor Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu À l'ouverture, la première ligne est sélectionnée, hors capture.
 * }
 */
TEST(EditorKeybindingsModelTest, SelectionParDefaut) {
    hmi::Localization catalog = frenchCatalog();
    hmi::EditorKeyBindings bindings;
    hmi::EditorKeybindingsModel model(catalog, bindings, scratchPath());

    EXPECT_EQ(model.selectedIndex(), 0);
    EXPECT_FALSE(model.isCapturing());
    EXPECT_EQ(model.rowLabel(0), "Sauvegarder");
    EXPECT_EQ(model.rowLabel(hmi::EditorKeybindingsModel::RESET_ROW), "Reinitialiser");
    EXPECT_EQ(model.rowLabel(hmi::EditorKeybindingsModel::BACK_ROW), "Retour");
}

/**
 * @brief Une touche pressée pendant la capture lie l'action, sauvegarde, et renvoie Rebound.
 * \castest{<b>Une touche pressée pendant la capture lie l'action, sauvegarde, et renvoie
 * Rebound.</b><br/>
 * \tcat Unitaire · Editor Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une touche pressée pendant la capture lie l'action et renvoie Rebound.
 * }
 */
TEST(EditorKeybindingsModelTest, ToucheCaptureeLieLActionEtRenvoieRebound) {
    hmi::Localization catalog = frenchCatalog();
    hmi::EditorKeyBindings bindings;
    const std::filesystem::path path = scratchPath();
    std::filesystem::remove(path);
    hmi::EditorKeybindingsModel model(catalog, bindings, path);

    (void)model.update(keyPress(hmi::Key::Enter));  // entre en capture (ligne 0 : Sauvegarder)
    const std::optional<hmi::EditorKeybindingsAction> action =
        model.update(keyPress(hmi::Key::A));

    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::EditorKeybindingsAction::Rebound);
    EXPECT_FALSE(model.isCapturing());
    EXPECT_EQ(bindings.key(hmi::EditorAction::Save), hmi::Key::A);

    std::filesystem::remove(path);
}

/**
 * @brief `Échap` pendant la capture l'annule sans modifier le binding.
 * \castest{<b>`Échap` pendant la capture l'annule sans modifier le binding.</b><br/>
 * \tcat Unitaire · Editor Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `Échap` pendant la capture l'annule sans modifier le binding.
 * }
 */
TEST(EditorKeybindingsModelTest, EchapPendantCaptureAnnule) {
    hmi::Localization catalog = frenchCatalog();
    hmi::EditorKeyBindings bindings;
    hmi::EditorKeybindingsModel model(catalog, bindings, scratchPath());

    (void)model.update(keyPress(hmi::Key::Enter));
    ASSERT_TRUE(model.isCapturing());
    const std::optional<hmi::EditorKeybindingsAction> action =
        model.update(keyPress(hmi::Key::Escape));

    EXPECT_FALSE(action.has_value());
    EXPECT_FALSE(model.isCapturing());
    EXPECT_EQ(bindings.key(hmi::EditorAction::Save), hmi::Key::S);  // inchange
}

/**
 * @brief Confirmer « Réinitialiser » restaure les défauts et renvoie Reset.
 * \castest{<b>Confirmer « Réinitialiser » restaure les défauts et renvoie Reset.</b><br/>
 * \tcat Unitaire · Editor Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Confirmer « Réinitialiser » restaure les défauts et renvoie Reset.
 * }
 */
TEST(EditorKeybindingsModelTest, ConfirmerReinitialiserRestaureLesDefauts) {
    hmi::Localization catalog = frenchCatalog();
    hmi::EditorKeyBindings bindings;
    const std::filesystem::path path = scratchPath();
    std::filesystem::remove(path);
    hmi::EditorKeybindingsModel model(catalog, bindings, path);

    (void)model.update(keyPress(hmi::Key::Enter));  // capture ligne 0
    (void)model.update(keyPress(hmi::Key::A));       // Sauvegarder -> A
    for (int i = 0; i < hmi::EditorKeybindingsModel::RESET_ROW; ++i) {
        (void)model.update(keyPress(hmi::Key::Down));
    }
    const std::optional<hmi::EditorKeybindingsAction> action =
        model.update(keyPress(hmi::Key::Enter));

    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::EditorKeybindingsAction::Reset);
    EXPECT_EQ(bindings.key(hmi::EditorAction::Save), hmi::Key::S);  // restaure

    std::filesystem::remove(path);
}

/**
 * @brief Confirmer « Retour » renvoie l'action Back.
 * \castest{<b>Confirmer « Retour » renvoie l'action Back.</b><br/>
 * \tcat Unitaire · Editor Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Confirmer « Retour » renvoie l'action Back.
 * }
 */
TEST(EditorKeybindingsModelTest, ConfirmerRetourRenvoieBack) {
    hmi::Localization catalog = frenchCatalog();
    hmi::EditorKeyBindings bindings;
    hmi::EditorKeybindingsModel model(catalog, bindings, scratchPath());

    for (int i = 0; i < hmi::EditorKeybindingsModel::BACK_ROW; ++i) {
        (void)model.update(keyPress(hmi::Key::Down));
    }
    const std::optional<hmi::EditorKeybindingsAction> action =
        model.update(keyPress(hmi::Key::Enter));

    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(*action, hmi::EditorKeybindingsAction::Back);
}

/**
 * @brief Remapper une action sur une touche déjà liée à une autre échange les deux (conflit).
 * \castest{<b>Remapper une action sur une touche déjà liée à une autre échange les deux
 * (conflit).</b><br/>
 * \tcat Unitaire · Editor Keybindings Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Remapper sur une touche déjà liée échange les deux touches.
 * }
 */
TEST(EditorKeybindingsModelTest, RemapperSurConflitEchange) {
    hmi::Localization catalog = frenchCatalog();
    hmi::EditorKeyBindings bindings;
    const std::filesystem::path path = scratchPath();
    std::filesystem::remove(path);
    hmi::EditorKeybindingsModel model(catalog, bindings, path);

    // Ligne 0 = Sauvegarder (defaut S) ; la lier a Z (defaut d'Annuler) doit echanger les deux.
    (void)model.update(keyPress(hmi::Key::Enter));
    (void)model.update(keyPress(hmi::Key::Z));

    EXPECT_EQ(bindings.key(hmi::EditorAction::Save), hmi::Key::Z);
    EXPECT_EQ(bindings.key(hmi::EditorAction::Undo), hmi::Key::S);

    std::filesystem::remove(path);
}
