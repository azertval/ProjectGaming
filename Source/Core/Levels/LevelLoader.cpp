#include "Core/Levels/LevelLoader.h"

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

namespace core {

namespace {

// Construit un résultat d'échec avec un message.
[[nodiscard]] LevelLoadResult failure(std::string message) {
    return LevelLoadResult{std::nullopt, std::move(message)};
}

// Convertit un nom de type de tuile du fichier en TileType (nullopt si inconnu).
[[nodiscard]] std::optional<TileType> parseTileType(const std::string& name) {
    if (name == "empty") {
        return TileType::Empty;
    }
    if (name == "solid") {
        return TileType::Solid;
    }
    if (name == "danger") {
        return TileType::Danger;
    }
    if (name == "entry") {
        return TileType::Entry;
    }
    if (name == "exit") {
        return TileType::Exit;
    }
    if (name == "switch") {
        return TileType::Switch;
    }
    if (name == "door") {
        return TileType::Door;
    }
    return std::nullopt;
}

// Une porte lue, avec la référence (opensWith) à résoudre en position d'interrupteur.
struct DoorLink {
    GridPosition position;
    std::string opensWith;
};

}  // namespace

// Charge un niveau depuis une chaine JSON.
LevelLoadResult LevelLoader::loadFromString(std::string_view json) {
    try {
        const nlohmann::json root = nlohmann::json::parse(json);

        if (!root.contains("width") || !root.contains("height") || !root.contains("tiles")) {
            return failure("Champ obligatoire manquant (width, height ou tiles)");
        }
        if (!root.at("tiles").is_array()) {
            return failure("Le champ 'tiles' doit etre une liste");
        }

        const int width = root.at("width").get<int>();
        const int height = root.at("height").get<int>();
        if (width <= 0 || height <= 0) {
            return failure("Dimensions invalides (width et height doivent etre > 0)");
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

        // Chaque objet de 'tiles' place une tuile dans la grille.
        for (const nlohmann::json& tile : root.at("tiles")) {
            const int x = tile.at("x").get<int>();
            const int y = tile.at("y").get<int>();
            const std::string typeName = tile.at("type").get<std::string>();

            const std::optional<TileType> type = parseTileType(typeName);
            if (!type) {
                return failure("Type de tuile inconnu : " + typeName);
            }
            if (!map.inBounds(x, y)) {
                return failure("Tuile hors bornes en (" + std::to_string(x) + ", " +
                               std::to_string(y) + ")");
            }
            if (!occupiedPositions.emplace(x, y).second) {
                return failure("Deux tuiles a la meme position (" + std::to_string(x) + ", " +
                               std::to_string(y) + ")");
            }
            map.setTile(x, y, *type);

            if (*type == TileType::Entry) {
                entry = GridPosition{x, y};
                ++entryCount;
            } else if (*type == TileType::Exit) {
                exit = GridPosition{x, y};
                ++exitCount;
            } else if (*type == TileType::Switch) {
                const std::string id = tile.value("id", std::string{});
                if (id.empty()) {
                    return failure("Interrupteur sans 'id' en (" + std::to_string(x) + ", " +
                                   std::to_string(y) + ")");
                }
                if (!switchesById.emplace(id, GridPosition{x, y}).second) {
                    return failure("Identifiant d'interrupteur en double : " + id);
                }
            } else if (*type == TileType::Door) {
                doors.push_back(
                    DoorLink{GridPosition{x, y}, tile.value("opensWith", std::string{})});
            }
        }

        // Validation : exactement une entrée et une sortie (EX-LVL-004).
        if (entryCount == 0) {
            return failure("Niveau sans entree (aucune tuile 'entry')");
        }
        if (entryCount > 1) {
            return failure("Plusieurs entrees dans le niveau (une seule attendue)");
        }
        if (exitCount == 0) {
            return failure("Niveau sans sortie (aucune tuile 'exit')");
        }
        if (exitCount > 1) {
            return failure("Plusieurs sorties dans le niveau (une seule attendue)");
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
                return failure("Porte liee a un interrupteur inexistant : " + door.opensWith);
            }
            mechanisms.push_back(Mechanism{found->second, door.position});
        }

        return LevelLoadResult{Level(std::move(name), std::move(map), entry, exit,
                                     std::move(mechanisms), jumpBudget, dashBudget),
                               {}};
    } catch (const nlohmann::json::exception& error) {
        return failure(std::string("JSON invalide : ") + error.what());
    }
}

// Charge un niveau depuis un fichier (lecture binaire puis delegation a loadFromString).
LevelLoadResult LevelLoader::loadFromFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return failure("Fichier de niveau introuvable : " + path.string());
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return loadFromString(buffer.str());
}

}  // namespace core
