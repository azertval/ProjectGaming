#pragma once

#include <cstddef>
#include <vector>

#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"

/**
 * @file Core/Gameplay/MechanismController.h
 * @brief Comportement des mécanismes interrupteur↔porte : état + grille de collision.
 */

namespace core {

struct Aabb;

/**
 * @brief Fait vivre les mécanismes **interrupteur ↔ porte** d'un niveau (`EX-GP-020`, `EX-GP-021`).
 *
 * Logique **pure** (aucun rendu ni fenêtre) : quand la boîte du personnage **recouvre** un
 * interrupteur (au **front** — première frame de contact), l'état de la **porte** liée **bascule**.
 * Une porte **fermée** est **solide** (bloque), **ouverte** est franchissable. Le contrôleur
 * maintient une **copie mutable** du `TileMap` (grille de **collision**) que la physique consomme,
 * laissant la carte du `Level` intacte (source de vérité). Déterministe au pas fixe (`EX-NFR-002`).
 */
class MechanismController {
public:
    /// Construit le contrôleur depuis @p level : portes **fermées** (solides) au départ.
    explicit MechanismController(const Level& level);

    /**
     * @brief Met à jour les mécanismes pour un pas : bascule les interrupteurs touchés (front) et
     *        applique l'état des portes dans la grille de collision.
     * @param playerBox Boîte englobante du personnage, en unités monde.
     */
    void update(const Aabb& playerBox);

    /// @return La grille de collision courante (portes à jour), à passer à la physique.
    [[nodiscard]] const TileMap& collisionMap() const noexcept {
        return _collision;
    }

    /// @return Les liaisons de mécanismes (positions interrupteur/porte).
    [[nodiscard]] const std::vector<Mechanism>& mechanisms() const noexcept {
        return _mechanisms;
    }

    /// @return true si la porte du mécanisme @p index est **ouverte** (interrupteur activé).
    [[nodiscard]] bool isDoorOpen(std::size_t index) const {
        return _switchOn[index];
    }

private:
    TileMap _collision;  ///< Copie mutable : portes Solid (fermées) / Door (ouvertes).
    std::vector<Mechanism> _mechanisms;  ///< Liaisons interrupteur↔porte (positions).
    std::vector<bool> _switchOn;         ///< État de chaque interrupteur (porte ouverte ?).
    std::vector<bool>
        _playerOnSwitchPrev;  ///< Le personnage était-il sur l'interrupteur au pas précédent ?
};

}  // namespace core
