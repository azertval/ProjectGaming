#include "Core/Diagnostics/LogFormat.h"

#include <ctime>

namespace core {

std::string_view fileName(std::string_view path) {
    // Conserve la portion après le dernier séparateur de chemin (Windows ou POSIX).
    const std::size_t separator = path.find_last_of("/\\");
    if (separator == std::string_view::npos) {
        return path;
    }
    return path.substr(separator + 1);
}

/**
 * @brief Compose une ligne de journalisation.
 * @param timestamp Horodatage déjà formaté (injecté, pour rester testable).
 * @param level     Niveau du message.
 * @param category  Catégorie / sous-système émetteur (ex. "HMI").
 * @param file      Fichier source (le nom seul est conservé dans la ligne).
 * @param line      Ligne source à l'origine du message.
 * @param message   Texte du message.
 * @return Ligne de la forme "[timestamp][NIVEAU][catégorie][fichier:ligne] message".
 */
std::string formatLogLine(std::string_view timestamp, LogLevel level, std::string_view category,
                          std::string_view file, int line, std::string_view message) {
    const std::string_view name = fileName(file);

    std::string result;
    result.reserve(timestamp.size() + category.size() + name.size() + message.size() + 32);
    result += '[';
    result += timestamp;
    result += "][";
    result += toString(level);
    result += "][";
    result += category;
    result += "][";
    result += name;
    result += ':';
    result += std::to_string(line);
    result += "] ";
    result += message;
    return result;
}

/**
 * @brief Horodatage courant local, au format "HH:MM:SS".
 * @return La chaîne d'horodatage.
 */
std::string currentTimestamp() {
    const std::time_t now = std::time(nullptr);
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    char buffer[16] = {};
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &localTime);
    return buffer;
}

}  // namespace core
