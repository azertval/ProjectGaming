#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Entity.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/GridPosition.h"
#include "HMI/Editor/DecorGesture.h"
#include "HMI/Graphics/LayerVisibility.h"
#include "HMI/Graphics/SpriteRenderer.h"

/**
 * @file HMI/Graphics/DraftRenderer.h
 * @brief Rendu d'un brouillon d'édition (`core::LevelDraft`) dans le viewport (LOT-35).
 */

namespace core {
class LevelDraft;
}

namespace hmi {

class SpriteBatch;
class TextureAtlas;
class TextureCache;
class Camera2D;

/**
 * @brief État d'affichage des liens de mécanismes (LOT-37), fourni par le viewport à chaque rendu.
 *
 * Purement informatif pour `DraftRenderer` : ne modifie jamais le brouillon, seulement la
 * présentation (surbrillance, trait provisoire).
 */
struct LinkOverlayState {
    /// Case survolée par la souris (surbrillance des liens incidents ; extrémité du trait
    /// provisoire si `pendingLink` est posé).
    std::optional<core::GridPosition> hoveredCell;
    /// Case du déclencheur/de la cible en attente d'appariement (outil « Lien », premier clic).
    std::optional<core::GridPosition> pendingLink;
    /// Liaison sélectionnée dans le panneau « Liens » (déclencheur, cible) : mise en surbrillance.
    std::optional<std::pair<core::GridPosition, core::GridPosition>> selectedLink;
};

/**
 * @brief État d'affichage de la manipulation de décors (`LOT-50` TACHE-02/03), fourni par le
 *        viewport à chaque rendu — jamais écrit dans le brouillon, purement présentatif.
 */
struct DecorOverlayState {
    /// Décor actuellement sélectionné (rang dans `core::LevelDraft::decors()`), si un l'est.
    std::optional<std::size_t> selectedIndex;
    /// Action d'**aperçu** du geste en cours (`hmi::updateDecorGesture`), appliquée par-dessus le
    /// décor sélectionné pour l'affichage **sans** toucher au brouillon — c'est ce qui permet de
    /// montrer une manipulation « avant validation » (TACHE-03). `std::nullopt` hors glisser.
    std::optional<DecorGestureAction> preview;
    /// Aimantation sur la grille active (indication visuelle : grille de repère accentuée).
    bool snapToGrid = false;
};

/**
 * @brief Dessine la grille d'un `core::LevelDraft` en cours d'édition.
 *
 * Réutilise le pipeline de rendu du jeu : chaque tuile non vide du brouillon devient une entité
 * (`Transform` + `Sprite`) d'un `core::World` interne, composée par `hmi::composeWorldSprites`
 * (lecture seule, `EX-ARCH-012`). Le `core::World` n'est **reconstruit** que lorsque le brouillon
 * change (`invalidate()`), pas à chaque frame. Les blocs à taille réduite sont dessinés à leur
 * échelle réelle (`core::tileVisualScale`), comme en jeu — cohérence visuelle stricte.
 *
 * Depuis `LOT-40`, **toutes** les primitives d'une image (tuiles, grille de repère, liens de
 * mécanismes, aperçu de sélection) sont composées dans une seule `hmi::ComposedScene` puis
 * soumises en bloc : les aides d'édition portent le calque `RenderLayer::EditorOverlay`, qui les
 * place au-dessus de tout le reste par construction plutôt que par l'ordre des appels de dessin.
 * La liste obtenue est donc inspectable sans GPU (`EX-NFR-004`) et soumise au culling
 * (`EX-NFR-005`).
 */
class DraftRenderer {
public:
    DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache);

    /// Rend le brouillon avec la caméra donnée (reconstruit la scène si invalidée). Si @p showGrid,
    /// superpose la grille de repère (frontières de cases + frontières de salles) — aide au
    /// placement, équivalent de la bascule `F10` de l'éditeur historique (`EX-EDIT-023`). Si
    /// @p highlight est présent, met en surbrillance la zone (bornes min/max incluses) — aperçu de
    /// l'outil Rectangle/Sélection. @p linkOverlay pilote l'affichage des liens de mécanismes
    /// (flèches, trait provisoire, surbrillance — `EX-IHM-030`). @p mode choisit le rendu
    /// Physique ou Texture des tuiles (`EX-REN-046`, `LOT-41`) ; les **aides d'édition** (grille,
    /// liens, aperçu) restent identiques dans les deux modes — ce sont des repères d'édition, pas
    /// de l'habillage. Si @p showTextureOverrides, les cases portant une surcharge de texture par
    /// instance (`EX-EDIT-043`, `LOT-45`) sont signalées sur le calque d'édition — actif seulement
    /// quand l'outil « Texture par instance » l'est, pour ne pas encombrer les autres outils.
    /// @p deltaSeconds avance l'aperçu des tuiles animées (`LOT-46` TACHE-05) en **temps réel** —
    /// contrairement à la simulation (`hmi::GameSession`), l'aperçu d'édition n'a aucune exigence
    /// de déterminisme (`EX-NFR-002` ne s'applique qu'en jeu) ; `0` (par défaut) fige l'animation.
    /// @p decorOverlay pilote le cadre de sélection, les poignées et l'aperçu de manipulation des
    /// décors (`LOT-50` TACHE-02/03) — décor par défaut (aucune sélection) si omis. Ces aides,
    /// comme le reste du calque `RenderLayer::EditorOverlay`, ne sont **jamais** composées en jeu
    /// ni en essai (`hmi::GameSession` ne passe jamais par `DraftRenderer`). @p visibility pilote
    /// le mode d'inspection « définition des textures » (`LOT-51`, `EX-EDIT-044`) — tout visible
    /// par défaut, sans effet sur les aides d'édition ci-dessus (jamais un calque de contenu).
    void render(const core::LevelDraft& draft, const Camera2D& camera, bool showGrid,
                const std::optional<std::pair<core::GridPosition, core::GridPosition>>& highlight,
                const LinkOverlayState& linkOverlay, RenderMode mode,
                bool showTextureOverrides = false, float deltaSeconds = 0.0f,
                const DecorOverlayState& decorOverlay = {}, const LayerVisibility& visibility = {});

    /// Marque la scène comme périmée : elle sera reconstruite au prochain `render` (à appeler après
    /// toute mutation du brouillon — peinture, undo/redo, chargement, redimensionnement).
    void invalidate() noexcept {
        _dirty = true;
    }

    /// @return La scène composée à la dernière image (primitives soumises et compteurs).
    [[nodiscard]] const ComposedScene& lastScene() const noexcept {
        return _scene;
    }

    /**
     * @brief Désigne le catalogue de skins et le jeu à utiliser en mode Texture (`LOT-42`).
     *
     * Le catalogue n'est **pas** copié : l'appelant en reste propriétaire. Réassigner un skin s'y
     * voit à l'image suivante, sans reconstruire le brouillon ni la scène.
     * @param skins   Catalogue, ou `nullptr` pour retomber entièrement sur le damier.
     * @param skinSet Nom du jeu courant ; vide pour le jeu par défaut du catalogue.
     */
    void setSkins(const SkinCatalog* skins, std::string skinSet = {}) {
        _skins = skins;
        _skinSet = std::move(skinSet);
    }

private:
    void rebuild(const core::LevelDraft& draft);
    /// Compose la grille de repère (frontières de cases + de salles) sur le calque d'édition. Si
    /// @p accentuate, les lignes sont plus opaques — indication visuelle de l'aimantation active
    /// du geste de décors (`LOT-50` TACHE-03), sinon l'auteur ne comprend pas pourquoi sa position
    /// « saute ».
    void composeGrid(const core::LevelDraft& draft, bool accentuate);
    /// Compose les liens de mécanismes (flèches déclencheur → cible) sur le calque d'édition.
    void composeLinks(const core::LevelDraft& draft, const LinkOverlayState& overlay);
    /// Compose le parcours de chaque plateforme mobile (trait + pointe de flèche, `EX-GP-026`,
    /// `LOT-63`) sur le calque d'édition.
    void composeMovingPlatformPaths(const core::LevelDraft& draft);
    /// Compose le voile d'aperçu d'une zone (outil Rectangle/Sélection) sur le calque d'édition.
    void composeHighlight(const core::GridPosition& minimum, const core::GridPosition& maximum);
    /// Signale les cases portant une surcharge de texture par instance sur le calque d'édition
    /// (`EX-EDIT-043`, `LOT-45`).
    void composeTextureOverrideMarkers(const core::LevelDraft& draft);
    /// Compose le cadre de sélection et les poignées du décor sélectionné (`LOT-50` TACHE-03),
    /// à sa position d'**aperçu** si un glisser est en cours (`DecorOverlayState::preview`) — sans
    /// jamais lire ni écrire `_draft` au-delà de la copie locale nécessaire à l'aperçu. Sans effet
    /// si aucun décor n'est sélectionné.
    void composeDecorSelection(const core::LevelDraft& draft, const DecorOverlayState& decorOverlay,
                               const Camera2D& camera);

    SpriteBatch& _batch;
    const TextureAtlas& _atlas;
    TextureCache& _cache;
    const SkinCatalog* _skins = nullptr;  // non possédé
    std::string _skinSet;
    ComposedScene _scene;
    core::World _world;
    /// Entités de décor de `_world`, dans le même ordre que `core::LevelDraft::decors()`
    /// (peuplé par `rebuild`) — permet d'appliquer l'aperçu d'un geste en cours (`LOT-50`
    /// TACHE-03) directement à l'entité concernée, sans reconstruire toute la scène à chaque
    /// image glissée.
    std::vector<core::Entity> _decorEntities;
    bool _dirty = true;
    /// Horloge d'animation partagee par asset (LOT-46 TACHE-05), avancee en temps reel a chaque
    /// render() -- distincte de celle de GameSession (pas fixe) : l'apercu d'edition n'a pas
    /// besoin d'etre deterministe.
    std::unordered_map<std::string, core::Animation> _tileAnimations;
    /// Assets deja signales pour une combinaison exclue (bitmask16/silhouette + animation) :
    /// memorise pour ne jamais journaliser a chaque image.
    std::set<std::string> _warnedExcludedAnimations;
};

}  // namespace hmi
