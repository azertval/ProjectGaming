#pragma once

#include <QString>
#include <QWindow>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Core/Levels/Decor.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Rect.h"
#include "Core/Time/FixedTimestep.h"
#include "HMI/Editor/DecorGesture.h"
#include "HMI/Editor/EditorTool.h"
#include "HMI/Game/GameSession.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/LayerVisibility.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Input/EditorKeyBindings.h"
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
class BitmapFont;
}  // namespace hmi

namespace hmi {

/**
 * @brief Fenêtre native (`QWindow`) où Direct3D 11 présente, embarquée dans Qt.
 *
 * **Mode édition** (LOT-35) : affiche le brouillon (`core::LevelDraft`) via `hmi::DraftRenderer`,
 * caméra cadrant le niveau entier par défaut. Le clic/glisser gauche **peint** le type de tuile
 * actif (`setActiveTile`, fourni par la palette) à la case survolée (`core::LevelDraft::
 * paintTile`) ; `Ctrl+Z`/`Ctrl+Y` annulent/refont. La molette **zoome** manuellement (borné entre
 * l'ajustement automatique et 4 cases visibles sur le plus petit axe) et le glisser **bouton
 * droit** déplace la caméra (« pan ») ; `0` revient au cadrage automatique (LOT-15). **Mode essai**
 * (LOT-35 TACHE-04) : une `hmi::GameSession` rejoue le niveau ; sa boucle à pas fixe
 * (`core::FixedTimestep`) et ses entrées (Qt + XInput) reprennent la discipline du `LOT-33`.
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

    /// Définit l'asset assigné au clic par l'outil « Texture par instance » (`LOT-45`), relié à la
    /// sélection de la bibliothèque « Objets » ; vide si aucun asset n'est sélectionné.
    void setActiveTextureAsset(std::optional<std::string> asset) noexcept {
        _activeTextureAsset = std::move(asset);
    }

    /// Définit l'asset posé au clic par l'outil Décor (`LOT-49`), relié à la sélection de la
    /// bibliothèque de décors du panneau « Outils » ; vide si aucun asset n'est sélectionné.
    void setActiveDecorAsset(std::optional<std::string> asset) noexcept {
        _activeDecorAsset = std::move(asset);
    }

    /// Définit la couche du décor posé au clic (`LOT-49`, `EX-DEC-002`), reliée au sélecteur de
    /// couche du panneau « Outils ».
    void setActiveDecorLayer(core::DecorLayer layer) noexcept {
        _activeDecorLayer = layer;
    }

    /// Active/désactive l'aimantation sur la grille du geste de décors (`LOT-50`, `EX-DEC-001`) —
    /// optionnelle et jamais imposée, un décor reste libre par construction.
    void setDecorSnapToGrid(bool enabled) noexcept {
        _decorSnapToGrid = enabled;
    }

    /**
     * @brief Définit l'outil d'édition actif (pinceau, rectangle, sélection, lien, texture par
     *        instance).
     *
     * Émet `toolChanged` si l'outil change réellement, pour que la barre d'outils reste
     * synchronisée quand le changement vient d'ailleurs qu'un clic sur son panneau — le raccourci
     * clavier de l'outil « Texture par instance » (`LOT-45`), aujourd'hui seul cas.
     */
    void setTool(hmi::EditorTool tool);

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

    /// @return Le décor actuellement sélectionné (rang dans `draft().decors()`), si un l'est —
    /// consommé par la section « Décors » du panneau « Textures » (`LOT-50` TACHE-04).
    [[nodiscard]] std::optional<std::size_t> selectedDecorIndex() const noexcept {
        return _decorGesture.selectedIndex;
    }

    /// Met en surbrillance la liaison (déclencheur, cible) dans le viewport (sélection depuis le
    /// panneau « Liens ») ; n'affecte que le rendu, pas le brouillon.
    void setHighlightedLink(std::optional<std::pair<core::GridPosition, core::GridPosition>> link);

    /// Supprime la liaison dont la cible est @p targetPosition (bouton « Supprimer » du panneau
    /// « Liens ») ; sans effet si @p targetPosition n'a pas de liaison.
    void unlinkMechanism(core::GridPosition targetPosition);

    /// Retire l'override de texture de @p position (bouton « Retirer » de la section « Objets »,
    /// `LOT-45`) ; sans effet si @p position n'en a pas.
    void removeTextureOverride(core::GridPosition position);

    /// Met en surbrillance une case portant un override (sélection depuis la section « Objets »,
    /// `LOT-45`) ; n'affecte que le rendu, pas le brouillon. Réutilise le même voile que l'outil
    /// Rectangle/Sélection (case unique).
    void setHighlightedTextureOverride(std::optional<core::GridPosition> position);

    /**
     * @brief Assigne l'asset de fond du niveau courant (section « Fond », `LOT-44`).
     * @param background Nom de l'asset (`Assets/Backgrounds/*.png`), ou absent pour retirer le
     *                    fond posé.
     */
    void setLevelBackground(std::optional<std::string> background);

    /**
     * @brief Assigne le jeu de skins du niveau courant (section « Fond », `LOT-44`).
     *
     * Propriété **persistée du niveau** (`core::LevelDraft::skinSet`) : distincte du jeu de skins
     * **courant d'édition** (`setSkinSet`, session de l'éditeur, `LOT-42`) — les deux ne se
     * contaminent jamais l'un l'autre.
     * @param skinSet Nom du jeu (`skins.json`), ou absent pour le jeu par défaut.
     */
    void setLevelSkinSet(std::optional<std::string> skinSet);

    /// @return Le mode de rendu courant, commun à l'édition, à l'essai et au jeu réel.
    [[nodiscard]] RenderMode renderMode() const noexcept {
        return _renderMode;
    }

    /**
     * @brief Jeu de visibilités par calque du mode d'inspection « définition des textures »
     *        (`LOT-51`, `EX-EDIT-044`) — édition uniquement, jamais lu par `hmi::GameSession`.
     * @return Le jeu de visibilités courant.
     */
    [[nodiscard]] const LayerVisibility& layerVisibility() const noexcept {
        return _layerVisibility;
    }

    /// Affiche ou masque un calque du mode d'inspection (section « Calques » du panneau
    /// « Textures », `LOT-51`). Comme `setRenderMode`, purement visuel : la scène ECS n'est jamais
    /// invalidée, seule la résolution d'apparence de l'image suivante en tient compte. Aucune
    /// persistance entre deux sessions (TACHE-01) : contrairement à `setRenderMode`, rien n'est
    /// écrit dans les préférences.
    void setLayerVisible(RenderLayer layer, bool visible) noexcept {
        _layerVisibility.setVisible(layer, visible);
    }

    /// Rétablit la visibilité de tous les calques (action « tout afficher », TACHE-03).
    void showAllLayers() noexcept {
        _layerVisibility.showAll();
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

    /**
     * @brief Sélectionne (ou désélectionne) un décor depuis la section « Décors » du panneau
     *        « Textures » (`LOT-50` TACHE-04) — l'autre sens de la sélection croisée avec le
     *        canevas (`hmi::DecorGesture`, TACHE-02).
     * @param index Rang du décor à sélectionner, ou `std::nullopt` pour désélectionner.
     */
    void selectDecor(std::optional<std::size_t> index);

    /// Réordonne le décor @p index d'un cran vers l'avant de sa couche (bouton « Avancer » de la
    /// section « Décors »), annulable (`core::LevelDraft::bringDecorForward`).
    void bringDecorForward(std::size_t index);
    /// Symétrique de `bringDecorForward` (bouton « Reculer »).
    void sendDecorBackward(std::size_t index);
    /// Change la couche du décor @p index (section « Décors »), annulable
    /// (`core::LevelDraft::setDecorLayer`).
    void setDecorLayer(std::size_t index, core::DecorLayer layer);
    /// Supprime le décor @p index (bouton « Supprimer » de la section « Décors »).
    void removeDecor(std::size_t index);
    /// Recadre la caméra d'édition sur le décor @p index (bouton « Centrer »), à son zoom courant
    /// — un décor hors du cadrage automatique (grand niveau) redevient ainsi atteignable sans
    /// avoir à le chercher à la souris.
    void centerCameraOnDecor(std::size_t index);

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
    /// L'outil actif vient de changer par un moyen autre que le panneau Outils (raccourci clavier,
    /// `LOT-45`) — le panneau se resynchronise sans reboucler.
    void toolChanged(hmi::EditorTool tool);
    /// La sélection de décor vient de changer, par un moyen autre que la section « Décors »
    /// (clic/pose/suppression au canevas, `LOT-50` TACHE-04) — le panneau se resynchronise.
    void decorSelectionChanged(std::optional<std::size_t> index);

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
    /// @p deltaSeconds : temps réel écoulé depuis l'image précédente (LOT-46 TACHE-05, avance
    /// l'aperçu des tuiles animées de l'éditeur) ; `0` (défaut) pour un redessin sans avancer
    /// l'aperçu (redimensionnement).
    void renderFrame(float deltaSeconds = 0.0f);
    void stopPlaytest();  ///< Termine l'essai et restitue l'éditeur (brouillon intact).
    void loadGameLevel(std::size_t index);  ///< Charge le niveau @p index de la séquence de jeu.
    void updateMousePosition(const QMouseEvent* event);

    /// Recale la caméra d'édition : cadrage automatique sur le niveau entier, sauf pan/zoom manuel
    /// actif (molette/glisser bouton droit), auquel cas la caméra manuelle est réappliquée.
    void updateEditCamera();
    /// Position écran (pixels physiques) d'un événement souris, convertie par le ratio d'affichage
    /// — même conversion que `updateMousePosition`, réutilisée pour le pan manuel.
    [[nodiscard]] core::Vector2 screenPosition(const QMouseEvent* event) const;
    /// @return Le zoom manuel minimal autorisé : celui de l'ajustement automatique du brouillon
    /// courant (rien à voir de plus loin).
    [[nodiscard]] float minManualZoom() const;
    /// @return Le zoom manuel maximal autorisé : la valeur laissant encore 4 cases visibles sur le
    /// plus petit axe de l'écran (précision jugée suffisante pour poser un bloc).
    [[nodiscard]] float maxManualZoom() const;
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
    /// Résout le clic de l'outil « Texture par instance » (`hmi::resolveTextureAssignClick`) et
    /// l'applique (assignation/retrait) ; @p rightClick distingue le clic droit (retrait explicite).
    void handleTextureAssignClick(const QMouseEvent* event, bool rightClick);
    /// Convertit la position écran d'un événement souris en position **monde**, non calée sur la
    /// grille (`EX-DEC-001`) — contrairement à `cellAt`/`clampedCell`, utilisées par les autres
    /// outils.
    [[nodiscard]] core::Vector2 worldPositionAt(const QMouseEvent* event);
    /// Dimensions réelles (en pixels) de l'asset d'un décor, damier de repli si introuvable —
    /// résolution directe via `_textureCache`, même repli que `hmi::resolveDecorAppearance`
    /// (`LOT-49`) mais sans construire une `hmi::SceneTextures` complète.
    [[nodiscard]] core::Vector2 decorPixelSize(const std::string& assetName) const;
    /// Rectangles englobants de tous les décors du brouillon courant (`hmi::decorWorldBounds`),
    /// alignés avec `_draft.decors()` — pour la désignation et le geste de l'outil Décor
    /// (`LOT-50` TACHE-02).
    [[nodiscard]] std::vector<core::Rect> decorBoundsForGesture() const;
    /// Poignées du décor actuellement sélectionné (`hmi::decorHandleLayout`), si un l'est.
    [[nodiscard]] std::optional<hmi::DecorHandleLayout> selectedDecorHandles() const;
    /// Appui (outil Décor, `LOT-50` TACHE-02) : désigne l'élément sous le clic. Un décor/une
    /// poignée désigné(e) arme le geste de manipulation (`hmi::beginDecorGesture`) ; sinon, pose un
    /// nouveau décor si un asset est sélectionné dans la bibliothèque (`LOT-49`), ou déselectionne.
    void handleDecorPress(const QMouseEvent* event);
    /// Glisser en cours (outil Décor) : fait progresser le geste, met à jour l'aperçu
    /// (`LOT-50` TACHE-03) sans jamais muter `_draft`.
    void handleDecorMove(const QMouseEvent* event);
    /// Relâchement (outil Décor) : termine le geste et applique l'action finale aux mutateurs de
    /// TACHE-01 (un seul appel, donc une seule entrée d'historique par geste complet) ; clic droit
    /// retire le décor visé.
    void handleDecorRelease(const QMouseEvent* event, bool rightClick);
    /// Abandonne un glisser de décor en cours (`Échap`, `LOT-50` TACHE-02) : restaure l'aperçu,
    /// aucune mutation du brouillon (rien n'y avait été écrit pendant le glisser).
    void cancelDecorGesture();
    /// Applique l'action finale d'un geste de décors (`hmi::endDecorGesture`) aux mutateurs de
    /// TACHE-01 ; sans effet pour `DecorGestureActionKind::None` (simple clic/sélection).
    void applyDecorGestureAction(const hmi::DecorGestureAction& action);
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
    /// Police bitmap de l'affichage tête haute (`LOT-52`), chargée une fois au démarrage comme
    /// `_atlas` (pas de rechargement à chaud).
    std::unique_ptr<hmi::BitmapFont> _font;
    /// Catalogue des jeux de skins (`LOT-42`), lu au démarrage depuis `Assets/skins.json`. Vide si
    /// le fichier est absent ou illisible : tout retombe alors sur le damier, sans bloquer.
    hmi::SkinCatalog _skins;
    /// Jeu de skins courant ; vide pour le jeu par défaut du catalogue.
    std::string _skinSet;
    hmi::GameKeyBindings _gameBindings;
    hmi::GamepadBindings _gamepadBindings;
    hmi::EditorKeyBindings _editorBindings;  ///< Raccourcis d'éditeur remappables (`LOT-45`).
    hmi::InputState _input;
    hmi::GamepadPoller _gamepad;
    core::FixedTimestep _timestep;
    Clock::time_point _previousFrame;
    bool _loopStarted = false;

    core::LevelDraft _draft;  ///< Brouillon en cours d'édition (source de vérité).
    hmi::Camera2D _camera;    ///< Caméra d'édition (cadre le niveau entier par défaut).
    /// `true` dès que la molette ou un glisser bouton droit a été utilisé : la caméra suit
    /// `_manualZoom`/`_manualCenter` plutôt que le cadrage automatique, jusqu'à la réinitialisation
    /// (`0`) ou l'ouverture d'un autre niveau. Restaure le pan/zoom manuel de l'éditeur historique
    /// (`EditorScreen`, LOT-15), perdu lors de la réécriture Direct3D11/Qt.
    bool _manualCamera = false;
    float _manualZoom = 1.0f;            ///< Zoom manuel courant, actif si `_manualCamera`.
    core::Vector2 _manualCenter{};       ///< Centre manuel courant, actif si `_manualCamera`.
    bool _rightDragging = false;         ///< Un glisser bouton droit est en cours (pan potentiel).
    /// `true` dès qu'un glisser bouton droit a dépassé le seuil de mouvement : distingue un pan
    /// d'un simple clic droit (retrait d'override, outil « Texture par instance »).
    bool _cameraPanned = false;
    core::Vector2 _rightDragLastScreen{};  ///< Dernière position écran du glisser droit en cours.
    core::TileType _activeTile = core::TileType::Solid;  ///< Type peint au clic (palette).
    /// Asset assigné au clic par l'outil « Texture par instance » (`LOT-45`), vide si aucun n'est
    /// sélectionné dans la bibliothèque « Objets ».
    std::optional<std::string> _activeTextureAsset;
    /// Asset posé au clic par l'outil Décor (`LOT-49`), vide si aucun n'est sélectionné dans la
    /// bibliothèque de décors.
    std::optional<std::string> _activeDecorAsset;
    /// Couche du décor posé au clic (`LOT-49`, `EX-DEC-002`) ; `Decor` par défaut (couche de
    /// référence, `EX-DEC-006`).
    core::DecorLayer _activeDecorLayer = core::DecorLayer::Decor;
    /// État du geste de manipulation de décors (`LOT-50` TACHE-02) ; `selectedIndex` porte la
    /// sélection courante de l'éditeur, au-delà de la durée d'un seul geste.
    hmi::DecorGestureState _decorGesture;
    /// Aimantation sur la grille du geste de décors (`LOT-50`), optionnelle, désactivée par défaut.
    bool _decorSnapToGrid = false;
    /// Aperçu courant du geste de décors en cours (`LOT-50` TACHE-03) — jamais écrit dans `_draft`,
    /// seul `hmi::DraftRenderer` le consomme pour afficher la manipulation avant validation.
    /// `std::nullopt` hors glisser.
    std::optional<hmi::DecorGestureAction> _decorPreview;
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
    /// Case sélectionnée dans la section « Objets » du panneau « Textures » (`LOT-45`) : mise en
    /// surbrillance, même voile que l'outil Rectangle/Sélection.
    std::optional<core::GridPosition> _highlightedOverride;
    /// Presse-papiers local (types de tuiles, `[ligne][colonne]`), pour `Ctrl+C`/`Ctrl+V`.
    std::vector<std::vector<core::TileType>> _clipboard;
    bool _dirty = false;    ///< Modifications non enregistrées (garde-fou d'ouverture).
    bool _showGrid = true;  ///< Grille de repère (cases + salles) affichée (bascule F10).
    /// Mode de rendu courant (bascule `F8`, `EX-REN-046`), restauré des préférences au démarrage
    /// et réécrit à chaque bascule. Porté par le viewport parce qu'il est le widget **unique**
    /// derrière l'édition, l'essai et le jeu réel (`EX-IHM-002`) : un seul état, jamais dupliqué.
    RenderMode _renderMode = DEFAULT_RENDER_MODE;
    /// Jeu de visibilités par calque du mode d'inspection « définition des textures » (`LOT-51`) —
    /// tout visible par défaut, jamais persisté (TACHE-01), édition uniquement.
    LayerVisibility _layerVisibility;
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
