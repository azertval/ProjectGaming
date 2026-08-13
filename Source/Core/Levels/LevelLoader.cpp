#include "Core/Levels/LevelLoader.h"

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "Core/Levels/LevelsLog.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Levels/TileTypeName.h"

namespace core {

namespace {

// Construit un résultat d'échec avec un message et un code categorise (LOT-15, EX-EDIT-012).
// Journalise systematiquement la raison ici (point unique) : chaque site d'appel n'a pas a le
// refaire, et un echec de chargement reste tracable meme hors du contexte HMI (tests, outillage).
[[nodiscard]] LevelLoadResult failure(std::string message, LevelValidationError code) {
    LEVELS_LOG_WARNING("Echec du chargement : " + message);
    return LevelLoadResult{.level = std::nullopt, .error = std::move(message), .errorCode = code};
}

// Vrai pour les tuiles "déclencheur" liables à une porte (interrupteur ou plaque de pression,
// EX-GP-020/EX-GP-025) : les deux partagent la même règle d'identifiant.
[[nodiscard]] bool isTriggerType(TileType type) {
    return type == TileType::Switch || type == TileType::PressurePlate;
}

// Une porte lue, avec la référence (opensWith) à résoudre en position d'interrupteur.
struct DoorLink {
    GridPosition position;
    std::string opensWith;
};

// Une porte verrouillée lue (EX-GP-023), même forme que DoorLink : la référence (opensWith) est
// résolue en position de clé plutôt que d'interrupteur, mais elle est OBLIGATOIRE ici (contraire
// a une porte classique, ou l'absence de mecanisme est une simple tuile) -- voir la resolution
// dediee plus bas.
struct LockedDoorLink {
    GridPosition position;
    std::string opensWith;
};

// Un danger commuté lu (EX-GP-052), avec la référence (opensWith) à résoudre en position
// d'interrupteur — même schéma que DoorLink, tuile cible différente.
struct DangerSwitchedLink {
    GridPosition position;
    std::string opensWith;
};

// Convertit le champ optionnel "axis" d'un dangerMover ("horizontal"/"vertical") ; valeur de
// conception par défaut (horizontal) si absent ou non reconnu.
[[nodiscard]] DangerMoverAxis parseMoverAxis(const nlohmann::json& tile) {
    return tile.value("axis", std::string{"horizontal"}) == "vertical"
               ? DangerMoverAxis::Vertical
               : DangerMoverAxis::Horizontal;
}

// Convertit le champ "layer" d'un décor ("background"/"decor"/"foreground") ; valeur de
// conception par défaut (Decor) si absent ou non reconnu -- symétrique à decorLayerName
// (LevelWriter.cpp).
[[nodiscard]] DecorLayer parseDecorLayer(const nlohmann::json& decor) {
    const std::string layer = decor.value("layer", std::string{"decor"});
    if (layer == "background") {
        return DecorLayer::Background;
    }
    if (layer == "foreground") {
        return DecorLayer::Foreground;
    }
    return DecorLayer::Decor;
}

// Accumulateurs remplis case par case par parseTile() ci-dessous -- toutes des références vers
// les variables locales de LevelLoader::loadFromString, un seul jeu construit pour tout le
// tableau `tiles`.
struct TileParseState {
    TileMap& map;
    GridPosition& entry;
    GridPosition& exit;
    int& entryCount;
    int& exitCount;
    std::set<std::pair<int, int>>& occupiedPositions;
    std::unordered_map<std::string, GridPosition>& switchesById;
    std::vector<DoorLink>& doors;
    std::vector<DangerSwitchedLink>& switchedDangers;
    std::unordered_map<std::string, GridPosition>& keysById;
    std::vector<LockedDoorLink>& lockedDoors;
    std::vector<DangerMoverConfig>& moverConfigs;
    std::vector<DangerBlinkConfig>& blinkConfigs;
    std::vector<TileTextureOverride>& textureOverrides;
    std::vector<MovingPlatformConfig>& platformConfigs;
};

// Traite UNE entrée du tableau `tiles` : pose la tuile dans la grille et alimente les
// accumulateurs de @p state (portes/dangers à résoudre, décompte entrée/sortie...). Extrait de
// LevelLoader::loadFromString ci-dessous (seule sa taille, pas son comportement) : std::nullopt
// en cas de succès, sinon l'échec à renvoyer IMMÉDIATEMENT -- aucun état partiel n'est jamais
// renvoyé avec succès.
[[nodiscard]] std::optional<LevelLoadResult> parseTile(const nlohmann::json& tile,
                                                       TileParseState& state) {
    const int x = tile.at("x").get<int>();
    const int y = tile.at("y").get<int>();
    const std::string typeName = tile.at("type").get<std::string>();

    const std::optional<TileType> type = parseTileType(typeName);
    if (!type) {
        return failure("Type de tuile inconnu : " + typeName,
                       LevelValidationError::UnknownTileType);
    }
    if (!state.map.inBounds(x, y)) {
        return failure(
            "Tuile hors bornes en (" + std::to_string(x) + ", " + std::to_string(y) + ")",
            LevelValidationError::OutOfBounds);
    }
    if (!state.occupiedPositions.emplace(x, y).second) {
        return failure(
            "Deux tuiles a la meme position (" + std::to_string(x) + ", " + std::to_string(y) + ")",
            LevelValidationError::DuplicatePosition);
    }
    state.map.setTile(x, y, *type);

    // Texture assignee par instance (EX-EDIT-043), independante du type de tuile : pas de liste
    // blanche (usage purement visuel, contrairement aux liens de mecanismes).
    if (tile.contains("texture")) {
        state.textureOverrides.push_back(
            TileTextureOverride{.position = GridPosition{.column = x, .row = y},
                                .assetName = tile.at("texture").get<std::string>()});
    }

    if (*type == TileType::Entry) {
        state.entry = GridPosition{.column = x, .row = y};
        ++state.entryCount;
    } else if (*type == TileType::Exit) {
        state.exit = GridPosition{.column = x, .row = y};
        ++state.exitCount;
    } else if (isTriggerType(*type)) {
        // Interrupteur ou plaque de pression (EX-GP-020/EX-GP-025) : meme regle d'identifiant,
        // partagee avec les portes via 'opensWith'.
        const std::string id = tile.value("id", std::string{});
        if (id.empty()) {
            return failure(
                "Declencheur sans 'id' en (" + std::to_string(x) + ", " + std::to_string(y) + ")",
                LevelValidationError::MissingSwitchId);
        }
        if (!state.switchesById.emplace(id, GridPosition{.column = x, .row = y}).second) {
            return failure("Identifiant de declencheur en double : " + id,
                           LevelValidationError::DuplicateSwitchId);
        }
    } else if (*type == TileType::Door) {
        state.doors.push_back(DoorLink{.position = GridPosition{.column = x, .row = y},
                                       .opensWith = tile.value("opensWith", std::string{})});
    } else if (*type == TileType::Key) {
        // Meme regle d'identifiant que Switch/PressurePlate (EX-GP-023), mais espace de liaison
        // distinct (keysById) : une cle doit obligatoirement etre liee, contrairement a un simple
        // declencheur, verifie plus bas.
        const std::string id = tile.value("id", std::string{});
        if (id.empty()) {
            return failure(
                "Cle sans 'id' en (" + std::to_string(x) + ", " + std::to_string(y) + ")",
                LevelValidationError::MissingSwitchId);
        }
        if (!state.keysById.emplace(id, GridPosition{.column = x, .row = y}).second) {
            return failure("Identifiant de cle en double : " + id,
                           LevelValidationError::DuplicateSwitchId);
        }
    } else if (*type == TileType::LockedDoor) {
        state.lockedDoors.push_back(
            LockedDoorLink{.position = GridPosition{.column = x, .row = y},
                           .opensWith = tile.value("opensWith", std::string{})});
    } else if (*type == TileType::DangerSwitched) {
        state.switchedDangers.push_back(
            DangerSwitchedLink{.position = GridPosition{.column = x, .row = y},
                               .opensWith = tile.value("opensWith", std::string{})});
    } else if (*type == TileType::DangerMover) {
        const DangerMoverAxis axis = parseMoverAxis(tile);
        const int range = tile.value("range", 2);
        const int farColumn = axis == DangerMoverAxis::Horizontal ? x + range : x;
        const int farRow = axis == DangerMoverAxis::Vertical ? y + range : y;
        if (range < 0 || !state.map.inBounds(farColumn, farRow)) {
            return failure("Portee de danger mobile hors bornes en (" + std::to_string(x) + ", " +
                               std::to_string(y) + ")",
                           LevelValidationError::OutOfBounds);
        }
        state.moverConfigs.push_back(DangerMoverConfig{
            .startPosition = GridPosition{.column = x, .row = y}, .axis = axis, .range = range});
    } else if (*type == TileType::DangerBlink) {
        state.blinkConfigs.push_back(
            DangerBlinkConfig{.position = GridPosition{.column = x, .row = y},
                              .period = tile.value("period", 120),
                              .phase = tile.value("phase", 0),
                              .activeDuration = tile.value("activeDuration", 60)});
    } else if (*type == TileType::MovingPlatform) {
        // Second point du parcours (EX-GP-026) : par defaut la meme case que le depart (parcours
        // nul, plateforme immobile) -- une erreur de configuration plutot qu'un rejet, coherent
        // avec le reste du format (defauts silencieux, EX-NFR-040).
        const int endColumn = tile.value("endX", x);
        const int endRow = tile.value("endY", y);
        if (!state.map.inBounds(endColumn, endRow)) {
            return failure("Point d'arrivee de plateforme mobile hors bornes en (" +
                               std::to_string(x) + ", " + std::to_string(y) + ")",
                           LevelValidationError::OutOfBounds);
        }
        state.platformConfigs.push_back(
            MovingPlatformConfig{.startPosition = GridPosition{.column = x, .row = y},
                                 .endPosition = GridPosition{.column = endColumn, .row = endRow},
                                 .speed = tile.value("speed", 2.0f),
                                 .phase = tile.value("phase", 0)});
    }
    return std::nullopt;
}

// Traite le tableau racine optionnel "decors" (EX-DEC-001, LOT-49) : absent = aucun décor
// (rétrocompatibilité, EX-LVL-005), auquel cas @p decors reste vide. L'ordre du tableau est
// préservé tel quel (rang = superposition intra-couche, TACHE-01), aucune validation d'existence
// de l'asset (EX-NFR-011 : Core ignore tout du dossier d'assets). Extrait de
// LevelLoader::loadFromString ci-dessous : std::nullopt en cas de succès, sinon l'échec à
// renvoyer immédiatement.
[[nodiscard]] std::optional<LevelLoadResult> parseDecors(const nlohmann::json& root,
                                                         std::vector<Decor>& decors) {
    if (!root.contains("decors")) {
        return std::nullopt;
    }
    if (!root.at("decors").is_array()) {
        return failure("Le champ 'decors' doit etre une liste", LevelValidationError::ParseError);
    }
    for (const nlohmann::json& decorJson : root.at("decors")) {
        Decor decor;
        decor.assetName = decorJson.at("asset").get<std::string>();
        decor.position.x = decorJson.at("x").get<float>();
        decor.position.y = decorJson.at("y").get<float>();
        decor.scale.x = decorJson.value("scaleX", 1.0F);
        decor.scale.y = decorJson.value("scaleY", 1.0F);
        decor.rotation = decorJson.value("rotation", 0.0F);
        decor.layer = parseDecorLayer(decorJson);
        decor.manipulable = decorJson.value("manipulable", false);
        decors.push_back(std::move(decor));
    }
    return std::nullopt;
}

// Valide les champs d'en-tête obligatoires (width/height/tiles, dimensions strictement positives,
// version de format gérée) et extrait @p width/@p height. Extrait de
// LevelLoader::loadFromString : std::nullopt en cas de succès, sinon l'échec à renvoyer
// immédiatement.
[[nodiscard]] std::optional<LevelLoadResult> parseHeader(const nlohmann::json& root, int& width,
                                                         int& height) {
    if (!root.contains("width") || !root.contains("height") || !root.contains("tiles")) {
        return failure("Champ obligatoire manquant (width, height ou tiles)",
                       LevelValidationError::ParseError);
    }
    if (!root.at("tiles").is_array()) {
        return failure("Le champ 'tiles' doit etre une liste", LevelValidationError::ParseError);
    }

    width = root.at("width").get<int>();
    height = root.at("height").get<int>();
    if (width <= 0 || height <= 0) {
        return failure("Dimensions invalides (width et height doivent etre > 0)",
                       LevelValidationError::ParseError);
    }

    // Version du format (EX-LVL-005) : absente = version initiale (0), sans erreur ni
    // avertissement (rétrocompatibilité des niveaux antérieurs à ce champ, LOT-44).
    const int version = root.value("version", 0);
    if (version > kLevelFormatVersion) {
        return failure("Version de format non geree : " + std::to_string(version) +
                           " (maximum gere : " + std::to_string(kLevelFormatVersion) + ")",
                       LevelValidationError::UnsupportedFormatVersion);
    }
    return std::nullopt;
}

// Exactement une entrée et une sortie (EX-LVL-004). Extrait de LevelLoader::loadFromString :
// std::nullopt en cas de succès, sinon l'échec à renvoyer immédiatement.
[[nodiscard]] std::optional<LevelLoadResult> validateEntryExitCounts(int entryCount,
                                                                     int exitCount) {
    if (entryCount == 0) {
        return failure("Niveau sans entree (aucune tuile 'entry')",
                       LevelValidationError::InvalidEntryCount);
    }
    if (entryCount > 1) {
        return failure("Plusieurs entrees dans le niveau (une seule attendue)",
                       LevelValidationError::InvalidEntryCount);
    }
    if (exitCount == 0) {
        return failure("Niveau sans sortie (aucune tuile 'exit')",
                       LevelValidationError::InvalidExitCount);
    }
    if (exitCount > 1) {
        return failure("Plusieurs sorties dans le niveau (une seule attendue)",
                       LevelValidationError::InvalidExitCount);
    }
    return std::nullopt;
}

}  // namespace

// Charge un niveau depuis une chaine JSON.
LevelLoadResult LevelLoader::loadFromString(std::string_view json) {
    try {
        const nlohmann::json root = nlohmann::json::parse(json);

        int width = 0;
        int height = 0;
        if (std::optional<LevelLoadResult> headerError = parseHeader(root, width, height)) {
            return std::move(*headerError);
        }

        std::string name = root.value("name", std::string{});
        // Budgets de mouvements optionnels (EX-GP-024) ; -1 = illimite.
        const int jumpBudget = root.value("jumpBudget", -1);
        const int dashBudget = root.value("dashBudget", -1);
        // Asset de fond et jeu de skins du niveau (EX-REN-044/EX-EDIT-024) : chaines optionnelles,
        // Core ignore tout du dossier d'assets et du mode de rendu (EX-NFR-011).
        std::optional<std::string> background;
        if (root.contains("background")) {
            background = root.at("background").get<std::string>();
        }
        std::optional<std::string> skinSet;
        if (root.contains("skinSet")) {
            skinSet = root.at("skinSet").get<std::string>();
        }
        TileMap map(width, height);

        GridPosition entry{};
        GridPosition exit{};
        int entryCount = 0;
        int exitCount = 0;
        std::set<std::pair<int, int>> occupiedPositions;
        std::unordered_map<std::string, GridPosition> switchesById;
        std::vector<DoorLink> doors;
        std::vector<DangerSwitchedLink> switchedDangers;
        std::unordered_map<std::string, GridPosition> keysById;
        std::vector<LockedDoorLink> lockedDoors;
        std::vector<DangerMoverConfig> moverConfigs;
        std::vector<DangerBlinkConfig> blinkConfigs;
        std::vector<TileTextureOverride> textureOverrides;
        std::vector<MovingPlatformConfig> platformConfigs;

        // Chaque objet de 'tiles' place une tuile dans la grille.
        TileParseState tileState{map,
                                 entry,
                                 exit,
                                 entryCount,
                                 exitCount,
                                 occupiedPositions,
                                 switchesById,
                                 doors,
                                 switchedDangers,
                                 keysById,
                                 lockedDoors,
                                 moverConfigs,
                                 blinkConfigs,
                                 textureOverrides,
                                 platformConfigs};
        for (const nlohmann::json& tile : root.at("tiles")) {
            std::optional<LevelLoadResult> tileError = parseTile(tile, tileState);
            if (tileError) {
                return std::move(*tileError);
            }
        }

        if (std::optional<LevelLoadResult> countError =
                validateEntryExitCounts(entryCount, exitCount)) {
            return std::move(*countError);
        }

        // Résout les liaisons interrupteur↔porte par identifiant. Une porte sans 'opensWith'
        // est une simple tuile (pas de mécanisme).
        std::vector<Mechanism> mechanisms;
        for (const DoorLink& door : doors) {
            if (door.opensWith.empty()) {
                continue;
            }
            const auto found = switchesById.find(door.opensWith);
            if (found == switchesById.end()) {
                return failure("Porte liee a un interrupteur inexistant : " + door.opensWith,
                               LevelValidationError::UnresolvedMechanism);
            }
            mechanisms.push_back(
                Mechanism{.switchPosition = found->second, .doorPosition = door.position});
        }

        // Résout les liaisons clé↔porte verrouillée (EX-GP-023), append à la MÊME liste que
        // ci-dessus (aucune nouvelle notion de liaison) : `MechanismController` distingue leur
        // comportement au type de la tuile déclencheur, pas à leur provenance dans ce vecteur.
        // Contrairement à une porte classique, le lien est OBLIGATOIRE dans les deux sens : une
        // porte verrouillée sans 'opensWith' (ou vers une clé inexistante) et une clé qu'aucune
        // porte ne referme sont toutes deux des niveaux invalides.
        std::set<std::string> usedKeyIds;
        for (const LockedDoorLink& lockedDoor : lockedDoors) {
            if (lockedDoor.opensWith.empty()) {
                return failure("Porte verrouillee sans cle liee en (" +
                                   std::to_string(lockedDoor.position.column) + ", " +
                                   std::to_string(lockedDoor.position.row) + ")",
                               LevelValidationError::UnresolvedMechanism);
            }
            const auto found = keysById.find(lockedDoor.opensWith);
            if (found == keysById.end()) {
                return failure(
                    "Porte verrouillee liee a une cle inexistante : " + lockedDoor.opensWith,
                    LevelValidationError::UnresolvedMechanism);
            }
            usedKeyIds.insert(lockedDoor.opensWith);
            mechanisms.push_back(
                Mechanism{.switchPosition = found->second, .doorPosition = lockedDoor.position});
        }
        for (const auto& keyEntry : keysById) {
            if (usedKeyIds.find(keyEntry.first) == usedKeyIds.end()) {
                return failure("Cle sans porte verrouillee liee : " + keyEntry.first,
                               LevelValidationError::UnresolvedMechanism);
            }
        }

        // Résout les liaisons interrupteur↔danger commuté (EX-GP-052), même règle que ci-dessus :
        // un dangerSwitched sans 'opensWith' est une simple tuile inerte (jamais mortelle).
        std::vector<DangerLink> dangerLinks;
        for (const DangerSwitchedLink& danger : switchedDangers) {
            if (danger.opensWith.empty()) {
                continue;
            }
            const auto found = switchesById.find(danger.opensWith);
            if (found == switchesById.end()) {
                return failure(
                    "Danger commute lie a un interrupteur inexistant : " + danger.opensWith,
                    LevelValidationError::UnresolvedMechanism);
            }
            dangerLinks.push_back(
                DangerLink{.triggerPosition = found->second, .dangerPosition = danger.position});
        }

        std::vector<Decor> decors;
        if (std::optional<LevelLoadResult> decorsError = parseDecors(root, decors)) {
            return std::move(*decorsError);
        }

        LEVELS_LOG_TRACE("Niveau charge : '" + name + "' (" + std::to_string(width) + "x" +
                         std::to_string(height) + ", " + std::to_string(mechanisms.size()) +
                         " mecanisme(s))");
        return LevelLoadResult{
            .level =
                Level(std::move(name), std::move(map), entry, exit, std::move(mechanisms),
                      jumpBudget, dashBudget, std::move(dangerLinks), std::move(moverConfigs),
                      std::move(blinkConfigs), std::move(background), std::move(skinSet),
                      std::move(textureOverrides), std::move(decors), std::move(platformConfigs)),
            .error = {}};
    } catch (const nlohmann::json::exception& error) {
        return failure(std::string("JSON invalide : ") + error.what(),
                       LevelValidationError::ParseError);
    }
}

// Charge un niveau depuis un fichier (lecture binaire puis delegation a loadFromString).
LevelLoadResult LevelLoader::loadFromFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return failure("Fichier de niveau introuvable : " + path.string(),
                       LevelValidationError::FileNotFound);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return loadFromString(buffer.str());
}

}  // namespace core
