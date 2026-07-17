#include "Core/Diagnostics/Logger.h"

#include <utility>

namespace core {

void Logger::setMinimumLevel(LogLevel level) {
    _minimumLevel = level;
}

bool Logger::isEnabled(LogLevel level) const {
    return level >= _minimumLevel;
}

void Logger::addSink(std::unique_ptr<ILogSink> sink) {
    _sinks.push_back(std::move(sink));
}

void Logger::clearSinks() {
    _sinks.clear();
}

// Diffuse un message aux sinks s'il passe le filtre de niveau.
void Logger::log(LogLevel level, std::string_view message) {
    // Filtrage en amont : un message moins grave que le seuil n'est pas diffusé.
    if (!isEnabled(level)) {
        return;
    }
    for (const std::unique_ptr<ILogSink>& sink : _sinks) {
        sink->write(level, message);
    }
}

Logger& defaultLogger() {
    // Instance à durée de vie statique : construite au premier appel, partagée
    // par l'ensemble du programme via les macros de log.
    static Logger instance;
    return instance;
}

}  // namespace core
