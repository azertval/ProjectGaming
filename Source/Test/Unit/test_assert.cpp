/**
 * @file test_assert.cpp
 * @brief Tests unitaires des assertions du projet.
 */

#include <string>

#include <gtest/gtest.h>

#include "Core/Diagnostics/Assert.h"

/// Une condition vraie n'invoque pas le gestionnaire d'assertion.
TEST(AssertTest, ConditionVraieNInvoquePasLeHandler) {
    bool invoked = false;
    core::setAssertionHandler(
        [&](const char*, const char*, const char*, int) { invoked = true; });

    // Condition évaluée à l'exécution (évite l'avertissement de condition constante).
    volatile bool condition = true;
    PROJECTGAMING_ASSERT(condition, "ne doit pas echouer");

    EXPECT_FALSE(invoked);
    core::setAssertionHandler(nullptr);
}

/// Une condition fausse invoque le gestionnaire une fois, avec le message (Debug uniquement).
TEST(AssertTest, ConditionFausseInvoqueLeHandler) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    int count = 0;
    std::string capturedMessage;
    core::setAssertionHandler([&](const char*, const char* message, const char*, int) {
        ++count;
        capturedMessage = message;
    });

    // Condition évaluée à l'exécution (évite l'avertissement de condition constante).
    volatile bool condition = false;
    PROJECTGAMING_ASSERT(condition, "echec attendu");

    EXPECT_EQ(count, 1);
    EXPECT_EQ(capturedMessage, "echec attendu");
    core::setAssertionHandler(nullptr);
#endif
}
