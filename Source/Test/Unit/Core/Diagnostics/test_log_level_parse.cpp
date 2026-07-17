/**
 * @file test_log_level_parse.cpp
 * @brief Tests unitaires de l'analyse d'un niveau de log depuis une chaîne.
 */

#include <gtest/gtest.h>

#include "Core/Diagnostics/LogLevelParse.h"

/// Les noms de niveaux reconnus sont convertis (y compris l'alias « warn »).
TEST(LogLevelParseTest, NiveauxReconnus) {
    EXPECT_EQ(core::parseLogLevel("trace"), core::LogLevel::Trace);
    EXPECT_EQ(core::parseLogLevel("info"), core::LogLevel::Info);
    EXPECT_EQ(core::parseLogLevel("warning"), core::LogLevel::Warning);
    EXPECT_EQ(core::parseLogLevel("warn"), core::LogLevel::Warning);
    EXPECT_EQ(core::parseLogLevel("error"), core::LogLevel::Error);
}

/// L'analyse est insensible à la casse.
TEST(LogLevelParseTest, InsensibleALaCasse) {
    EXPECT_EQ(core::parseLogLevel("INFO"), core::LogLevel::Info);
    EXPECT_EQ(core::parseLogLevel("Warning"), core::LogLevel::Warning);
    EXPECT_EQ(core::parseLogLevel("Error"), core::LogLevel::Error);
}

/// Une valeur inconnue ou vide n'est pas convertie.
TEST(LogLevelParseTest, ValeurInconnue) {
    EXPECT_FALSE(core::parseLogLevel("verbose").has_value());
    EXPECT_FALSE(core::parseLogLevel("").has_value());
}
