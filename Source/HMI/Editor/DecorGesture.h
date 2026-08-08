#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "Core/Levels/Decor.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/DecorGeometry.h"

/**
 * @file HMI/Editor/DecorGesture.h
 * @brief Machine à états du geste de manipulation de décors (LOT-50 TACHE-02), pure et testable.
 *
 * Même parti que `hmi::LinkGesture` (`LOT-37`) : la logique de désignation, de distinction
 * clic/glisser et de calcul des poignées est une fonction déterministe sur des données, sans Qt ni
 * GPU — le viewport n'en est que le routeur d'événements. Le geste **ne mute rien** : il décrit une
 * action, l'appelant l'applique aux mutateurs de `LevelDraft` (`LOT-50` TACHE-01).
 */

namespace hmi {

/// Élément visé par un clic (voir `hmi::designateDecorAt`).
struct DecorHit {
    std::size_t index = 0;
    DecorHandle handle = DecorHandle::Body;
};

/// Seuil de déplacement (unités monde) distinguant un clic (sélection simple) d'un glisser
/// (manipulation) — sans lui, poser puis relâcher sans bouger produirait un micro-déplacement.
inline constexpr float DECOR_DRAG_THRESHOLD = 0.1f;

/**
 * @brief Désigne l'élément sous @p point : quel décor, et quelle poignée le cas échéant.
 *
 * Fonction **pure**. Priorité aux poignées du décor **déjà sélectionné** (@p selectedIndex/
 * @p selectedHandles, si fournis — un décor non sélectionné n'affiche pas de poignées) ; à défaut,
 * au **corps** des décors, du **dernier au premier** de @p bounds — l'ordre de superposition
 * (`LOT-49` TACHE-01) place le décor le plus au-dessus en dernière position du vecteur, donc le
 * plus probable visé par un clic sur une zone de recouvrement.
 * @param point           Point testé, en unités monde.
 * @param bounds          Rectangles englobants des décors du niveau (`hmi::decorWorldBounds`),
 *                        alignés avec `core::LevelDraft::decors()`.
 * @param selectedIndex   Rang du décor actuellement sélectionné, si un l'est.
 * @param selectedHandles Poignées du décor sélectionné (`hmi::decorHandleLayout`), si un l'est.
 * @return L'élément visé, ou `std::nullopt` si le clic ne touche rien.
 */
[[nodiscard]] std::optional<DecorHit> designateDecorAt(
    core::Vector2 point, const std::vector<core::Rect>& bounds,
    std::optional<std::size_t> selectedIndex,
    const std::optional<DecorHandleLayout>& selectedHandles) noexcept;

/// Nature de l'action produite par le geste.
enum class DecorGestureActionKind {
    None,    ///< Rien à appliquer (glisser encore sous le seuil, ou fin d'un simple clic).
    Move,    ///< Déplacer le décor vers `position`.
    Resize,  ///< Redimensionner le décor à `scale`, avec la nouvelle `position` du coin ancré.
    Rotate,  ///< Pivoter le décor à `rotation`.
};

/// Action à appliquer par l'appelant : en aperçu pendant le glisser (`hmi::updateDecorGesture`,
/// consommée par le rendu, `LOT-50` TACHE-03), ou pour de bon au relâchement
/// (`hmi::endDecorGesture`, consommée par les mutateurs de TACHE-01 — un seul appel par geste
/// complet, l'historique n'est jamais spammé par les positions intermédiaires).
struct DecorGestureAction {
    DecorGestureActionKind kind = DecorGestureActionKind::None;
    std::size_t index = 0;
    core::Vector2 position{};
    core::Vector2 scale{1.0f, 1.0f};
    float rotation = 0.0f;
};

/// Phase du geste en cours.
enum class DecorGesturePhase {
    Idle,      ///< Aucun geste en cours.
    Pressed,   ///< Bouton pressé, sous le seuil de glisser : pas encore de manipulation.
    Dragging,  ///< Glisser au-delà du seuil : une manipulation (déplacer/redimensionner/pivoter)
               ///< est en cours.
};

/**
 * @brief État complet du geste en cours, possédé par l'appelant (même patron que
 *        `hmi::PendingLink` pour `LinkGesture` : l'appelant porte l'état, ces fonctions le font
 *        évoluer).
 *
 * `selectedIndex` **persiste** au-delà d'un geste terminé (`Idle`) : c'est la sélection courante
 * de l'éditeur, distincte de la phase du geste.
 */
struct DecorGestureState {
    DecorGesturePhase phase = DecorGesturePhase::Idle;
    std::optional<std::size_t> selectedIndex;
    DecorHandle handle = DecorHandle::None;
    core::Vector2 pressPosition{};
    core::Vector2 initialPosition{};
    core::Vector2 initialScale{1.0f, 1.0f};
    float initialRotation = 0.0f;
    /// Taille du rectangle englobant du décor **au moment de l'appui** (`hmi::decorWorldBounds`,
    /// résolue par l'appelant) : nécessaire pour convertir un déplacement de coin en nouvelle
    /// échelle (`newScale = initialScale × newSize / initialSize`).
    core::Vector2 initialSize{};
};

/**
 * @brief Amorce un geste sur l'élément désigné par @p hit (`hmi::designateDecorAt`).
 *
 * Sélectionne immédiatement le décor (`state.selectedIndex`), avant même de savoir si un glisser
 * suivra — un simple clic doit déjà sélectionner. Capture le transform de départ (pour calculer
 * les deltas des mises à jour suivantes, et pouvoir tout restaurer sur abandon).
 * @param state         État du geste, muté en place.
 * @param hit           Élément désigné (`hmi::designateDecorAt`).
 * @param worldPosition Position du clic, en unités monde.
 * @param decor         Décor visé, à l'instant du clic (transform de départ).
 * @param bounds        Rectangle englobant du décor à l'instant du clic (`hmi::decorWorldBounds`).
 */
void beginDecorGesture(DecorGestureState& state, DecorHit hit, core::Vector2 worldPosition,
                       const core::Decor& decor, const core::Rect& bounds) noexcept;

/**
 * @brief Fait progresser un glisser en cours.
 *
 * Sous `DECOR_DRAG_THRESHOLD`, reste `Pressed` et renvoie `DecorGestureActionKind::None` — un
 * clic sans mouvement ne produit donc aucun micro-déplacement. Au-delà, passe `Dragging` et
 * renvoie l'action d'**aperçu** courante (`LOT-50` TACHE-03) : cette fonction ne mute jamais le
 * brouillon, seul `hmi::endDecorGesture` produit l'action à appliquer pour de bon.
 * @param state         État du geste, muté en place.
 * @param worldPosition Position courante du curseur, en unités monde.
 * @param snapToGrid    Aimante la position/les coins sur la grille entière si `true` — optionnelle
 *                      et jamais imposée (`EX-DEC-001`).
 * @return L'action d'aperçu courante.
 */
[[nodiscard]] DecorGestureAction updateDecorGesture(DecorGestureState& state,
                                                    core::Vector2 worldPosition,
                                                    bool snapToGrid) noexcept;

/**
 * @brief Termine le geste (relâchement) et renvoie l'action **finale** à appliquer.
 *
 * `DecorGestureActionKind::None` si le glisser n'a jamais dépassé le seuil (un simple clic, déjà
 * traduit en sélection par `beginDecorGesture`) — c'est l'appelant qui décide alors de ne rien
 * passer aux mutateurs de TACHE-01. Remet la phase à `Idle` ; la sélection, elle, persiste.
 * @param state         État du geste, muté en place.
 * @param worldPosition Position du relâchement, en unités monde.
 * @param snapToGrid    Même paramètre que `updateDecorGesture`, pour que la valeur finale
 *                      corresponde exactement au dernier aperçu affiché.
 * @return L'action à appliquer aux mutateurs de TACHE-01.
 */
[[nodiscard]] DecorGestureAction endDecorGesture(DecorGestureState& state,
                                                 core::Vector2 worldPosition,
                                                 bool snapToGrid) noexcept;

/**
 * @brief Abandonne le geste en cours (`Échap`).
 *
 * Aucune action à appliquer : le brouillon n'a jamais été touché pendant le glisser (seul
 * l'aperçu l'était, côté rendu) — il suffit à l'appelant de cesser d'afficher l'aperçu. Remet la
 * phase à `Idle` ; la sélection persiste.
 * @param state État du geste, muté en place.
 */
void cancelDecorGesture(DecorGestureState& state) noexcept;

}  // namespace hmi
