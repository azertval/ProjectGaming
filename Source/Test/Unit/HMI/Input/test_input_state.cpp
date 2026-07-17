/**
 * @file test_input_state.cpp
 * @brief Tests unitaires de l'état des entrées : fronts clavier/souris et position.
 */

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"

/// Une touche passée d'« absente » à « présente » est « pressée » exactement une frame.
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

/// Une touche restée enfoncée n'est « pressée » qu'à la première frame.
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

/// Le relâchement d'une touche est détecté « relâchée » pendant exactement une frame.
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

/// Les touches sont indépendantes : un front sur l'une n'affecte pas les autres.
TEST(InputStateTest, TouchesIndependantes) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Up);
    EXPECT_TRUE(input.keyPressed(hmi::Key::Up));
    EXPECT_FALSE(input.keyDown(hmi::Key::Down));
    EXPECT_FALSE(input.keyPressed(hmi::Key::Down));
}

/// Un bouton de souris suit la même logique pressé/cliqué/relâché que les touches.
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

/// La position de la souris reflète le dernier déplacement injecté.
TEST(InputStateTest, PositionSouris) {
    hmi::InputState input;

    input.onMouseMove(42, 99);
    EXPECT_EQ(input.mouseX(), 42);
    EXPECT_EQ(input.mouseY(), 99);

    input.onMouseMove(-3, 7);  // coordonnées signées (souris hors zone client)
    EXPECT_EQ(input.mouseX(), -3);
    EXPECT_EQ(input.mouseY(), 7);
}
