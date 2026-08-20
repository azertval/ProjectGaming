// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

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
    draft._platformConfigs = level.platformConfigs();
    draft._cameraFraming = level.cameraFraming();
    draft._airJumps = level.airJumps();
    draft._dashCharges = level.dashCharges();
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

    // Meme defaut que LevelLoader (EX-GP-026/EX-GP-051) : une plateforme ou un danger mobile
    // fraichement pose porte IMMEDIATEMENT sa configuration par defaut. Sans cette entree, l'outil
    // Parcours ne le trouve dans aucun des vecteurs qu'il parcourt (hmi::designatePathAt) tant que
    // le niveau n'a pas ete sauvegarde puis recharge (seul LevelLoader la creait jusqu'ici) : la
    // tuile reste invisible pour la selection, et son parcours impossible a commencer.
    if (type == TileType::MovingPlatform) {
        _platformConfigs.push_back(MovingPlatformConfig{.startPosition = position});
    } else if (type == TileType::DangerMover) {
        _moverConfigs.push_back(DangerMoverConfig{.startPosition = position});
    }
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

void LevelDraft::setPlatformConfig(GridPosition position, std::vector<GridPosition> waypoints,
                                   PlatformPathMode mode, float speed, int phase) {
    PROJECTGAMING_ASSERT(
        _tileMap.inBounds(position.column, position.row) &&
            _tileMap.tile(position.column, position.row) == TileType::MovingPlatform,
        "setPlatformConfig : la position ne porte pas une MovingPlatform");
    pushUndo();
    std::erase_if(_platformConfigs, [position](const MovingPlatformConfig& config) {
        return config.startPosition == position;
    });
    _platformConfigs.push_back(MovingPlatformConfig{.startPosition = position,
                                                    .waypoints = std::move(waypoints),
                                                    .mode = mode,
                                                    .speed = speed,
                                                    .phase = phase});
}

// Renvoie la configuration de la plateforme en `position`, en la creant aux valeurs de conception
// par defaut si elle n'existait pas encore. Empile UN snapshot (pushUndo) avant toute mutation :
// tous les mutateurs granulaires ci-dessous passent par ici, ce qui garantit un seul pas d'annu-
// lation par geste utilisateur meme quand le geste cree la configuration au passage.
MovingPlatformConfig& LevelDraft::platformConfigForEdit(GridPosition position) {
    PROJECTGAMING_ASSERT(
        _tileMap.inBounds(position.column, position.row) &&
            _tileMap.tile(position.column, position.row) == TileType::MovingPlatform,
        "platformConfigForEdit : la position ne porte pas une MovingPlatform");
    pushUndo();
    const auto found = std::find_if(_platformConfigs.begin(), _platformConfigs.end(),
                                    [position](const MovingPlatformConfig& config) {
                                        return config.startPosition == position;
                                    });
    if (found != _platformConfigs.end()) {
        return *found;
    }
    _platformConfigs.push_back(MovingPlatformConfig{.startPosition = position});
    return _platformConfigs.back();
}

void LevelDraft::addPlatformWaypoint(GridPosition position, GridPosition waypoint) {
    platformConfigForEdit(position).waypoints.push_back(waypoint);
}

void LevelDraft::insertPlatformWaypoint(GridPosition position, std::size_t index,
                                        GridPosition waypoint) {
    MovingPlatformConfig& config = platformConfigForEdit(position);
    if (index > config.waypoints.size()) {
        return;
    }
    config.waypoints.insert(config.waypoints.begin() + static_cast<std::ptrdiff_t>(index),
                            waypoint);
}

void LevelDraft::movePlatformWaypoint(GridPosition position, std::size_t index,
                                      GridPosition waypoint) {
    MovingPlatformConfig& config = platformConfigForEdit(position);
    if (index >= config.waypoints.size()) {
        return;
    }
    config.waypoints[index] = waypoint;
}

void LevelDraft::removePlatformWaypoint(GridPosition position, std::size_t index) {
    MovingPlatformConfig& config = platformConfigForEdit(position);
    if (index >= config.waypoints.size()) {
        return;
    }
    config.waypoints.erase(config.waypoints.begin() + static_cast<std::ptrdiff_t>(index));
}

void LevelDraft::setPlatformMode(GridPosition position, PlatformPathMode mode) {
    platformConfigForEdit(position).mode = mode;
}

void LevelDraft::setPlatformSpeed(GridPosition position, float speed) {
    platformConfigForEdit(position).speed = speed;
}

void LevelDraft::setPlatformPhase(GridPosition position, int phase) {
    platformConfigForEdit(position).phase = phase;
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

// --- Plans picturaux (EX-DEC-040, LOT-69) ---
//
// Discipline commune : un rang hors bornes ne fait RIEN et n'empile RIEN (sinon annuler un geste
// sans effet consommerait un pas d'historique), et une valeur refusee n'empile pas davantage. Le
// pushUndo() vient donc toujours APRES les validations, jamais avant.
//
// Le deplacement ne raisonne pas par couche, contrairement aux decors : le rang dans la liste EST
// l'ordre de superposition, la profondeur etant une propriete independante (EX-DEC-042).

void LevelDraft::addPlane(Plane plane) {
    pushUndo();
    _planes.push_back(std::move(plane));
}

void LevelDraft::removePlane(std::size_t index) {
    if (index >= _planes.size()) {
        return;
    }
    pushUndo();
    _planes.erase(_planes.begin() + static_cast<std::ptrdiff_t>(index));
}

bool LevelDraft::setPlaneDensity(std::size_t index, int pixelsPerUnit) {
    if (index >= _planes.size() || !isValidPlaneDensity(pixelsPerUnit)) {
        return false;
    }
    pushUndo();
    _planes[index].pixelsPerUnit = pixelsPerUnit;
    return true;
}

bool LevelDraft::setPlaneParallax(std::size_t index, float parallaxX, float parallaxY) {
    if (index >= _planes.size() || !std::isfinite(parallaxX) || !std::isfinite(parallaxY)) {
        return false;
    }
    pushUndo();
    _planes[index].parallaxX = parallaxX;
    _planes[index].parallaxY = parallaxY;
    return true;
}

bool LevelDraft::setPlaneOpacity(std::size_t index, float opacity) {
    // Comparaisons ecrites en positif pour rejeter aussi NaN, qui echoue toute comparaison.
    if (index >= _planes.size() || !(opacity >= 0.0F) || !(opacity <= 1.0F)) {
        return false;
    }
    pushUndo();
    _planes[index].opacity = opacity;
    return true;
}

bool LevelDraft::setPlaneDepth(std::size_t index, PlaneDepth depth) {
    if (index >= _planes.size()) {
        return false;
    }
    pushUndo();
    _planes[index].depth = depth;
    return true;
}

std::optional<std::size_t> LevelDraft::movePlaneForward(std::size_t index) {
    if (index >= _planes.size()) {
        return std::nullopt;
    }
    if (index + 1 == _planes.size()) {
        return index;  // deja le plus en avant : succes sans effet, rien d'empile.
    }
    pushUndo();
    std::swap(_planes[index], _planes[index + 1]);
    return index + 1;
}

std::optional<std::size_t> LevelDraft::movePlaneBackward(std::size_t index) {
    if (index >= _planes.size()) {
        return std::nullopt;
    }
    if (index == 0) {
        return index;  // deja le plus en arriere.
    }
    pushUndo();
    std::swap(_planes[index], _planes[index - 1]);
    return index - 1;
}

std::optional<std::size_t> LevelDraft::movePlaneToFront(std::size_t index) {
    if (index >= _planes.size()) {
        return std::nullopt;
    }
    const std::size_t last = _planes.size() - 1;
    if (index == last) {
        return index;
    }
    pushUndo();
    Plane moved = std::move(_planes[index]);
    _planes.erase(_planes.begin() + static_cast<std::ptrdiff_t>(index));
    _planes.push_back(std::move(moved));
    return last;
}

std::optional<std::size_t> LevelDraft::movePlaneToBack(std::size_t index) {
    if (index >= _planes.size()) {
        return std::nullopt;
    }
    if (index == 0) {
        return index;
    }
    pushUndo();
    Plane moved = std::move(_planes[index]);
    _planes.erase(_planes.begin() + static_cast<std::ptrdiff_t>(index));
    _planes.insert(_planes.begin(), std::move(moved));
    return 0;
}

void LevelDraft::setParallaxEnabled(bool enabled) {
    pushUndo();
    _parallaxEnabled = enabled;
}

void LevelDraft::setBackground(std::optional<std::string> background) {
    pushUndo();
    _background = std::move(background);
}

void LevelDraft::setSkinSet(std::optional<std::string> skinSet) {
    pushUndo();
    _skinSet = std::move(skinSet);
}

// Budgets et capacites : tous les quatre annulables comme les autres proprietes de niveau
// (fond, jeu de skins, cadrage). Les deux budgets ne l'etaient PAS avant ce lot -- seul manque
// dans la famille, corrige ici (LOT-67).
void LevelDraft::setJumpBudget(int jumpBudget) {
    pushUndo();
    _jumpBudget = jumpBudget;
}

void LevelDraft::setDashBudget(int dashBudget) {
    pushUndo();
    _dashBudget = dashBudget;
}

void LevelDraft::setAirJumps(std::optional<int> airJumps) {
    pushUndo();
    _airJumps = airJumps;
}

void LevelDraft::setDashCharges(std::optional<int> dashCharges) {
    pushUndo();
    _dashCharges = dashCharges;
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
    // Une route est indivisible : si son depart OU l'un de ses points sort du niveau retaille, la
    // configuration entiere part -- amputer la route donnerait un parcours silencieusement
    // different de celui dessine par le level designer.
    std::erase_if(_platformConfigs, [this](const MovingPlatformConfig& config) {
        return !_tileMap.inBounds(config.startPosition.column, config.startPosition.row) ||
               std::any_of(config.waypoints.begin(), config.waypoints.end(),
                           [this](const GridPosition& waypoint) {
                               return !_tileMap.inBounds(waypoint.column, waypoint.row);
                           });
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
        if (outOfBounds(config.startPosition) ||
            std::any_of(config.waypoints.begin(), config.waypoints.end(), outOfBounds)) {
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
                 .platformConfigs = _platformConfigs,
                 .cameraFraming = _cameraFraming,
                 .airJumps = _airJumps,
                 .dashCharges = _dashCharges,
                 .planes = _planes,
                 .parallaxEnabled = _parallaxEnabled};
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
    _platformConfigs = std::move(state.platformConfigs);
    _cameraFraming = state.cameraFraming;
    _airJumps = state.airJumps;
    _dashCharges = state.dashCharges;
    _planes = std::move(state.planes);
    _parallaxEnabled = state.parallaxEnabled;
}

void LevelDraft::pushUndo() {
    _undoHistory.push_back(snapshot());
    _redoHistory.clear();  // une nouvelle mutation invalide la branche de refaire
}

LevelLoadResult LevelDraft::toLevel() const {
    const std::string json = LevelWriter::buildJson(
        _name, _tileMap, _mechanisms, _jumpBudget, _dashBudget, _dangerLinks, _moverConfigs,
        _blinkConfigs, _background, _skinSet, _textureOverrides, _platformConfigs,
        _cameraFraming, _airJumps, _dashCharges, _planes, _parallaxEnabled);
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
