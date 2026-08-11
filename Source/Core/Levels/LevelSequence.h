#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/**
 * @file Core/Levels/LevelSequence.h
 * @brief Séquence de niveaux jouée en partie réelle, comme **donnée de contenu** (`EX-LVL-013`) --
 *        un fichier JSON à côté des niveaux, plus un littéral C++ dans `Source/HMI`.
 */

namespace core {

/// Catégorie d'échec de chargement/validation d'une séquence (même rôle que
/// `LevelValidationError` pour `LevelLoader`, `EX-NFR-040`).
enum class LevelSequenceError {
    None,                      ///< Pas d'erreur (chargement réussi).
    FileNotFound,              ///< Fichier de séquence introuvable sur disque.
    ParseError,                ///< JSON malformé ou champ obligatoire manquant/mal typé.
    EmptySequence,             ///< La liste `levels` est vide.
    MissingLevelFile,          ///< Un niveau référencé est introuvable sur disque.
    UnsupportedFormatVersion,  ///< `"version"` du fichier supérieure à celle gérée (`EX-LVL-005`).
};

/// Version courante du format de séquence JSON (`EX-LVL-005`), indépendante de
/// `core::kLevelFormatVersion` (fichier distinct, cycle de vie distinct).
inline constexpr int kLevelSequenceFormatVersion = 1;

/**
 * @brief Séquence de niveaux chargée : ordre exact du fichier, plus une clé de titre traduisible.
 *
 * `levels` contient des **noms de fichiers** (ex. `"demo-saut.json"`), pas des chemins complets :
 * résolus par l'appelant relativement au dossier où il place ses niveaux (`hmi::executableDirectory
 * () / "Levels"` aujourd'hui) -- `Core` ne connaît pas ce dossier (`EX-NFR-011`), il ne fait que
 * vérifier, au chargement depuis un fichier, que chaque entrée existe **à côté du fichier de
 * séquence lui-même** (c'est là que vivent les niveaux, `Source/Elements/Levels`).
 */
struct LevelSequence {
    std::string titleKey;             ///< Clé de traduction du titre de la séquence (`EX-REN-033`).
    std::vector<std::string> levels;  ///< Noms de fichiers de niveaux, dans l'ordre de jeu.
};

/**
 * @brief Résultat d'un chargement de séquence : soit une `LevelSequence`, soit une **erreur**
 *        décrite -- même forme que `LevelLoadResult`.
 */
struct LevelSequenceLoadResult {
    std::optional<LevelSequence> sequence;
    std::string error;
    LevelSequenceError errorCode = LevelSequenceError::None;

    /// @return true si le chargement a réussi.
    [[nodiscard]] bool ok() const noexcept {
        return sequence.has_value();
    }
};

/**
 * @brief Charge une séquence de niveaux au format **JSON**
 *        (`{version, titleKey, levels: [...]}`), `EX-LVL-013`.
 *
 * Ne lève **aucune exception** vers l'appelant (`EX-NFR-040`) : toute erreur (JSON malformé, champ
 * manquant, liste vide, version non gérée) est renvoyée dans le `LevelSequenceLoadResult`.
 */
class LevelSequenceLoader {
public:
    /// @brief Charge une séquence depuis une chaîne JSON. Ne vérifie **pas** l'existence des
    ///        niveaux référencés (aucun dossier de base à résoudre depuis une chaîne) --
    ///        réservé à `loadFromFile`.
    [[nodiscard]] static LevelSequenceLoadResult loadFromString(std::string_view json);

    /// @brief Charge une séquence depuis un fichier, puis vérifie que chaque niveau référencé
    ///        existe dans le **même dossier** que le fichier de séquence.
    [[nodiscard]] static LevelSequenceLoadResult loadFromFile(const std::filesystem::path& path);
};

}  // namespace core
