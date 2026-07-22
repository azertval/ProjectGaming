#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Core/Levels/LevelDraft.h"
#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/LevelPicker.h"
#include "HMI/Editor/TextInputField.h"
#include "HMI/Editor/TilePalette.h"
#include "HMI/Editor/ToolBar.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Interface/IScreen.h"

/**
 * @file HMI/Interface/EditorScreen.h
 * @brief Écran « Mode Édition » : éditeur de niveau intégré (LOT-14).
 */

namespace hmi {

class SpriteBatch;
class TextureAtlas;
class GameScreen;

/**
 * @brief Éditeur de niveau intégré : grille peignable à la souris depuis une palette de tuiles.
 *
 * Réutilise le rendu Direct3D 11 (`SpriteBatch`, `TextureAtlas`, `Camera2D`) et le modèle
 * d'édition de `Core` (`core::LevelDraft`, LOT-14 TACHE-01) — aucune duplication de la logique de
 * niveau (`EX-EDIT-010`, `EX-EDIT-030`, `EX-EDIT-031`). Un clic sur la palette change le type de
 * tuile actif ; un clic (ou glisser-clic) sur la grille peint ce type à la position visée
 * (`EX-EDIT-002`, y compris l'entrée/la sortie, dont l'unicité est gérée par `LevelDraft`).
 *
 * **Liaison de mécanismes** (`EX-EDIT-003`) : `Maj` + clic sur une tuile `Switch` puis, `Maj`
 * toujours enfoncé, sur une tuile `Door` (ou l'inverse) les lie ; répéter la même paire la délie
 * (bascule). **Redimensionnement** (`EX-EDIT-005`) : les flèches ↑/↓ réduisent/agrandissent la
 * hauteur, ←/→ la largeur. **Annuler/refaire** (`EX-EDIT-005`) : `Ctrl+Z`/`Ctrl+Y`, délégués à
 * `core::LevelDraft::undo`/`redo`.
 *
 * **Enregistrement** (`Ctrl+S`, `EX-EDIT-006`/`007`) : convertit le brouillon en `core::Level`
 * validé (`LevelDraft::toLevel`) et l'écrit dans le dossier `Levels` de l'application ; un
 * brouillon invalide affiche un message d'erreur compréhensible, **aucun fichier n'est écrit**.
 * **Essai immédiat** (`P`, `EX-EDIT-008`) : sur un brouillon valide, lance une session de jeu
 * **intégrée** (délégation à un `GameScreen` interne, sans transition d'écran) qui rejoue le
 * niveau en cours d'édition ; **Échap** (ou la fin du niveau) y met fin et **restitue l'éditeur
 * intact** (brouillon et historique undo/redo jamais touchés pendant l'essai). **Échap** hors
 * essai revient au menu.
 *
 * À l'ouverture, un **sélecteur** (`LevelPicker`) propose « Nouveau niveau » ou l'un des fichiers
 * du dossier `Levels` de l'application (`EX-EDIT-001`) — navigation `↑`/`↓` + `Entrée` — avant
 * d'entrer réellement en édition.
 */
class EditorScreen : public IScreen {
public:
    /**
     * @brief Construit l'éditeur ; affiche d'abord le sélecteur de niveau (nouveau/existant).
     * @param batch          Lot de sprites partagé (rendu ; réutilisé pour l'essai immédiat).
     * @param atlas          Atlas de tuiles fournissant les régions de sprites.
     * @param viewportWidth  Largeur initiale de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur initiale de la surface de rendu, en pixels.
     */
    EditorScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth,
                int viewportHeight);

    /// Nécessaire : `_playtest` est un `unique_ptr` sur un type incomplet dans cet en-tête.
    ~EditorScreen() override;

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    /// Convertit une position souris (pixels écran) en case de grille, si elle est dans les
    /// bornes du brouillon courant.
    [[nodiscard]] std::optional<core::GridPosition> hoveredCell(float mouseX, float mouseY) const;

    /// Traite un clic Maj+souris pour la liaison de mécanismes (voir la doc de la classe).
    void handleLinkClick(float mouseX, float mouseY);

    /// Borne la cible au plafond (`EX-EDIT-017`), puis redimensionne si l'opération est anodine ;
    /// sinon pose une confirmation (`EX-EDIT-012`) et n'applique rien tant qu'elle n'est pas
    /// acceptée. Seul point de passage des flèches et de la boîte de dialogue (`Ctrl+R`).
    void requestResize(int rawWidth, int rawHeight);

    /// Convertit une position souris en case de grille, **bornée** à la grille courante (jamais
    /// `nullopt`) — utilisé par les outils Rectangle/Sélection, dont le glisser doit rester
    /// utilisable même si le curseur dépasse légèrement la grille.
    [[nodiscard]] core::GridPosition clampedCell(float mouseX, float mouseY) const;

    /// Dessine la grille du brouillon (tuiles non vides), les liaisons de mécanismes, la
    /// sélection de liaison en attente et la case survolée en surbrillance.
    void renderGrid(RenderContext& context);

    /// Dessine la palette (couleurs des types + surbrillance de la sélection).
    void renderPalette(RenderContext& context);

    /// Dessine le message de statut courant (erreur de validation, confirmation), s'il y en a un.
    void renderStatus(RenderContext& context);

    /// Enregistre le brouillon (`Ctrl+S`) : valide, puis écrit ; message de statut mis à jour.
    void saveDraft();

    /// Démarre l'essai immédiat (`P`) sur un brouillon valide ; message d'erreur sinon.
    void startPlaytest();

    /// Dessine le sélecteur de niveau (liste « Nouveau niveau » + fichiers existants).
    void renderPicker(RenderContext& context);

    /// Dessine le champ de saisie du nom (création ou renommage `F2`), avec un message de refus
    /// si la dernière tentative de confirmation a été invalide.
    void renderNameInput(RenderContext& context);

    /// Écrit @p level à @p path, met à jour le message de statut et le drapeau `_dirty`.
    void writeLevelToDisk(const core::Level& level, const std::filesystem::path& path);

    /// Dessine la barre d'outils (icônes + surbrillance de la sélection).
    void renderToolBar(RenderContext& context);

    /// Dessine l'aperçu des raccourcis (bascule `F1`) ou l'indice permanent replié.
    void renderHelp(RenderContext& context);

    /// Dessine le fond opaque du panneau latéral, sous la palette/barre d'outils (`EX-EDIT-015`) :
    /// garantit qu'aucune tuile de la grille ne reste visible « sous » l'interface.
    void renderPanelBackground(RenderContext& context);

    const TextureAtlas& _atlas;
    SpriteBatch& _batch;
    int _viewportWidth;
    int _viewportHeight;
    /// Actif tant qu'aucun choix n'a été confirmé ; l'édition ne démarre qu'après.
    std::optional<LevelPicker> _picker;
    core::LevelDraft _draft;
    Camera2D _camera;
    TilePalette _palette;
    bool _paintingDrag = false;  ///< Le clic gauche en cours peint-il la grille (vs. la palette) ?
    float _mouseX = 0.0f;        ///< Dernière position souris connue (pour le rendu du survol).
    float _mouseY = 0.0f;
    /// Première case (Switch ou Door) choisie pour une liaison, en attente de sa contrepartie.
    std::optional<core::GridPosition> _pendingLink;
    /// Session de jeu intégrée active pendant un essai immédiat ; `nullptr` en mode édition normal.
    std::unique_ptr<GameScreen> _playtest;
    std::string _statusMessage;  ///< Dernier message d'erreur/confirmation affiché à l'écran.

    /// Confirmation en attente (`EX-EDIT-012`) : `Entrée` exécute `onConfirm` et applique sa
    /// transition, `Échap` l'annule sans effet. Bloque le reste de l'interaction tant qu'affichée.
    struct PendingConfirmation {
        std::string message;
        std::function<ScreenTransition()> onConfirm;
    };
    std::optional<PendingConfirmation> _pendingConfirmation;
    bool _dirty = false;  ///< `true` si le brouillon a des modifications non enregistrées.

    /// Actif pendant la saisie du nom (création d'un niveau vierge ou renommage `F2`).
    std::optional<TextInputField> _nameInput;
    /// `true` si `_nameInput` sert à nommer un niveau tout juste créé (annuler revient au
    /// sélecteur) ; `false` s'il s'agit d'un renommage en cours d'édition (annuler ne change rien).
    bool _nameInputIsCreation = false;
    /// Chemin du fichier dont le brouillon a été chargé, s'il en existe un (absent pour un niveau
    /// tout juste créé) — sert à distinguer une mise à jour normale d'un écrasement à l'enregistrement.
    std::optional<std::filesystem::path> _loadedFrom;

    /// `true` si la caméra est pilotée manuellement (molette/glisser droit) ; `false` tant que le
    /// cadrage automatique (LOT-14) s'applique — réinitialisable via `Key::D0` (`EX-EDIT-013`).
    bool _manualCamera = false;
    float _cameraZoom = 1.0f;         ///< Zoom courant (manuel ou dernier calcul automatique).
    core::Vector2 _cameraCenter{};    ///< Centre courant (manuel ou dernier calcul automatique).

    /// Outil actif (`EX-EDIT-014`), changé par clic (barre) ou `Tab` (`selectNext`) ; source de
    /// vérité de l'outil courant (comme `_palette` pour le type de tuile).
    ToolBar _toolBar;
    /// `true` pendant un glisser Rectangle/Sélection en cours (mutuellement exclusif avec
    /// `_paintingDrag`, actif seulement quand `_toolBar.selected() != EditorTool::Paint`).
    bool _areaDragActive = false;
    core::GridPosition _areaDragStart{};  ///< Case de départ du glisser Rectangle/Sélection en cours.
    /// Dernière sélection validée par l'outil Sélection (bornes inclusives min/max), pour `Ctrl+C`
    /// ; invalidée par tout redimensionnement ou changement de niveau (peut sortir des bornes).
    std::optional<std::pair<core::GridPosition, core::GridPosition>> _selection;
    /// Presse-papiers local (types de tuiles, `[ligne][colonne]`), pour `Ctrl+C`/`Ctrl+V`.
    std::vector<std::vector<core::TileType>> _clipboard;

    /// `true` si l'aperçu des raccourcis (`F1`) est affiché ; sinon, un indice permanent discret
    /// le rappelle en bas d'écran (`EX-EDIT-015`).
    bool _showHelp = false;

    /// `true` si les lignes de la grille de repère sont dessinées par-dessus les tuiles, pour
    /// simplifier le repérage d'une case avant d'y peindre (bascule `F10`).
    bool _showGridLines = false;
};

}  // namespace hmi
