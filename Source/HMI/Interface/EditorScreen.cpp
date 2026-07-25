#include "HMI/Interface/EditorScreen.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Levels/DangerGeometry.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"  // core::Mechanism
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/EditorLayout.h"
#include "HMI/Editor/EditorLog.h"
#include "HMI/Editor/LevelNameValidation.h"
#include "HMI/Editor/LevelSizeValidation.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/KeyName.h"
#include "HMI/Interface/GameScreen.h"
#include "HMI/Interface/RenderContext.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {

// Dimensions par defaut d'un brouillon vierge (comparables aux niveaux livres, ex. demo3/demo4).
constexpr int DEFAULT_WIDTH = 14;
constexpr int DEFAULT_HEIGHT = 8;

// Traduit un resultat de validation (LevelLoader) en phrase comprehensible par un non-codeur
// (EX-EDIT-007) : bascule sur le code d'erreur categorise (LOT-15), pas sur une recherche de
// sous-chaine dans le message technique — insensible a un changement de formulation dans
// LevelLoader.
[[nodiscard]] std::string describeValidationError(const core::LevelLoadResult& result) {
    switch (result.errorCode) {
        case core::LevelValidationError::InvalidEntryCount:
            return "Il manque une entree (ou il y en a plusieurs) : la grille doit porter "
                   "exactement une tuile Entree.";
        case core::LevelValidationError::InvalidExitCount:
            return "Il manque une sortie (ou il y en a plusieurs) : la grille doit porter "
                   "exactement une tuile Sortie.";
        case core::LevelValidationError::UnresolvedMechanism:
        case core::LevelValidationError::MissingSwitchId:
        case core::LevelValidationError::DuplicateSwitchId:
            return "Une porte n'est pas reliee a un interrupteur valide.";
        case core::LevelValidationError::OutOfBounds:
        case core::LevelValidationError::DuplicatePosition:
            return "La grille contient une tuile en dehors des bornes ou en double.";
        case core::LevelValidationError::UnknownTileType:
        case core::LevelValidationError::ParseError:
        case core::LevelValidationError::FileNotFound:
        case core::LevelValidationError::None:
            break;
    }
    return "Niveau invalide : " + result.error;
}

// Libelle court affiche sous chaque icone de la barre d'outils (decouvrabilite, EX-EDIT-015).
[[nodiscard]] std::string toolLabel(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return "Pinceau";
        case EditorTool::Rectangle:
            return "Rectangle";
        case EditorTool::Selection:
            return "Selection";
    }
    return "";
}

// Construit un quad texture a partir d'une region d'atlas et d'un rectangle (espace quelconque,
// pilote par la projection active du SpriteBatch), avec une teinte optionnelle.
SpriteQuad quadFor(const core::AtlasRegion& region, float x, float y, float width, float height,
                   const TextureAtlas& atlas, core::Color tint = core::Color{}) {
    const float atlasWidth = static_cast<float>(atlas.width());
    const float atlasHeight = static_cast<float>(atlas.height());
    SpriteQuad quad;
    quad.x = x;
    quad.y = y;
    quad.width = width;
    quad.height = height;
    quad.u0 = static_cast<float>(region.x) / atlasWidth;
    quad.v0 = static_cast<float>(region.y) / atlasHeight;
    quad.u1 = static_cast<float>(region.x + region.width) / atlasWidth;
    quad.v1 = static_cast<float>(region.y + region.height) / atlasHeight;
    quad.r = tint.r;
    quad.g = tint.g;
    quad.b = tint.b;
    quad.a = tint.a;
    return quad;
}

}  // namespace

// Construit l'editeur : affiche d'abord le selecteur nouveau/existant (EX-EDIT-001).
EditorScreen::EditorScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                           int viewportHeight, const EditorKeyBindings& editorBindings,
                           const GameKeyBindings& gameBindings,
                           const GamepadBindings& gamepadBindings)
    : _atlas(atlas),
      _batch(batch),
      _editorBindings(editorBindings),
      _gameBindings(gameBindings),
      _gamepadBindings(gamepadBindings),
      _viewportWidth(viewportWidth),
      _viewportHeight(viewportHeight),
      _picker(LevelPicker::forDirectory(hmi::executableDirectory() / "Levels")),
      _draft(core::LevelDraft::empty("Nouveau niveau", DEFAULT_WIDTH, DEFAULT_HEIGHT)),
      _camera(viewportWidth, viewportHeight) {
    // La palette est un accordéon à hauteur variable, pouvant déborder de la fenêtre (défilement,
    // LOT-27) : connaît la hauteur disponible dès la construction, avant tout rendu ou clic.
    _palette.setViewportHeight(static_cast<float>(_viewportHeight));
    _toolBar.relayout(_palette.bottom() + PANEL_SECTION_GAP);
    HMI_LOG_TRACE("EditorScreen cree (selecteur : " + std::to_string(_picker->choices().size()) +
                  " choix)");
}

// Definition necessaire ici (GameScreen complet), pour le unique_ptr<GameScreen> sur type
// incomplet dans l'en-tete.
EditorScreen::~EditorScreen() = default;

// Convertit une position souris en case de grille, si elle est dans les bornes du brouillon.
std::optional<core::GridPosition> EditorScreen::hoveredCell(float mouseX, float mouseY) const {
    const core::Vector2 world = _camera.screenToWorld(core::Vector2{mouseX, mouseY});
    const int column = static_cast<int>(std::floor(world.x));
    const int row = static_cast<int>(std::floor(world.y));
    if (!_draft.tileMap().inBounds(column, row)) {
        return std::nullopt;
    }
    return core::GridPosition{column, row};
}

// Vrai pour les tuiles "declencheur" liables a une porte ou un danger commute (interrupteur ou
// plaque de pression, EX-GP-020/EX-GP-025) : les deux se lient au meme geste (Maj+clic).
namespace {
bool isTriggerTile(core::TileType type) {
    return type == core::TileType::Switch || type == core::TileType::PressurePlate;
}

// Vrai pour les tuiles "cible" liables a un declencheur : une porte (EX-GP-021) ou un danger
// commute (EX-GP-052, LOT-31) -- meme geste de liaison pour les deux (core::LevelDraft::
// linkMechanism dispatche vers _mechanisms ou _dangerLinks selon le type reel de la cible).
bool isLinkTargetTile(core::TileType type) {
    return type == core::TileType::Door || type == core::TileType::DangerSwitched;
}
}  // namespace

// Traite un clic Maj+souris pour la liaison de mecanismes (bascule si la paire est deja liee).
void EditorScreen::handleLinkClick(float mouseX, float mouseY) {
    const std::optional<core::GridPosition> cell = hoveredCell(mouseX, mouseY);
    if (!cell) {
        return;
    }
    const core::TileType type = _draft.tileMap().tile(cell->column, cell->row);
    if (!isTriggerTile(type) && !isLinkTargetTile(type)) {
        return;  // rien a lier sur cette case
    }

    if (_pendingLink) {
        const core::TileType pendingType =
            _draft.tileMap().tile(_pendingLink->column, _pendingLink->row);
        if (!isTriggerTile(pendingType) && !isLinkTargetTile(pendingType)) {
            _pendingLink.reset();  // la case en attente a change de type entre-temps
        }
    }

    if (!_pendingLink) {
        _pendingLink = cell;
        return;
    }

    const core::TileType pendingType =
        _draft.tileMap().tile(_pendingLink->column, _pendingLink->row);
    if (isTriggerTile(pendingType) == isTriggerTile(type)) {
        // Deux cases de la meme categorie (deux declencheurs, quel que soit leur type exact, ou
        // deux cibles, porte ou danger commute) : on recommence avec la nouvelle case plutot que
        // de rester bloque.
        _pendingLink = cell;
        return;
    }

    const bool pendingIsTrigger = isTriggerTile(pendingType);
    const core::GridPosition switchPosition = pendingIsTrigger ? *_pendingLink : *cell;
    const core::GridPosition targetPosition = pendingIsTrigger ? *cell : *_pendingLink;
    const core::TileType targetType = pendingIsTrigger ? type : pendingType;

    const bool alreadyLinked =
        targetType == core::TileType::Door
            ? std::any_of(_draft.mechanisms().begin(), _draft.mechanisms().end(),
                          [&](const core::Mechanism& mechanism) {
                              return mechanism.switchPosition == switchPosition &&
                                     mechanism.doorPosition == targetPosition;
                          })
            : std::any_of(_draft.dangerLinks().begin(), _draft.dangerLinks().end(),
                          [&](const core::DangerLink& link) {
                              return link.triggerPosition == switchPosition &&
                                     link.dangerPosition == targetPosition;
                          });
    const std::string switchLabel = "(" + std::to_string(switchPosition.column) + ", " +
                                    std::to_string(switchPosition.row) + ")";
    const std::string targetLabel = "(" + std::to_string(targetPosition.column) + ", " +
                                    std::to_string(targetPosition.row) + ")";
    if (alreadyLinked) {
        _draft.unlinkMechanism(targetPosition);
        EDITOR_LOG_TRACE("Mecanisme delie : declencheur " + switchLabel + " / cible " +
                         targetLabel);
    } else {
        _draft.linkMechanism(switchPosition, targetPosition);
        EDITOR_LOG_TRACE("Mecanisme lie : declencheur " + switchLabel + " / cible " + targetLabel);
    }
    _dirty = true;
    _pendingLink.reset();
}

// Selecteur de niveau (nouveau/existant) ; puis clic palette -> selection ; clic/glisser sur la
// grille -> peinture ; Maj+clic -> liaison de mecanismes ; fleches -> redimensionnement ;
// Ctrl+S -> enregistrer ; P -> essai immediat ; Echap -> menu (ou fin de l'essai, sans quitter).
ScreenTransition EditorScreen::update(const InputState& input, float fixedDelta) {
    if (_picker) {
        if (input.keyPressed(Key::Escape)) {
            return ScreenTransition::switchTo(ScreenId::Menu);
        }
        const std::optional<int> confirmed =
            _picker->update(input, static_cast<float>(_viewportHeight));
        if (confirmed) {
            const LevelPicker::Choice& choice = _picker->choices()[*confirmed];
            if (choice.path) {
                const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(*choice.path);
                if (loaded.ok()) {
                    _draft = core::LevelDraft::fromLevel(*loaded.level);
                    _loadedFrom = choice.path;
                    _dirty = false;
                    _manualCamera = false;  // repart du cadrage automatique sur ce niveau
                    _selection.reset();     // une selection precedente peut deborder ce niveau
                    _picker.reset();
                } else {
                    // Fichier corrompu : message recuperable (EX-NFR-040), on reste au selecteur.
                    _statusMessage = "Impossible de charger ce niveau : " + loaded.error;
                    HMI_LOG_WARNING(_statusMessage);
                }
            } else {
                // Niveau vierge : le nom est demande avant d'entrer en edition (EX-EDIT-009), pas
                // fige a "Nouveau niveau" comme au LOT-14.
                _picker.reset();
                _textPrompt = TextInputField("", &isValidLevelName);
                _textPromptPurpose = TextPromptPurpose::CreateLevelName;
            }
        }
        return ScreenTransition::none();
    }

    if (_textPrompt) {
        _textPrompt->update(input);
        if (_textPrompt->confirmed()) {
            switch (_textPromptPurpose) {
                case TextPromptPurpose::CreateLevelName: {
                    const std::string name = trimLevelName(_textPrompt->text());
                    _draft = core::LevelDraft::empty(name, DEFAULT_WIDTH, DEFAULT_HEIGHT);
                    _loadedFrom.reset();
                    _dirty = false;
                    _manualCamera = false;  // repart du cadrage automatique sur ce niveau
                    _selection.reset();     // une selection precedente peut deborder ce niveau
                    break;
                }
                case TextPromptPurpose::RenameLevel:
                    _draft.setName(trimLevelName(_textPrompt->text()));
                    _dirty = true;  // renommer est une modification du brouillon comme une autre
                    break;
                case TextPromptPurpose::ResizeGrid: {
                    // Deja garanti analysable : isValidLevelSize (le validateur du champ) n'aurait
                    // pas confirme sinon. Meme chemin que les fleches (EX-EDIT-012 inclus).
                    const std::optional<std::pair<int, int>> size =
                        parseLevelSize(_textPrompt->text());
                    requestResize(size->first, size->second);
                    break;
                }
            }
            _textPrompt.reset();
            _statusMessage.clear();
        } else if (_textPrompt->cancelled()) {
            if (_textPromptPurpose == TextPromptPurpose::CreateLevelName) {
                // Aucun brouillon n'existe encore a ce stade : rien a perdre, retour au selecteur.
                _textPrompt.reset();
                _picker = LevelPicker::forDirectory(hmi::executableDirectory() / "Levels");
            } else {
                _textPrompt.reset();  // renommage/redimensionnement annule : rien ne change
            }
        }
        return ScreenTransition::none();
    }

    if (_playtest) {
        // Session de jeu interne : on lui delegue entierement la frame. Une transition (Echap,
        // fin de niveau -> retour menu) signale la fin de l'essai : on la consomme localement,
        // sans jamais quitter reellement l'ecran editeur (brouillon et historique intacts).
        if (_playtest->update(input, fixedDelta).kind != ScreenTransition::Kind::None) {
            _playtest.reset();
            _statusMessage.clear();
        }
        return ScreenTransition::none();
    }

    if (_pendingConfirmation) {
        // Bloque le reste de l'interaction tant que la confirmation est affichée : aucune
        // peinture/liaison/redimensionnement supplémentaire ne doit s'intercaler.
        if (input.keyPressed(Key::Enter)) {
            const ScreenTransition transition = _pendingConfirmation->onConfirm();
            _pendingConfirmation.reset();
            _statusMessage.clear();
            return transition;
        }
        if (input.keyPressed(Key::Escape)) {
            _pendingConfirmation.reset();
            _statusMessage.clear();
        }
        return ScreenTransition::none();
    }

    if (input.keyPressed(Key::Escape)) {
        if (_dirty) {
            _pendingConfirmation = PendingConfirmation{
                "Quitter sans enregistrer les modifications ? (Entree = oui, Echap = non)",
                [this]() { return ScreenTransition::switchTo(ScreenId::Menu); }};
            return ScreenTransition::none();
        }
        return ScreenTransition::switchTo(ScreenId::Menu);
    }

    if (input.keyDown(Key::Control) && input.keyPressed(_editorBindings.key(EditorAction::Save))) {
        saveDraft();
    }
    if (input.keyPressed(_editorBindings.key(EditorAction::Playtest))) {
        startPlaytest();
    }
    if (input.keyPressed(_editorBindings.key(EditorAction::Rename))) {
        // Renommage (EX-EDIT-009) : le champ est pre-rempli du nom courant ; annuler le laisse
        // inchange (traite par le bloc _textPrompt ci-dessus a la prochaine frame).
        _textPrompt = TextInputField(_draft.name(), &isValidLevelName);
        _textPromptPurpose = TextPromptPurpose::RenameLevel;
        return ScreenTransition::none();
    }
    if (input.keyDown(Key::Control) && input.keyPressed(Key::R)) {
        // Redimensionnement par saisie directe (EX-EDIT-017) : champ pre-rempli de la taille
        // courante au format "largeur x hauteur", accepte par le parseur qui le confirmera.
        const std::string currentSize = std::to_string(_draft.tileMap().width()) + "x" +
                                        std::to_string(_draft.tileMap().height());
        _textPrompt = TextInputField(currentSize, &isValidLevelSize);
        _textPromptPurpose = TextPromptPurpose::ResizeGrid;
        return ScreenTransition::none();
    }

    // Changement d'outil (EX-EDIT-014/015) : Tab fait defiler Pinceau -> Rectangle -> Selection ->
    // Pinceau (clic sur la barre : voir plus bas). Changer d'outil pendant un glisser en cours
    // l'annule (aucune application partielle).
    if (input.keyPressed(Key::Tab)) {
        _areaDragActive = false;
        _toolBar.selectNext();
    }

    // Aide des raccourcis (EX-EDIT-015) : bascule affichee/repliee.
    if (input.keyPressed(_editorBindings.key(EditorAction::ToggleHelp))) {
        _showHelp = !_showHelp;
    }

    // Grille de repere (EX-EDIT-015) : raccourci clavier, pas un bouton du panneau d'edition.
    if (input.keyPressed(_editorBindings.key(EditorAction::ToggleGrid))) {
        _showGridLines = !_showGridLines;
    }

    // Copier/coller de l'outil Selection (EX-EDIT-014) : Ctrl+C lit directement la grille (aucun
    // ajout Core necessaire cote copie) ; Ctrl+V colle via paintRegion (un seul undo).
    if (input.keyDown(Key::Control) && input.keyPressed(_editorBindings.key(EditorAction::Copy))) {
        if (_selection) {
            _clipboard.clear();
            for (int row = _selection->first.row; row <= _selection->second.row; ++row) {
                std::vector<core::TileType> rowTiles;
                for (int column = _selection->first.column; column <= _selection->second.column;
                     ++column) {
                    if (_draft.tileMap().inBounds(column, row)) {
                        rowTiles.push_back(_draft.tileMap().tile(column, row));
                    }
                }
                _clipboard.push_back(std::move(rowTiles));
            }
        }
    } else if (input.keyDown(Key::Control) &&
               input.keyPressed(_editorBindings.key(EditorAction::Paste))) {
        if (!_clipboard.empty()) {
            // Position souris de la frame courante (pas _mouseX/_mouseY, pas encore mis a jour a
            // ce point de la frame — voir le bloc camera plus bas).
            const core::GridPosition cell =
                clampedCell(static_cast<float>(input.mouseX()), static_cast<float>(input.mouseY()));
            _draft.paintRegion(cell.column, cell.row, _clipboard);
            _dirty = true;
        }
    }

    // Camera manuelle (EX-EDIT-013) : molette = zoom, glisser bouton droit = pan, "0" = retour au
    // cadrage automatique. _mouseX/_mouseY portent encore la position de la frame precedente ici :
    // le delta de glisser se calcule avant de les mettre a jour, plus bas.
    if (input.keyPressed(Key::D0)) {
        _manualCamera = false;
    }

    // Fenetrage du defilement de la palette (LOT-27) : synchronise avant tout clic/molette de
    // cette frame, un redimensionnement de fenetre pouvant changer le nombre de lignes visibles
    // sans qu'aucun clic ni depliage n'ait eu lieu.
    _palette.setViewportHeight(static_cast<float>(_viewportHeight));
    _toolBar.relayout(_palette.bottom() + PANEL_SECTION_GAP);

    const int wheel = input.wheelDelta();
    if (wheel != 0 && static_cast<float>(input.mouseX()) < PANEL_WIDTH) {
        // Molette au-dessus du panneau lateral (LOT-27, EX-EDIT-018) : fait defiler la palette
        // (accordeon pouvant depasser la hauteur de fenetre une fois largement depliee), plutot
        // que de zoomer la camera comme au-dessus du canevas.
        _palette.scroll(wheel);
        _toolBar.relayout(_palette.bottom() + PANEL_SECTION_GAP);
    } else if (wheel != 0) {
        constexpr float WHEEL_NOTCH = 120.0f;  // WHEEL_DELTA Win32 : un cran de molette standard.
        constexpr float MIN_VISIBLE_TILES = 4.0f;  // zoom max : au moins 4 cases visibles.
        const float canvasWidth =
            (std::max)(1.0f, static_cast<float>(_viewportWidth) - PANEL_WIDTH);
        const float viewportHeight = static_cast<float>(_viewportHeight);
        const int width = _draft.tileMap().width();
        const int height = _draft.tileMap().height();
        // Zoom minimal : jamais moins que le cadrage automatique (rien a voir au-dela du niveau,
        // meme formule que renderGrid, LOT-16 fitZoom) ; zoom maximal : au moins MIN_VISIBLE_TILES
        // cases sur le plus petit axe (precision suffisante pour poser un bloc, pas plus).
        const float minZoom =
            Camera2D::fitZoom(canvasWidth, viewportHeight, static_cast<float>(width),
                              static_cast<float>(height), 0.85f);
        const float maxZoom =
            (std::max)(minZoom, std::floor((std::min)(canvasWidth, viewportHeight) /
                                           (MIN_VISIBLE_TILES * Camera2D::PIXELS_PER_UNIT)));
        _cameraZoom = std::clamp(_cameraZoom + std::round(static_cast<float>(wheel) / WHEEL_NOTCH),
                                 minZoom, maxZoom);
        _manualCamera = true;
    }
    const float newMouseX = static_cast<float>(input.mouseX());
    const float newMouseY = static_cast<float>(input.mouseY());
    if (input.mouseButtonDown(MouseButton::Right) &&
        !input.mouseButtonPressed(MouseButton::Right)) {
        // Glisser en cours (pas le tout premier frame du clic, pour eviter un saut initial).
        const float scale = Camera2D::PIXELS_PER_UNIT * _cameraZoom;
        _cameraCenter.x -= (newMouseX - _mouseX) / scale;
        _cameraCenter.y -= (newMouseY - _mouseY) / scale;
        _manualCamera = true;
    }
    _mouseX = newMouseX;
    _mouseY = newMouseY;

    if (input.mouseButtonPressed(MouseButton::Left)) {
        const bool paletteHit = _palette.handleClick(_mouseX, _mouseY);
        // La palette peut venir de replier/deplier une categorie ou un sous-groupe (LOT-27), donc
        // changer de hauteur : repositionne la barre d'outils AVANT son propre test de clic, pour
        // qu'un second clic dans la meme frame (rare) et le rendu qui suit visent la bonne case.
        _toolBar.relayout(_palette.bottom() + PANEL_SECTION_GAP);
        if (paletteHit || _toolBar.handleClick(_mouseX, _mouseY)) {
            // La palette et la barre d'outils, dans le panneau lateral, sont prioritaires.
            _paintingDrag = false;
            _areaDragActive = false;
        } else if (_mouseX < PANEL_WIDTH) {
            // Reste du panneau (zone vide, sous/entre les boutons) : rien a faire — surtout pas
            // peindre une case de la grille qui serait visuellement cachee par le panneau.
            _paintingDrag = false;
            _areaDragActive = false;
        } else if (input.keyDown(Key::Shift)) {
            // La liaison de mecanismes reste disponible quel que soit l'outil actif (EX-EDIT-014).
            handleLinkClick(_mouseX, _mouseY);
            _paintingDrag = false;
            _areaDragActive = false;
        } else if (_toolBar.selected() == EditorTool::Paint) {
            _paintingDrag = true;
        } else {
            _areaDragActive = true;
            _areaDragStart = clampedCell(_mouseX, _mouseY);
        }
    }

    if (_paintingDrag && input.mouseButtonDown(MouseButton::Left)) {
        if (const std::optional<core::GridPosition> cell = hoveredCell(_mouseX, _mouseY)) {
            _draft.paintTile(cell->column, cell->row, _palette.selected());
            _dirty = true;
        }
    }

    if (_areaDragActive && input.mouseButtonReleased(MouseButton::Left)) {
        // Relachement d'un glisser Rectangle/Selection : applique (Rectangle) ou memorise
        // (Selection) la zone entre le point de depart et la case courante, bornee a la grille.
        _areaDragActive = false;
        const core::GridPosition end = clampedCell(_mouseX, _mouseY);
        const int minColumn = (std::min)(_areaDragStart.column, end.column);
        const int maxColumn = (std::max)(_areaDragStart.column, end.column);
        const int minRow = (std::min)(_areaDragStart.row, end.row);
        const int maxRow = (std::max)(_areaDragStart.row, end.row);
        if (_toolBar.selected() == EditorTool::Rectangle) {
            const std::vector<core::TileType> rowTiles(
                static_cast<std::size_t>(maxColumn - minColumn + 1), _palette.selected());
            const std::vector<std::vector<core::TileType>> block(
                static_cast<std::size_t>(maxRow - minRow + 1), rowTiles);
            _draft.paintRegion(minColumn, minRow, block);
            _dirty = true;
        } else if (_toolBar.selected() == EditorTool::Selection) {
            _selection = std::make_pair(core::GridPosition{minColumn, minRow},
                                        core::GridPosition{maxColumn, maxRow});
        }
    }

    // Annuler/refaire (EX-EDIT-005) : Ctrl+ la touche liee (par defaut Z/Y).
    if (input.keyDown(Key::Control) && input.keyPressed(_editorBindings.key(EditorAction::Undo))) {
        if (_draft.undo()) {
            _dirty = true;
            EDITOR_LOG_TRACE("Annuler (Ctrl+Z)");
        }
    } else if (input.keyDown(Key::Control) &&
               input.keyPressed(_editorBindings.key(EditorAction::Redo))) {
        if (_draft.redo()) {
            _dirty = true;
            EDITOR_LOG_TRACE("Refaire (Ctrl+Y)");
        }
    }

    // Redimensionnement (EX-EDIT-005) : largeur par Gauche/Droite, hauteur par Haut/Bas ;
    // confirmation si destructeur (EX-EDIT-012, requestResize).
    const int width = _draft.tileMap().width();
    const int height = _draft.tileMap().height();
    if (input.keyPressed(Key::Right)) {
        requestResize(width + 1, height);
    } else if (input.keyPressed(Key::Left)) {
        requestResize((std::max)(1, width - 1), height);
    }
    if (input.keyPressed(Key::Down)) {
        requestResize(_draft.tileMap().width(), height + 1);
    } else if (input.keyPressed(Key::Up)) {
        requestResize(_draft.tileMap().width(), (std::max)(1, height - 1));
    }

    return ScreenTransition::none();
}

// Convertit une position souris en case de grille, bornee a la grille courante (jamais nullopt) :
// utilise par les outils Rectangle/Selection, dont le glisser doit rester utilisable meme si le
// curseur depasse legerement la grille.
core::GridPosition EditorScreen::clampedCell(float mouseX, float mouseY) const {
    const core::Vector2 world = _camera.screenToWorld(core::Vector2{mouseX, mouseY});
    const int column =
        std::clamp(static_cast<int>(std::floor(world.x)), 0, _draft.tileMap().width() - 1);
    const int row =
        std::clamp(static_cast<int>(std::floor(world.y)), 0, _draft.tileMap().height() - 1);
    return core::GridPosition{column, row};
}

// Redimensionne directement si l'opération est anodine ; sinon pose une confirmation et n'agit
// qu'une fois acceptée (Entree), sans effet si annulée (Echap) — EX-EDIT-012. Une selection
// Rectangle/Selection en cours devient potentiellement hors bornes : invalidee dans tous les cas.
// Borne systematiquement au plafond (EX-EDIT-017) : seul point de passage des flèches et de la
// boite de dialogue (Ctrl+R), aucune des deux voies ne peut donc le depasser.
void EditorScreen::requestResize(int rawWidth, int rawHeight) {
    const int width = std::clamp(rawWidth, 1, MAX_LEVEL_DIMENSION);
    const int height = std::clamp(rawHeight, 1, MAX_LEVEL_DIMENSION);
    const int previousWidth = _draft.tileMap().width();
    const int previousHeight = _draft.tileMap().height();
    if (width == previousWidth && height == previousHeight) {
        return;  // deja a la cible (bornage a absorbe le changement demande) : rien a faire
    }
    const std::string resizeLabel = std::to_string(previousWidth) + "x" +
                                    std::to_string(previousHeight) + " -> " +
                                    std::to_string(width) + "x" + std::to_string(height);
    if (_draft.wouldResizeDropContent(width, height)) {
        EDITOR_LOG_TRACE("Redimensionnement destructeur en attente de confirmation (" +
                         resizeLabel + ")");
        _pendingConfirmation = PendingConfirmation{
            "Ce redimensionnement supprimerait l'entree, la sortie ou une liaison. "
            "Confirmer ? (Entree = oui, Echap = non)",
            [this, width, height, resizeLabel]() {
                _draft.resize(width, height);
                _dirty = true;
                _selection.reset();
                EDITOR_LOG_TRACE("Redimensionnement confirme (" + resizeLabel + ")");
                return ScreenTransition::none();
            }};
        return;
    }
    _draft.resize(width, height);
    _dirty = true;
    _selection.reset();
    EDITOR_LOG_TRACE("Redimensionnement (" + resizeLabel + ")");
}

// Valide le brouillon puis l'ecrit dans le dossier Levels de l'application (EX-EDIT-006/007).
// Echec de validation : message non-codeur, aucun fichier ecrit. Ecraser un fichier different de
// celui d'origine du brouillon est confirme au prealable (EX-EDIT-009, EX-EDIT-012).
void EditorScreen::saveDraft() {
    const core::LevelLoadResult result = _draft.toLevel();
    if (!result.ok()) {
        _statusMessage = describeValidationError(result);
        HMI_LOG_WARNING("Enregistrement refuse (brouillon invalide) : " + result.error);
        return;
    }

    const std::filesystem::path directory = hmi::executableDirectory() / "Levels";
    std::error_code errorCode;
    std::filesystem::create_directories(directory, errorCode);
    const std::filesystem::path path = directory / (_draft.name() + ".json");

    std::error_code equivalenceError;
    const bool sameAsOrigin = _loadedFrom.has_value() &&
                              std::filesystem::equivalent(*_loadedFrom, path, equivalenceError);
    const bool overwritesOtherFile = !sameAsOrigin && std::filesystem::exists(path, errorCode);
    if (overwritesOtherFile) {
        const core::Level level = *result.level;  // copie : capturee par la confirmation differee
        _pendingConfirmation =
            PendingConfirmation{"Un niveau \"" + _draft.name() +
                                    "\" existe deja. L'ecraser ? (Entree = oui, Echap = non)",
                                [this, level, path]() {
                                    writeLevelToDisk(level, path);
                                    return ScreenTransition::none();
                                }};
        return;
    }
    writeLevelToDisk(*result.level, path);
}

// Ecrit level a path, met a jour le message de statut et le drapeau dirty (EX-EDIT-006).
void EditorScreen::writeLevelToDisk(const core::Level& level, const std::filesystem::path& path) {
    if (core::LevelWriter::saveToFile(level, path)) {
        _statusMessage = "Niveau enregistre : " + path.filename().string();
        _dirty = false;
        _loadedFrom = path;
        HMI_LOG_INFO("Niveau enregistre : " + path.string());
    } else {
        _statusMessage = "Echec de l'enregistrement (verifiez les droits d'ecriture).";
        HMI_LOG_WARNING("Echec d'ecriture du niveau : " + path.string());
    }
}

// Sur un brouillon valide, lance une session de jeu interne rejouant le niveau en cours
// d'edition (EX-EDIT-008) ; message d'erreur non-codeur sinon, sans lancer l'essai. Le niveau
// valide est transmis directement en memoire (LOT-15) : aucun fichier temporaire.
void EditorScreen::startPlaytest() {
    const core::LevelLoadResult result = _draft.toLevel();
    if (!result.ok()) {
        _statusMessage = describeValidationError(result);
        HMI_LOG_WARNING("Essai immediat refuse (brouillon invalide) : " + result.error);
        return;
    }

    _playtest = std::make_unique<GameScreen>(_batch, _atlas, _viewportWidth, _viewportHeight,
                                             *result.level, _gameBindings, _gamepadBindings);
    _statusMessage.clear();
    HMI_LOG_INFO("Essai immediat demarre.");
}

// Dessine la grille du brouillon (tuiles non vides) et la case survolee en surbrillance. Cadrage
// automatique (ajuste a la fenetre) tant qu'aucun pan/zoom manuel n'est intervenu (EX-EDIT-013) ;
// sinon _cameraCenter/_cameraZoom (pilotes par update()) restent tels quels.
void EditorScreen::renderGrid(RenderContext& context) {
    const int width = _draft.tileMap().width();
    const int height = _draft.tileMap().height();

    // La grille se cadre dans le canevas (a droite du panneau lateral), pas sur toute la fenetre
    // (EX-EDIT-015) — evite qu'une case ne se retrouve visuellement sous la palette/barre d'outils.
    const float canvasWidth =
        (std::max)(1.0f, static_cast<float>(context.viewportWidth) - PANEL_WIDTH);
    if (!_manualCamera) {
        // Zoom pour faire tenir la grille entiere dans le canevas (LOT-16, EX-EDIT-013) : entier
        // (nettete pixel art) tant que le niveau tient a l'echelle x1, fractionnaire au-dela pour
        // qu'aucune case ne reste hors champ.
        _cameraZoom =
            Camera2D::fitZoom(canvasWidth, static_cast<float>(context.viewportHeight),
                              static_cast<float>(width), static_cast<float>(height), 0.85f);
        // Decale le centre vers la gauche, en unites monde, de la moitie du panneau converti a
        // l'echelle courante : le centre APPARENT de la grille se retrouve au milieu du canevas
        // plutot qu'au milieu de la fenetre entiere.
        const float scale = Camera2D::PIXELS_PER_UNIT * _cameraZoom;
        _cameraCenter =
            core::Vector2{static_cast<float>(width) * 0.5f - (PANEL_WIDTH * 0.5f) / scale,
                          static_cast<float>(height) * 0.5f};
    }
    _camera.setCenter(_cameraCenter);
    _camera.setZoom(_cameraZoom);

    context.spriteBatch.begin(_camera.projectionMatrix(), _atlas.textureView());
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            const core::TileType type = _draft.tileMap().tile(column, row);
            if (type == core::TileType::Empty) {
                continue;
            }
            // Aperçu fidèle (EX-GP-005) : un bloc réduit se dessine centré et à l'échelle, comme
            // sa boîte de collision réelle (core::BlockController::boxAt, même formule de marge)
            // — pas un carré plein, pour que l'éditeur reflète exactement ce que le jeu affichera.
            // Même principe pour un danger directionnel (EX-GP-050) : bande décalée sur son bord
            // plutôt que centrée, via core::dangerHitbox (LOT-31) — même géométrie que la hitbox
            // réelle (LevelOutcome.cpp), pas un carré plein qui masquerait le côté traversable.
            if (core::isDangerTileType(type)) {
                const core::Aabb box = core::dangerHitbox(type, column, row);
                context.spriteBatch.draw(quadFor(regionForTile(type, _atlas), box.min.x, box.min.y,
                                                 box.max.x - box.min.x, box.max.y - box.min.y,
                                                 _atlas));
            } else {
                const float scale = core::tileVisualScale(type);
                const float margin = (1.0f - scale) * 0.5f;
                context.spriteBatch.draw(
                    quadFor(regionForTile(type, _atlas), static_cast<float>(column) + margin,
                            static_cast<float>(row) + margin, scale, scale, _atlas));
            }
        }
    }

    // Grille de repere (bouton du panneau lateral) : fines lignes sur chaque bord de case,
    // par-dessus les tuiles deja peintes, pour simplifier le reperage d'une case avant d'y peindre.
    if (_showGridLines) {
        constexpr core::Color GRID_LINE_TINT{1.0f, 1.0f, 1.0f, 0.18f};
        constexpr float LINE_THICKNESS = 0.035f;  // en unites monde (fraction d'une case)
        for (int column = 0; column <= width; ++column) {
            context.spriteBatch.draw(
                quadFor(_atlas.tile(0, 0), static_cast<float>(column) - LINE_THICKNESS * 0.5f, 0.0f,
                        LINE_THICKNESS, static_cast<float>(height), _atlas, GRID_LINE_TINT));
        }
        for (int row = 0; row <= height; ++row) {
            context.spriteBatch.draw(
                quadFor(_atlas.tile(0, 0), 0.0f, static_cast<float>(row) - LINE_THICKNESS * 0.5f,
                        static_cast<float>(width), LINE_THICKNESS, _atlas, GRID_LINE_TINT));
        }

        // Quadrillage de SALLES (LOT-32, EX-EDIT-023) : memes lignes epaisses/plus opaques que le
        // repere fin ci-dessus, mais uniquement aux frontieres de salles (RoomGrid), pas case par
        // case -- aide a aligner les couloirs inter-salles. Meme bascule (F10) que le repere fin ;
        // aucun changement du cadrage camera de l'editeur (pan/zoom manuel, LOT-15/16, inchange).
        constexpr core::Color ROOM_LINE_TINT{1.0f, 0.85f, 0.3f, 0.5f};
        constexpr float ROOM_LINE_THICKNESS = 0.09f;  // plus epais que LINE_THICKNESS, pour rester
                                                      // distinguable du repere fin case par case.
        const RoomGrid roomGrid(width, height);
        for (int column = 0; column <= roomGrid.columns(); ++column) {
            const float x =
                static_cast<float>((std::min)(column * RoomGrid::ROOM_WIDTH_TILES, width));
            context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), x - ROOM_LINE_THICKNESS * 0.5f,
                                             0.0f, ROOM_LINE_THICKNESS, static_cast<float>(height),
                                             _atlas, ROOM_LINE_TINT));
        }
        for (int row = 0; row <= roomGrid.rows(); ++row) {
            const float y =
                static_cast<float>((std::min)(row * RoomGrid::ROOM_HEIGHT_TILES, height));
            context.spriteBatch.draw(
                quadFor(_atlas.tile(0, 0), 0.0f, y - ROOM_LINE_THICKNESS * 0.5f,
                        static_cast<float>(width), ROOM_LINE_THICKNESS, _atlas, ROOM_LINE_TINT));
        }
    }

    // Mecanismes lies : une teinte par interrupteur (pas de primitive de ligne dans SpriteBatch —
    // un quad ne peut pas etre incline — d'ou cette association par couleur plutot qu'un trait
    // reliant les deux cases). Plusieurs liaisons simultanees restent distinguables (EX-EDIT-016) :
    // chaque porte reprend la teinte de son interrupteur, cycliquement au-dela de LINK_TINTS.
    constexpr core::Color LINK_TINTS[] = {
        core::Color{0.3f, 1.0f, 1.0f, 0.45f},    // cyan
        core::Color{1.0f, 0.55f, 0.15f, 0.45f},  // orange
        core::Color{0.55f, 1.0f, 0.35f, 0.45f},  // vert
        core::Color{1.0f, 0.35f, 0.75f, 0.45f},  // rose
        core::Color{0.75f, 0.55f, 1.0f, 0.45f},  // violet
        core::Color{1.0f, 0.9f, 0.25f, 0.45f},   // jaune
    };
    constexpr std::size_t LINK_TINT_COUNT = sizeof(LINK_TINTS) / sizeof(LINK_TINTS[0]);

    // Deux cibles partagent le meme espace de couleurs par declencheur (une porte, EX-EDIT-016 ;
    // un danger commute, EX-GP-052) : un declencheur qui active les deux reprend la meme teinte
    // sur ses trois cases (declencheur, porte, danger) plutot que deux echelles de couleur
    // independantes.
    std::vector<core::GridPosition> uniqueSwitches;
    const auto rememberSwitch = [&](core::GridPosition position) {
        if (std::find(uniqueSwitches.begin(), uniqueSwitches.end(), position) ==
            uniqueSwitches.end()) {
            uniqueSwitches.push_back(position);
        }
    };
    for (const core::Mechanism& mechanism : _draft.mechanisms()) {
        rememberSwitch(mechanism.switchPosition);
    }
    for (const core::DangerLink& link : _draft.dangerLinks()) {
        rememberSwitch(link.triggerPosition);
    }
    const auto tintForSwitch = [&](core::GridPosition position) {
        const auto switchIt = std::find(uniqueSwitches.begin(), uniqueSwitches.end(), position);
        const std::size_t index =
            static_cast<std::size_t>(std::distance(uniqueSwitches.begin(), switchIt));
        return LINK_TINTS[index % LINK_TINT_COUNT];
    };
    for (const core::Mechanism& mechanism : _draft.mechanisms()) {
        const core::Color tint = tintForSwitch(mechanism.switchPosition);
        context.spriteBatch.draw(
            quadFor(_atlas.tile(0, 0), static_cast<float>(mechanism.switchPosition.column),
                    static_cast<float>(mechanism.switchPosition.row), 1.0f, 1.0f, _atlas, tint));
        context.spriteBatch.draw(
            quadFor(_atlas.tile(0, 0), static_cast<float>(mechanism.doorPosition.column),
                    static_cast<float>(mechanism.doorPosition.row), 1.0f, 1.0f, _atlas, tint));
    }
    for (const core::DangerLink& link : _draft.dangerLinks()) {
        const core::Color tint = tintForSwitch(link.triggerPosition);
        context.spriteBatch.draw(
            quadFor(_atlas.tile(0, 0), static_cast<float>(link.triggerPosition.column),
                    static_cast<float>(link.triggerPosition.row), 1.0f, 1.0f, _atlas, tint));
        context.spriteBatch.draw(
            quadFor(_atlas.tile(0, 0), static_cast<float>(link.dangerPosition.column),
                    static_cast<float>(link.dangerPosition.row), 1.0f, 1.0f, _atlas, tint));
    }

    // Case en attente de liaison (premier clic Maj+souris d'une paire), teinte magenta.
    if (_pendingLink) {
        constexpr core::Color PENDING_TINT{1.0f, 0.3f, 1.0f, 0.55f};
        context.spriteBatch.draw(
            quadFor(_atlas.tile(0, 0), static_cast<float>(_pendingLink->column),
                    static_cast<float>(_pendingLink->row), 1.0f, 1.0f, _atlas, PENDING_TINT));
    }

    // Surbrillance de la case survolee : quad blanc translucide par-dessus la tuile.
    if (const std::optional<core::GridPosition> hovered = hoveredCell(_mouseX, _mouseY)) {
        constexpr core::Color HIGHLIGHT_TINT{1.0f, 1.0f, 1.0f, 0.35f};
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), static_cast<float>(hovered->column),
                                         static_cast<float>(hovered->row), 1.0f, 1.0f, _atlas,
                                         HIGHLIGHT_TINT));
    }

    // Previsualisation du glisser Rectangle/Selection en cours, ou selection validee restante
    // (outil Selection) — teintes distinctes pour ne pas les confondre (EX-EDIT-014).
    if (_toolBar.selected() != EditorTool::Paint) {
        std::optional<core::GridPosition> rangeStart;
        std::optional<core::GridPosition> rangeEnd;
        core::Color tint{1.0f, 0.6f, 0.15f, 0.35f};  // orange : glisser en cours
        if (_areaDragActive) {
            rangeStart = _areaDragStart;
            rangeEnd = clampedCell(_mouseX, _mouseY);
        } else if (_toolBar.selected() == EditorTool::Selection && _selection) {
            rangeStart = _selection->first;
            rangeEnd = _selection->second;
            tint = core::Color{0.4f, 0.7f, 1.0f, 0.30f};  // bleu : selection validee
        }
        if (rangeStart && rangeEnd) {
            const int minColumn = (std::min)(rangeStart->column, rangeEnd->column);
            const int maxColumn = (std::max)(rangeStart->column, rangeEnd->column);
            const int minRow = (std::min)(rangeStart->row, rangeEnd->row);
            const int maxRow = (std::max)(rangeStart->row, rangeEnd->row);
            for (int row = minRow; row <= maxRow; ++row) {
                for (int column = minColumn; column <= maxColumn; ++column) {
                    context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), static_cast<float>(column),
                                                     static_cast<float>(row), 1.0f, 1.0f, _atlas,
                                                     tint));
                }
            }
        }
    }
    context.spriteBatch.end();
}

// Dessine le fond opaque du panneau lateral (EX-EDIT-015) : la grille (canevas a droite du
// panneau, cf. renderGrid) ne doit jamais rester visible « sous » l'interface.
void EditorScreen::renderPanelBackground(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, _atlas.textureView());
    constexpr core::Color PANEL_TINT{0.09f, 0.09f, 0.12f, 0.92f};
    context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), 0.0f, 0.0f, PANEL_WIDTH,
                                     static_cast<float>(context.viewportHeight), _atlas,
                                     PANEL_TINT));
    context.spriteBatch.end();
}

// Dessine la palette (panneau lateral) : une icone par type, la selection encadree, un libelle a
// droite de chaque icone (decouvrabilite, EX-EDIT-015).
void EditorScreen::renderPalette(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, _atlas.textureView());

    constexpr float BORDER = 3.0f;
    constexpr core::Color SELECTION_TINT{1.0f, 1.0f, 0.4f, 1.0f};
    for (const TilePalette::Entry& entry : _palette.entries()) {
        if (entry.type == _palette.selected()) {
            context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), entry.x - BORDER, entry.y - BORDER,
                                             entry.width + 2.0f * BORDER,
                                             entry.height + 2.0f * BORDER, _atlas, SELECTION_TINT));
        }
        context.spriteBatch.draw(quadFor(regionForTile(entry.type, _atlas), entry.x, entry.y,
                                         entry.width, entry.height, _atlas));
    }

    // Barre de defilement (piste + curseur), uniquement si l'accordeon deplie deborde de la
    // fenetre visible (LOT-27, EX-EDIT-018) -- meme principe que LevelPicker::renderPicker.
    const int totalRows = _palette.totalRowCount();
    const int visibleRows =
        TilePalette::visibleRowCount(static_cast<float>(context.viewportHeight));
    if (totalRows > visibleRows) {
        constexpr float SCROLLBAR_WIDTH = 5.0f;
        constexpr float SCROLLBAR_MARGIN_RIGHT = 4.0f;
        const float trackX = PANEL_WIDTH - SCROLLBAR_MARGIN_RIGHT - SCROLLBAR_WIDTH;
        const float trackTop = PALETTE_TOP;
        const float trackHeight = static_cast<float>(visibleRows) * PANEL_ROW_PITCH;
        const float thumbHeight = (std::max)(trackHeight * static_cast<float>(visibleRows) /
                                                 static_cast<float>(totalRows),
                                             SCROLLBAR_WIDTH * 2.0f);
        const float maxThumbTravel = trackHeight - thumbHeight;
        const int maxOffset = totalRows - visibleRows;
        const float thumbY =
            trackTop + (maxOffset > 0
                            ? maxThumbTravel * static_cast<float>(_palette.scrollOffset()) /
                                  static_cast<float>(maxOffset)
                            : 0.0f);
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), trackX, trackTop, SCROLLBAR_WIDTH,
                                         trackHeight, _atlas,
                                         core::Color{1.0f, 1.0f, 1.0f, 0.12f}));
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), trackX, thumbY, SCROLLBAR_WIDTH,
                                         thumbHeight, _atlas,
                                         core::Color{1.0f, 1.0f, 1.0f, 0.45f}));
    }
    context.spriteBatch.end();

    // Deuxieme passe (texture de police distincte de l'atlas de tuiles, cf. SpriteBatch) : un
    // libelle court a droite de chaque icone, verticalement centre.
    context.spriteBatch.begin(projection, context.font.textureView());
    constexpr float LABEL_SCALE = 1.0f;
    constexpr float LABEL_GAP = 6.0f;
    for (const TilePalette::Entry& entry : _palette.entries()) {
        const float labelY = entry.y + (entry.height - context.font.lineHeight(LABEL_SCALE)) * 0.5f;
        context.font.drawText(context.spriteBatch, entry.label, entry.x + entry.width + LABEL_GAP,
                              labelY, LABEL_SCALE, core::Color{0.85f, 0.85f, 0.90f, 1.0f});
    }
    context.spriteBatch.end();
}

// Dessine la barre d'outils (panneau lateral, sous la palette) : une icone teintee, la selection
// encadree, un libelle a droite de chaque ligne (EX-EDIT-014/015). La grille de repere n'a pas de
// bouton dedie : bascule au clavier (F10, cf. update()).
void EditorScreen::renderToolBar(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);

    constexpr float BORDER = 3.0f;
    constexpr core::Color SELECTION_TINT{1.0f, 1.0f, 0.4f, 1.0f};
    constexpr core::Color TOOL_TINT{0.55f, 0.55f, 0.68f, 1.0f};

    context.spriteBatch.begin(projection, _atlas.textureView());
    for (const ToolBar::Entry& entry : _toolBar.entries()) {
        if (entry.tool == _toolBar.selected()) {
            context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), entry.x - BORDER, entry.y - BORDER,
                                             entry.width + 2.0f * BORDER,
                                             entry.height + 2.0f * BORDER, _atlas, SELECTION_TINT));
        }
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), entry.x, entry.y, entry.width,
                                         entry.height, _atlas, TOOL_TINT));
    }
    context.spriteBatch.end();

    context.spriteBatch.begin(projection, context.font.textureView());
    constexpr float LABEL_SCALE = 1.0f;
    constexpr float LABEL_GAP = 6.0f;
    for (const ToolBar::Entry& entry : _toolBar.entries()) {
        const float labelY = entry.y + (entry.height - context.font.lineHeight(LABEL_SCALE)) * 0.5f;
        context.font.drawText(context.spriteBatch, toolLabel(entry.tool),
                              entry.x + entry.width + LABEL_GAP, labelY, LABEL_SCALE,
                              core::Color{0.85f, 0.85f, 0.90f, 1.0f});
    }
    context.spriteBatch.end();
}

// Dessine l'apercu des raccourcis (bascule F1, dans le canevas a droite du panneau lateral) ou,
// replie, un indice permanent discret en haut a droite de l'ecran (decouvrabilite, EX-EDIT-015).
void EditorScreen::renderHelp(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, context.font.textureView());

    if (_showHelp) {
        // Noms de touches interpoles depuis les bindings courants (pas des libelles fixes) :
        // resteraient faux apres un remappage sinon (LOT-29, Options -> Touches de l'editeur).
        const std::string ctrlCopy =
            "Ctrl+" + keyDisplayName(_editorBindings.key(EditorAction::Copy));
        const std::string ctrlPaste =
            "Ctrl+" + keyDisplayName(_editorBindings.key(EditorAction::Paste));
        const std::string ctrlUndo =
            "Ctrl+" + keyDisplayName(_editorBindings.key(EditorAction::Undo));
        const std::string ctrlRedo =
            "Ctrl+" + keyDisplayName(_editorBindings.key(EditorAction::Redo));
        const std::string renameKey = keyDisplayName(_editorBindings.key(EditorAction::Rename));
        const std::string ctrlSave =
            "Ctrl+" + keyDisplayName(_editorBindings.key(EditorAction::Save));
        const std::string playtestKey = keyDisplayName(_editorBindings.key(EditorAction::Playtest));
        const std::string gridKey = keyDisplayName(_editorBindings.key(EditorAction::ToggleGrid));
        const std::string helpKey = keyDisplayName(_editorBindings.key(EditorAction::ToggleHelp));

        const std::vector<std::string> LINES = {
            "Clic gauche : peindre (outil actif)   |   Maj+clic : lier interrupteur/porte",
            "Molette : zoom camera   |   Glisser bouton droit : deplacer la vue   |   0 : reinit.",
            "Tab (ou barre d'outils) : changer d'outil (Pinceau / Rectangle / Selection)",
            ctrlCopy + " / " + ctrlPaste + " : copier / coller la selection",
            "Fleches : redimensionner la grille   |   " + ctrlUndo + " / " + ctrlRedo +
                " : annuler / refaire",
            renameKey + " : renommer   |   " + ctrlSave + " : enregistrer   |   " + playtestKey +
                " : essai immediat",
            gridKey + " : grille de repere   |   Echap : quitter (ou fin de l'essai)",
            helpKey + " : fermer cette aide",
        };
        constexpr float SCALE = 1.6f;
        // A droite du panneau lateral (pas x = 12) : eviter tout chevauchement avec la palette et
        // la barre d'outils (EX-EDIT-015).
        const float marginX = PANEL_WIDTH + 12.0f;
        float y = 12.0f;
        for (const std::string& line : LINES) {
            context.font.drawText(context.spriteBatch, line, marginX, y, SCALE,
                                  core::Color{0.92f, 0.92f, 0.95f, 1.0f});
            y += context.font.lineHeight(SCALE) + 4.0f;
        }
    } else {
        constexpr float SCALE = 1.4f;
        const std::string hint =
            keyDisplayName(_editorBindings.key(EditorAction::ToggleHelp)) + " : aide";
        const float x =
            static_cast<float>(context.viewportWidth) - context.font.textWidth(hint, SCALE) - 10.0f;
        context.font.drawText(context.spriteBatch, hint, x, 10.0f, SCALE,
                              core::Color{0.7f, 0.7f, 0.75f, 0.85f});
    }
    context.spriteBatch.end();
}

// Dessine le message de statut courant (bande de texte en bas de l'ecran) : la confirmation en
// attente est prioritaire sur un simple message d'erreur/information, s'il y en a un.
void EditorScreen::renderStatus(RenderContext& context) {
    const std::string& message =
        _pendingConfirmation ? _pendingConfirmation->message : _statusMessage;
    if (message.empty()) {
        return;
    }
    constexpr float SCALE = 2.0f;
    constexpr float MARGIN = 12.0f;
    const float y =
        static_cast<float>(context.viewportHeight) - context.font.lineHeight(SCALE) - MARGIN;
    const core::Color color = _pendingConfirmation ? core::Color{1.0f, 0.75f, 0.30f, 1.0f}
                                                   : core::Color{0.90f, 0.85f, 0.55f, 1.0f};

    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, context.font.textureView());
    context.font.drawText(context.spriteBatch, message, MARGIN, y, SCALE, color);
    context.spriteBatch.end();
}

// Dessine la liste "Nouveau niveau" + fichiers existants, la selection en surbrillance. Une
// liste plus longue que la fenetre visible ne dessine que cette fenetre (defilement, EX-EDIT-001)
// et une barre de defilement (piste + curseur), pour rester utilisable quel que soit le nombre de
// niveaux dans le dossier.
void EditorScreen::renderPicker(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);

    const std::vector<LevelPicker::Choice>& choices = _picker->choices();
    const int visible = LevelPicker::visibleCount(static_cast<float>(context.viewportHeight));
    const int total = static_cast<int>(choices.size());
    const int scrollOffset = _picker->scrollOffset();
    const int lastVisible = (std::min)(total, scrollOffset + visible);

    // Barre de defilement (piste + curseur), uniquement si tous les choix ne tiennent pas dans
    // la fenetre visible -- passe distincte (texture d'atlas, teintee) de celle du texte ci-apres.
    if (total > visible) {
        constexpr float SCROLLBAR_WIDTH = 10.0f;
        constexpr float SCROLLBAR_MARGIN_RIGHT = 24.0f;
        const float trackX =
            static_cast<float>(context.viewportWidth) - SCROLLBAR_MARGIN_RIGHT - SCROLLBAR_WIDTH;
        const float trackTop = LevelPicker::OPTIONS_TOP;
        const float trackHeight = static_cast<float>(visible) * LevelPicker::OPTION_SPACING;
        const float thumbHeight =
            (std::max)(trackHeight * static_cast<float>(visible) / static_cast<float>(total),
                       SCROLLBAR_WIDTH * 2.0f);
        const float maxThumbTravel = trackHeight - thumbHeight;
        const int maxOffset = total - visible;
        const float thumbY =
            trackTop + (maxOffset > 0 ? maxThumbTravel * static_cast<float>(scrollOffset) /
                                            static_cast<float>(maxOffset)
                                      : 0.0f);

        context.spriteBatch.begin(projection, _atlas.textureView());
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), trackX, trackTop, SCROLLBAR_WIDTH,
                                         trackHeight, _atlas,
                                         core::Color{1.0f, 1.0f, 1.0f, 0.12f}));
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), trackX, thumbY, SCROLLBAR_WIDTH,
                                         thumbHeight, _atlas,
                                         core::Color{1.0f, 1.0f, 1.0f, 0.45f}));
        context.spriteBatch.end();
    }

    context.spriteBatch.begin(projection, context.font.textureView());

    context.font.drawText(context.spriteBatch, "Choisir un niveau", LevelPicker::MARGIN_X,
                          LevelPicker::TITLE_Y, 4.0f, core::Color{0.90f, 0.90f, 0.95f, 1.0f});

    for (int index = scrollOffset; index < lastVisible; ++index) {
        const bool isSelected = index == _picker->selected();
        const core::Color color = isSelected ? core::Color{1.0f, 0.85f, 0.35f, 1.0f}
                                             : core::Color{0.75f, 0.75f, 0.80f, 1.0f};
        const std::string label =
            (isSelected ? "> " : "  ") + choices[static_cast<std::size_t>(index)].label;
        const float y = LevelPicker::OPTIONS_TOP +
                        static_cast<float>(index - scrollOffset) * LevelPicker::OPTION_SPACING;
        context.font.drawText(context.spriteBatch, label, LevelPicker::MARGIN_X, y,
                              LevelPicker::OPTION_SCALE, color);
    }

    if (!_statusMessage.empty()) {
        const float y =
            static_cast<float>(context.viewportHeight) - context.font.lineHeight(2.0f) - 12.0f;
        context.font.drawText(context.spriteBatch, _statusMessage, LevelPicker::MARGIN_X, y, 2.0f,
                              core::Color{0.90f, 0.55f, 0.55f, 1.0f});
    }
    context.spriteBatch.end();
}

// Dessine le champ de saisie generique (nom a la creation, renommage F2, ou taille Ctrl+R), avec
// un message de refus si la derniere tentative de confirmation etait invalide (EX-EDIT-009/017).
void EditorScreen::renderTextPrompt(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, context.font.textureView());

    std::string title;
    std::string rejectionMessage;
    switch (_textPromptPurpose) {
        case TextPromptPurpose::CreateLevelName:
            title = "Nom du nouveau niveau";
            rejectionMessage =
                "Nom invalide : evitez un nom vide et les caracteres \\ / : * ? \" < > |";
            break;
        case TextPromptPurpose::RenameLevel:
            title = "Renommer le niveau";
            rejectionMessage =
                "Nom invalide : evitez un nom vide et les caracteres \\ / : * ? \" < > |";
            break;
        case TextPromptPurpose::ResizeGrid:
            title = "Nouvelle taille (largeur x hauteur)";
            rejectionMessage =
                "Taille invalide : format attendu \"largeurxhauteur\" (ex. 40x30 "
                "ou 40*30), chaque valeur entre 1 et " +
                std::to_string(MAX_LEVEL_DIMENSION) + ".";
            break;
    }
    context.font.drawText(context.spriteBatch, title, LevelPicker::MARGIN_X, LevelPicker::TITLE_Y,
                          4.0f, core::Color{0.90f, 0.90f, 0.95f, 1.0f});

    const std::string display = _textPrompt->text() + "_";
    context.font.drawText(context.spriteBatch, display, LevelPicker::MARGIN_X,
                          LevelPicker::OPTIONS_TOP, LevelPicker::OPTION_SCALE,
                          core::Color{1.0f, 0.85f, 0.35f, 1.0f});

    if (_textPrompt->rejected()) {
        const float y =
            static_cast<float>(context.viewportHeight) - context.font.lineHeight(2.0f) - 12.0f;
        context.font.drawText(context.spriteBatch, rejectionMessage, LevelPicker::MARGIN_X, y, 2.0f,
                              core::Color{0.90f, 0.55f, 0.55f, 1.0f});
    }
    context.spriteBatch.end();
}

// Dessine le selecteur de niveau, le champ de saisie generique, la session de jeu interne pendant
// un essai immediat, ou sinon la grille du niveau en cours d'edition, la palette et le statut.
void EditorScreen::render(RenderContext& context) {
    if (_picker) {
        renderPicker(context);
        return;
    }
    if (_textPrompt) {
        renderTextPrompt(context);
        return;
    }
    if (_playtest) {
        _playtest->render(context);
        return;
    }
    _camera.setViewportSize(context.viewportWidth, context.viewportHeight);
    renderGrid(context);
    renderPanelBackground(context);
    renderPalette(context);
    renderToolBar(context);
    renderHelp(context);
    renderStatus(context);
}

}  // namespace hmi
