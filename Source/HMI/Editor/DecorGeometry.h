#pragma once

#include "Core/Levels/Decor.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Editor/DecorGeometry.h
 * @brief Géométrie **partagée** du corps et des poignées d'un décor (LOT-50), pure et testable.
 *
 * Source unique pour le rendu (`hmi::DraftRenderer`, TACHE-03) et la détection (`hmi::DecorGesture`,
 * TACHE-02) : si chacun calculait son propre rectangle, ils finiraient par diverger au premier
 * ajustement de taille, et les poignées cesseraient de répondre là où elles s'affichent.
 */

namespace hmi {

/// Élément d'un décor visé par un clic/geste : son corps (déplacement), une poignée de coin
/// (redimensionnement) ou la poignée dédiée (rotation). `None` : rien de tout cela.
enum class DecorHandle {
    None,
    Body,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Rotation,
};

/// Taille écran (en pixels) d'une poignée de coin, **constante quel que soit le zoom** — sinon
/// les poignées deviennent inutilisables aux extrêmes (TACHE-03).
inline constexpr float DECOR_HANDLE_SCREEN_SIZE = 10.0f;
/// Distance écran (en pixels) entre le centre du bord supérieur et la poignée de rotation.
inline constexpr float DECOR_ROTATION_HANDLE_SCREEN_OFFSET = 24.0f;

/**
 * @brief Rectangle englobant d'un décor, en unités monde.
 *
 * Fonction **pure** : `core::Decor` ne connaît pas les dimensions réelles de son asset (résolues
 * côté `HMI` via `hmi::TextureCache`, comme `hmi::resolveDecorAppearance`, `LOT-49`) — @p pixelSize
 * est donc fourni par l'appelant plutôt que recalculé ici. `position` reste le coin **haut-gauche**
 * du rectangle (même convention que `hmi::SpriteQuad`) ; la rotation n'est **pas** prise en compte
 * (comme au rendu, `LOT-49` TACHE-02 : le pipeline de quads ne dessine que des rectangles alignés
 * aux axes).
 * @param decor     Décor dont on calcule le rectangle.
 * @param pixelSize Dimensions réelles de l'asset chargé, en pixels (`hmi::DecorAppearance`).
 * @return Le rectangle englobant, en unités monde.
 */
[[nodiscard]] core::Rect decorWorldBounds(const core::Decor& decor, core::Vector2 pixelSize) noexcept;

/// Rectangles des cinq poignées d'un décor sélectionné (quatre coins + rotation), en unités monde.
struct DecorHandleLayout {
    core::Rect topLeft;
    core::Rect topRight;
    core::Rect bottomLeft;
    core::Rect bottomRight;
    core::Rect rotation;
};

/**
 * @brief Calcule les rectangles des poignées d'un décor sélectionné.
 *
 * Fonction **pure**. Les poignées ont une taille **écran constante** (`DECOR_HANDLE_SCREEN_SIZE`) :
 * calculée en pixels puis convertie en unités monde via @p worldUnitsPerScreenPixel, jamais
 * l'inverse — sinon leur taille apparente varierait avec le zoom.
 * @param bounds                    Rectangle englobant du décor (`decorWorldBounds`).
 * @param worldUnitsPerScreenPixel  Échelle inverse de la caméra courante
 *                                  (`1 / (hmi::Camera2D::PIXELS_PER_UNIT * zoom)`).
 * @return Les cinq rectangles de poignées, en unités monde.
 */
[[nodiscard]] DecorHandleLayout decorHandleLayout(const core::Rect& bounds,
                                                  float worldUnitsPerScreenPixel) noexcept;

/**
 * @brief Poignée de @p layout sous @p point, s'il y en a une.
 *
 * Fonction **pure**. Teste la poignée de rotation en priorité, puis les quatre coins ; ne teste
 * jamais le corps (`DecorHandle::Body`) — c'est au rectangle englobant lui-même de le faire
 * (`core::Rect::contains`), à la charge de l'appelant (voir `hmi::designateDecorAt`).
 * @param point  Point testé, en unités monde.
 * @param layout Poignées du décor sélectionné.
 * @return La poignée touchée, ou `DecorHandle::None` si aucune.
 */
[[nodiscard]] DecorHandle hitTestDecorHandles(core::Vector2 point,
                                              const DecorHandleLayout& layout) noexcept;

}  // namespace hmi
