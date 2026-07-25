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
 * @brief Catégorie d'échec de chargement/validation d'un niveau (LOT-15, `EX-EDIT-012`).
 *
 * Complète le message technique (`LevelLoadResult::error`) d'un code **programmatique**, pour
 * qu'un appelant (l'éditeur) puisse traduire une erreur en message non-codeur sans dépendre du
 * texte exact du message — une correspondance de sous-chaînes s'y casserait silencieusement au
 * moindre changement de formulation.
 */
enum class LevelValidationError {
    None,               ///< Pas d'erreur (chargement réussi).
    ParseError,         ///< JSON malformé, champ obligatoire manquant, ou de mauvais type.
    UnknownTileType,    ///< Type de tuile non reconnu.
    OutOfBounds,        ///< Tuile positionnée hors des dimensions déclarées.
    DuplicatePosition,  ///< Deux tuiles à la même position.
    MissingSwitchId,    ///< Interrupteur sans identifiant.
    DuplicateSwitchId,  ///< Deux interrupteurs partagent le même identifiant.
    InvalidEntryCount,  ///< Zéro ou plusieurs tuiles d'entrée (une seule attendue).
    InvalidExitCount,   ///< Zéro ou plusieurs tuiles de sortie (une seule attendue).
    UnresolvedMechanism,  ///< Porte ou danger commuté lié à un identifiant d'interrupteur
                          ///< inexistant.
    FileNotFound,         ///< Fichier de niveau introuvable sur disque.
};

/**
 * @brief Résultat d'un chargement de niveau : soit un `Level`, soit une **erreur** décrite.
 *
 * En cas de succès, `level` contient le niveau, `error` est vide et `errorCode` vaut `None`. En
 * cas d'échec (récupérable, `EX-NFR-040`), `level` est vide, `error` décrit le problème de façon
 * exploitable pour les journaux/tests, et `errorCode` catégorise l'échec pour un traitement
 * programmatique (traduction non-codeur, LOT-15).
 */
struct LevelLoadResult {
    std::optional<Level> level;
    std::string error;
    LevelValidationError errorCode = LevelValidationError::None;

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
