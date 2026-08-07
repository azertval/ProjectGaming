#pragma once

#include <QString>
#include <QWindow>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileType.h"
#include "Core/Time/FixedTimestep.h"
#include "HMI/Editor/EditorTool.h"
#include "HMI/Game/GameSession.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"

/**
 * @file HMI/Game/GameViewport.h
 * @brief Viewport de l'éditeur : rendu Direct3D 11 du brouillon d'édition et de l'essai
 * (LOT-34/35).
 */

namespace hmi {
class GraphicsDevice;
class SpriteBatch;
class TextureAtlas;
class TextureCache;
class DraftRenderer;
class Localization;
}  // namespace hmi

namespace hmi {

/**
 * @brief Fenêtre native (`QWindow`) où Direct3D 11 présente, embarquée dans Qt.
 *
 * **Mode édition** (LOT-35) : affiche le brouillon (`core::LevelDraft`) via `hmi::DraftRenderer`,
 * caméra cadrant le niveau entier. Le clic/glisser gauche **peint** le type de tuile actif
 * (`setActiveTile`, fourni par la palette) à la case survolée (`core::LevelDraft::paintTile`) ;
 * `Ctrl+Z`/`Ctrl+Y` annulent/refont. **Mode essai** (LOT-35 TACHE-04) : une `hmi::GameSession`
 * rejoue le niveau ; sa boucle à pas fixe (`core::FixedTimestep`) et ses entrées (Qt + XInput)
 * reprennent la discipline du `LOT-33`.
 */
class GameViewport : public QWindow {
    Q_OBJECT

public:
    explicit GameViewport(QWindow* parent = nullptr);
    ~GameViewport() override;

    GameViewport(const GameViewport&) = delete;
    GameViewport& operator=(const GameViewport&) = delete;

    /// Définit le type de tuile peint au clic (relié à la sélection de la palette).
    void setActiveTile(core::TileType type) noexcept {
        _activeTile = type;
    }

    /// Définit l'outil d'édition actif (pinceau, rectangle, sélection ; relié à la barre d'outils).
    void setTool(hmi::EditorTool tool) noexcept {
        _tool = tool;
    }

    /// Fournit le catalogue de traduction (les messages d'état émis sont alors localisés).
    void setLocalization(const Localization* loc) noexcept {
        _loc = loc;
    }

    /// Accès **modifiable** aux touches de jeu (pour le remappage) : la session de jeu lit ces
    /// mêmes bindings, donc un changement s'applique immédiatement.
    [[nodiscard]] hmi::GameKeyBindings& gameBindings() noexcept {
        return _gameBindings;
    }

    /// Accès **modifiable** aux boutons manette (pour le remappage), même principe que
    /// `gameBindings`.
    [[nodiscard]] hmi::GamepadBindings& gamepadBindings() noexcept {
        return _gamepadBindings;
    }

    /// Enregistre le brouillon (`Ctrl+S`) : valide (`LevelDraft::toLevel`) puis écrit le fichier ;
    /// un brouillon invalide n'écrit rien et rapporte l'erreur (`statusMessage`).
    void save();

    /// Ouvre un fichier de niveau comme brouillon d'édition (remplace le brouillon courant).
    void openLevel(const std::filesystem::path& path);

    /// @return true si le brouillon a des modifications non enregistrées (garde-fou d'ouverture).
    [[nodiscard]] bool isDirty() const noexcept {
        return _dirty;
    }

    /// Lance l'essai immédiat (`P`) sur un brouillon valide ; message d'erreur sinon.
    void startPlaytest();

    /// Lance le **jeu** : joue la séquence de niveaux @p levels (mode « Jouer » du menu). `Échap`
    /// ou la fin de la séquence émet `exitToMenuRequested`.
    void startGame(std::vector<std::filesystem::path> levels);

    /// Active/désactive la synchronisation verticale (`EX-REN-022`) ; appliqué au device D3D11.
    void setVSync(bool enabled) noexcept;
    /// @return true si la V-Sync est active.
    [[nodiscard]] bool vsyncEnabled() const noexcept {
        return _vsync;
    }

    /// Redimensionne le niveau en cours d'édition (`EX-EDIT-005`).
    void resizeLevel(int width, int height);

    /// @return true si redimensionner à (@p width, @p height) supprimerait du contenu posé.
    [[nodiscard]] bool wouldResizeDrop(int width, int height) const;

    /// @return Dimensions courantes du niveau, en cases.
    [[nodiscard]] int levelWidth() const;
    [[nodiscard]] int levelHeight() const;

    /// @return Le brouillon en cours d'édition (lecture seule) — consommé par le panneau « Liens ».
    [[nodiscard]] const core::LevelDraft& draft() const noexcept {
        return _draft;
    }

    /// Met en surbrillance la liaison (déclencheur, cible) dans le viewport (sélection depuis le
    /// panneau « Liens ») ; n'affecte que le rendu, pas le brouillon.
    void setHighlightedLink(std::optional<std::pair<core::GridPosition, core::GridPosition>> link);

    /// Supprime la liaison dont la cible est @p targetPosition (bouton « Supprimer » du panneau
    /// « Liens ») ; sans effet si @p targetPosition n'a pas de liaison.
    void unlinkMechanism(core::GridPosition targetPosition);

    /// @return Le mode de rendu courant, commun à l'édition, à l'essai et au jeu réel.
    [[nodiscard]] RenderMode renderMode() const noexcept {
        return _renderMode;
    }

    /**
     * @brief Catalogue des jeux de skins, édité par le panneau « Textures » (`LOT-42`).
     *
     * Le viewport en reste **propriétaire** : le panneau agit dessus, et le rendu voit les mêmes
     * assignations à l'image suivante, sans reconstruire la scène.
     * @return Le catalogue.
     */
    [[nodiscard]] SkinCatalog& skinCatalog() noexcept {
        return _skins;
    }

    /**
     * @brief Désigne le jeu de skins à utiliser pour le rendu (`LOT-42`).
     * @param setName Nom du jeu ; vide pour le jeu par défaut du catalogue.
     */
    void setSkinSet(const std::string& setName);

    /**
     * @brief Rechargement à chaud **global** des assets graphiques (`LOT-43` TACHE-03).
     *
     * Invalide tout le `TextureCache` et relit `skins.json` — un asset a pu être modifié,
     * renommé ou ajouté hors de l'application. Ne touche ni au brouillon en cours d'édition ni à
     * son historique d'annulation : c'est une opération de **présentation**, pas d'édition.
     * L'apparence est résolue à la composition de chaque image (`hmi::DraftRenderer`), donc
     * l'image suivante suffit à montrer le résultat — aucune reconstruction de scène nécessaire.
     */
    void reloadAssets();

    /**
     * @brief Rechargement à chaud **ciblé** d'un seul asset (`LOT-43` TACHE-03).
     *
     * Évite de relire toute la bibliothèque à chaque sauvegarde — utilisé par l'atelier pixel art
     * (`LOT-54`) après chaque enregistrement.
     * @param fileName Nom logique de l'asset à invalider (relatif à `Assets/`).
     */
    void invalidateAsset(const std::string& fileName);

    /**
     * @brief Choisit le mode de rendu et **persiste** le choix (`EX-REN-046`, `EX-IHM-011`).
     *
     * Purement visuel : ni la simulation, ni la scène ECS, ni le brouillon ne sont touchés — la
     * bascule ne coûte donc pas un pas fixe ni un rechargement de niveau.
     * @param mode Mode à appliquer.
     */
    void setRenderMode(RenderMode mode);

signals:
    /// Message d'état à afficher (enregistrement, essai, erreur de validation…).
    void statusMessage(const QString& message);
    /// Demande de retour au menu principal (fin de partie ou `Échap` en mode jeu).
    void exitToMenuRequested();
    /// Le brouillon vient d'être modifié (peinture, undo/redo, chargement, lien…) — le panneau
    /// « Liens » se resynchronise dessus (`refresh`).
    void draftChanged();
    /// Le mode de rendu vient de basculer (`F8`) — la palette met ses vignettes à jour pour rester
    /// fidèle au canevas (`EX-EDIT-027`).
    void renderModeChanged(RenderMode mode);

protected:
    bool event(QEvent* event) override;
    void exposeEvent(QExposeEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void ensureResources();
    void tick();
    void renderFrame();
    void stopPlaytest();  ///< Termine l'essai et restitue l'éditeur (brouillon intact).
    void loadGameLevel(std::size_t index);  ///< Charge le niveau @p index de la séquence de jeu.
    void updateMousePosition(const QMouseEvent* event);

    /// Recale la caméra d'édition pour cadrer le niveau entier dans la surface courante.
    void updateEditCamera();
    /// Case de grille sous la position écran donnée (pixels physiques), si dans les bornes.
    [[nodiscard]] std::optional<core::GridPosition> cellAt(const QMouseEvent* event);
    /// Case de grille sous @p event, **bornée** à la grille (jamais hors bornes) — pour les
    /// glissers rectangle/sélection dont le curseur peut dépasser légèrement les bords.
    [[nodiscard]] core::GridPosition clampedCell(const QMouseEvent* event);
    /// Peint le type actif à la case sous @p event, si valide (invalide la scène rendue).
    void paintAt(const QMouseEvent* event);
    /// Remplit le rectangle (bornes @p a, @p b incluses) du type actif (outil Rectangle).
    void applyRectangle(core::GridPosition a, core::GridPosition b);
    /// Copie la zone sélectionnée dans le presse-papiers local (`Ctrl+C`).
    void copySelection();
    /// Colle le presse-papiers à la case survolée (`Ctrl+V`).
    void pasteClipboard();
    /// Zone à mettre en surbrillance (glisser en cours, sinon sélection mémorisée), le cas échéant.
    [[nodiscard]] std::optional<std::pair<core::GridPosition, core::GridPosition>> highlight()
        const;
    /// Résout le clic de l'outil Lien (`hmi::resolveLinkClick`) et l'applique (attente, liaison).
    void handleLinkClick(const QMouseEvent* event);
    /// @return true si (@p switchPosition, @p targetPosition) est déjà une liaison du brouillon.
    [[nodiscard]] bool linkExists(core::GridPosition switchPosition,
                                  core::GridPosition targetPosition) const;
    /// Invalide le rendu du brouillon et notifie les panneaux dépendants (`draftChanged`) — à
    /// appeler après toute mutation de `_draft` (peinture, lien, undo/redo, chargement…).
    void markDraftMutated();

    [[nodiscard]] int pixelWidth() const;
    [[nodiscard]] int pixelHeight() const;

    /// Texte localisé d'une clé de statut (repli sur la clé si aucun catalogue).
    [[nodiscard]] QString statusText(const char* key) const;

    using Clock = std::chrono::steady_clock;

    std::unique_ptr<hmi::GraphicsDevice> _graphics;
    std::unique_ptr<hmi::SpriteBatch> _spriteBatch;
    std::unique_ptr<hmi::TextureAtlas> _atlas;
    std::unique_ptr<hmi::TextureCache> _textureCache;
    std::unique_ptr<hmi::DraftRenderer> _draftRenderer;
    /// Catalogue des jeux de skins (`LOT-42`), lu au démarrage depuis `Assets/skins.json`. Vide si
    /// le fichier est absent ou illisible : tout retombe alors sur le damier, sans bloquer.
    hmi::SkinCatalog _skins;
    /// Jeu de skins courant ; vide pour le jeu par défaut du catalogue.
    std::string _skinSet;
    hmi::GameKeyBindings _gameBindings;
    hmi::GamepadBindings _gamepadBindings;
    hmi::InputState _input;
    hmi::GamepadPoller _gamepad;
    core::FixedTimestep _timestep;
    Clock::time_point _previousFrame;
    bool _loopStarted = false;

    core::LevelDraft _draft;  ///< Brouillon en cours d'édition (source de vérité).
    hmi::Camera2D _camera;    ///< Caméra d'édition (cadre le niveau entier).
    core::TileType _activeTile = core::TileType::Solid;  ///< Type peint au clic (palette).
    hmi::EditorTool _tool = hmi::EditorTool::Paint;  ///< Outil d'édition actif (barre d'outils).
    bool _painting = false;             ///< Un glisser de peinture (Pinceau) est en cours.
    bool _dragging = false;             ///< Un glisser Rectangle/Sélection est en cours.
    core::GridPosition _dragStart{};    ///< Case de départ du glisser Rectangle/Sélection.
    core::GridPosition _dragCurrent{};  ///< Case courante du glisser (pour l'aperçu).
    core::GridPosition _hoverCell{};    ///< Dernière case survolée (cible du collage).
    /// Sélection mémorisée (bornes min/max incluses), pour copier (`Ctrl+C`).
    const Localization* _loc = nullptr;  ///< Catalogue pour localiser les messages d'état.

    std::optional<std::pair<core::GridPosition, core::GridPosition>> _selection;
    /// Case du déclencheur/de la cible en attente d'appariement (outil Lien, premier clic).
    std::optional<core::GridPosition> _pendingLink;
    /// Liaison sélectionnée dans le panneau « Liens » (déclencheur, cible) : mise en surbrillance.
    std::optional<std::pair<core::GridPosition, core::GridPosition>> _selectedLink;
    /// Presse-papiers local (types de tuiles, `[ligne][colonne]`), pour `Ctrl+C`/`Ctrl+V`.
    std::vector<std::vector<core::TileType>> _clipboard;
    bool _dirty = false;    ///< Modifications non enregistrées (garde-fou d'ouverture).
    bool _showGrid = true;  ///< Grille de repère (cases + salles) affichée (bascule F10).
    /// Mode de rendu courant (bascule `F8`, `EX-REN-046`), restauré des préférences au démarrage
    /// et réécrit à chaque bascule. Porté par le viewport parce qu'il est le widget **unique**
    /// derrière l'édition, l'essai et le jeu réel (`EX-IHM-002`) : un seul état, jamais dupliqué.
    RenderMode _renderMode = DEFAULT_RENDER_MODE;
    bool _vsync = true;      ///< Synchronisation verticale (appliquée au device D3D11).
    bool _gameMode = false;  ///< La session courante est une **partie** (menu Jouer) et
                             ///< non un essai depuis l'éditeur (enchaînement/retour menu).
    std::vector<std::filesystem::path> _gameLevels;  ///< Séquence de niveaux du mode jeu.
    std::size_t _gameLevel = 0;                      ///< Indice du niveau courant dans la séquence.

    /// Session de jeu de l'essai immédiat ; nulle en mode édition (essai ajouté au LOT-35
    /// TACHE-04).
    std::optional<hmi::GameSession> _session;
};

}  // namespace hmi
