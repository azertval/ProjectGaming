#include "HMI/Interface/SessionLog.h"

#include <fstream>
#include <system_error>

namespace hmi {

/**
 * @brief Assemble les messages de log en un texte (une ligne par message).
 * @param entries Messages accumulés.
 * @return Le texte complet, terminé par un saut de ligne (vide si aucun message).
 */
std::string serializeSessionLog(const std::vector<core::MemoryLogSink::Entry>& entries) {
    std::string text;
    for (const core::MemoryLogSink::Entry& entry : entries) {
        text += entry.message;
        text += '\n';
    }
    return text;
}

/**
 * @brief Enregistre les messages de log de la session dans un fichier.
 * @param entries Messages accumulés.
 * @param path    Chemin du fichier de destination (les dossiers parents sont créés).
 * @return true si le fichier a été écrit ; false (récupérable) en cas d'échec.
 */
bool saveSessionLog(const std::vector<core::MemoryLogSink::Entry>& entries,
                    const std::filesystem::path& path) {
    // Crée les dossiers parents si besoin (échec traité comme une erreur récupérable).
    std::error_code error;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            return false;
        }
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return false;
    }
    file << serializeSessionLog(entries);
    return file.good();
}

}  // namespace hmi
