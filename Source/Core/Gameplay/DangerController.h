// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "Core/Levels/Level.h"
#include "Core/Physics/Aabb.h"

/**
 * @file Core/Gameplay/DangerController.h
 * @brief Comportement des dangers **mobile** et **temporisé** : position/activation par pas fixe.
 */

namespace core {

/**
 * @brief Fait vivre les dangers **mobile** (`TileType::DangerMover`, `EX-GP-051`) et
 *        **temporisé** (`TileType::DangerBlink`, `EX-GP-053`) d'un niveau.
 *
 * Logique **pure** (aucun rendu ni fenêtre), déterministe au pas fixe (`EX-NFR-002`) : la position
 * d'un danger mobile et l'activation d'un danger temporisé ne dépendent que du nombre de pas fixes
 * écoulés depuis le chargement, jamais d'un temps réel accumulé en flottant. Ne gère **pas** le
 * danger **commuté** (`TileType::DangerSwitched`, `EX-GP-052`), résolu par `core::
 * MechanismController` (même détection front/continu que déclencheur↔porte, réutilisée telle
 * quelle) ; ne gère pas non plus le danger **directionnel** (`EX-GP-050`), purement géométrique
 * (`core::dangerHitbox`), sans état à faire vivre.
 */
class DangerController {
public:
    /// Construit le contrôleur depuis @p level : compteur de pas à zéro.
    explicit DangerController(const Level& level);

    /// Avance d'un pas fixe : fait progresser le compteur de pas consommé par les formules de
    /// position (mobile) et d'activation (temporisé) ci-dessous.
    void update() noexcept;

    /// @return Le nombre de dangers mobiles du niveau.
    [[nodiscard]] std::size_t moverCount() const noexcept {
        return _moverConfigs.size();
    }

    /**
     * @brief Boîte **mortelle courante** du danger mobile @p index (dans l'ordre de
     *        `Level::moverConfigs()`), en unités de grille.
     *
     * Aller-retour **linéaire déterministe** entre `startPosition` et `startPosition + range`
     * cases sur l'axe de la configuration (`core::DangerMoverConfig::axis`), à une vitesse de
     * conception fixe (`MOVER_SPEED`). Sa position **initiale** de la grille (`TileType::
     * DangerMover`) n'est donc plus à jour une fois le contrôleur avancé — c'est cette boîte,
     * pas un balayage de la grille statique, qui doit être testée pour la résolution de fin de
     * niveau (`core::evaluateOutcome`).
     */
    [[nodiscard]] Aabb moverBox(std::size_t index) const noexcept;

    /**
     * @brief Indique si le danger temporisé à @p position est actuellement **mortel**.
     *
     * `false` si @p position ne correspond à aucun `DangerBlinkConfig` connu (position invalide,
     * robustesse). Formule déterministe : mortel pendant `activeDuration` pas fixes sur chaque
     * cycle de `period` pas, décalé de `phase` — permet des motifs désynchronisés entre plusieurs
     * dangers temporisés d'un même niveau (`EX-GP-053`).
     */
    [[nodiscard]] bool isBlinkActive(GridPosition position) const noexcept;

private:
    std::vector<DangerMoverConfig> _moverConfigs;
    std::vector<DangerBlinkConfig> _blinkConfigs;
    long long _stepCount = 0;  ///< Nombre de pas fixes écoulés depuis le chargement.
};

}  // namespace core
