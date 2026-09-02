// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Physics/Aabb.h"

/**
 * @file Core/Gameplay/VolatileBlockController.h
 * @brief Blocs qui quittent la grille de collision : fragile (`EX-GP-028`) et éphémère
 *        (`EX-GP-029`).
 */

namespace core {

/**
 * @brief Fait vivre les blocs **fragile** (`TileType::FragileBlock`, `EX-GP-028`) et **éphémère**
 *        (`TileType::VanishingBlock`, `EX-GP-029`).
 *
 * Logique **pure** (aucun rendu ni fenêtre). Les deux types partagent ce contrôleur parce qu'ils
 * partagent tout sauf leur déclencheur : ni l'un ni l'autre ne **bouge** — ils sont solides comme
 * un `Solid` et se contentent de **quitter** la grille de collision. Deux règles de retrait, un
 * seul recouvrement ; deux contrôleurs jumeaux auraient dupliqué ce recouvrement et son ordre de
 * composition.
 *
 * Le patron est celui de `core::MechanismController`, qui tient déjà une **copie mutable** du
 * `TileMap` pour ouvrir et fermer ses portes : la carte du `Level` reste **immuable**, seule la
 * grille de collision résolue change (`collisionMap`).
 *
 * - **Bloc fragile** : détruit quand le personnage est en **ground pound** (`EX-GP-058`) et
 *   l'atteint **par le dessus**. Par ce geste et par lui seul — un dash, même vertical ou boosté,
 *   ne le brise pas.
 * - **Bloc éphémère** : le personnage s'y **pose** puis le **quitte** ; `VANISH_DELAY_STEPS` pas
 *   plus tard, le bloc disparaît. Le déclencheur est un **front** (reposait dessus au pas
 *   précédent, n'y repose plus), jamais un contact : passer dessous ou frôler le côté n'arme rien.
 *   Revenir dessus pendant le compte à rebours ne l'annule pas.
 *
 * Dans les deux cas, la disparition est **définitive** : rien ne réapparaît, et c'est la
 * reconstruction du contrôleur au rechargement du tableau qui remet tout à neuf.
 */
class VolatileBlockController {
public:
    /// Nombre de pas fixes entre le départ du personnage et la disparition d'un bloc éphémère.
    /// Constante du **moteur**, exprimée en pas (jamais en secondes) pour rester déterministe
    /// (`EX-NFR-002`) — même choix que `BlockController::FALL_INTERVAL_STEPS`. Un tiers de seconde
    /// à 60 pas/s : assez pour lire la mécanique, trop peu pour s'y attarder.
    static constexpr int VANISH_DELAY_STEPS = 20;

    /// Tolérance du test « repose sur le dessus », même ordre de grandeur que le portage des
    /// plateformes (`core::restsOnTopOfPlatform`).
    static constexpr float REST_TOLERANCE = 0.05F;

    /// Construit le contrôleur : repère chaque tuile `FragileBlock` et `VanishingBlock` de
    /// @p level, toutes intactes.
    explicit VolatileBlockController(const Level& level);

    /**
     * @brief Met à jour les blocs pour un pas : brise les blocs fragiles atteints par un ground
     *        pound, avance les comptes à rebours des blocs éphémères quittés.
     * @param playerBox      Boîte du personnage **avant** le pas physique courant (même convention
     *                       que `core::BlockController::update` : la destruction doit libérer la
     *                       case avant que la physique ne résolve le déplacement de ce même pas,
     *                       sans quoi le ground pound s'arrêterait un pas sur un bloc qu'il vient
     *                       de briser).
     * @param groundPounding `true` si le personnage était en ground pound à la fin du pas
     *                       précédent (`core::Player::groundPounding`).
     */
    void update(const Aabb& playerBox, bool groundPounding);

    /**
     * @brief Complète @p base en **retirant** les blocs volatils déjà disparus.
     * @param base Grille de collision déjà résolue par les mécanismes (portes ouvertes/fermées).
     * @return Une copie de @p base dont les cases des blocs disparus sont vides.
     *
     * Les blocs **intacts** n'ont rien à y ajouter : `core::isSolid` les donne déjà solides, comme
     * un `Solid` ordinaire. Ce n'est donc qu'une soustraction — jamais une modification de la
     * carte du `Level`.
     */
    [[nodiscard]] TileMap collisionMap(const TileMap& base) const;

    /// @return Le nombre de blocs volatils du niveau (disparus compris : les indices sont stables
    ///         sur toute la durée d'un tableau).
    [[nodiscard]] std::size_t count() const noexcept {
        return _blocks.size();
    }

    /// @return La position de grille du bloc @p index.
    [[nodiscard]] GridPosition positionAt(std::size_t index) const noexcept {
        return _blocks[index].position;
    }

    /// @return `true` si le bloc @p index a disparu (brisé ou effacé).
    [[nodiscard]] bool isGoneAt(std::size_t index) const noexcept {
        return _blocks[index].gone;
    }

    /// @return `true` si le bloc @p index est un bloc éphémère dont le compte à rebours court —
    ///         de quoi l'afficher clignotant, sans que ce contrôleur ne sache rien du rendu.
    [[nodiscard]] bool isVanishingAt(std::size_t index) const noexcept {
        return _blocks[index].countdown > 0;
    }

    /// @return Les positions des blocs ayant disparu **au dernier pas**, pour les effets de l'IHM
    ///         (éclats, son). Vidée et recalculée à chaque `update()`.
    [[nodiscard]] const std::vector<GridPosition>& blocksGoneThisStep() const noexcept {
        return _goneThisStep;
    }

private:
    /// État d'un bloc volatil.
    struct VolatileBlock {
        GridPosition position;   ///< Case du bloc ; il ne bouge jamais.
        bool fragile = false;    ///< `true` = `FragileBlock`, `false` = `VanishingBlock`.
        bool gone = false;       ///< Brisé ou effacé : ne bloque plus rien.
        bool wasRested = false;  ///< Le personnage reposait dessus au pas précédent (éphémère).
        int countdown = 0;       ///< Pas restants avant disparition ; `0` = pas encore armé.
    };

    std::vector<VolatileBlock> _blocks;
    std::vector<GridPosition> _goneThisStep;
};

}  // namespace core
