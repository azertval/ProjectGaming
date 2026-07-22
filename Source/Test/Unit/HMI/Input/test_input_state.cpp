/**
 * @file test_input_state.cpp
 * @brief Tests unitaires de l'état des entrées : fronts clavier/souris et position.
 */

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"

/**
 * @brief Une touche passée d'« absente » à « présente » est « pressée » exactement une frame.
 * \castest{<b>Une touche passée d'« absente » à « présente » est « pressée » exactement une
 * frame.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une touche passée d'« absente » à « présente » est « pressée » exactement une frame.
 * }
 */
TEST(InputStateTest, FrontMontantClavier) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Enter);
    EXPECT_TRUE(input.keyDown(hmi::Key::Enter));
    EXPECT_TRUE(input.keyPressed(hmi::Key::Enter));
    EXPECT_FALSE(input.keyReleased(hmi::Key::Enter));

    // Frame suivante sans nouvel événement : toujours maintenue, mais plus « pressée ».
    input.beginFrame();
    EXPECT_TRUE(input.keyDown(hmi::Key::Enter));
    EXPECT_FALSE(input.keyPressed(hmi::Key::Enter));
}

/**
 * @brief Une touche restée enfoncée n'est « pressée » qu'à la première frame.
 * \castest{<b>Une touche restée enfoncée n'est « pressée » qu'à la première frame.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une touche restée enfoncée n'est « pressée » qu'à la première frame.
 * }
 */
TEST(InputStateTest, MaintienClavier) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Down);
    EXPECT_TRUE(input.keyPressed(hmi::Key::Down));

    for (int frame = 0; frame < 3; ++frame) {
        input.beginFrame();  // aucune nouvelle entrée : la touche reste enfoncée
        EXPECT_TRUE(input.keyDown(hmi::Key::Down));
        EXPECT_FALSE(input.keyPressed(hmi::Key::Down));
        EXPECT_FALSE(input.keyReleased(hmi::Key::Down));
    }
}

/**
 * @brief Le relâchement d'une touche est détecté « relâchée » pendant exactement une frame.
 * \castest{<b>Le relâchement d'une touche est détecté « relâchée » pendant exactement une
 * frame.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le relâchement d'une touche est détecté « relâchée » pendant exactement une frame.
 * }
 */
TEST(InputStateTest, FrontDescendantClavier) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Space);
    input.beginFrame();  // maintenue

    input.beginFrame();
    input.onKeyUp(hmi::Key::Space);
    EXPECT_FALSE(input.keyDown(hmi::Key::Space));
    EXPECT_TRUE(input.keyReleased(hmi::Key::Space));

    input.beginFrame();
    EXPECT_FALSE(input.keyReleased(hmi::Key::Space));
}

/**
 * @brief Les touches sont indépendantes : un front sur l'une n'affecte pas les autres.
 * \castest{<b>Les touches sont indépendantes : un front sur l'une n'affecte pas les
 * autres.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les touches sont indépendantes : un front sur l'une n'affecte pas les autres.
 * }
 */
TEST(InputStateTest, TouchesIndependantes) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Up);
    EXPECT_TRUE(input.keyPressed(hmi::Key::Up));
    EXPECT_FALSE(input.keyDown(hmi::Key::Down));
    EXPECT_FALSE(input.keyPressed(hmi::Key::Down));
}

/**
 * @brief Un bouton de souris suit la même logique pressé/cliqué/relâché que les touches.
 * \castest{<b>Un bouton de souris suit la même logique pressé/cliqué/relâché que les
 * touches.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bouton de souris suit la même logique pressé/cliqué/relâché que les touches.
 * }
 */
TEST(InputStateTest, BoutonSouris) {
    hmi::InputState input;

    input.beginFrame();
    input.onMouseButtonDown(hmi::MouseButton::Left);
    EXPECT_TRUE(input.mouseButtonDown(hmi::MouseButton::Left));
    EXPECT_TRUE(input.mouseButtonPressed(hmi::MouseButton::Left));

    input.beginFrame();  // maintenu
    EXPECT_TRUE(input.mouseButtonDown(hmi::MouseButton::Left));
    EXPECT_FALSE(input.mouseButtonPressed(hmi::MouseButton::Left));

    input.beginFrame();
    input.onMouseButtonUp(hmi::MouseButton::Left);
    EXPECT_FALSE(input.mouseButtonDown(hmi::MouseButton::Left));
    EXPECT_TRUE(input.mouseButtonReleased(hmi::MouseButton::Left));
}

/**
 * @brief La position de la souris reflète le dernier déplacement injecté.
 * \castest{<b>La position de la souris reflète le dernier déplacement injecté.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La position de la souris reflète le dernier déplacement injecté.
 * }
 */
TEST(InputStateTest, PositionSouris) {
    hmi::InputState input;

    input.onMouseMove(42, 99);
    EXPECT_EQ(input.mouseX(), 42);
    EXPECT_EQ(input.mouseY(), 99);

    input.onMouseMove(-3, 7);  // coordonnées signées (souris hors zone client)
    EXPECT_EQ(input.mouseX(), -3);
    EXPECT_EQ(input.mouseY(), 7);
}

/**
 * @brief Les incréments de molette d'une frame s'additionnent et repartent de zéro ensuite.
 * \castest{<b>Les incréments de molette d'une frame s'additionnent et repartent de zéro
 * ensuite.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les incréments de molette d'une frame s'additionnent et repartent de zéro ensuite.
 * }
 */
TEST(InputStateTest, MoletteAccumuleEtSeReinitialise) {
    hmi::InputState input;

    input.beginFrame();
    EXPECT_EQ(input.wheelDelta(), 0);
    input.onMouseWheel(120);
    input.onMouseWheel(-40);
    EXPECT_EQ(input.wheelDelta(), 80);

    input.beginFrame();  // nouvelle frame sans nouvel événement molette
    EXPECT_EQ(input.wheelDelta(), 0);
}

/**
 * @brief Les caractères tapés s'accumulent dans l'ordre puis sont vidés à la frame suivante.
 * \castest{<b>Les caractères tapés s'accumulent dans l'ordre puis sont vidés à la frame
 * suivante.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les caractères tapés s'accumulent dans l'ordre puis sont vidés à la frame suivante.
 * }
 */
TEST(InputStateTest, CaracteresTapesAccumulesEtVides) {
    hmi::InputState input;

    input.beginFrame();
    EXPECT_TRUE(input.typedCharacters().empty());
    input.onCharTyped(L'N');
    input.onCharTyped(L'1');
    input.onCharTyped(L'\xE9');  // 'é', caractère accentué (saisie non anglophone, EX-EDIT-009)
    ASSERT_EQ(input.typedCharacters().size(), 3u);
    EXPECT_EQ(input.typedCharacters()[0], L'N');
    EXPECT_EQ(input.typedCharacters()[1], L'1');
    EXPECT_EQ(input.typedCharacters()[2], L'\xE9');

    input.beginFrame();
    EXPECT_TRUE(input.typedCharacters().empty());
}
