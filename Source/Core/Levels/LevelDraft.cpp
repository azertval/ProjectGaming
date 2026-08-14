#include "Core/Levels/LevelDraft.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "Core/Diagnostics/Assert.h"
#include "Core/Levels/LevelWriter.h"

namespace core {

LevelDraft::LevelDraft(std::string name, TileMap tileMap)
    : _name(std::move(name)), _tileMap(std::move(tileMap)) {}

LevelDraft LevelDraft::empty(std::string name, int width, int height) {
    return {std::move(name), TileMap(width, height)};
}

LevelDraft LevelDraft::fromLevel(const Level& level) {
    LevelDraft draft(level.name(), level.tileMap());
    draft._entry = level.entry();
    draft._exit = level.exit();
    draft._mechanisms = level.mechanisms();
    draft._jumpBudget = level.jumpBudget();
    draft._dashBudget = level.dashBudget();
    draft._dangerLinks = level.dangerLinks();
    draft._moverConfigs = level.moverConfigs();
    draft._blinkConfigs = level.blinkConfigs();
    draft._background = level.background();
    draft._skinSet = level.skinSet();
    draft._textureOverrides = level.textureOverrides();
    draft._decors = level.decors();
    draft._platformConfigs = level.platformConfigs();
    draft._cameraFraming = level.cameraFraming();
    return draft;
}

void LevelDraft::paintTile(int column, int row, TileType type) {
    pushUndo();
    paintTileInternal(column, row, type);
}

void LevelDraft::paintRegion(int originColumn, int originRow,
                             const std::vector<std::vector<TileType>>& block) {
    if (block.empty()) {
        return;
    }
    pushUndo();
    for (std::size_t rowOffset = 0; rowOffset < block.size(); ++rowOffset) {
        const std::vector<TileType>& rowTiles = block[rowOffset];
        for (std::size_t columnOffset = 0; columnOffset < rowTiles.size(); ++columnOffset) {
            const int column = originColumn + static_cast<int>(columnOffset);
            const int row = originRow + static_cast<int>(rowOffset);
            if (!_tileMap.inBounds(column, row)) {
                continue;  // decoupe silencieuse aux bords, meme principe que resize()
            }
            paintTileInternal(column, row, rowTiles[columnOffset]);
        }
    }
}

void LevelDraft::paintTileInternal(int column, int row, TileType type) {
    if (type == TileType::Entry) {
        setEntryInternal(column, row);
        return;
    }
    if (type == TileType::Exit) {
        setExitInternal(column, row);
        return;
    }

    const GridPosition position{.column = column, .row = row};
    if (_entry && *_entry == position) {
        _entry.reset();
    }
    if (_exit && *_exit == position) {
        _exit.reset();
    }
    // Reposer le meme type ne doit pas effacer un habillage (EX-EDIT-043) : un coup de pinceau
    // involontaire sur une case deja du bon type effacerait sinon un override de texture.
    const bool sameType = _tileMap.tile(column, row) == type;
    removeLinkedDataAt(position, /*keepTextureOverride=*/sameType);
    _tileMap.setTile(column, row, type);
}

void LevelDraft::setEntry(int column, int row) {
    pushUndo();
    setEntryInternal(column, row);
}

void LevelDraft::setEntryInternal(int column, int row) {
    const GridPosition position{.column = column, .row = row};
    if (_entry && *_entry != position) {
        _tileMap.setTile(_entry->column, _entry->row, TileType::Empty);
    }
    removeLinkedDataAt(position);
    _tileMap.setTile(column, row, TileType::Entry);
    _entry = position;
}

void LevelDraft::setExit(int column, int row) {
    pushUndo();
    setExitInternal(column, row);
}

void LevelDraft::setExitInternal(int column, int row) {
    const GridPosition position{.column = column, .row = row};
    if (_exit && *_exit != position) {
        _tileMap.setTile(_exit->column, _exit->row, TileType::Empty);
    }
    removeLinkedDataAt(position);
    _tileMap.setTile(column, row, TileType::Exit);
    _exit = position;
}

void LevelDraft::linkMechanism(GridPosition switchPosition, GridPosition targetPosition) {
    // [[maybe_unused]] : switchTile ne sert qu'au PROJECTGAMING_ASSERT qui suit, lequel se
    // compile en ((void)0) en Release (NDEBUG) -- sans cet attribut, la variable serait "non
    // utilisee" (C4189, /WX) uniquement dans cette configuration, jamais en Debug.
    [[maybe_unused]] const TileType switchTile =
        _tileMap.inBounds(switchPosition.column, switchPosition.row)
            ? _tileMap.tile(switchPosition.column, switchPosition.row)
            : TileType::Empty;
    PROJECTGAMING_ASSERT(switchTile == TileType::Switch || switchTile == TileType::PressurePlate ||
                             switchTile == TileType::Key,
                         "linkMechanism : la position source ne porte pas de declencheur "
                         "(interrupteur, plaque de pression ou cle)");
    const TileType targetTile = _tileMap.inBounds(targetPosition.column, targetPosition.row)
                                    ? _tileMap.tile(targetPosition.column, targetPosition.row)
                                    : TileType::Empty;
    PROJECTGAMING_ASSERT(targetTile == TileType::Door || targetTile == TileType::DangerSwitched ||
                             targetTile == TileType::LockedDoor,
                         "linkMechanism : la position cible ne porte pas de porte, de porte "
                         "verrouillee ni de danger commute");

    pushUndo();
    // Retrait direct (sans passer par unlinkMechanism, qui empilerait un second snapshot) :
    // lier remplace une eventuelle liaison existante en une seule action undoable.
    std::erase_if(_mechanisms, [targetPosition](const Mechanism& mechanism) {
        return mechanism.doorPosition == targetPosition;
    });
    std::erase_if(_dangerLinks, [targetPosition](const DangerLink& link) {
        return link.dangerPosition == targetPosition;
    });
    if (targetTile == TileType::Door || targetTile == TileType::LockedDoor) {
        // Meme vecteur pour Door ET LockedDoor (aucune notion de liaison dupliquee, LOT-63
        // TACHE-02) : core::MechanismController distingue leur comportement a la construction.
        _mechanisms.push_back(
            Mechanism{.switchPosition = switchPosition, .doorPosition = targetPosition});
    } else {
        _dangerLinks.push_back(
            DangerLink{.triggerPosition = switchPosition, .dangerPosition = targetPosition});
    }
}

void LevelDraft::unlinkMechanism(GridPosition targetPosition) {
    pushUndo();
    std::erase_if(_mechanisms, [targetPosition](const Mechanism& mechanism) {
        return mechanism.doorPosition == targetPosition;
    });
    std::erase_if(_dangerLinks, [targetPosition](const DangerLink& link) {
        return link.dangerPosition == targetPosition;
    });
}

void LevelDraft::setMoverConfig(GridPosition position, DangerMoverAxis axis, int range) {
    PROJECTGAMING_ASSERT(_tileMap.inBounds(position.column, position.row) &&
                             _tileMap.tile(position.column, position.row) == TileType::DangerMover,
                         "setMoverConfig : la position ne porte pas un DangerMover");
    pushUndo();
    std::erase_if(_moverConfigs, [position](const DangerMoverConfig& config) {
        return config.startPosition == position;
    });
    _moverConfigs.push_back(
        DangerMoverConfig{.startPosition = position, .axis = axis, .range = range});
}

void LevelDraft::setBlinkConfig(GridPosition position, int period, int phase, int activeDuration) {
    PROJECTGAMING_ASSERT(_tileMap.inBounds(position.column, position.row) &&
                             _tileMap.tile(position.column, position.row) == TileType::DangerBlink,
                         "setBlinkConfig : la position ne porte pas un DangerBlink");
    pushUndo();
    std::erase_if(_blinkConfigs, [position](const DangerBlinkConfig& config) {
        return config.position == position;
    });
    _blinkConfigs.push_back(DangerBlinkConfig{
        .position = position, .period = period, .phase = phase, .activeDuration = activeDuration});
}

void LevelDraft::setPlatformConfig(GridPosition position, GridPosition endPosition, float speed,
                                   int phase) {
    PROJECTGAMING_ASSERT(
        _tileMap.inBounds(position.column, position.row) &&
            _tileMap.tile(position.column, position.row) == TileType::MovingPlatform,
        "setPlatformConfig : la position ne porte pas une MovingPlatform");
    pushUndo();
    std::erase_if(_platformConfigs, [position](const MovingPlatformConfig& config) {
        return config.startPosition == position;
    });
    _platformConfigs.push_back(MovingPlatformConfig{
        .startPosition = position, .endPosition = endPosition, .speed = speed, .phase = phase});
}

void LevelDraft::setTextureOverride(GridPosition position, std::string assetName) {
    pushUndo();
    std::erase_if(_textureOverrides, [position](const TileTextureOverride& override) {
        return override.position == position;
    });
    _textureOverrides.push_back(
        TileTextureOverride{.position = position, .assetName = std::move(assetName)});
}

void LevelDraft::removeTextureOverride(GridPosition position) {
    pushUndo();
    std::erase_if(_textureOverrides, [position](const TileTextureOverride& override) {
        return override.position == position;
    });
}

void LevelDraft::addDecor(Decor decor) {
    pushUndo();
    _decors.push_back(std::move(decor));
}

void LevelDraft::removeDecor(std::size_t index) {
    if (index >= _decors.size()) {
        return;
    }
    pushUndo();
    _decors.erase(_decors.begin() + static_cast<std::ptrdiff_t>(index));
}

bool LevelDraft::moveDecor(std::size_t index, Vector2 position) {
    if (index >= _decors.size()) {
        return false;
    }
    pushUndo();
    _decors[index].position = position;
    return true;
}

bool LevelDraft::resizeDecor(std::size_t index, Vector2 position, Vector2 scale) {
    if (index >= _decors.size() || scale.x <= 0.0F || scale.y <= 0.0F) {
        return false;
    }
    pushUndo();
    _decors[index].position = position;
    _decors[index].scale = scale;
    return true;
}

bool LevelDraft::rotateDecor(std::size_t index, float rotation) {
    if (index >= _decors.size()) {
        return false;
    }
    pushUndo();
    // Normalise dans [0, 2*pi[ : std::fmod conserve le signe de son operande, un reste negatif
    // est donc ramene dans l'intervalle par un tour complet supplementaire.
    constexpr float TWO_PI = 6.28318530717958647692F;
    float normalized = std::fmod(rotation, TWO_PI);
    if (normalized < 0.0F) {
        normalized += TWO_PI;
    }
    _decors[index].rotation = normalized;
    return true;
}

std::optional<std::size_t> LevelDraft::setDecorLayer(std::size_t index, DecorLayer layer) {
    if (index >= _decors.size()) {
        return std::nullopt;
    }
    if (_decors[index].layer == layer) {
        return index;  // deja la couche courante : succes sans effet, aucun deplacement.
    }
    pushUndo();
    Decor moved = std::move(_decors[index]);
    moved.layer = layer;
    _decors.erase(_decors.begin() + static_cast<std::ptrdiff_t>(index));
    _decors.push_back(std::move(moved));  // meme convention que addDecor : rang le plus eleve.
    return _decors.size() - 1;
}

std::optional<std::size_t> LevelDraft::bringDecorForward(std::size_t index) {
    if (index >= _decors.size()) {
        return std::nullopt;
    }
    const DecorLayer layer = _decors[index].layer;
    for (std::size_t candidate = index + 1; candidate < _decors.size(); ++candidate) {
        if (_decors[candidate].layer == layer) {
            pushUndo();
            std::swap(_decors[index], _decors[candidate]);
            return candidate;
        }
    }
    return index;  // deja le plus en avant de sa couche : succes sans effet.
}

std::optional<std::size_t> LevelDraft::sendDecorBackward(std::size_t index) {
    if (index >= _decors.size()) {
        return std::nullopt;
    }
    const DecorLayer layer = _decors[index].layer;
    for (std::size_t candidate = index; candidate > 0; --candidate) {
        if (_decors[candidate - 1].layer == layer) {
            pushUndo();
            std::swap(_decors[index], _decors[candidate - 1]);
            return candidate - 1;
        }
    }
    return index;  // deja le plus en arriere de sa couche : succes sans effet.
}

std::optional<std::size_t> LevelDraft::bringDecorToFront(std::size_t index) {
    if (index >= _decors.size()) {
        return std::nullopt;
    }
    const DecorLayer layer = _decors[index].layer;
    // Rang le plus eleve deja occupe par un decor de cette couche.
    std::size_t lastSameLayer = index;
    for (std::size_t candidate = index + 1; candidate < _decors.size(); ++candidate) {
        if (_decors[candidate].layer == layer) {
            lastSameLayer = candidate;
        }
    }
    if (lastSameLayer == index) {
        return index;  // deja le plus en avant de sa couche : succes sans effet.
    }
    pushUndo();
    Decor moved = std::move(_decors[index]);
    _decors.erase(_decors.begin() + static_cast<std::ptrdiff_t>(index));
    // lastSameLayer pointait dans le vecteur AVANT l'effacement : un cran plus loin que l'index
    // d'insertion voulu, puisque l'effacement a decale tout ce qui suivait `index` d'un rang.
    const std::size_t insertAt = lastSameLayer;
    _decors.insert(_decors.begin() + static_cast<std::ptrdiff_t>(insertAt), std::move(moved));
    return insertAt;
}

std::optional<std::size_t> LevelDraft::sendDecorToBack(std::size_t index) {
    if (index >= _decors.size()) {
        return std::nullopt;
    }
    const DecorLayer layer = _decors[index].layer;
    // Rang le plus bas deja occupe par un decor de cette couche.
    std::size_t firstSameLayer = index;
    for (std::size_t candidate = index; candidate > 0; --candidate) {
        if (_decors[candidate - 1].layer == layer) {
            firstSameLayer = candidate - 1;
        }
    }
    if (firstSameLayer == index) {
        return index;  // deja le plus en arriere de sa couche : succes sans effet.
    }
    pushUndo();
    Decor moved = std::move(_decors[index]);
    _decors.erase(_decors.begin() + static_cast<std::ptrdiff_t>(index));
    _decors.insert(_decors.begin() + static_cast<std::ptrdiff_t>(firstSameLayer), std::move(moved));
    return firstSameLayer;
}

void LevelDraft::setBackground(std::optional<std::string> background) {
    pushUndo();
    _background = std::move(background);
}

void LevelDraft::setSkinSet(std::optional<std::string> skinSet) {
    pushUndo();
    _skinSet = std::move(skinSet);
}

void LevelDraft::setCameraFraming(CameraFramingConfig cameraFraming) {
    pushUndo();
    _cameraFraming = cameraFraming;
}

void LevelDraft::addCameraZone(CameraZone zone) {
    pushUndo();
    _cameraFraming.zones.push_back(zone);
}

void LevelDraft::removeCameraZone(std::size_t index) {
    if (index >= _cameraFraming.zones.size()) {
        return;
    }
    pushUndo();
    _cameraFraming.zones.erase(_cameraFraming.zones.begin() + static_cast<std::ptrdiff_t>(index));
}

void LevelDraft::resize(int width, int height) {
    pushUndo();
    TileMap resized(width, height);
    const int copyWidth = (std::min)(width, _tileMap.width());
    const int copyHeight = (std::min)(height, _tileMap.height());
    for (int row = 0; row < copyHeight; ++row) {
        for (int column = 0; column < copyWidth; ++column) {
            resized.setTile(column, row, _tileMap.tile(column, row));
        }
    }
    _tileMap = std::move(resized);

    if (_entry && !_tileMap.inBounds(_entry->column, _entry->row)) {
        _entry.reset();
    }
    if (_exit && !_tileMap.inBounds(_exit->column, _exit->row)) {
        _exit.reset();
    }
    std::erase_if(_mechanisms, [this](const Mechanism& mechanism) {
        return !_tileMap.inBounds(mechanism.switchPosition.column, mechanism.switchPosition.row) ||
               !_tileMap.inBounds(mechanism.doorPosition.column, mechanism.doorPosition.row);
    });
    std::erase_if(_dangerLinks, [this](const DangerLink& link) {
        return !_tileMap.inBounds(link.triggerPosition.column, link.triggerPosition.row) ||
               !_tileMap.inBounds(link.dangerPosition.column, link.dangerPosition.row);
    });
    std::erase_if(_moverConfigs, [this](const DangerMoverConfig& config) {
        return !_tileMap.inBounds(config.startPosition.column, config.startPosition.row);
    });
    std::erase_if(_blinkConfigs, [this](const DangerBlinkConfig& config) {
        return !_tileMap.inBounds(config.position.column, config.position.row);
    });
    std::erase_if(_platformConfigs, [this](const MovingPlatformConfig& config) {
        return !_tileMap.inBounds(config.startPosition.column, config.startPosition.row) ||
               !_tileMap.inBounds(config.endPosition.column, config.endPosition.row);
    });
    std::erase_if(_textureOverrides, [this](const TileTextureOverride& override) {
        return !_tileMap.inBounds(override.position.column, override.position.row);
    });
    // _decors n'est volontairement PAS filtre : contrairement aux autres donnees annexes (keyees
    // par case), un decor libre peut legitimement deborder du niveau (une branche qui depasse) --
    // le tronquer serait une perte de travail (TACHE-01).
}

bool LevelDraft::wouldResizeDropContent(int width, int height) const noexcept {
    const auto outOfBounds = [width, height](GridPosition position) {
        return position.column < 0 || position.column >= width || position.row < 0 ||
               position.row >= height;
    };
    if (_entry && outOfBounds(*_entry)) {
        return true;
    }
    if (_exit && outOfBounds(*_exit)) {
        return true;
    }
    for (const Mechanism& mechanism : _mechanisms) {
        if (outOfBounds(mechanism.switchPosition) || outOfBounds(mechanism.doorPosition)) {
            return true;
        }
    }
    for (const DangerLink& link : _dangerLinks) {
        if (outOfBounds(link.triggerPosition) || outOfBounds(link.dangerPosition)) {
            return true;
        }
    }
    for (const DangerMoverConfig& config : _moverConfigs) {
        if (outOfBounds(config.startPosition)) {
            return true;
        }
    }
    for (const DangerBlinkConfig& config : _blinkConfigs) {
        if (outOfBounds(config.position)) {
            return true;
        }
    }
    for (const MovingPlatformConfig& config : _platformConfigs) {
        if (outOfBounds(config.startPosition) || outOfBounds(config.endPosition)) {
            return true;
        }
    }
    for (const TileTextureOverride& override : _textureOverrides) {
        if (outOfBounds(override.position)) {
            return true;
        }
    }
    return false;
}

bool LevelDraft::undo() {
    if (_undoHistory.empty()) {
        return false;
    }
    _redoHistory.push_back(snapshot());
    restore(std::move(_undoHistory.back()));
    _undoHistory.pop_back();
    return true;
}

bool LevelDraft::redo() {
    if (_redoHistory.empty()) {
        return false;
    }
    _undoHistory.push_back(snapshot());
    restore(std::move(_redoHistory.back()));
    _redoHistory.pop_back();
    return true;
}

LevelDraft::State LevelDraft::snapshot() const {
    return State{.name = _name,
                 .tileMap = _tileMap,
                 .entry = _entry,
                 .exit = _exit,
                 .mechanisms = _mechanisms,
                 .jumpBudget = _jumpBudget,
                 .dashBudget = _dashBudget,
                 .dangerLinks = _dangerLinks,
                 .moverConfigs = _moverConfigs,
                 .blinkConfigs = _blinkConfigs,
                 .background = _background,
                 .skinSet = _skinSet,
                 .textureOverrides = _textureOverrides,
                 .decors = _decors,
                 .platformConfigs = _platformConfigs,
                 .cameraFraming = _cameraFraming};
}

void LevelDraft::restore(State state) {
    _name = std::move(state.name);
    _tileMap = std::move(state.tileMap);
    _entry = state.entry;
    _exit = state.exit;
    _mechanisms = std::move(state.mechanisms);
    _jumpBudget = state.jumpBudget;
    _dashBudget = state.dashBudget;
    _dangerLinks = std::move(state.dangerLinks);
    _moverConfigs = std::move(state.moverConfigs);
    _blinkConfigs = std::move(state.blinkConfigs);
    _background = std::move(state.background);
    _skinSet = std::move(state.skinSet);
    _textureOverrides = std::move(state.textureOverrides);
    _decors = std::move(state.decors);
    _platformConfigs = std::move(state.platformConfigs);
    _cameraFraming = state.cameraFraming;
}

void LevelDraft::pushUndo() {
    _undoHistory.push_back(snapshot());
    _redoHistory.clear();  // une nouvelle mutation invalide la branche de refaire
}

LevelLoadResult LevelDraft::toLevel() const {
    const std::string json =
        LevelWriter::buildJson(_name, _tileMap, _mechanisms, _jumpBudget, _dashBudget, _dangerLinks,
                               _moverConfigs, _blinkConfigs, _background, _skinSet,
                               _textureOverrides, _decors, _platformConfigs, _cameraFraming);
    return LevelLoader::loadFromString(json);
}

void LevelDraft::removeLinkedDataAt(GridPosition position, bool keepTextureOverride) {
    std::erase_if(_mechanisms, [position](const Mechanism& mechanism) {
        return mechanism.switchPosition == position || mechanism.doorPosition == position;
    });
    std::erase_if(_dangerLinks, [position](const DangerLink& link) {
        return link.triggerPosition == position || link.dangerPosition == position;
    });
    std::erase_if(_moverConfigs, [position](const DangerMoverConfig& config) {
        return config.startPosition == position;
    });
    std::erase_if(_blinkConfigs, [position](const DangerBlinkConfig& config) {
        return config.position == position;
    });
    std::erase_if(_platformConfigs, [position](const MovingPlatformConfig& config) {
        return config.startPosition == position;
    });
    if (!keepTextureOverride) {
        std::erase_if(_textureOverrides, [position](const TileTextureOverride& override) {
            return override.position == position;
        });
    }
}

}  // namespace core
