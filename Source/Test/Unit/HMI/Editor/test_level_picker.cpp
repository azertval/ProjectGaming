/**
 * @file test_level_picker.cpp
 * @brief Tests unitaires de la sélection de niveau à éditer (LOT-14 TACHE-06, EX-EDIT-001).
 */

#include <gtest/gtest.h>

#include "HMI/Editor/LevelPicker.h"
#include "HMI/Input/InputState.h"

namespace {

[[nodiscard]] std::vector<hmi::LevelPicker::Choice> threeChoices() {
    return {
        hmi::LevelPicker::Choice{"Nouveau niveau", std::nullopt},
        hmi::LevelPicker::Choice{"demo", std::filesystem::path("demo.json")},
        hmi::LevelPicker::Choice{"demo2", std::filesystem::path("demo2.json")},
    };
}

// Point situe a l'interieur du rectangle du choix index (mise en page a chasse fixe).
int choicePointX() {
    return static_cast<int>(hmi::LevelPicker::MARGIN_X) + 5;
}
int choicePointY(int index) {
    return static_cast<int>(hmi::LevelPicker::OPTIONS_TOP) +
          index * static_cast<int>(hmi::LevelPicker::OPTION_SPACING) + 5;
}

}  // namespace

/**
 * @brief La sélection initiale pointe le premier choix (« Nouveau niveau »).
 * \castest{<b>La sélection initiale pointe le premier choix (« Nouveau niveau »).</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La sélection initiale pointe le premier choix (« Nouveau niveau »).
 * }
 */
TEST(LevelPickerTest, SelectionInitialeEstLePremierChoix) {
    const hmi::LevelPicker picker(threeChoices());
    EXPECT_EQ(picker.selected(), 0);
    EXPECT_FALSE(picker.choices()[0].path.has_value());
}

/**
 * @brief Bas puis Haut déplacent la sélection, avec bouclage aux extrémités.
 * \castest{<b>Bas puis Haut déplacent la sélection, avec bouclage aux extrémités.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Bas puis Haut déplacent la sélection, avec bouclage aux extrémités.
 * }
 */
TEST(LevelPickerTest, FlechesDeplacentLaSelectionAvecBouclage) {
    hmi::LevelPicker picker(threeChoices());
    hmi::InputState input;

    // Bas : 0 -> 1 -> 2 -> 0 (bouclage). Chaque pression est isolee par une frame de
    // relachement, pour produire un veritable front montant a chaque fois.
    for (const int expected : {1, 2, 0}) {
        input.beginFrame();
        input.onKeyDown(hmi::Key::Down);
        static_cast<void>(picker.update(input));
        EXPECT_EQ(picker.selected(), expected);

        input.beginFrame();
        input.onKeyUp(hmi::Key::Down);
        static_cast<void>(picker.update(input));
    }

    // Haut, depuis 0 : boucle vers le dernier choix (2).
    input.beginFrame();
    input.onKeyDown(hmi::Key::Up);
    static_cast<void>(picker.update(input));
    EXPECT_EQ(picker.selected(), 2);
}

/**
 * @brief Entrée confirme l'indice actuellement sélectionné.
 * \castest{<b>Entrée confirme l'indice actuellement sélectionné.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Entrée confirme l'indice actuellement sélectionné.
 * }
 */
TEST(LevelPickerTest, EntreeConfirmeLaSelection) {
    hmi::LevelPicker picker(threeChoices());
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Down);
    static_cast<void>(picker.update(input));
    ASSERT_EQ(picker.selected(), 1);

    input.beginFrame();
    input.onKeyUp(hmi::Key::Down);
    input.onKeyDown(hmi::Key::Enter);
    const std::optional<int> confirmed = picker.update(input);

    ASSERT_TRUE(confirmed.has_value());
    EXPECT_EQ(*confirmed, 1);
    EXPECT_EQ(picker.choices()[*confirmed].path, std::filesystem::path("demo.json"));
}

/**
 * @brief Sans appui, update() ne confirme rien.
 * \castest{<b>Sans appui, update() ne confirme rien.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans appui, update() ne confirme rien.
 * }
 */
TEST(LevelPickerTest, SansAppuiAucuneConfirmation) {
    hmi::LevelPicker picker(threeChoices());
    hmi::InputState input;
    input.beginFrame();

    EXPECT_FALSE(picker.update(input).has_value());
}

/**
 * @brief Survoler un choix à la souris déplace la sélection dessus (sans confirmer).
 * \castest{<b>Survoler un choix à la souris déplace la sélection dessus (sans confirmer).</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Survoler un choix à la souris déplace la sélection dessus (sans confirmer).
 * }
 */
TEST(LevelPickerTest, SurvolSourisDeplaceLaSelection) {
    hmi::LevelPicker picker(threeChoices());
    hmi::InputState input;
    input.beginFrame();
    input.onMouseMove(choicePointX(), choicePointY(2));

    const std::optional<int> confirmed = picker.update(input);

    EXPECT_EQ(picker.selected(), 2);
    EXPECT_FALSE(confirmed.has_value());
}

/**
 * @brief Un clic gauche sur un choix survolé le confirme.
 * \castest{<b>Un clic gauche sur un choix survolé le confirme.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un clic gauche sur un choix survolé le confirme.
 * }
 */
TEST(LevelPickerTest, ClicGaucheConfirmeLeChoixSurvole) {
    hmi::LevelPicker picker(threeChoices());
    hmi::InputState input;
    input.beginFrame();
    input.onMouseMove(choicePointX(), choicePointY(1));
    input.onMouseButtonDown(hmi::MouseButton::Left);

    const std::optional<int> confirmed = picker.update(input);

    ASSERT_TRUE(confirmed.has_value());
    EXPECT_EQ(*confirmed, 1);
    EXPECT_EQ(picker.choices()[*confirmed].path, std::filesystem::path("demo.json"));
}

/**
 * @brief La souris hors de tout choix ne change ni la sélection ni la confirmation.
 * \castest{<b>La souris hors de tout choix ne change ni la sélection ni la confirmation.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La souris hors de tout choix ne change ni la sélection ni la confirmation.
 * }
 */
TEST(LevelPickerTest, SourisHorsChoixNeChangeRien) {
    hmi::LevelPicker picker(threeChoices());
    hmi::InputState input;
    input.beginFrame();
    input.onMouseMove(-100, -100);
    input.onMouseButtonDown(hmi::MouseButton::Left);

    const std::optional<int> confirmed = picker.update(input);

    EXPECT_EQ(picker.selected(), 0);
    EXPECT_FALSE(confirmed.has_value());
}

/**
 * @brief forDirectory sur un dossier inexistant ne propose que « Nouveau niveau ».
 * \castest{<b>forDirectory sur un dossier inexistant ne propose que « Nouveau niveau ».</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu forDirectory sur un dossier inexistant ne propose que « Nouveau niveau ».
 * }
 */
TEST(LevelPickerTest, ForDirectoryDossierInexistantNeProposeQueNouveauNiveau) {
    const hmi::LevelPicker picker =
        hmi::LevelPicker::forDirectory("chemin/totalement/inexistant/pas_la");
    ASSERT_EQ(picker.choices().size(), 1u);
    EXPECT_FALSE(picker.choices()[0].path.has_value());
}
