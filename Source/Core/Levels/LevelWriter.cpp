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

// Vrai pour les tuiles "déclencheur" liables à une cible (interrupteur, plaque de pression ou
// clé, EX-GP-020/EX-GP-025/EX-GP-023) : toutes partagent la même règle d'identifiant
// (LevelLoader.cpp).
[[nodiscard]] bool isTriggerType(TileType type) {
    return type == TileType::Switch || type == TileType::PressurePlate || type == TileType::Key;
}

// Vrai pour les tuiles "cible" d'une liaison de mécanisme (porte classique ou porte verrouillée) :
// les deux se résolvent depuis le même vecteur `mechanisms` (LevelLoader.cpp), écrivent le même
// champ 'opensWith'.
[[nodiscard]] bool isDoorLikeType(TileType type) {
    return type == TileType::Door || type == TileType::LockedDoor;
}

// Nom JSON d'une DecorLayer (LOT-49, EX-DEC-002), symétrique à decorLayerFromName ci-dessous
// (LevelLoader.cpp).
[[nodiscard]] const char* decorLayerName(DecorLayer layer) {
    switch (layer) {
        case DecorLayer::Background:
            return "background";
        case DecorLayer::Decor:
            return "decor";
        case DecorLayer::Foreground:
            return "foreground";
    }
    return "decor";
}

}  // namespace

std::string LevelWriter::toJsonString(const Level& level) {
    return buildJson(level.name(), level.tileMap(), level.mechanisms(), level.jumpBudget(),
                     level.dashBudget(), level.dangerLinks(), level.moverConfigs(),
                     level.blinkConfigs(), level.background(), level.skinSet(),
                     level.textureOverrides(), level.decors(), level.platformConfigs());
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
                                   const std::optional<std::string>& skinSet,
                                   const std::vector<TileTextureOverride>& textureOverrides,
                                   const std::vector<Decor>& decors,
                                   const std::vector<MovingPlatformConfig>& platformConfigs) {
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
        const auto found =
            switchIds.find(std::make_pair(link.triggerPosition.column, link.triggerPosition.row));
        if (found != switchIds.end()) {
            dangerOpensWith.emplace(
                std::make_pair(link.dangerPosition.column, link.dangerPosition.row), found->second);
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
    // Position de plateforme mobile -> configuration explicite, si posee (EX-GP-026), meme schema
    // que moverByPosition/blinkByPosition ci-dessus.
    std::map<std::pair<int, int>, MovingPlatformConfig> platformByPosition;
    for (const MovingPlatformConfig& config : platformConfigs) {
        platformByPosition.emplace(
            std::make_pair(config.startPosition.column, config.startPosition.row), config);
    }

    // Position -> nom d'asset de la texture assignee par instance (EX-EDIT-043), independamment
    // du type de la tuile a cette position.
    std::map<std::pair<int, int>, std::string> textureOverrideByPosition;
    for (const TileTextureOverride& override : textureOverrides) {
        textureOverrideByPosition.emplace(
            std::make_pair(override.position.column, override.position.row), override.assetName);
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
            } else if (isDoorLikeType(type)) {
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
                    tile["axis"] =
                        found->second.axis == DangerMoverAxis::Vertical ? "vertical" : "horizontal";
                    tile["range"] = found->second.range;
                }
            } else if (type == TileType::DangerBlink) {
                const auto found = blinkByPosition.find(std::make_pair(column, row));
                if (found != blinkByPosition.end()) {
                    tile["period"] = found->second.period;
                    tile["phase"] = found->second.phase;
                    tile["activeDuration"] = found->second.activeDuration;
                }
            } else if (type == TileType::MovingPlatform) {
                const auto found = platformByPosition.find(std::make_pair(column, row));
                if (found != platformByPosition.end()) {
                    tile["endX"] = found->second.endPosition.column;
                    tile["endY"] = found->second.endPosition.row;
                    tile["speed"] = found->second.speed;
                    tile["phase"] = found->second.phase;
                }
            }
            // Texture assignee par instance (EX-EDIT-043) : independante du type, peut
            // accompagner n'importe quel champ ci-dessus.
            const auto overrideFound = textureOverrideByPosition.find(std::make_pair(column, row));
            if (overrideFound != textureOverrideByPosition.end()) {
                tile["texture"] = overrideFound->second;
            }
            tiles.push_back(std::move(tile));
        }
    }
    root["tiles"] = std::move(tiles);

    // Tableau racine optionnel "decors" (EX-DEC-001, LOT-49), omis si vide (retrocompatibilite,
    // EX-LVL-005) : l'ordre du vecteur est preserve (rang = superposition intra-couche).
    if (!decors.empty()) {
        nlohmann::json decorsJson = nlohmann::json::array();
        for (const Decor& decor : decors) {
            nlohmann::json entry;
            entry["asset"] = decor.assetName;
            entry["x"] = decor.position.x;
            entry["y"] = decor.position.y;
            if (decor.scale.x != 1.0F || decor.scale.y != 1.0F) {
                entry["scaleX"] = decor.scale.x;
                entry["scaleY"] = decor.scale.y;
            }
            if (decor.rotation != 0.0F) {
                entry["rotation"] = decor.rotation;
            }
            entry["layer"] = decorLayerName(decor.layer);
            if (decor.manipulable) {
                entry["manipulable"] = true;
            }
            decorsJson.push_back(std::move(entry));
        }
        root["decors"] = std::move(decorsJson);
    }

    return root.dump();
}

}  // namespace core
