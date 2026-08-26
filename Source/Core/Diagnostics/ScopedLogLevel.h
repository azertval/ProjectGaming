// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Diagnostics/LogLevel.h"
#include "Core/Diagnostics/Logger.h"

/**
 * @file Core/Diagnostics/ScopedLogLevel.h
 * @brief Élévation temporaire (RAII) du niveau minimal d'un `Logger`.
 */

namespace core {

/**
 * @brief Relève le niveau minimal de @p logger à au moins @p floor pour la durée de vie de
 * l'objet, et restaure le niveau précédent à la destruction.
 *
 * N'assouplit jamais un niveau déjà plus strict que @p floor : un utilisateur qui a explicitement
 * demandé `--log-level=error` reste à `error` même à l'intérieur d'une portée qui ne demande que
 * `warning`. Sert notamment à faire taire les traces de cycle de vie (chargement de niveau,
 * déclenchement de mécanisme…) pendant une simulation IA headless (`HeadlessLevelEnvironment`),
 * rejouée des milliers de fois par entraînement/évaluation — un volume de journalisation sans
 * rapport avec une partie réelle, qui ralentit l'entraînement pour rien (`LOT-ANNEXE-21`).
 */
class ScopedLogLevel {
public:
    ScopedLogLevel(Logger& logger, LogLevel floor)
        : _logger(logger), _previousLevel(logger.minimumLevel()) {
        if (_previousLevel < floor) {
            _logger.setMinimumLevel(floor);
        }
    }

    ~ScopedLogLevel() { _logger.setMinimumLevel(_previousLevel); }

    ScopedLogLevel(const ScopedLogLevel&) = delete;
    ScopedLogLevel& operator=(const ScopedLogLevel&) = delete;

private:
    Logger& _logger;
    LogLevel _previousLevel;
};

}  // namespace core
