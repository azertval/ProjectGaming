#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "Core/Levels/Decor.h"
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Editor/DecorPlacementGesture.h
 * @brief Recherche du décor le plus proche d'un clic, pour l'outil « Décor » (`LOT-49` TACHE-04),
 *        pur et testable.
 */

namespace hmi {

/// Rayon de détection (en unités monde) pour retrouver un décor déjà posé au clic (suppression) :
/// un décor libre n'a pas de case qui le désigne sans ambiguïté, contrairement à une tuile.
inline constexpr float DECOR_PICK_RADIUS = 0.5f;

/**
 * @brief Trouve le rang du décor le plus proche de @p worldPosition, dans `DECOR_PICK_RADIUS`.
 *
 * Fonction **pure**, testable sans Qt/GPU (même patron que `hmi::resolveTextureAssignClick`) : ne
 * modifie rien, décrit seulement quel décor un clic de suppression viserait — l'appelant applique
 * `core::LevelDraft::removeDecor` avec le rang renvoyé. À distance égale, le décor de **rang le
 * plus élevé** (posé le plus récemment) est préféré : c'est celui dessiné par-dessus les autres de
 * sa couche (`EX-DEC-001` TACHE-01, ordre intra-couche), donc le plus probable visé par le clic.
 * @param worldPosition Position du clic, en unités monde (jamais calée sur la grille).
 * @param decors        Décors du brouillon courant, dans leur ordre (`core::LevelDraft::decors`).
 * @return Le rang du décor le plus proche, ou `std::nullopt` si aucun n'est à portée.
 */
[[nodiscard]] std::optional<std::size_t> nearestDecorAt(
    core::Vector2 worldPosition, const std::vector<core::Decor>& decors) noexcept;

}  // namespace hmi
