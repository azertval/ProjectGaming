/**
 * @file test_sinks.cpp
 * @brief Tests unitaires des sinks de journalisation.
 */

#include <gtest/gtest.h>

#include "Core/Diagnostics/MemoryLogSink.h"

/// Le sink mémoire conserve fidèlement niveau et texte, dans l'ordre.
TEST(MemoryLogSinkTest, ConserveNiveauEtTexteDansLOrdre) {
    core::MemoryLogSink sink;
    sink.write(core::LogLevel::Info, "premier");
    sink.write(core::LogLevel::Error, "second");

    ASSERT_EQ(sink.entries().size(), 2u);
    EXPECT_EQ(sink.entries()[0].level, core::LogLevel::Info);
    EXPECT_EQ(sink.entries()[0].message, "premier");
    EXPECT_EQ(sink.entries()[1].level, core::LogLevel::Error);
    EXPECT_EQ(sink.entries()[1].message, "second");
}

/// clear vide les messages mémorisés.
TEST(MemoryLogSinkTest, ClearVideLesMessages) {
    core::MemoryLogSink sink;
    sink.write(core::LogLevel::Trace, "x");
    sink.clear();
    EXPECT_TRUE(sink.entries().empty());
}
