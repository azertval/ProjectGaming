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
 * @brief Fait vivre les mécanismes **interrupteur/plaque ↔ porte** d'un niveau (`EX-GP-020`,
 *        `EX-GP-021`, `EX-GP-025`).
 *
 * Logique **pure** (aucun rendu ni fenêtre). Deux comportements de déclencheur, selon la tuile
 * d'origine (figée au chargement) :
 * - **Interrupteur** (`TileType::Switch`) : quand la boîte du personnage **recouvre** la case au
 *   **front** (première frame de contact), l'état de la porte liée **bascule** et persiste.
 * - **Plaque de pression** (`TileType::PressurePlate`, `EX-GP-025`) : la porte reste **ouverte**
 *   tant qu'un poids suffisant y repose (comparé à `MIN_TRIGGER_MASS`), et se **referme** dès que
 *   ce n'est plus le cas — activation **continue**, pas de front.
 *
 * Une porte **fermée** est **solide** (bloque), **ouverte** est franchissable. Le contrôleur
 * maintient une **copie mutable** du `TileMap` (grille de **collision**) que la physique consomme,
 * laissant la carte du `Level` intacte (source de vérité). Déterministe au pas fixe (`EX-NFR-002`).
 */
class MechanismController {
public:
    /// Construit le contrôleur depuis @p level : portes **fermées** (solides) au départ.
    explicit MechanismController(const Level& level);

    /**
     * @brief Met à jour les mécanismes pour un pas : bascule les interrupteurs touchés (front),
     *        ouvre/referme les plaques de pression selon le poids présent, applique l'état des
     *        portes dans la grille de collision.
     * @param playerBox  Boîte englobante du personnage, en unités monde.
     * @param playerMass Masse du personnage (`core::Player::mass`, `EX-GP-019`), comparée au seuil
     *                    des plaques de pression (`MIN_TRIGGER_MASS`) ; sans effet sur les
     *                    interrupteurs classiques. Valeur par défaut = masse par défaut du
     *                    personnage (compatibilité des appels existants).
     */
    void update(const Aabb& playerBox, float playerMass = 1.0f);

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
    std::vector<Mechanism> _mechanisms;  ///< Liaisons déclencheur↔porte (positions).
    std::vector<bool> _switchOn;         ///< État de chaque déclencheur (porte ouverte ?).
    std::vector<bool>
        _playerOnSwitchPrev;  ///< Le personnage était-il sur le déclencheur au pas précédent ?
    std::vector<bool> _continuous;  ///< true = plaque de pression (activation continue), figé au
                                    ///< chargement ; false = interrupteur à bascule classique.
};

}  // namespace core
