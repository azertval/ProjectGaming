/**
 * @file test_logger.cpp
 * @brief Tests unitaires du journaliseur (filtrage et diffusion).
 */

#include <memory>

#include <gtest/gtest.h>

#include "Core/Diagnostics/Logger.h"
#include "Core/Diagnostics/MemoryLogSink.h"

/// Un message au-dessus du niveau minimal est diffusé ; en dessous, il est filtré.
TEST(LoggerTest, FiltreParNiveauMinimal) {
    core::Logger logger;
    auto sink = std::make_unique<core::MemoryLogSink>();
    core::MemoryLogSink* observed = sink.get();
    logger.addSink(std::move(sink));
    logger.setMinimumLevel(core::LogLevel::Warning);

    logger.log(core::LogLevel::Info, "ignore");
    logger.log(core::LogLevel::Error, "garde");

    ASSERT_EQ(observed->entries().size(), 1u);
    EXPECT_EQ(observed->entries()[0].level, core::LogLevel::Error);
    EXPECT_EQ(observed->entries()[0].message, "garde");
}

/// Le même message atteint tous les sinks enregistrés.
TEST(LoggerTest, DiffuseAPlusieursSinks) {
    core::Logger logger;
    auto first = std::make_unique<core::MemoryLogSink>();
    auto second = std::make_unique<core::MemoryLogSink>();
    core::MemoryLogSink* firstObserved = first.get();
    core::MemoryLogSink* secondObserved = second.get();
    logger.addSink(std::move(first));
    logger.addSink(std::move(second));

    logger.log(core::LogLevel::Info, "diffuse");

    EXPECT_EQ(firstObserved->entries().size(), 1u);
    EXPECT_EQ(secondObserved->entries().size(), 1u);
}

/// clearSinks retire les destinations : plus rien n'est diffusé ensuite.
TEST(LoggerTest, ClearSinksArreteLaDiffusion) {
    core::Logger logger;
    auto sink = std::make_unique<core::MemoryLogSink>();
    core::MemoryLogSink* observed = sink.get();
    logger.addSink(std::move(sink));

    logger.clearSinks();
    logger.log(core::LogLevel::Error, "personne");

    EXPECT_TRUE(observed->entries().empty());
}
