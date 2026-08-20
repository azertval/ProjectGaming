// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Graphics/Parallax.h
 * @brief Calcul du décalage de parallaxe (`EX-DEC-043`, LOT-69).
 *
 * Le facteur n'est plus une constante par couche : il est **porté par le plan** (`core::Plane`),
 * donc réglable niveau par niveau. Ces fonctions ne connaissent qu'un `float` — elles ignorent
 * d'où il vient.
 */

namespace hmi {

/// Facteur **neutre** : le contenu est solidaire du niveau et ne subit aucun décalage.
inline constexpr float PARALLAX_FACTOR_NONE = 1.0f;

/**
 * @brief Calcule la position de **rendu** d'un contenu, décalée par sa parallaxe.
 *
 * Fonction **pure**, sans état (LOT-49 TACHE-03) : le décalage est calculé **relativement au
 * centre de la salle courante** (@p cameraBounds, le rectangle cadré par la caméra — solidaire de
 * la salle, `EX-REN-015`) et non en espace niveau absolu. Formulation : la position de rendu est
 * le centre de la salle, plus le déplacement du décor **par rapport à ce centre**, mis à l'échelle
 * du facteur — de sorte qu'à facteur `1.0` (`PARALLAX_FACTOR_NONE`) la position est
 * **inchangée**, et que deux contenus à la même position relative dans deux salles différentes se
 * retrouvent au même endroit à l'écran, quelle que soit la position absolue de chaque salle.
 *
 * C'est le seul comportement cohérent avec une caméra à coupure nette entre salles
 * (`EX-REN-015`, `LOT-32`) : un décalage absolu en espace niveau ferait sauter le contenu à
 * chaque bascule (artefact visible) ; ce décalage-ci se replace au moment exact où toute l'image
 * change déjà (invisible).
 * @param modelPosition Position en espace **modèle**, en unités monde — **jamais modifiée** par
 *                      cette fonction : la parallaxe est purement visuelle (`EX-ARCH-012`).
 * @param factor        Facteur de parallaxe appliqué.
 * @param cameraBounds  Rectangle cadré par la caméra (`hmi::Camera2D::visibleBounds`), en unités
 *                      monde — solidaire de la salle courante puisque la caméra ne défile jamais.
 * @return La position à utiliser pour le rendu (et le culling) de ce décor.
 */
[[nodiscard]] core::Vector2 parallaxRenderPosition(core::Vector2 modelPosition, float factor,
                                                   const core::Rect& cameraBounds) noexcept;

/**
 * @brief Inverse de `parallaxRenderPosition` : retrouve la position **simulée** équivalente à un
 * point exprimé en espace de rendu (`LOT-50` TACHE-02).
 *
 * Nécessaire à l'éditeur : le curseur converti par `hmi::Camera2D::screenToWorld` est comparable
 * à la position de **rendu** d'un contenu parallaxé (c'est elle qui occupe l'écran), jamais
 * directement à sa position modèle dès que le facteur diffère de `1.0`. Cette fonction ramène donc
 * un point d'écran dans l'espace modèle, pour que ce qu'on manipule reste visuellement « collé »
 * au curseur quel que soit son facteur de parallaxe.
 * @param renderPosition Position en espace de rendu, en unités monde.
 * @param factor         Facteur de parallaxe appliqué.
 * @param cameraBounds   Même rectangle caméra que celui utilisé pour `parallaxRenderPosition`.
 * @return La position modèle correspondante ; @p renderPosition inchangée si @p factor est nul
 *         (robustesse -- aucune couche du projet n'a un facteur nul).
 */
[[nodiscard]] core::Vector2 parallaxModelPosition(core::Vector2 renderPosition, float factor,
                                                  const core::Rect& cameraBounds) noexcept;

/**
 * @brief Arrondit une position monde au pixel écran le plus proche, pour un zoom pixel art net.
 *
 * Un décalage de parallaxe fractionnaire produirait une image floue ou tremblante : le zoom de la
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
