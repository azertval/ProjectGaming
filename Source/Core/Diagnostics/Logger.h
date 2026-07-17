#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "Core/Diagnostics/ILogSink.h"
#include "Core/Diagnostics/LogLevel.h"

/**
 * @file Core/Diagnostics/Logger.h
 * @brief Journaliseur : filtrage par niveau et diffusion vers des sinks.
 */

namespace core {

/**
 * @brief Filtre les messages par niveau minimal et les diffuse aux sinks enregistrés.
 *
 * Le journaliseur **possède** ses sinks (RAII). Il n'effectue aucune entrée-sortie
 * lui-même : celle-ci est déléguée aux sinks.
 */
class Logger {
public:
    Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /// Définit le niveau minimal ; les messages moins graves sont ignorés.
    void setMinimumLevel(LogLevel level);

    /**
     * @brief Indique si un niveau passe le filtre courant.
     * @param level Niveau à tester.
     * @return true si un message de ce niveau serait diffusé.
     */
    [[nodiscard]] bool isEnabled(LogLevel level) const;

    /// Ajoute un sink (le journaliseur en prend la propriété).
    void addSink(std::unique_ptr<ILogSink> sink);

    /// Retire tous les sinks.
    void clearSinks();

    /**
     * @brief Diffuse un message aux sinks s'il passe le filtre de niveau.
     * @param level   Niveau du message.
     * @param message Texte du message.
     */
    void log(LogLevel level, std::string_view message);

private:
    LogLevel _minimumLevel = LogLevel::Trace;
    std::vector<std::unique_ptr<ILogSink>> _sinks;
};

/**
 * @brief Journaliseur global de l'application (utilisé par les macros de log).
 * @return Référence vers l'instance unique.
 */
[[nodiscard]] Logger& defaultLogger();

}  // namespace core
