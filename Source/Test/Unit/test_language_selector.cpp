/**
 * @file test_language_selector.cpp
 * @brief Tests unitaires du bouton de langue : ancrage, bascule et détection du clic.
 */

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Interface/LanguageSelector.h"

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

/// Le bouton est ancré au coin bas-droit, marge comprise.
TEST(LanguageSelectorTest, RectangleAncreBasDroite) {
    const hmi::LanguageSelector::Rect rect =
        hmi::LanguageSelector::rect(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    EXPECT_FLOAT_EQ(rect.width, hmi::LanguageSelector::BUTTON_WIDTH);
    EXPECT_FLOAT_EQ(rect.height, hmi::LanguageSelector::BUTTON_HEIGHT);
    EXPECT_FLOAT_EQ(rect.x, VIEWPORT_WIDTH - hmi::LanguageSelector::MARGIN -
                                hmi::LanguageSelector::BUTTON_WIDTH);
    EXPECT_FLOAT_EQ(rect.y, VIEWPORT_HEIGHT - hmi::LanguageSelector::MARGIN -
                                hmi::LanguageSelector::BUTTON_HEIGHT);
}

/// La bascule renvoie l'autre langue (français ↔ anglais).
TEST(LanguageSelectorTest, BasculeAlterneLesLangues) {
    EXPECT_EQ(hmi::LanguageSelector::other("fr"), "en");
    EXPECT_EQ(hmi::LanguageSelector::other("en"), "fr");
    EXPECT_EQ(hmi::LanguageSelector::other("xx"), "en");  // défaut robuste
}

/// Un clic dans le bouton demande la bascule vers l'autre langue.
TEST(LanguageSelectorTest, ClicDansLeBoutonBascule) {
    const hmi::LanguageSelector selector;
    const hmi::LanguageSelector::Rect rect =
        hmi::LanguageSelector::rect(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    const int x = static_cast<int>(rect.x + rect.width * 0.5f);
    const int y = static_cast<int>(rect.y + rect.height * 0.5f);

    const hmi::LanguageSelector::Toggle toggle =
        selector.update(mouseClick(x, y), "fr", VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    EXPECT_TRUE(toggle.requested);
    EXPECT_EQ(toggle.next, "en");
}

/// Un clic hors du bouton ne demande aucune bascule.
TEST(LanguageSelectorTest, ClicHorsBoutonNeBasculePas) {
    const hmi::LanguageSelector selector;

    const hmi::LanguageSelector::Toggle toggle =
        selector.update(mouseClick(10, 10), "fr", VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    EXPECT_FALSE(toggle.requested);
}

/// Sans clic, aucune bascule même si la souris est sur le bouton.
TEST(LanguageSelectorTest, SansClicPasDeBascule) {
    const hmi::LanguageSelector selector;
    const hmi::LanguageSelector::Rect rect =
        hmi::LanguageSelector::rect(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    hmi::InputState input;
    input.beginFrame();
    input.onMouseMove(static_cast<int>(rect.x + 5.0f), static_cast<int>(rect.y + 5.0f));

    const hmi::LanguageSelector::Toggle toggle =
        selector.update(input, "fr", VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    EXPECT_FALSE(toggle.requested);
}
