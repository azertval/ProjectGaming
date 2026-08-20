// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "Core/Gameplay/PlatformPath.h"
#include "Core/Levels/Level.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlatformSample.h"

/**
 * @file Core/Gameplay/PlatformController.h
 * @brief Comportement des plateformes mobiles : position continue par pas fixe (`EX-GP-026`).
 */

namespace core {

class TileMap;

/**
 * @brief Fait vivre les **plateformes mobiles** d'un niveau (`TileType::MovingPlatform`,
 *        `EX-GP-026`).
 *
 * Logique **pure** (aucun rendu ni fenêtre), sur le modèle de `core::DangerController` (danger
 * mobile, `EX-GP-051`) plutôt que de `core::BlockController` : contrairement à un bloc poussable,
 * la position d'une plateforme est **continue** (pas alignée sur la grille), fonction
 * **déterministe** du nombre de pas fixes écoulés depuis le chargement (`EX-NFR-002`) — jamais
 * d'accumulation flottante (`position += vitesse * dt`), qui dériverait sur une session longue.
 *
 * Parcours **linéaire** de la route de `MovingPlatformConfig` (`startPosition` puis `waypoints`) à
 * vitesse constante (`speed`, cases par seconde), en aller-retour ou en circuit fermé selon
 * `mode` — même principe que `DangerController::moverBox`, généralisé d'un axe unique à un
 * polyligne 2D quelconque (`EX-GP-054`). La géométrie est portée par `core::PlatformPath`,
 * partagée avec l'overlay d'édition pour que le trajet dessiné soit celui réellement parcouru.
 *
 * Le contrôleur ne résout **aucune** collision lui-même : il n'expose que la boîte courante et la
 * boîte au pas précédent de chaque plateforme. L'appelant (`hmi::GameSession`) compose la
 * collision continue (`core::sweepAabbVsAabb`, comme pour les blocs réduits, `EX-GP-005`), porte
 * le personnage et les blocs (`core::BlockController`), et interpole l'affichage
 * (`hmi::PreviousPosition`) à partir de `previousBoxAt`/`boxAt`.
 */
class PlatformController {
public:
    /// Construit le contrôleur depuis @p level : compteur de pas à zéro (toutes les plateformes à
    /// leur position de départ).
    explicit PlatformController(const Level& level);

    /// Avance d'un pas fixe : fait progresser le compteur de pas consommé par `boxAt`.
    void update() noexcept;

    /// @return Le nombre de plateformes mobiles du niveau.
    [[nodiscard]] std::size_t count() const noexcept {
        return _configs.size();
    }

    /// @return La boîte **courante** (après le dernier `update()`) de la plateforme @p index, en
    ///         unités monde.
    [[nodiscard]] Aabb boxAt(std::size_t index) const noexcept;

    /// @return La boîte de la plateforme @p index **au pas précédent** (juste avant le dernier
    ///         `update()`) — pour le delta de portage et l'interpolation d'affichage.
    [[nodiscard]] Aabb previousBoxAt(std::size_t index) const noexcept;

    /// @return Le déplacement (`boxAt(index).min - previousBoxAt(index).min`) survenu au dernier
    ///         pas — c'est ce déplacement qui porte le personnage/les blocs posés dessus.
    [[nodiscard]] Vector2 deltaAt(std::size_t index) const noexcept;

    /// @return `{previousBoxAt(index), boxAt(index)}` — commodité pour l'appelant qui compose le
    ///         portage/la collision continue via `PlatformSample` (`CharacterPhysicsSystem`,
    ///         `BlockController`) sans appeler les deux accesseurs séparément.
    [[nodiscard]] PlatformSample sampleAt(std::size_t index) const noexcept {
        return PlatformSample{.previousBox = previousBoxAt(index), .currentBox = boxAt(index)};
    }

    /// @return Tous les échantillons courants (`sampleAt`), dans l'ordre de `Level::
    ///         platformConfigs()` — commodité pour l'appelant qui compose la liste complète en un
    ///         appel plutôt que d'itérer `count()`/`sampleAt` lui-même.
    [[nodiscard]] std::vector<PlatformSample> samples() const;

private:
    /// Position déterministe de la plateforme @p index au pas @p stepCount (fonction pure du
    /// numéro de pas, jamais d'état accumulé).
    [[nodiscard]] Aabb boxAtStep(std::size_t index, long long stepCount) const noexcept;

    std::vector<MovingPlatformConfig> _configs;
    /// Routes précalculées, une par entrée de `_configs` et dans le même ordre (`EX-GP-054`).
    std::vector<PlatformPath> _paths;
    long long _stepCount = 0;  ///< Nombre de pas fixes écoulés depuis le chargement.
};

/**
 * @brief Indique si @p box repose sur le **dessus** de @p platformBox (chevauchement horizontal,
 *        bord bas de @p box au contact du bord haut de @p platformBox).
 *
 * Même tolérance de contact que `core::BlockController` (poussée) : sert à décider si une entité
 * (personnage, bloc) est **portée** par la plateforme ce pas-ci. Fonction pure, testable sans
 * contexte de jeu.
 * @param box         Boîte de l'entité potentiellement portée.
 * @param platformBox Boîte de la plateforme (`PlatformController::boxAt`).
 * @return `true` si @p box repose sur @p platformBox.
 */
[[nodiscard]] bool restsOnTopOfPlatform(const Aabb& box, const Aabb& platformBox) noexcept;

/**
 * @brief Indique si @p carriedBox (le personnage, après portage) est **écrasé** : elle chevauche
 *        une tuile solide de @p collision (`EX-GP-026`, cas d'écrasement plafond).
 *
 * Décision de cadrage (`TACHE-03`) : une plateforme montante qui écraserait le personnage contre
 * un plafond est **mortelle**, comme un danger — plutôt que de mettre la plateforme en pause, ce
 * qui casserait sa position purement fonctionnelle du numéro de pas (`EX-NFR-002`). L'appelant
 * traduit un résultat vrai en boîte de danger supplémentaire pour `core::evaluateOutcome`.
 * @param carriedBox Boîte du personnage après application du déplacement de portage.
 * @param collision  Grille de collision courante (mécanismes/blocs déjà résolus).
 * @return `true` si @p carriedBox chevauche une tuile solide de @p collision.
 */
[[nodiscard]] bool isSquishedByPlatform(const Aabb& carriedBox, const TileMap& collision) noexcept;

}  // namespace core
