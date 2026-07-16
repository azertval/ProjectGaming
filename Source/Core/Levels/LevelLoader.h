#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "Core/Levels/Level.h"

/**
 * @file Core/Levels/LevelLoader.h
 * @brief Chargement d'un niveau depuis le format JSON (liste de tuiles-objets).
 */

namespace core {

/**
 * @brief Résultat d'un chargement de niveau : soit un `Level`, soit une **erreur** décrite.
 *
 * En cas de succès, `level` contient le niveau et `error` est vide. En cas d'échec (récupérable,
 * `EX-NFR-040`), `level` est vide et `error` décrit le problème de façon exploitable.
 */
struct LevelLoadResult {
    std::optional<Level> level;
    std::string error;

    /// @return true si le chargement a réussi.
    [[nodiscard]] bool ok() const noexcept {
        return level.has_value();
    }
};

/**
 * @brief Charge un niveau au format **JSON** (objet `{name, width, height, tiles}` où `tiles`
 *        est une liste d'objets `{x, y, type, …}`), `EX-LVL-001`/`EX-LVL-003`.
 *
 * Le chargement ne lève **aucune exception** vers l'appelant : toute erreur (JSON malformé,
 * champ manquant, type de tuile inconnu, tuile hors bornes, liaison de mécanisme non résolue…)
 * est renvoyée dans le `LevelLoadResult`. La **validation métier** additionnelle (unicité de
 * l'entrée/sortie, positions en double) relève de la validation du niveau.
 */
class LevelLoader {
public:
    /// @brief Charge un niveau depuis une chaîne JSON. @param json Contenu JSON. @return Résultat.
    [[nodiscard]] static LevelLoadResult loadFromString(std::string_view json);

    /// @brief Charge un niveau depuis un fichier. @param path Chemin du fichier. @return Résultat.
    [[nodiscard]] static LevelLoadResult loadFromFile(const std::filesystem::path& path);
};

}  // namespace core
