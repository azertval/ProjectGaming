#include "Core/Levels/LevelWriter.h"

#include <fstream>
#include <map>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Levels/TileTypeName.h"

namespace core {

namespace {

// Vrai pour les tuiles "déclencheur" liables à une porte (interrupteur ou plaque de pression,
// EX-GP-020/EX-GP-025) : les deux partagent la même règle d'identifiant (LevelLoader.cpp).
[[nodiscard]] bool isTriggerType(TileType type) {
    return type == TileType::Switch || type == TileType::PressurePlate;
}

}  // namespace

std::string LevelWriter::toJsonString(const Level& level) {
    return buildJson(level.name(), level.tileMap(), level.mechanisms(), level.jumpBudget(),
                     level.dashBudget(), level.dangerLinks(), level.moverConfigs(),
                     level.blinkConfigs(), level.background(), level.skinSet());
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
                                   int dashBudget, const std::vector<DangerLink>& dangerLinks,
                                   const std::vector<DangerMoverConfig>& moverConfigs,
                                   const std::vector<DangerBlinkConfig>& blinkConfigs,
                                   const std::optional<std::string>& background,
                                   const std::optional<std::string>& skinSet) {
    nlohmann::json root;
    root["version"] = kLevelFormatVersion;
    root["name"] = name;
    root["width"] = tileMap.width();
    root["height"] = tileMap.height();
    if (jumpBudget != -1) {
        root["jumpBudget"] = jumpBudget;
    }
    if (dashBudget != -1) {
        root["dashBudget"] = dashBudget;
    }
    if (background) {
        root["background"] = *background;
    }
    if (skinSet) {
        root["skinSet"] = *skinSet;
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

    // Position de danger commuté -> identifiant du déclencheur qui l'active (EX-GP-052), même
    // schéma que doorOpensWith ci-dessus.
    std::map<std::pair<int, int>, std::string> dangerOpensWith;
    for (const DangerLink& link : dangerLinks) {
        const auto found = switchIds.find(
            std::make_pair(link.triggerPosition.column, link.triggerPosition.row));
        if (found != switchIds.end()) {
            dangerOpensWith.emplace(
                std::make_pair(link.dangerPosition.column, link.dangerPosition.row),
                found->second);
        }
    }

    // Position de danger mobile/temporisé -> configuration explicite, si posée (EX-GP-051/053).
    std::map<std::pair<int, int>, DangerMoverConfig> moverByPosition;
    for (const DangerMoverConfig& config : moverConfigs) {
        moverByPosition.emplace(
            std::make_pair(config.startPosition.column, config.startPosition.row), config);
    }
    std::map<std::pair<int, int>, DangerBlinkConfig> blinkByPosition;
    for (const DangerBlinkConfig& config : blinkConfigs) {
        blinkByPosition.emplace(std::make_pair(config.position.column, config.position.row),
                                config);
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
            } else if (type == TileType::DangerSwitched) {
                const auto found = dangerOpensWith.find(std::make_pair(column, row));
                if (found != dangerOpensWith.end()) {
                    tile["opensWith"] = found->second;
                }
            } else if (type == TileType::DangerMover) {
                const auto found = moverByPosition.find(std::make_pair(column, row));
                if (found != moverByPosition.end()) {
                    tile["axis"] = found->second.axis == DangerMoverAxis::Vertical ? "vertical"
                                                                                   : "horizontal";
                    tile["range"] = found->second.range;
                }
            } else if (type == TileType::DangerBlink) {
                const auto found = blinkByPosition.find(std::make_pair(column, row));
                if (found != blinkByPosition.end()) {
                    tile["period"] = found->second.period;
                    tile["phase"] = found->second.phase;
                    tile["activeDuration"] = found->second.activeDuration;
                }
            }
            tiles.push_back(std::move(tile));
        }
    }
    root["tiles"] = std::move(tiles);

    return root.dump();
}

}  // namespace core
