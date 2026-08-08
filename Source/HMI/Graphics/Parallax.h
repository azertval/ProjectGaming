#pragma once

#include "Core/Levels/Decor.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Graphics/Parallax.h
 * @brief Facteur de défilement par couche et calcul du décalage de parallaxe (`EX-DEC-006`,
 *        LOT-49 TACHE-03).
 */

namespace hmi {

/// Facteur de la couche `core::DecorLayer::Background` : défile **moins vite** que le niveau.
inline constexpr float PARALLAX_FACTOR_BACKGROUND = 0.5f;
/// Facteur de la couche `core::DecorLayer::Decor` : **solidaire** du niveau (valeur de référence,
/// `EX-DEC-006`) — un décor de cette couche ne subit jamais de décalage de parallaxe.
inline constexpr float PARALLAX_FACTOR_DECOR = 1.0f;
/// Facteur de la couche `core::DecorLayer::Foreground` : défile **plus vite** que le niveau.
inline constexpr float PARALLAX_FACTOR_FOREGROUND = 1.15f;

/**
 * @brief Facteur de défilement de la couche d'un décor (`EX-DEC-006`).
 * @param layer Couche du décor.
 * @return Le facteur associé (constante nommée, jamais une valeur magique en aval).
 */
[[nodiscard]] float parallaxFactor(core::DecorLayer layer) noexcept;

/**
 * @brief Calcule la position de **rendu** d'un décor, décalée par sa parallaxe.
 *
 * Fonction **pure**, sans état (LOT-49 TACHE-03) : le décalage est calculé **relativement au
 * centre de la salle courante** (@p cameraBounds, le rectangle cadré par la caméra — solidaire de
 * la salle, `EX-REN-015`) et non en espace niveau absolu. Formulation : la position de rendu est
 * le centre de la salle, plus le déplacement du décor **par rapport à ce centre**, mis à l'échelle
 * du facteur — de sorte qu'à facteur `1.0` (`PARALLAX_FACTOR_DECOR`) la position est **inchangée**,
 * et que deux décors à la même position relative dans deux salles différentes se retrouvent au
 * même endroit à l'écran, quelle que soit la position absolue de chaque salle.
 *
 * C'est le seul comportement cohérent avec une caméra à coupure nette entre salles
 * (`EX-REN-015`, `LOT-32`) : un décalage absolu en espace niveau ferait sauter le décor à chaque
 * bascule (artefact visible) ; ce décalage-ci se replace au moment exact où toute l'image change
 * déjà (invisible).
 * @param decorPosition Position simulée du décor (`core::Decor::position`), en unités monde —
 *                      **jamais modifiée** par cette fonction : la parallaxe est purement visuelle
 *                      (`EX-ARCH-012`).
 * @param factor        Facteur de la couche du décor (`parallaxFactor`).
 * @param cameraBounds  Rectangle cadré par la caméra (`hmi::Camera2D::visibleBounds`), en unités
 *                      monde — solidaire de la salle courante puisque la caméra ne défile jamais.
 * @return La position à utiliser pour le rendu (et le culling) de ce décor.
 */
[[nodiscard]] core::Vector2 parallaxRenderPosition(core::Vector2 decorPosition, float factor,
                                                    const core::Rect& cameraBounds) noexcept;

/**
 * @brief Arrondit une position monde au pixel écran le plus proche, pour un zoom pixel art net.
 *
 * Un décalage de parallaxe fractionnaire produirait un décor flou ou tremblant : le zoom de la
 * caméra est entier quand c'est possible (`EX-ARCH-022`), mais un décalage de parallaxe
 * quelconque, lui, ne l'est pas — cette fonction referme cet écart, **après** application de la
 * parallaxe. Fonction **pure**, sans dépendance GPU.
 * @param worldPosition     Position à arrondir, en unités monde.
 * @param pixelsPerWorldUnit Échelle courante (`hmi::Camera2D::PIXELS_PER_UNIT * zoom`) ; @p
 *                      worldPosition est renvoyée inchangée si cette valeur n'est pas strictement
 *                      positive (robustesse).
 * @return La position arrondie au pixel écran entier le plus proche.
 */
[[nodiscard]] core::Vector2 roundToScreenPixel(core::Vector2 worldPosition,
                                               float pixelsPerWorldUnit) noexcept;

}  // namespace hmi
