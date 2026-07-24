/**
 * @file test_level_picker.cpp
 * @brief Tests unitaires de la sélection de niveau à éditer (LOT-14 TACHE-06, EX-EDIT-001).
 */

#include <gtest/gtest.h>

#include "HMI/Editor/LevelPicker.h"
#include "HMI/Input/InputState.h"

namespace {

// Hauteur de viewport par defaut (fenetre initiale, EX-EDIT-013) : large marge sous OPTIONS_TOP
// pour que les listes courtes de ces tests (3 choix) ne defilent jamais, sauf tests dedies.
constexpr float VIEWPORT_HEIGHT = 720.0f;

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

// N choix numerotes, pour forcer un defilement (EX-EDIT-001) sans depende du contenu reel.
[[nodiscard]] std::vector<hmi::LevelPicker::Choice> manyChoices(int count) {
    std::vector<hmi::LevelPicker::Choice> choices;
    choices.reserve(static_cast<std::size_t>(count));
    for (int index = 0; index < count; ++index) {
        choices.push_back(
            hmi::LevelPicker::Choice{"niveau" + std::to_string(index), std::nullopt});
    }
    return choices;
}

// Hauteur de viewport choisie pour que visibleCount() vale exactement 3 (voir la formule dans
// LevelPicker.cpp : floor((h - OPTIONS_TOP) / OPTION_SPACING)).
constexpr float SMALL_VIEWPORT_HEIGHT = 500.0f;

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
        static_cast<void>(picker.update(input, VIEWPORT_HEIGHT));
        EXPECT_EQ(picker.selected(), expected);

        input.beginFrame();
        input.onKeyUp(hmi::Key::Down);
        static_cast<void>(picker.update(input, VIEWPORT_HEIGHT));
    }

    // Haut, depuis 0 : boucle vers le dernier choix (2).
    input.beginFrame();
    input.onKeyDown(hmi::Key::Up);
    static_cast<void>(picker.update(input, VIEWPORT_HEIGHT));
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
    static_cast<void>(picker.update(input, VIEWPORT_HEIGHT));
    ASSERT_EQ(picker.selected(), 1);

    input.beginFrame();
    input.onKeyUp(hmi::Key::Down);
    input.onKeyDown(hmi::Key::Enter);
    const std::optional<int> confirmed = picker.update(input, VIEWPORT_HEIGHT);

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

    EXPECT_FALSE(picker.update(input, VIEWPORT_HEIGHT).has_value());
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

    const std::optional<int> confirmed = picker.update(input, VIEWPORT_HEIGHT);

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

    const std::optional<int> confirmed = picker.update(input, VIEWPORT_HEIGHT);

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

    const std::optional<int> confirmed = picker.update(input, VIEWPORT_HEIGHT);

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

/**
 * @brief visibleCount() renvoie au moins 1, même pour une hauteur de viewport minuscule.
 * \castest{<b>visibleCount() renvoie au moins 1, même pour une hauteur de viewport minuscule.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu visibleCount() renvoie au moins 1, même pour une hauteur de viewport minuscule.
 * }
 */
TEST(LevelPickerTest, VisibleCountAuMoinsUn) {
    EXPECT_EQ(hmi::LevelPicker::visibleCount(0.0f), 1);
    EXPECT_EQ(hmi::LevelPicker::visibleCount(SMALL_VIEWPORT_HEIGHT), 3);
}

/**
 * @brief Une liste plus longue que la fenêtre visible défile pour suivre la sélection vers le bas
 * (`EX-EDIT-001`) : les choix au-delà de la fenêtre visible restent atteignables.
 * \castest{<b>Une liste plus longue que la fenêtre visible défile pour suivre la sélection vers le
 * bas.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une liste plus longue que la fenêtre visible défile pour suivre la sélection vers le
 * bas.
 * }
 */
TEST(LevelPickerTest, DefilementSuitLaSelectionVersLeBas) {
    hmi::LevelPicker picker(manyChoices(6));
    hmi::InputState input;
    EXPECT_EQ(picker.scrollOffset(), 0);

    // 0 -> 1 -> 2 -> 3 -> 4 : la fenetre (3 lignes) doit suivre des que la selection depasse la
    // derniere ligne visible (indice 2, tant que scrollOffset == 0).
    for (int step = 0; step < 4; ++step) {
        input.beginFrame();
        input.onKeyDown(hmi::Key::Down);
        static_cast<void>(picker.update(input, SMALL_VIEWPORT_HEIGHT));
        input.beginFrame();
        input.onKeyUp(hmi::Key::Down);
        static_cast<void>(picker.update(input, SMALL_VIEWPORT_HEIGHT));
    }

    ASSERT_EQ(picker.selected(), 4);
    EXPECT_EQ(picker.scrollOffset(), 2);  // fenetre [2, 5) : la selection (4) y reste visible
}

/**
 * @brief Depuis une position défilée, remonter la sélection jusqu'en haut ramène le défilement à
 * zéro.
 * \castest{<b>Depuis une position défilée, remonter la sélection jusqu'en haut ramène le
 * défilement à zéro.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Depuis une position défilée, remonter la sélection jusqu'en haut ramène le défilement
 * à zéro.
 * }
 */
TEST(LevelPickerTest, DefilementSuitLaSelectionVersLeHaut) {
    hmi::LevelPicker picker(manyChoices(6));
    hmi::InputState input;
    for (int step = 0; step < 4; ++step) {
        input.beginFrame();
        input.onKeyDown(hmi::Key::Down);
        static_cast<void>(picker.update(input, SMALL_VIEWPORT_HEIGHT));
        input.beginFrame();
        input.onKeyUp(hmi::Key::Down);
        static_cast<void>(picker.update(input, SMALL_VIEWPORT_HEIGHT));
    }
    ASSERT_EQ(picker.scrollOffset(), 2);

    for (int step = 0; step < 4; ++step) {
        input.beginFrame();
        input.onKeyDown(hmi::Key::Up);
        static_cast<void>(picker.update(input, SMALL_VIEWPORT_HEIGHT));
        input.beginFrame();
        input.onKeyUp(hmi::Key::Up);
        static_cast<void>(picker.update(input, SMALL_VIEWPORT_HEIGHT));
    }

    ASSERT_EQ(picker.selected(), 0);
    EXPECT_EQ(picker.scrollOffset(), 0);
}

/**
 * @brief La molette défile la liste sans changer la sélection courante.
 * \castest{<b>La molette défile la liste sans changer la sélection courante.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La molette défile la liste sans changer la sélection courante.
 * }
 */
TEST(LevelPickerTest, MoletteDefileSansChangerLaSelection) {
    hmi::LevelPicker picker(manyChoices(6));
    hmi::InputState input;
    input.beginFrame();
    input.onMouseWheel(-120);  // un cran vers l'arriere : defile vers le bas d'une ligne
    static_cast<void>(picker.update(input, SMALL_VIEWPORT_HEIGHT));

    EXPECT_EQ(picker.selected(), 0);
    EXPECT_EQ(picker.scrollOffset(), 1);
}

/**
 * @brief Un choix défilé hors de la fenêtre visible n'est pas cliquable à son ancienne position
 * écran (seule la fenêtre affichée l'est).
 * \castest{<b>Un choix défilé hors de la fenêtre visible n'est pas cliquable à son ancienne
 * position écran.</b><br/>
 * \tcat Unitaire · Level Picker<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un choix défilé hors de la fenêtre visible n'est pas cliquable à son ancienne position
 * écran.
 * }
 */
TEST(LevelPickerTest, ChoixDefileHorsFenetreNestPasCliquable) {
    hmi::LevelPicker picker(manyChoices(6));
    hmi::InputState input;
    input.beginFrame();
    input.onMouseWheel(-240);  // deux crans : scrollOffset -> 2 (choix 0 et 1 hors fenetre)
    static_cast<void>(picker.update(input, SMALL_VIEWPORT_HEIGHT));
    ASSERT_EQ(picker.scrollOffset(), 2);

    // Position ecran ou le choix 0 aurait ete dessine SANS defilement (rang 0 de la mise en
    // page) : desormais occupee par le choix 2 (premier de la fenetre visible), pas le choix 0.
    input.beginFrame();
    input.onMouseMove(choicePointX(), choicePointY(0));
    input.onMouseButtonDown(hmi::MouseButton::Left);
    const std::optional<int> confirmed = picker.update(input, SMALL_VIEWPORT_HEIGHT);

    ASSERT_TRUE(confirmed.has_value());
    EXPECT_EQ(*confirmed, 2);  // le clic touche le choix REELLEMENT affiche a cette position
}
