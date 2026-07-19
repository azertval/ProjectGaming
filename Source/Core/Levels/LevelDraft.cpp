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
    return draft;
}

void LevelDraft::paintTile(int column, int row, TileType type) {
    if (type == TileType::Entry) {
        setEntry(column, row);  // pushUndo() vit dans setEntry (une seule capture par action)
        return;
    }
    if (type == TileType::Exit) {
        setExit(column, row);  // pushUndo() vit dans setExit
        return;
    }

    pushUndo();
    const GridPosition position{column, row};
    if (_entry && *_entry == position) {
        _entry.reset();
    }
    if (_exit && *_exit == position) {
        _exit.reset();
    }
    removeMechanismsAt(position);
    _tileMap.setTile(column, row, type);
}

void LevelDraft::setEntry(int column, int row) {
    pushUndo();
    const GridPosition position{column, row};
    if (_entry && *_entry != position) {
        _tileMap.setTile(_entry->column, _entry->row, TileType::Empty);
    }
    removeMechanismsAt(position);
    _tileMap.setTile(column, row, TileType::Entry);
    _entry = position;
}

void LevelDraft::setExit(int column, int row) {
    pushUndo();
    const GridPosition position{column, row};
    if (_exit && *_exit != position) {
        _tileMap.setTile(_exit->column, _exit->row, TileType::Empty);
    }
    removeMechanismsAt(position);
    _tileMap.setTile(column, row, TileType::Exit);
    _exit = position;
}

void LevelDraft::linkMechanism(GridPosition switchPosition, GridPosition doorPosition) {
    PROJECTGAMING_ASSERT(_tileMap.inBounds(switchPosition.column, switchPosition.row) &&
                             _tileMap.tile(switchPosition.column, switchPosition.row) ==
                                 TileType::Switch,
                         "linkMechanism : la position source ne porte pas d'interrupteur");
    PROJECTGAMING_ASSERT(_tileMap.inBounds(doorPosition.column, doorPosition.row) &&
                             _tileMap.tile(doorPosition.column, doorPosition.row) ==
                                 TileType::Door,
                         "linkMechanism : la position cible ne porte pas de porte");

    pushUndo();
    // Retrait direct (sans passer par unlinkMechanism, qui empilerait un second snapshot) :
    // lier remplace une eventuelle liaison existante en une seule action undoable.
    _mechanisms.erase(std::remove_if(_mechanisms.begin(), _mechanisms.end(),
                                     [doorPosition](const Mechanism& mechanism) {
                                         return mechanism.doorPosition == doorPosition;
                                     }),
                      _mechanisms.end());
    _mechanisms.push_back(Mechanism{switchPosition, doorPosition});
}

void LevelDraft::unlinkMechanism(GridPosition doorPosition) {
    pushUndo();
    _mechanisms.erase(std::remove_if(_mechanisms.begin(), _mechanisms.end(),
                                     [doorPosition](const Mechanism& mechanism) {
                                         return mechanism.doorPosition == doorPosition;
                                     }),
                      _mechanisms.end());
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
    return State{_name, _tileMap, _entry, _exit, _mechanisms, _jumpBudget, _dashBudget};
}

void LevelDraft::restore(State state) {
    _name = std::move(state.name);
    _tileMap = std::move(state.tileMap);
    _entry = state.entry;
    _exit = state.exit;
    _mechanisms = std::move(state.mechanisms);
    _jumpBudget = state.jumpBudget;
    _dashBudget = state.dashBudget;
}

void LevelDraft::pushUndo() {
    _undoHistory.push_back(snapshot());
    _redoHistory.clear();  // une nouvelle mutation invalide la branche de refaire
}

LevelLoadResult LevelDraft::toLevel() const {
    const std::string json =
        LevelWriter::buildJson(_name, _tileMap, _mechanisms, _jumpBudget, _dashBudget);
    return LevelLoader::loadFromString(json);
}

void LevelDraft::removeMechanismsAt(GridPosition position) {
    _mechanisms.erase(std::remove_if(_mechanisms.begin(), _mechanisms.end(),
                                     [position](const Mechanism& mechanism) {
                                         return mechanism.switchPosition == position ||
                                                mechanism.doorPosition == position;
                                     }),
                      _mechanisms.end());
}

}  // namespace core
