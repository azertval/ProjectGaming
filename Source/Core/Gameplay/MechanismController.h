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
 *
 * Résout aussi les liaisons **déclencheur ↔ danger commuté** (`DangerLink`, `TileType::
 * DangerSwitched`, `EX-GP-052`) : même détection front/continu que déclencheur↔porte ci-dessus,
 * réutilisée telle quelle (pas de duplication de cette logique) — seule différence, un danger
 * commuté n'a **aucun effet** sur la grille de collision (il n'est jamais solide), seul son état
 * **actif/inactif** est exposé (`isDangerActive`), consommé par `core::evaluateOutcome` via les
 * boîtes supplémentaires assemblées par l'appelant (`hmi::GameSession`).
 */
class MechanismController {
public:
    /// Construit le contrôleur depuis @p level : portes **fermées** (solides) au départ.
    explicit MechanismController(const Level& level);

    /**
     * @brief Met à jour les mécanismes pour un pas : bascule les interrupteurs touchés (front),
     *        ouvre/referme les plaques de pression selon le poids présent, applique l'état des
     *        portes dans la grille de collision, et met à jour l'activation des dangers commutés.
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

    /// @return Les liaisons de danger commuté (positions interrupteur/danger, `EX-GP-052`).
    [[nodiscard]] const std::vector<DangerLink>& dangerLinks() const noexcept {
        return _dangerLinks;
    }

    /// @return true si le danger commuté à @p dangerPosition est **actif** (mortel) — son
    ///         déclencheur lié est actionné. `false` si @p dangerPosition ne correspond à aucune
    ///         liaison connue (danger commuté non lié, inerte).
    [[nodiscard]] bool isDangerActive(GridPosition dangerPosition) const noexcept {
        for (std::size_t index = 0; index < _dangerLinks.size(); ++index) {
            if (_dangerLinks[index].dangerPosition == dangerPosition) {
                return _dangerActive[index];
            }
        }
        return false;
    }

private:
    TileMap _collision;  ///< Copie mutable : portes Solid (fermées) / Door (ouvertes).
    std::vector<Mechanism> _mechanisms;  ///< Liaisons déclencheur↔porte (positions).
    std::vector<bool> _switchOn;         ///< État de chaque déclencheur (porte ouverte ?).
    std::vector<bool>
        _playerOnSwitchPrev;  ///< Le personnage était-il sur le déclencheur au pas précédent ?
    std::vector<bool> _continuous;  ///< true = plaque de pression (activation continue), figé au
                                    ///< chargement ; false = interrupteur à bascule classique.
    std::vector<DangerLink> _dangerLinks;      ///< Liaisons déclencheur↔danger commuté.
    std::vector<bool> _dangerActive;           ///< État de chaque danger commuté (actif = mortel ?).
    std::vector<bool> _playerOnDangerTriggerPrev;  ///< Front, même principe que
                                                    ///< `_playerOnSwitchPrev`, pour `_dangerLinks`.
    std::vector<bool> _dangerContinuous;  ///< true = plaque de pression, même principe que
                                          ///< `_continuous`, pour `_dangerLinks`.
};

}  // namespace core
