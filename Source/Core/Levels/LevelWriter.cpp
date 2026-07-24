#include "Core/Levels/LevelWriter.h"

#include <fstream>
#include <map>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

namespace core {

namespace {

// Convertit un TileType en son nom JSON (inverse de parseTileType, LevelLoader.cpp).
[[nodiscard]] std::string tileTypeName(TileType type) {
    switch (type) {
        case TileType::Empty:
            return "empty";  // jamais émis : les cases vides sont omises de 'tiles'.
        case TileType::Solid:
            return "solid";
        case TileType::Danger:
            return "danger";
        case TileType::Entry:
            return "entry";
        case TileType::Exit:
            return "exit";
        case TileType::Switch:
            return "switch";
        case TileType::Door:
            return "door";
        case TileType::PressurePlate:
            return "pressurePlate";
        case TileType::Block:
            return "block";
        case TileType::SlopeUpRight:
            return "slopeUpRight";
        case TileType::SlopeUpLeft:
            return "slopeUpLeft";
        case TileType::RoundedUpRight:
            return "roundedUpRight";
        case TileType::RoundedUpLeft:
            return "roundedUpLeft";
        case TileType::BlockHalf:
            return "blockHalf";
        case TileType::BlockQuarter:
            return "blockQuarter";
        case TileType::SlopeDownRight:
            return "slopeDownRight";
        case TileType::SlopeDownLeft:
            return "slopeDownLeft";
        case TileType::RoundedDownRight:
            return "roundedDownRight";
        case TileType::RoundedDownLeft:
            return "roundedDownLeft";
    }
    return "empty";
}

// Vrai pour les tuiles "déclencheur" liables à une porte (interrupteur ou plaque de pression,
// EX-GP-020/EX-GP-025) : les deux partagent la même règle d'identifiant (LevelLoader.cpp).
[[nodiscard]] bool isTriggerType(TileType type) {
    return type == TileType::Switch || type == TileType::PressurePlate;
}

}  // namespace

std::string LevelWriter::toJsonString(const Level& level) {
    return buildJson(level.name(), level.tileMap(), level.mechanisms(), level.jumpBudget(),
                     level.dashBudget());
}

bool LevelWriter::saveToFile(const Level& level, const std::filesystem::path& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    const std::string json = toJsonString(level);
    file.write(json.data(), static_cast<std::streamsize>(json.size()));
    return file.good();
}

std::string LevelWriter::buildJson(const std::string& name, const TileMap& tileMap,
                                   const std::vector<Mechanism>& mechanisms, int jumpBudget,
                                   int dashBudget) {
    nlohmann::json root;
    root["name"] = name;
    root["width"] = tileMap.width();
    root["height"] = tileMap.height();
    if (jumpBudget != -1) {
        root["jumpBudget"] = jumpBudget;
    }
    if (dashBudget != -1) {
        root["dashBudget"] = dashBudget;
    }

    // Identifiants de déclencheurs (interrupteur ou plaque de pression) régénérés de façon
    // déterministe (balayage ligne par ligne) : ni Level ni LevelDraft ne conservent les
    // identifiants du fichier d'origine.
    std::map<std::pair<int, int>, std::string> switchIds;
    int nextSwitchId = 0;
    for (int row = 0; row < tileMap.height(); ++row) {
        for (int column = 0; column < tileMap.width(); ++column) {
            if (isTriggerType(tileMap.tile(column, row))) {
                switchIds.emplace(std::make_pair(column, row),
                                  "s" + std::to_string(nextSwitchId++));
            }
        }
    }

    // Position de porte -> identifiant de l'interrupteur qui l'ouvre, d'après les mécanismes.
    std::map<std::pair<int, int>, std::string> doorOpensWith;
    for (const Mechanism& mechanism : mechanisms) {
        const auto found = switchIds.find(
            std::make_pair(mechanism.switchPosition.column, mechanism.switchPosition.row));
        if (found != switchIds.end()) {
            doorOpensWith.emplace(
                std::make_pair(mechanism.doorPosition.column, mechanism.doorPosition.row),
                found->second);
        }
    }

    nlohmann::json tiles = nlohmann::json::array();
    for (int row = 0; row < tileMap.height(); ++row) {
        for (int column = 0; column < tileMap.width(); ++column) {
            const TileType type = tileMap.tile(column, row);
            if (type == TileType::Empty) {
                continue;
            }
            nlohmann::json tile;
            tile["x"] = column;
            tile["y"] = row;
            tile["type"] = tileTypeName(type);
            if (isTriggerType(type)) {
                const auto found = switchIds.find(std::make_pair(column, row));
                if (found != switchIds.end()) {
                    tile["id"] = found->second;
                }
            } else if (type == TileType::Door) {
                const auto found = doorOpensWith.find(std::make_pair(column, row));
                if (found != doorOpensWith.end()) {
                    tile["opensWith"] = found->second;
                }
            }
            tiles.push_back(std::move(tile));
        }
    }
    root["tiles"] = std::move(tiles);

    return root.dump();
}

}  // namespace core
