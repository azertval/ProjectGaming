#include "HMI/Interface/EditorScreen.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"  // core::Mechanism
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/GameScreen.h"
#include "HMI/Interface/RenderContext.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {

// Dimensions par defaut d'un brouillon vierge (comparables aux niveaux livres, ex. demo3/demo4).
constexpr int DEFAULT_WIDTH = 14;
constexpr int DEFAULT_HEIGHT = 8;

// Chemin du fichier temporaire utilise pour l'essai immediat (EX-EDIT-008) : le brouillon y est
// ecrit avant de lancer une session de jeu interne, sans jamais toucher aux niveaux enregistres.
[[nodiscard]] std::filesystem::path playtestFilePath() {
    return std::filesystem::temp_directory_path() / "projectgaming_playtest_level.json";
}

// Traduit un message d'erreur de validation (LevelLoader) en phrase comprehensible par un
// non-codeur (EX-EDIT-007) : evite d'exposer le jargon interne (JSON, cle manquante...).
[[nodiscard]] std::string describeValidationError(const std::string& technicalMessage) {
    if (technicalMessage.find("entree") != std::string::npos) {
        return "Il manque une entree : placez une tuile Entree sur la grille.";
    }
    if (technicalMessage.find("sortie") != std::string::npos) {
        return "Il manque une sortie : placez une tuile Sortie sur la grille.";
    }
    if (technicalMessage.find("interrupteur") != std::string::npos ||
        technicalMessage.find("Porte liee") != std::string::npos) {
        return "Une porte n'est pas reliee a un interrupteur valide.";
    }
    return "Niveau invalide : " + technicalMessage;
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
                           int viewportHeight)
    : _atlas(atlas),
      _batch(batch),
      _viewportWidth(viewportWidth),
      _viewportHeight(viewportHeight),
      _picker(LevelPicker::forDirectory(hmi::executableDirectory() / "Levels")),
      _draft(core::LevelDraft::empty("Nouveau niveau", DEFAULT_WIDTH, DEFAULT_HEIGHT)),
      _camera(viewportWidth, viewportHeight) {
    HMI_LOG_TRACE("EditorScreen cree (selecteur : " +
                 std::to_string(_picker->choices().size()) + " choix)");
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

// Traite un clic Maj+souris pour la liaison de mecanismes (bascule si la paire est deja liee).
void EditorScreen::handleLinkClick(float mouseX, float mouseY) {
    const std::optional<core::GridPosition> cell = hoveredCell(mouseX, mouseY);
    if (!cell) {
        return;
    }
    const core::TileType type = _draft.tileMap().tile(cell->column, cell->row);
    if (type != core::TileType::Switch && type != core::TileType::Door) {
        return;  // rien a lier sur cette case
    }

    if (_pendingLink) {
        const core::TileType pendingType =
            _draft.tileMap().tile(_pendingLink->column, _pendingLink->row);
        if (pendingType != core::TileType::Switch && pendingType != core::TileType::Door) {
            _pendingLink.reset();  // la case en attente a change de type entre-temps
        }
    }

    if (!_pendingLink) {
        _pendingLink = cell;
        return;
    }

    const core::TileType pendingType =
        _draft.tileMap().tile(_pendingLink->column, _pendingLink->row);
    if (pendingType == type) {
        // Deux cases du meme type (deux interrupteurs, deux portes) : on recommence avec
        // la nouvelle case plutot que de rester bloque.
        _pendingLink = cell;
        return;
    }

    const bool pendingIsSwitch = pendingType == core::TileType::Switch;
    const core::GridPosition switchPosition = pendingIsSwitch ? *_pendingLink : *cell;
    const core::GridPosition doorPosition = pendingIsSwitch ? *cell : *_pendingLink;

    const bool alreadyLinked =
        std::any_of(_draft.mechanisms().begin(), _draft.mechanisms().end(),
                   [&](const core::Mechanism& mechanism) {
                       return mechanism.switchPosition == switchPosition &&
                              mechanism.doorPosition == doorPosition;
                   });
    if (alreadyLinked) {
        _draft.unlinkMechanism(doorPosition);
    } else {
        _draft.linkMechanism(switchPosition, doorPosition);
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
        const std::optional<int> confirmed = _picker->update(input);
        if (confirmed) {
            const LevelPicker::Choice& choice = _picker->choices()[*confirmed];
            if (choice.path) {
                const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(*choice.path);
                if (loaded.ok()) {
                    _draft = core::LevelDraft::fromLevel(*loaded.level);
                    _picker.reset();
                } else {
                    // Fichier corrompu : message recuperable (EX-NFR-040), on reste au selecteur.
                    _statusMessage = "Impossible de charger ce niveau : " + loaded.error;
                    HMI_LOG_WARNING(_statusMessage);
                }
            } else {
                _draft = core::LevelDraft::empty("Nouveau niveau", DEFAULT_WIDTH, DEFAULT_HEIGHT);
                _picker.reset();
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

    if (input.keyDown(Key::Control) && input.keyPressed(Key::S)) {
        saveDraft();
    }
    if (input.keyPressed(Key::P)) {
        startPlaytest();
    }

    _mouseX = static_cast<float>(input.mouseX());
    _mouseY = static_cast<float>(input.mouseY());

    if (input.mouseButtonPressed(MouseButton::Left)) {
        if (_palette.handleClick(_mouseX, _mouseY)) {
            // La palette, dessinee par-dessus la grille, est prioritaire sur le reste.
            _paintingDrag = false;
        } else if (input.keyDown(Key::Shift)) {
            handleLinkClick(_mouseX, _mouseY);
            _paintingDrag = false;
        } else {
            _paintingDrag = true;
        }
    }

    if (_paintingDrag && input.mouseButtonDown(MouseButton::Left)) {
        if (const std::optional<core::GridPosition> cell = hoveredCell(_mouseX, _mouseY)) {
            _draft.paintTile(cell->column, cell->row, _palette.selected());
            _dirty = true;
        }
    }

    // Annuler/refaire (EX-EDIT-005) : Ctrl+Z / Ctrl+Y.
    if (input.keyDown(Key::Control) && input.keyPressed(Key::Z)) {
        _dirty = _draft.undo() || _dirty;
    } else if (input.keyDown(Key::Control) && input.keyPressed(Key::Y)) {
        _dirty = _draft.redo() || _dirty;
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

// Redimensionne directement si l'opération est anodine ; sinon pose une confirmation et n'agit
// qu'une fois acceptée (Entree), sans effet si annulée (Echap) — EX-EDIT-012.
void EditorScreen::requestResize(int width, int height) {
    if (_draft.wouldResizeDropContent(width, height)) {
        _pendingConfirmation = PendingConfirmation{
            "Ce redimensionnement supprimerait l'entree, la sortie ou une liaison. "
            "Confirmer ? (Entree = oui, Echap = non)",
            [this, width, height]() {
                _draft.resize(width, height);
                _dirty = true;
                return ScreenTransition::none();
            }};
        return;
    }
    _draft.resize(width, height);
    _dirty = true;
}

// Valide le brouillon puis l'ecrit dans le dossier Levels de l'application (EX-EDIT-006/007).
// Echec de validation : message non-codeur, aucun fichier ecrit.
void EditorScreen::saveDraft() {
    const core::LevelLoadResult result = _draft.toLevel();
    if (!result.ok()) {
        _statusMessage = describeValidationError(result.error);
        HMI_LOG_WARNING("Enregistrement refuse (brouillon invalide) : " + result.error);
        return;
    }

    const std::filesystem::path directory = hmi::executableDirectory() / "Levels";
    std::error_code errorCode;
    std::filesystem::create_directories(directory, errorCode);

    const std::filesystem::path path = directory / (_draft.name() + ".json");
    if (core::LevelWriter::saveToFile(*result.level, path)) {
        _statusMessage = "Niveau enregistre : " + path.filename().string();
        _dirty = false;
        HMI_LOG_INFO("Niveau enregistre : " + path.string());
    } else {
        _statusMessage = "Echec de l'enregistrement (verifiez les droits d'ecriture).";
        HMI_LOG_WARNING("Echec d'ecriture du niveau : " + path.string());
    }
}

// Sur un brouillon valide, lance une session de jeu interne rejouant le niveau en cours
// d'edition (EX-EDIT-008) ; message d'erreur non-codeur sinon, sans lancer l'essai.
void EditorScreen::startPlaytest() {
    const core::LevelLoadResult result = _draft.toLevel();
    if (!result.ok()) {
        _statusMessage = describeValidationError(result.error);
        HMI_LOG_WARNING("Essai immediat refuse (brouillon invalide) : " + result.error);
        return;
    }

    const std::filesystem::path path = playtestFilePath();
    if (!core::LevelWriter::saveToFile(*result.level, path)) {
        _statusMessage = "Impossible de preparer l'essai immediat (fichier temporaire).";
        return;
    }

    _playtest = std::make_unique<GameScreen>(_batch, _atlas, _viewportWidth, _viewportHeight,
                                             std::vector<std::filesystem::path>{path});
    _statusMessage.clear();
    HMI_LOG_INFO("Essai immediat demarre.");
}

// Dessine la grille du brouillon (tuiles non vides) et la case survolee en surbrillance.
void EditorScreen::renderGrid(RenderContext& context) {
    const int width = _draft.tileMap().width();
    const int height = _draft.tileMap().height();
    _camera.setCenter(
        core::Vector2{static_cast<float>(width) * 0.5f, static_cast<float>(height) * 0.5f});

    // Zoom pour faire tenir la grille dans la fenetre, en facteur entier (nettete pixel art).
    const float fitX = static_cast<float>(context.viewportWidth) /
                       (static_cast<float>(width) * Camera2D::PIXELS_PER_UNIT);
    const float fitY = static_cast<float>(context.viewportHeight) /
                       (static_cast<float>(height) * Camera2D::PIXELS_PER_UNIT);
    const float zoom = (std::max)(1.0f, std::floor((std::min)(fitX, fitY) * 0.85f));
    _camera.setZoom(zoom);

    context.spriteBatch.begin(_camera.projectionMatrix(), _atlas.textureView());
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            const core::TileType type = _draft.tileMap().tile(column, row);
            if (type == core::TileType::Empty) {
                continue;
            }
            context.spriteBatch.draw(quadFor(regionForTile(type, _atlas),
                                             static_cast<float>(column),
                                             static_cast<float>(row), 1.0f, 1.0f, _atlas));
        }
    }

    // Mecanismes lies : meme teinte cyan sur les deux tuiles d'une liaison (pas de primitive de
    // ligne dans SpriteBatch — un quad ne peut pas etre incline — d'ou cette association par
    // couleur plutot qu'un trait reliant les deux cases).
    constexpr core::Color LINK_TINT{0.3f, 1.0f, 1.0f, 0.45f};
    for (const core::Mechanism& mechanism : _draft.mechanisms()) {
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0),
                                         static_cast<float>(mechanism.switchPosition.column),
                                         static_cast<float>(mechanism.switchPosition.row), 1.0f,
                                         1.0f, _atlas, LINK_TINT));
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0),
                                         static_cast<float>(mechanism.doorPosition.column),
                                         static_cast<float>(mechanism.doorPosition.row), 1.0f,
                                         1.0f, _atlas, LINK_TINT));
    }

    // Case en attente de liaison (premier clic Maj+souris d'une paire), teinte magenta.
    if (_pendingLink) {
        constexpr core::Color PENDING_TINT{1.0f, 0.3f, 1.0f, 0.55f};
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), static_cast<float>(_pendingLink->column),
                                         static_cast<float>(_pendingLink->row), 1.0f, 1.0f, _atlas,
                                         PENDING_TINT));
    }

    // Surbrillance de la case survolee : quad blanc translucide par-dessus la tuile.
    if (const std::optional<core::GridPosition> hovered = hoveredCell(_mouseX, _mouseY)) {
        constexpr core::Color HIGHLIGHT_TINT{1.0f, 1.0f, 1.0f, 0.35f};
        context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), static_cast<float>(hovered->column),
                                         static_cast<float>(hovered->row), 1.0f, 1.0f, _atlas,
                                         HIGHLIGHT_TINT));
    }
    context.spriteBatch.end();
}

// Dessine la palette : une couleur par type, la selection encadree.
void EditorScreen::renderPalette(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, _atlas.textureView());

    constexpr float BORDER = 3.0f;
    constexpr core::Color SELECTION_TINT{1.0f, 1.0f, 0.4f, 1.0f};
    for (const TilePalette::Entry& entry : _palette.entries()) {
        if (entry.type == _palette.selected()) {
            context.spriteBatch.draw(quadFor(_atlas.tile(0, 0), entry.x - BORDER,
                                             entry.y - BORDER, entry.width + 2.0f * BORDER,
                                             entry.height + 2.0f * BORDER, _atlas,
                                             SELECTION_TINT));
        }
        context.spriteBatch.draw(
            quadFor(regionForTile(entry.type, _atlas), entry.x, entry.y, entry.width,
                   entry.height, _atlas));
    }
    context.spriteBatch.end();
}

// Dessine le message de statut courant (bande de texte en bas de l'ecran) : la confirmation en
// attente est prioritaire sur un simple message d'erreur/information, s'il y en a un.
void EditorScreen::renderStatus(RenderContext& context) {
    const std::string& message = _pendingConfirmation ? _pendingConfirmation->message : _statusMessage;
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

// Dessine la liste "Nouveau niveau" + fichiers existants, la selection en surbrillance.
void EditorScreen::renderPicker(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, context.font.textureView());

    context.font.drawText(context.spriteBatch, "Choisir un niveau", LevelPicker::MARGIN_X,
                          LevelPicker::TITLE_Y, 4.0f, core::Color{0.90f, 0.90f, 0.95f, 1.0f});

    const std::vector<LevelPicker::Choice>& choices = _picker->choices();
    for (std::size_t index = 0; index < choices.size(); ++index) {
        const bool isSelected = static_cast<int>(index) == _picker->selected();
        const core::Color color = isSelected ? core::Color{1.0f, 0.85f, 0.35f, 1.0f}
                                              : core::Color{0.75f, 0.75f, 0.80f, 1.0f};
        const std::string label = (isSelected ? "> " : "  ") + choices[index].label;
        const float y = LevelPicker::OPTIONS_TOP + static_cast<float>(index) * LevelPicker::OPTION_SPACING;
        context.font.drawText(context.spriteBatch, label, LevelPicker::MARGIN_X, y,
                              LevelPicker::OPTION_SCALE, color);
    }

    if (!_statusMessage.empty()) {
        const float y = static_cast<float>(context.viewportHeight) - context.font.lineHeight(2.0f) - 12.0f;
        context.font.drawText(context.spriteBatch, _statusMessage, LevelPicker::MARGIN_X, y, 2.0f,
                              core::Color{0.90f, 0.55f, 0.55f, 1.0f});
    }
    context.spriteBatch.end();
}

// Dessine le selecteur de niveau, la session de jeu interne pendant un essai immediat, ou sinon
// la grille du niveau en cours d'edition, la palette et le message de statut.
void EditorScreen::render(RenderContext& context) {
    if (_picker) {
        renderPicker(context);
        return;
    }
    if (_playtest) {
        _playtest->render(context);
        return;
    }
    _camera.setViewportSize(context.viewportWidth, context.viewportHeight);
    renderGrid(context);
    renderPalette(context);
    renderStatus(context);
}

}  // namespace hmi
