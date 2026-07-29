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
    return LevelLoadResult{std::nullopt, std::move(message), code};
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

// Un danger commuté lu (EX-GP-052), avec la référence (opensWith) à résoudre en position
// d'interrupteur — même schéma que DoorLink, tuile cible différente.
struct DangerSwitchedLink {
    GridPosition position;
    std::string opensWith;
};

// Convertit le champ optionnel "axis" d'un dangerMover ("horizontal"/"vertical") ; valeur de
// conception par défaut (horizontal) si absent ou non reconnu.
[[nodiscard]] DangerMoverAxis parseMoverAxis(const nlohmann::json& tile) {
    return tile.value("axis", std::string{"horizontal"}) == "vertical" ? DangerMoverAxis::Vertical
                                                                        : DangerMoverAxis::Horizontal;
}

}  // namespace

// Charge un niveau depuis une chaine JSON.
LevelLoadResult LevelLoader::loadFromString(std::string_view json) {
    try {
        const nlohmann::json root = nlohmann::json::parse(json);

        if (!root.contains("width") || !root.contains("height") || !root.contains("tiles")) {
            return failure("Champ obligatoire manquant (width, height ou tiles)",
                           LevelValidationError::ParseError);
        }
        if (!root.at("tiles").is_array()) {
            return failure("Le champ 'tiles' doit etre une liste", LevelValidationError::ParseError);
        }

        const int width = root.at("width").get<int>();
        const int height = root.at("height").get<int>();
        if (width <= 0 || height <= 0) {
            return failure("Dimensions invalides (width et height doivent etre > 0)",
                           LevelValidationError::ParseError);
        }

        std::string name = root.value("name", std::string{});
        // Budgets de mouvements optionnels (EX-GP-024) ; -1 = illimite.
        const int jumpBudget = root.value("jumpBudget", -1);
        const int dashBudget = root.value("dashBudget", -1);
        TileMap map(width, height);

        GridPosition entry{};
        GridPosition exit{};
        int entryCount = 0;
        int exitCount = 0;
        std::set<std::pair<int, int>> occupiedPositions;
        std::unordered_map<std::string, GridPosition> switchesById;
        std::vector<DoorLink> doors;
        std::vector<DangerSwitchedLink> switchedDangers;
        std::vector<DangerMoverConfig> moverConfigs;
        std::vector<DangerBlinkConfig> blinkConfigs;

        // Chaque objet de 'tiles' place une tuile dans la grille.
        for (const nlohmann::json& tile : root.at("tiles")) {
            const int x = tile.at("x").get<int>();
            const int y = tile.at("y").get<int>();
            const std::string typeName = tile.at("type").get<std::string>();

            const std::optional<TileType> type = parseTileType(typeName);
            if (!type) {
                return failure("Type de tuile inconnu : " + typeName,
                               LevelValidationError::UnknownTileType);
            }
            if (!map.inBounds(x, y)) {
                return failure("Tuile hors bornes en (" + std::to_string(x) + ", " +
                                   std::to_string(y) + ")",
                               LevelValidationError::OutOfBounds);
            }
            if (!occupiedPositions.emplace(x, y).second) {
                return failure("Deux tuiles a la meme position (" + std::to_string(x) + ", " +
                                   std::to_string(y) + ")",
                               LevelValidationError::DuplicatePosition);
            }
            map.setTile(x, y, *type);

            if (*type == TileType::Entry) {
                entry = GridPosition{x, y};
                ++entryCount;
            } else if (*type == TileType::Exit) {
                exit = GridPosition{x, y};
                ++exitCount;
            } else if (isTriggerType(*type)) {
                // Interrupteur ou plaque de pression (EX-GP-020/EX-GP-025) : meme regle
                // d'identifiant, partagee avec les portes via 'opensWith'.
                const std::string id = tile.value("id", std::string{});
                if (id.empty()) {
                    return failure("Declencheur sans 'id' en (" + std::to_string(x) + ", " +
                                       std::to_string(y) + ")",
                                   LevelValidationError::MissingSwitchId);
                }
                if (!switchesById.emplace(id, GridPosition{x, y}).second) {
                    return failure("Identifiant de declencheur en double : " + id,
                                   LevelValidationError::DuplicateSwitchId);
                }
            } else if (*type == TileType::Door) {
                doors.push_back(
                    DoorLink{GridPosition{x, y}, tile.value("opensWith", std::string{})});
            } else if (*type == TileType::DangerSwitched) {
                switchedDangers.push_back(DangerSwitchedLink{
                    GridPosition{x, y}, tile.value("opensWith", std::string{})});
            } else if (*type == TileType::DangerMover) {
                const DangerMoverAxis axis = parseMoverAxis(tile);
                const int range = tile.value("range", 2);
                const int farColumn = axis == DangerMoverAxis::Horizontal ? x + range : x;
                const int farRow = axis == DangerMoverAxis::Vertical ? y + range : y;
                if (range < 0 || !map.inBounds(farColumn, farRow)) {
                    return failure("Portee de danger mobile hors bornes en (" +
                                       std::to_string(x) + ", " + std::to_string(y) + ")",
                                   LevelValidationError::OutOfBounds);
                }
                moverConfigs.push_back(DangerMoverConfig{GridPosition{x, y}, axis, range});
            } else if (*type == TileType::DangerBlink) {
                blinkConfigs.push_back(DangerBlinkConfig{GridPosition{x, y},
                                                         tile.value("period", 120),
                                                         tile.value("phase", 0),
                                                         tile.value("activeDuration", 60)});
            }
        }

        // Validation : exactement une entrée et une sortie (EX-LVL-004).
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
            mechanisms.push_back(Mechanism{found->second, door.position});
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
            dangerLinks.push_back(DangerLink{found->second, danger.position});
        }

        LEVELS_LOG_TRACE("Niveau charge : '" + name + "' (" + std::to_string(width) + "x" +
                         std::to_string(height) + ", " + std::to_string(mechanisms.size()) +
                         " mecanisme(s))");
        return LevelLoadResult{Level(std::move(name), std::move(map), entry, exit,
                                     std::move(mechanisms), jumpBudget, dashBudget,
                                     std::move(dangerLinks), std::move(moverConfigs),
                                     std::move(blinkConfigs)),
                               {}};
    } catch (const nlohmann::json::exception& error) {
        return failure(std::string("JSON invalide : ") + error.what(), LevelValidationError::ParseError);
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
