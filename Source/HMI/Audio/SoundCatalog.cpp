#include "HMI/Audio/SoundCatalog.h"

#include <fstream>
#include <sstream>
#include <utility>

#include <nlohmann/json.hpp>

#include "HMI/Audio/AudioLog.h"

namespace hmi {

namespace {

// Noms des champs du format. Nommes plutot que repetes en litteraux : le lecteur et l'ecrivain ne
// peuvent pas diverger sur une faute de frappe (meme discipline que hmi::SkinCatalog).
constexpr const char* FIELD_VERSION = "version";
constexpr const char* FIELD_SOUNDS = "sons";

// Construit un resultat d'echec, en journalisant la raison en un point unique (meme patron que
// hmi::SkinCatalog::failure).
[[nodiscard]] SoundCatalogResult failure(std::string message, SoundCatalogError code) {
    AUDIO_LOG_WARNING("sounds.json : " + message);
    return SoundCatalogResult{
        .catalog = std::nullopt, .error = std::move(message), .errorCode = code};
}

}  // namespace

SoundCatalogResult SoundCatalog::loadFromString(std::string_view json) {
    // accept() puis parse() : nlohmann leve par defaut, et aucune exception ne doit franchir cette
    // frontiere (EX-NFR-040).
    if (!nlohmann::json::accept(json)) {
        return failure("JSON malforme.", SoundCatalogError::ParseError);
    }
    const nlohmann::json root = nlohmann::json::parse(json, nullptr, false);
    if (!root.is_object()) {
        return failure("La racine du document n'est pas un objet.", SoundCatalogError::ParseError);
    }

    // Version : absente vaut 1. Superieure a celle geree : refus explicite.
    int version = FORMAT_VERSION;
    if (root.contains(FIELD_VERSION)) {
        if (!root[FIELD_VERSION].is_number_integer()) {
            return failure("Le champ « version » n'est pas un entier.",
                           SoundCatalogError::MalformedStructure);
        }
        version = root[FIELD_VERSION].get<int>();
    }
    if (version > FORMAT_VERSION) {
        return failure("Version de format " + std::to_string(version) +
                           " non geree (cette version du jeu lit jusqu'a " +
                           std::to_string(FORMAT_VERSION) + ").",
                       SoundCatalogError::UnsupportedVersion);
    }

    SoundCatalog catalog;

    if (root.contains(FIELD_SOUNDS)) {
        if (!root[FIELD_SOUNDS].is_object()) {
            return failure("Le champ « sons » n'est pas un objet.",
                           SoundCatalogError::MalformedStructure);
        }
        for (const auto& [eventId, fileJson] : root[FIELD_SOUNDS].items()) {
            if (!fileJson.is_string() || fileJson.get<std::string>().empty()) {
                return failure("Evenement « " + eventId + " » sans fichier exploitable.",
                               SoundCatalogError::MalformedStructure);
            }
            catalog.assign(eventId, fileJson.get<std::string>());
        }
    }

    return SoundCatalogResult{
        .catalog = std::move(catalog), .error = {}, .errorCode = SoundCatalogError::None};
}

SoundCatalogResult SoundCatalog::loadFromFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        return failure("Fichier introuvable ou illisible : " + path.string(),
                       SoundCatalogError::FileNotFound);
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return loadFromString(contents.str());
}

std::optional<std::string> SoundCatalog::resolve(std::string_view eventId) const {
    const auto entry = _sounds.find(eventId);
    if (entry == _sounds.end()) {
        return std::nullopt;
    }
    return entry->second;
}

std::vector<std::string> SoundCatalog::eventIds() const {
    std::vector<std::string> ids;
    ids.reserve(_sounds.size());
    for (const auto& entry : _sounds) {
        ids.push_back(entry.first);
    }
    return ids;  // deja triees : _sounds est un std::map.
}

void SoundCatalog::assign(const std::string& eventId, std::string file) {
    _sounds[eventId] = std::move(file);
}

}  // namespace hmi
