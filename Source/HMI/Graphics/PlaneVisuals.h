// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Core/Levels/Plane.h"
#include "Core/Math/Rect.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/PlaneVisibility.h"
#include "HMI/Graphics/RenderMode.h"

/**
 * @file HMI/Graphics/PlaneVisuals.h
 * @brief Projection profondeur → calque, résolution et composition des **plans picturaux**
 *        (`EX-REN-049`, `EX-DEC-040` à `EX-DEC-042`, LOT-69 TACHE-05).
 */

namespace hmi {

class TextureCache;

/// Sous-dossier des images de plans, relatif au dossier des **niveaux** — un plan est une donnée de
/// niveau, pas un asset réutilisable : il ne vit pas sous `Assets/`.
inline const std::string PLANES_SUBDIRECTORY = "Plans/";

/**
 * @brief Projette la profondeur d'un plan sur le calque de rendu correspondant.
 *
 * `Core` ne connaît pas `hmi::RenderLayer` (`EX-NFR-011`) : la projection est le travail de `HMI`,
 * et elle vit **ici**, en un point unique. Deux profondeurs, deux calques déjà réservés par
 * `LOT-40` — aucune valeur d'énumération n'est ajoutée, et `RENDER_LAYER_COUNT` ne bouge pas, alors
 * même que le **nombre de plans** est libre : c'est le rang dans la liste qui les ordonne à
 * l'intérieur d'un calque, pas une valeur de calque par plan (`EX-REN-014`).
 * @param depth Côté de l'ordonnancement déclaré par le plan.
 * @return `RenderLayer::Plane` derrière les tuiles, `RenderLayer::Foreground` devant le personnage.
 */
[[nodiscard]] RenderLayer planeRenderLayer(core::PlaneDepth depth) noexcept;

/**
 * @brief Texture d'un plan déjà résolue, prête à être composée.
 *
 * Même patron que `hmi::BackgroundTexture` vis-à-vis de `hmi::composeBackground` : sépare la
 * **résolution** (disque/GPU) de la **composition** (pure), ce qui rend la seconde testable sans
 * carte graphique (`EX-NFR-004`).
 */
struct PlaneTexture {
    /// Texture à lier : l'image du plan si elle est chargeable, le damier de repli sinon
    /// (`EX-NFR-040`). `nullptr` si même le damier a échoué.
    TextureHandle texture = nullptr;
    /// Largeur de la texture, en pixels.
    int width = 0;
    /// Hauteur de la texture, en pixels.
    int height = 0;
};

/**
 * @brief Résout les images des plans d'un niveau (accès disque/GPU), dans l'ordre de la liste.
 *
 * Repli en damier sur image absente ou illisible (`EX-NFR-040`) : un plan pointant un fichier
 * disparu reste un niveau **valide** (`Core` ne vérifie pas l'existence, `EX-NFR-011`) — c'est ici
 * que le manque devient visible plutôt que silencieux.
 * @param cache     Cache de textures (propriétaire du damier partagé et de la mémoïsation).
 * @param directory Dossier des images de plans (typiquement `Levels/Plans`).
 * @param planes    Plans du niveau, dans l'ordre déclaré.
 * @return Une entrée par plan, **alignée sur @p planes**.
 */
[[nodiscard]] std::vector<PlaneTexture> resolvePlaneTextures(
    TextureCache& cache, const std::filesystem::path& directory,
    const std::vector<core::Plane>& planes);

/**
 * @brief Cadrage courant et échelle, pour le décalage de parallaxe des plans (`EX-DEC-043`).
 *
 * Regroupés en une structure plutôt qu'ajoutés un à un à `composePlanes` : les trois valeurs n'ont
 * de sens qu'ensemble, et une signature qui les sépare invite à en oublier une. `active` à `false`
 * (le défaut) rend les deux autres sans effet — c'est l'état de l'éditeur et du cadrage *niveau
 * entier* (`hmi::planeParallaxActive`).
 */
struct PlaneParallax {
    /// La parallaxe s'applique-t-elle ? Décidé par `hmi::planeParallaxActive`.
    bool active = false;
    /// Rectangle cadré par la caméra (`hmi::Camera2D::visibleBounds`), en unités monde.
    core::Rect cameraBounds{};
    /// Échelle courante (`hmi::Camera2D::PIXELS_PER_UNIT * zoom`), pour l'arrondi au pixel écran.
    float pixelsPerWorldUnit = 0.0f;
};

/**
 * @brief Compose les plans picturaux d'un niveau, dans l'ordre déclaré.
 *
 * Fonction **pure** (aucun accès disque/GPU), testable via `hmi::QuadRecorder` (`EX-NFR-004`).
 * Émet **un quad par plan**, couvrant exactement `[0,0]`–`[levelWidth, levelHeight]` en unités
 * monde, UV pleines. Trois propriétés méritent d'être connues :
 *
 * - **La densité ne change pas la géométrie.** Un plan à 8 px/unité couvre le même rectangle, avec
 *   les mêmes UV, qu'un plan natif : seule sa texture est deux fois plus petite, donc plus
 * grossière à l'écran. C'est le sens même du réglage (`EX-DEC-041`) — un compromis de **mémoire**,
 * pas de cadrage.
 * - **L'ordre de composition porte l'ordre de dessin.** `hmi::ComposedScene::sort()` intercale le
 *   rang de **première apparition de texture** entre le calque et le `sortOrder` ; chaque plan
 *   portant sa propre image, ce sont donc les rangs de texture qui les ordonnent. La propriété
 *   « les plans ressortent dans l'ordre du niveau » n'est vraie que parce qu'ils sont composés
 *   **en premier et dans l'ordre**. Elle est invisible à la lecture : un test la fige.
 * - **Rien n'est émis en `RenderMode::Physique`**, comme pour le fond : la lecture des collisions
 *   reste sans distraction (`EX-REN-046`).
 *
 * Le coût est **constant en taille de niveau** : un quad et une passe par plan, quelles que soient
 * les dimensions — contrairement aux tuiles. C'est ce que `TACHE-09` mesure.
 * @param scene       Scène composée, cible de l'ajout (non vidée : la composition est cumulative).
 * @param planes      Plans du niveau, dans l'ordre déclaré.
 * @param textures    Textures résolues, **alignées** sur @p planes ; une entrée sans texture n'émet
 *                    rien.
 * @param levelWidth  Largeur du niveau, en unités monde (une case = une unité).
 * @param levelHeight Hauteur du niveau, en unités monde.
 * @param mode        Mode de rendu courant ; sans effet hors `RenderMode::Texture`.
 * @param visibility  Visibilité par rang (aide d'édition) : tout visible par défaut, donc sans
 *                    effet en jeu.
 * @param parallax    Cadrage et échelle pour le décalage de parallaxe ; inactif par défaut, le
 *                    plan est alors composé exactement sur les bornes du niveau.
 */
void composePlanes(ComposedScene& scene, const std::vector<core::Plane>& planes,
                   const std::vector<PlaneTexture>& textures, int levelWidth, int levelHeight,
                   RenderMode mode, const PlaneVisibility& visibility = PlaneVisibility{},
                   const PlaneParallax& parallax = PlaneParallax{});

}  // namespace hmi
