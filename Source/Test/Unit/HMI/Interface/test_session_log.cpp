/**
 * @file test_session_log.cpp
 * @brief Tests unitaires de la sérialisation des logs de session.
 */

#include <vector>

#include <gtest/gtest.h>

#include "Core/Diagnostics/MemoryLogSink.h"
#include "HMI/Interface/SessionLog.h"

/// Chaque message donne une ligne, dans l'ordre d'arrivée.
TEST(SessionLogTest, SerialiseUneLigneParMessage) {
    const std::vector<core::MemoryLogSink::Entry> entries = {
        {core::LogLevel::Info, "premier message"},
        {core::LogLevel::Warning, "second message"},
    };

    EXPECT_EQ(hmi::serializeSessionLog(entries), "premier message\nsecond message\n");
}

/// Une session sans message produit un texte vide.
TEST(SessionLogTest, VideDonneChaineVide) {
    EXPECT_EQ(hmi::serializeSessionLog({}), "");
}
