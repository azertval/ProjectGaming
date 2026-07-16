#include "HMI/Localization/Localization.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace hmi {

namespace {
/// @return @p text sans les espaces et tabulations de début et de fin.
[[nodiscard]] std::string_view trim(std::string_view text) noexcept {
    const auto isSpace = [](char character) {
        return character == ' ' || character == '\t' || character == '\r' || character == '\n';
    };
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && isSpace(text[begin])) {
        ++begin;
    }
    while (end > begin && isSpace(text[end - 1])) {
        --end;
    }
    return text.substr(begin, end - begin);
}
}  // namespace

/**
 * @brief Construit un catalogue vide.
 * @param directory Dossier contenant les fichiers `<langue>.lang`.
 */
Localization::Localization(std::filesystem::path directory) : _directory(std::move(directory)) {}

/**
 * @brief Analyse un contenu `clé = valeur` en table de traductions.
 * @param content Texte source (UTF-8).
 * @return La table clé → valeur.
 *
 * Chaque ligne est traitée indépendamment : une ligne vide ou commençant (après espaces) par
 * `#` est ignorée ; sinon, le **premier** `=` sépare la clé de la valeur, toutes deux
 * débarrassées de leurs espaces de bordure. Un `=` ultérieur appartient donc à la valeur.
 */
std::unordered_map<std::string, std::string> Localization::parseCatalog(std::string_view content) {
    std::unordered_map<std::string, std::string> strings;
    std::size_t position = 0;

    while (position <= content.size()) {
        const std::size_t newline = content.find('\n', position);
        const std::size_t lineEnd = newline == std::string_view::npos ? content.size() : newline;
        const std::string_view line = content.substr(position, lineEnd - position);
        position = lineEnd + 1;

        const std::string_view trimmed = trim(line);
        if (trimmed.empty() || trimmed.front() == '#') {
            if (newline == std::string_view::npos) {
                break;
            }
            continue;  // ligne vide ou commentaire
        }

        const std::size_t separator = trimmed.find('=');
        if (separator != std::string_view::npos) {
            const std::string_view key = trim(trimmed.substr(0, separator));
            const std::string_view value = trim(trimmed.substr(separator + 1));
            if (!key.empty()) {
                strings.insert_or_assign(std::string(key), std::string(value));
            }
        }

        if (newline == std::string_view::npos) {
            break;
        }
    }
    return strings;
}

/**
 * @brief Définit la langue par défaut (source de repli) et l'active dessus.
 * @param language Identifiant de langue.
 * @param strings  Table de traductions déjà analysée.
 */
void Localization::setDefaultCatalog(std::string language,
                                     std::unordered_map<std::string, std::string> strings) {
    _defaultLanguage = language;
    _defaultStrings = std::move(strings);
    // La langue active démarre sur la langue par défaut tant qu'aucune autre n'est chargée.
    _activeLanguage = std::move(language);
    _activeStrings = _defaultStrings;
}

/**
 * @brief Définit la langue active à partir d'une table déjà analysée.
 * @param language Identifiant de langue.
 * @param strings  Table de traductions déjà analysée.
 */
void Localization::setActiveCatalog(std::string language,
                                    std::unordered_map<std::string, std::string> strings) {
    _activeLanguage = std::move(language);
    _activeStrings = std::move(strings);
}

/**
 * @brief Charge la langue par défaut depuis `<directory>/<language>.lang` et l'active.
 * @param language Identifiant de langue.
 * @return true si le fichier a été lu ; false (récupérable) sinon.
 */
bool Localization::loadDefaultLanguage(const std::string& language) {
    std::string content;
    if (!readFile(_directory / (language + ".lang"), content)) {
        return false;
    }
    setDefaultCatalog(language, parseCatalog(content));
    return true;
}

/**
 * @brief Charge la langue active depuis `<directory>/<language>.lang`.
 * @param language Identifiant de langue.
 * @return true si le fichier a été lu ; false (récupérable) sinon — l'active est conservée.
 */
bool Localization::loadLanguage(const std::string& language) {
    std::string content;
    if (!readFile(_directory / (language + ".lang"), content)) {
        return false;  // langue absente : on garde la langue active en cours
    }
    setActiveCatalog(language, parseCatalog(content));
    return true;
}

/**
 * @brief Résout le texte associé à @p key.
 * @param key Clé de traduction.
 * @return La chaîne de la langue active, sinon de la langue par défaut, sinon la clé.
 *
 * Le repli garantit un affichage toujours défini : une clé oubliée dans une traduction
 * réapparaît dans la langue par défaut, et une clé inconnue s'affiche telle quelle (utile
 * pour repérer un oubli sans planter).
 */
std::string Localization::text(std::string_view key) const {
    const std::string lookup(key);
    if (const auto active = _activeStrings.find(lookup); active != _activeStrings.end()) {
        return active->second;
    }
    if (const auto fallback = _defaultStrings.find(lookup); fallback != _defaultStrings.end()) {
        return fallback->second;
    }
    return lookup;
}

/**
 * @brief Lit tout le contenu d'un fichier texte.
 * @param path Chemin du fichier.
 * @param out  Reçoit le contenu en cas de succès.
 * @return true si le fichier a pu être ouvert et lu.
 */
bool Localization::readFile(const std::filesystem::path& path, std::string& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

}  // namespace hmi
