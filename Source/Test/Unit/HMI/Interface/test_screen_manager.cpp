/**
 * @file test_screen_manager.cpp
 * @brief Tests unitaires du gestionnaire d'écrans : exécution et transitions.
 */

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Interface/IScreen.h"
#include "HMI/Interface/ScreenManager.h"

namespace {

/// Écran de test : renvoie une transition scriptée et compte ses mises à jour.
struct FakeScreen : hmi::IScreen {
    hmi::ScreenTransition nextTransition = hmi::ScreenTransition::none();
    int updateCount = 0;

    hmi::ScreenTransition update(const hmi::InputState&, float) override {
        ++updateCount;
        return nextTransition;
    }

    void render(hmi::RenderContext&) override {}
};

/// Fabrique d'écrans instrumentée : mémorise les écrans demandés et créés.
struct Harness {
    std::vector<hmi::ScreenId> requested;
    std::vector<FakeScreen*> created;

    hmi::ScreenManager::Factory factory() {
        return [this](hmi::ScreenId id) -> std::unique_ptr<hmi::IScreen> {
            requested.push_back(id);
            auto screen = std::make_unique<FakeScreen>();
            created.push_back(screen.get());
            return screen;
        };
    }
};

}  // namespace

/// La construction fabrique l'écran initial demandé.
TEST(ScreenManagerTest, ConstructionCreeEcranInitial) {
    Harness harness;
    hmi::ScreenManager manager(harness.factory(), hmi::ScreenId::Menu);

    ASSERT_EQ(harness.requested.size(), 1u);
    EXPECT_EQ(harness.requested.front(), hmi::ScreenId::Menu);
    EXPECT_NE(manager.currentScreen(), nullptr);
}

/// Une transition « rester » conserve l'écran actif et n'en fabrique pas d'autre.
TEST(ScreenManagerTest, TransitionNoneResteSurEcran) {
    Harness harness;
    hmi::ScreenManager manager(harness.factory(), hmi::ScreenId::Menu);
    harness.created.front()->nextTransition = hmi::ScreenTransition::none();
    hmi::IScreen* before = manager.currentScreen();

    const hmi::InputState input;
    EXPECT_FALSE(manager.update(input, 0.0f));
    EXPECT_EQ(manager.currentScreen(), before);
    EXPECT_EQ(harness.requested.size(), 1u);  // aucun nouvel écran fabriqué
}

/// Une transition « basculer » remplace l'écran actif par celui demandé.
TEST(ScreenManagerTest, TransitionSwitchRemplaceEcran) {
    Harness harness;
    hmi::ScreenManager manager(harness.factory(), hmi::ScreenId::Menu);
    harness.created.front()->nextTransition =
        hmi::ScreenTransition::switchTo(hmi::ScreenId::Editor);
    hmi::IScreen* before = manager.currentScreen();

    const hmi::InputState input;
    EXPECT_FALSE(manager.update(input, 0.0f));
    ASSERT_EQ(harness.requested.size(), 2u);
    EXPECT_EQ(harness.requested.back(), hmi::ScreenId::Editor);
    EXPECT_NE(manager.currentScreen(), before);
    EXPECT_FALSE(manager.shouldQuit());
}

/// Une transition « quitter » ferme l'application et se propage à la boucle.
TEST(ScreenManagerTest, TransitionQuitFermeApplication) {
    Harness harness;
    hmi::ScreenManager manager(harness.factory(), hmi::ScreenId::Menu);
    harness.created.front()->nextTransition = hmi::ScreenTransition::quit();

    const hmi::InputState input;
    EXPECT_TRUE(manager.update(input, 0.0f));
    EXPECT_TRUE(manager.shouldQuit());
    EXPECT_EQ(manager.currentScreen(), nullptr);

    // Une mise à jour ultérieure reste une demande de fermeture (idempotent).
    EXPECT_TRUE(manager.update(input, 0.0f));
}
