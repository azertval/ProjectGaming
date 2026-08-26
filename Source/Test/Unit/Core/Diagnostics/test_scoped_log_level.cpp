// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_scoped_log_level.cpp
 * @brief Tests unitaires de l'élévation temporaire (RAII) du niveau minimal d'un journaliseur.
 */

#include <gtest/gtest.h>

#include "Core/Diagnostics/Logger.h"
#include "Core/Diagnostics/ScopedLogLevel.h"

/**
 * @brief Un niveau plus permissif (Trace) que le plancher demandé (Warning) est relevé pendant la
 * portée, puis restauré à la sortie.
 * \castest{<b>ScopedLogLevel relève un niveau permissif, puis le restaure.</b><br/>
 * \tcat Unitaire · ScopedLogLevel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Logger au niveau Trace.<br/>2. Ouvrir une portée `ScopedLogLevel(logger,
 * Warning)`.<br/>3. Fermer la portée.<br/>
 * \tattendu Niveau Warning pendant la portée, Trace restauré après.}
 */
TEST(ScopedLogLevelTest, ReleveUnNiveauPermissifPuisLeRestaure) {
    core::Logger logger;
    logger.setMinimumLevel(core::LogLevel::Trace);

    {
        const core::ScopedLogLevel guard(logger, core::LogLevel::Warning);
        EXPECT_EQ(logger.minimumLevel(), core::LogLevel::Warning);
        EXPECT_FALSE(logger.isEnabled(core::LogLevel::Info));
    }

    EXPECT_EQ(logger.minimumLevel(), core::LogLevel::Trace);
    EXPECT_TRUE(logger.isEnabled(core::LogLevel::Trace));
}

/**
 * @brief Un niveau déjà plus strict que le plancher demandé n'est jamais assoupli.
 * \castest{<b>ScopedLogLevel n'assouplit jamais un niveau déjà plus strict.</b><br/>
 * \tcat Unitaire · ScopedLogLevel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Logger au niveau Error.<br/>2. Ouvrir une portée `ScopedLogLevel(logger,
 * Warning)`.<br/>
 * \tattendu Le niveau reste Error pendant la portée et après.}
 */
TEST(ScopedLogLevelTest, NAssouplitJamaisUnNiveauDejaPlusStrict) {
    core::Logger logger;
    logger.setMinimumLevel(core::LogLevel::Error);

    {
        const core::ScopedLogLevel guard(logger, core::LogLevel::Warning);
        EXPECT_EQ(logger.minimumLevel(), core::LogLevel::Error);
    }

    EXPECT_EQ(logger.minimumLevel(), core::LogLevel::Error);
}
