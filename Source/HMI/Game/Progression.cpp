#include "HMI/Game/Progression.h"

#include <fstream>
#include <string>
#include <system_error>

#include <nlohmann/json.hpp>

#include "HMI/HmiLog.h"

namespace hmi {

bool Progression::save(const std::filesystem::path& path) const {
    nlohmann::json root = nlohmann::json::object();
    root["sequenceId"] = _sequenceId;
    root["currentLevel"] = _currentLevel;
    root["completedLevels"] = _completedLevels;

    std::error_code error;
    const std::filesystem::path directory = path.parent_path();
    std::filesystem::create_directories(directory, error);

    // Écriture atomique : fichier temporaire dans le même dossier (donc le même volume, condition
    // pour que le remplacement soit atomique), puis remplacement en une seule opération -- même
    // patron que hmi::encodeImageFile (HMI/Graphics/TextureLoader.cpp, LOT-54). Une fermeture
    // brutale pendant l'écriture laisse au pire le fichier temporaire, jamais progression.json
    // à demi écrit.
    const std::filesystem::path temporary =
        path.parent_path() / (path.filename().wstring() + L".tmp");
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) {
            return false;
        }
        const std::string dumped = root.dump(2);
        output.write(dumped.data(), static_cast<std::streamsize>(dumped.size()));
        if (!output.good()) {
            std::filesystem::remove(temporary, error);
            return false;
        }
    }
    std::filesystem::rename(temporary, path, error);
    if (error) {
        std::filesystem::remove(temporary, error);
        return false;
    }
    return true;
}

Progression Progression::load(const std::filesystem::path& path) {
    Progression progression;  // partie neuve, ecrasee ci-dessous champ par champ si le fichier lit.

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return progression;  // fichier absent (premier lancement) : partie neuve, rien a signaler.
    }

    nlohmann::json root;
    try {
        input >> root;
    } catch (const nlohmann::json::exception& error) {
        HMI_LOG_WARNING(std::string("progression.json illisible : ") + error.what());
        return progression;
    }
    if (!root.is_object()) {
        HMI_LOG_WARNING("progression.json : racine invalide (objet attendu), partie neuve.");
        return progression;
    }

    if (const auto sequenceId = root.find("sequenceId");
        sequenceId != root.end() && sequenceId->is_string()) {
        progression._sequenceId = sequenceId->get<std::string>();
    }
    if (const auto currentLevel = root.find("currentLevel");
        currentLevel != root.end() && currentLevel->is_string()) {
        progression._currentLevel = currentLevel->get<std::string>();
    }
    if (const auto completed = root.find("completedLevels");
        completed != root.end() && completed->is_array()) {
        for (const nlohmann::json& entry : *completed) {
            if (entry.is_string()) {
                progression._completedLevels.insert(entry.get<std::string>());
            }
            // Entrée non textuelle ignorée (EX-NFR-040) : le reste de la progression reste valide.
        }
    }
    return progression;
}

bool isLevelUnlocked(const Progression& progression, const std::vector<std::string>& sequence,
                     const std::string& levelName) {
    if (progression.isCompleted(levelName)) {
        return true;
    }
    for (const std::string& level : sequence) {
        if (progression.isCompleted(level)) {
            continue;
        }
        // Premier tableau non termine rencontre dans l'ordre de la sequence : jouable s'il s'agit
        // de celui demande, verrouille sinon (tous les suivants) -- sortie immediate, la boucle
        // n'evalue jamais un candidat plus loin dans la sequence.
        return level == levelName;
    }
    return false;  // sequence entierement terminee et levelName absent : n'existe pas (ou n'est
                   // pas dans cette sequence), jamais jouable par cette regle.
}

}  // namespace hmi
