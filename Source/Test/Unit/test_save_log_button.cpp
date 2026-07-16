/**
 * @file test_save_log_button.cpp
 * @brief Tests unitaires du bouton d'enregistrement des logs : ancrage et détection du clic.
 */

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Interface/LanguageSelector.h"
#include "HMI/Interface/SaveLogButton.h"

namespace {

/// @return Un état d'entrées avec la souris en (@p x, @p y) et le bouton gauche cliqué.
hmi::InputState mouseClick(int x, int y) {
    hmi::InputState input;
    input.beginFrame();
    input.onMouseMove(x, y);
    input.onMouseButtonDown(hmi::MouseButton::Left);
    return input;
}

constexpr int VIEWPORT_WIDTH = 1280;
constexpr int VIEWPORT_HEIGHT = 720;

}  // namespace

/// Le bouton est à gauche du bouton de langue, aligné sur le même bord bas.
TEST(SaveLogButtonTest, AGaucheDuBoutonLangue) {
    const hmi::SaveLogButton::Rect save =
        hmi::SaveLogButton::rect(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    const hmi::LanguageSelector::Rect language =
        hmi::LanguageSelector::rect(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    EXPECT_LT(save.x + save.width, language.x);  // à gauche, sans chevauchement
    EXPECT_FLOAT_EQ(save.y + save.height,
                    static_cast<float>(VIEWPORT_HEIGHT) - hmi::LanguageSelector::MARGIN);
}

/// Un clic dans le bouton est détecté.
TEST(SaveLogButtonTest, ClicDansLeBouton) {
    const hmi::SaveLogButton button;
    const hmi::SaveLogButton::Rect rect =
        hmi::SaveLogButton::rect(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    const int x = static_cast<int>(rect.x + rect.width * 0.5f);
    const int y = static_cast<int>(rect.y + rect.height * 0.5f);

    EXPECT_TRUE(button.clicked(mouseClick(x, y), VIEWPORT_WIDTH, VIEWPORT_HEIGHT));
}

/// Un clic hors du bouton n'est pas détecté.
TEST(SaveLogButtonTest, ClicHorsBouton) {
    const hmi::SaveLogButton button;
    EXPECT_FALSE(button.clicked(mouseClick(5, 5), VIEWPORT_WIDTH, VIEWPORT_HEIGHT));
}
