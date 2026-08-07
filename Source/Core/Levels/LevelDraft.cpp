#include "Core/Levels/LevelDraft.h"

#include <algorithm>
#include <utility>

#include "Core/Diagnostics/Assert.h"
#include "Core/Levels/LevelWriter.h"

namespace core {

LevelDraft::LevelDraft(std::string name, TileMap tileMap)
    : _name(std::move(name)), _tileMap(std::move(tileMap)) {}

LevelDraft LevelDraft::empty(std::string name, int width, int height) {
    return LevelDraft(std::move(name), TileMap(width, height));
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

    const GridPosition position{column, row};
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
    const GridPosition position{column, row};
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
    const GridPosition position{column, row};
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
    PROJECTGAMING_ASSERT(switchTile == TileType::Switch || switchTile == TileType::PressurePlate,
                         "linkMechanism : la position source ne porte pas de declencheur "
                         "(interrupteur ou plaque de pression)");
    const TileType targetTile = _tileMap.inBounds(targetPosition.column, targetPosition.row)
                                     ? _tileMap.tile(targetPosition.column, targetPosition.row)
                                     : TileType::Empty;
    PROJECTGAMING_ASSERT(targetTile == TileType::Door || targetTile == TileType::DangerSwitched,
                         "linkMechanism : la position cible ne porte pas de porte ni de danger "
                         "commute");

    pushUndo();
    // Retrait direct (sans passer par unlinkMechanism, qui empilerait un second snapshot) :
    // lier remplace une eventuelle liaison existante en une seule action undoable.
    _mechanisms.erase(std::remove_if(_mechanisms.begin(), _mechanisms.end(),
                                     [targetPosition](const Mechanism& mechanism) {
                                         return mechanism.doorPosition == targetPosition;
                                     }),
                      _mechanisms.end());
    _dangerLinks.erase(std::remove_if(_dangerLinks.begin(), _dangerLinks.end(),
                                      [targetPosition](const DangerLink& link) {
                                          return link.dangerPosition == targetPosition;
                                      }),
                       _dangerLinks.end());
    if (targetTile == TileType::Door) {
        _mechanisms.push_back(Mechanism{switchPosition, targetPosition});
    } else {
        _dangerLinks.push_back(DangerLink{switchPosition, targetPosition});
    }
}

void LevelDraft::unlinkMechanism(GridPosition targetPosition) {
    pushUndo();
    _mechanisms.erase(std::remove_if(_mechanisms.begin(), _mechanisms.end(),
                                     [targetPosition](const Mechanism& mechanism) {
                                         return mechanism.doorPosition == targetPosition;
                                     }),
                      _mechanisms.end());
    _dangerLinks.erase(std::remove_if(_dangerLinks.begin(), _dangerLinks.end(),
                                      [targetPosition](const DangerLink& link) {
                                          return link.dangerPosition == targetPosition;
                                      }),
                       _dangerLinks.end());
}

void LevelDraft::setMoverConfig(GridPosition position, DangerMoverAxis axis, int range) {
    PROJECTGAMING_ASSERT(_tileMap.inBounds(position.column, position.row) &&
                             _tileMap.tile(position.column, position.row) == TileType::DangerMover,
                         "setMoverConfig : la position ne porte pas un DangerMover");
    pushUndo();
    _moverConfigs.erase(std::remove_if(_moverConfigs.begin(), _moverConfigs.end(),
                                       [position](const DangerMoverConfig& config) {
                                           return config.startPosition == position;
                                       }),
                        _moverConfigs.end());
    _moverConfigs.push_back(DangerMoverConfig{position, axis, range});
}

void LevelDraft::setBlinkConfig(GridPosition position, int period, int phase,
                                int activeDuration) {
    PROJECTGAMING_ASSERT(_tileMap.inBounds(position.column, position.row) &&
                             _tileMap.tile(position.column, position.row) == TileType::DangerBlink,
                         "setBlinkConfig : la position ne porte pas un DangerBlink");
    pushUndo();
    _blinkConfigs.erase(std::remove_if(_blinkConfigs.begin(), _blinkConfigs.end(),
                                       [position](const DangerBlinkConfig& config) {
                                           return config.position == position;
                                       }),
                        _blinkConfigs.end());
    _blinkConfigs.push_back(DangerBlinkConfig{position, period, phase, activeDuration});
}

void LevelDraft::setTextureOverride(GridPosition position, std::string assetName) {
    pushUndo();
    _textureOverrides.erase(std::remove_if(_textureOverrides.begin(), _textureOverrides.end(),
                                           [position](const TileTextureOverride& override) {
                                               return override.position == position;
                                           }),
                            _textureOverrides.end());
    _textureOverrides.push_back(TileTextureOverride{position, std::move(assetName)});
}

void LevelDraft::removeTextureOverride(GridPosition position) {
    pushUndo();
    _textureOverrides.erase(std::remove_if(_textureOverrides.begin(), _textureOverrides.end(),
                                           [position](const TileTextureOverride& override) {
                                               return override.position == position;
                                           }),
                            _textureOverrides.end());
}

void LevelDraft::setBackground(std::optional<std::string> background) {
    pushUndo();
    _background = std::move(background);
}

void LevelDraft::setSkinSet(std::optional<std::string> skinSet) {
    pushUndo();
    _skinSet = std::move(skinSet);
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
    _mechanisms.erase(std::remove_if(_mechanisms.begin(), _mechanisms.end(),
                                     [this](const Mechanism& mechanism) {
                                         return !_tileMap.inBounds(mechanism.switchPosition.column,
                                                                    mechanism.switchPosition.row) ||
                                                !_tileMap.inBounds(mechanism.doorPosition.column,
                                                                    mechanism.doorPosition.row);
                                     }),
                      _mechanisms.end());
    _dangerLinks.erase(std::remove_if(_dangerLinks.begin(), _dangerLinks.end(),
                                      [this](const DangerLink& link) {
                                          return !_tileMap.inBounds(link.triggerPosition.column,
                                                                     link.triggerPosition.row) ||
                                                 !_tileMap.inBounds(link.dangerPosition.column,
                                                                     link.dangerPosition.row);
                                      }),
                       _dangerLinks.end());
    _moverConfigs.erase(std::remove_if(_moverConfigs.begin(), _moverConfigs.end(),
                                       [this](const DangerMoverConfig& config) {
                                           return !_tileMap.inBounds(
                                               config.startPosition.column,
                                               config.startPosition.row);
                                       }),
                        _moverConfigs.end());
    _blinkConfigs.erase(std::remove_if(_blinkConfigs.begin(), _blinkConfigs.end(),
                                       [this](const DangerBlinkConfig& config) {
                                           return !_tileMap.inBounds(config.position.column,
                                                                      config.position.row);
                                       }),
                        _blinkConfigs.end());
    _textureOverrides.erase(
        std::remove_if(_textureOverrides.begin(), _textureOverrides.end(),
                       [this](const TileTextureOverride& override) {
                           return !_tileMap.inBounds(override.position.column,
                                                       override.position.row);
                       }),
        _textureOverrides.end());
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
    return State{_name,        _tileMap,     _entry,        _exit,         _mechanisms,
                 _jumpBudget,  _dashBudget,  _dangerLinks,  _moverConfigs, _blinkConfigs,
                 _background,  _skinSet,     _textureOverrides};
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
}

void LevelDraft::pushUndo() {
    _undoHistory.push_back(snapshot());
    _redoHistory.clear();  // une nouvelle mutation invalide la branche de refaire
}

LevelLoadResult LevelDraft::toLevel() const {
    const std::string json = LevelWriter::buildJson(
        _name, _tileMap, _mechanisms, _jumpBudget, _dashBudget, _dangerLinks, _moverConfigs,
        _blinkConfigs, _background, _skinSet, _textureOverrides);
    return LevelLoader::loadFromString(json);
}

void LevelDraft::removeLinkedDataAt(GridPosition position, bool keepTextureOverride) {
    _mechanisms.erase(std::remove_if(_mechanisms.begin(), _mechanisms.end(),
                                     [position](const Mechanism& mechanism) {
                                         return mechanism.switchPosition == position ||
                                                mechanism.doorPosition == position;
                                     }),
                      _mechanisms.end());
    _dangerLinks.erase(std::remove_if(_dangerLinks.begin(), _dangerLinks.end(),
                                      [position](const DangerLink& link) {
                                          return link.triggerPosition == position ||
                                                 link.dangerPosition == position;
                                      }),
                       _dangerLinks.end());
    _moverConfigs.erase(std::remove_if(_moverConfigs.begin(), _moverConfigs.end(),
                                       [position](const DangerMoverConfig& config) {
                                           return config.startPosition == position;
                                       }),
                        _moverConfigs.end());
    _blinkConfigs.erase(std::remove_if(_blinkConfigs.begin(), _blinkConfigs.end(),
                                       [position](const DangerBlinkConfig& config) {
                                           return config.position == position;
                                       }),
                        _blinkConfigs.end());
    if (!keepTextureOverride) {
        _textureOverrides.erase(std::remove_if(_textureOverrides.begin(), _textureOverrides.end(),
                                               [position](const TileTextureOverride& override) {
                                                   return override.position == position;
                                               }),
                                _textureOverrides.end());
    }
}

}  // namespace core
