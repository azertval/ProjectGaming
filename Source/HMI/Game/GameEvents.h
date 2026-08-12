#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "Core/Ecs/Components/Player.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/LevelOutcome.h"

/**
 * @file HMI/Game/GameEvents.h
 * @brief Détection pure des transitions d'état du jeu et de l'IHM (`EX-REN-047`, `LOT-60`).
 */

namespace hmi {

/**
 * @brief Événement discret survenu sur un pas de simulation ou une action d'interface.
 *
 * Unifie les transitions de jeu (personnage, mécanismes) et d'interface (menus, écrans du
 * `LOT-59`) sous une seule énumération : c'est ce qui permet à `hmi::SoundTriggers` de leur
 * associer un son par une table exhaustive unique, et à `LOT-53` de réutiliser la même
 * énumération pour ses particules sans dupliquer la détection.
 */
enum class GameEvent {
    // Personnage (détecté par `detectPlayerEvents`, un pas de simulation à la fois).
    Jumped,
    Landed,
    Dashed,
    WallContactEnter,
    // Mécanismes (détecté par `detectMechanismEvents`).
    SwitchToggled,
    DoorOpened,
    DoorClosed,
    PressurePlatePressed,
    PressurePlateReleased,
    BlockPushed,
    // Issue du tableau (détecté par `detectOutcomeEvent`, depuis `core::LevelOutcome`).
    Died,
    LevelCompleted,
    // Interface (`LOT-59`) : raisés directement aux points de signal Qt existants, pas par
    // diffusion d'état — la table de sons ci-dessous les couvre malgré tout, pour l'uniformité.
    MenuNavigate,
    MenuConfirm,
    MenuBack,
    PauseOpened,
    SequenceCompleted,
};

/// Nombre de valeurs de `GameEvent` — pour les tests d'exhaustivité (parcours de l'énumération).
inline constexpr int GAME_EVENT_COUNT = static_cast<int>(GameEvent::SequenceCompleted) + 1;

/**
 * @brief Sous-ensemble de `core::Player` pertinent à la détection d'événements.
 *
 * Ne recopie que les champs dont une **transition** produit un événement — pas tout `Player`,
 * dont la plupart des champs (minuteries, budgets) n'intéressent que la physique.
 */
struct PlayerEventState {
    bool grounded = false;
    float dashTimer = 0.0f;
    float wallDirection = 0.0f;
    bool justJumped = false;

    /// @return L'état pertinent extrait de @p player, à un pas de simulation donné.
    [[nodiscard]] static PlayerEventState capture(const core::Player& player) noexcept {
        return PlayerEventState{.grounded = player.grounded,
                                .dashTimer = player.dashTimer,
                                .wallDirection = player.wallDirection,
                                .justJumped = player.justJumped};
    }
};

/// État « porte ouverte » de chaque mécanisme, même index que
/// `core::MechanismController::mechanisms()`.
struct MechanismEventState {
    std::vector<bool> doorOpen;

    /// @return L'état extrait de @p mechanisms, à un pas de simulation donné.
    [[nodiscard]] static MechanismEventState capture(const core::MechanismController& mechanisms) {
        MechanismEventState state;
        state.doorOpen.reserve(mechanisms.mechanisms().size());
        for (std::size_t index = 0; index < mechanisms.mechanisms().size(); ++index) {
            state.doorOpen.push_back(mechanisms.isDoorOpen(index));
        }
        return state;
    }
};

/**
 * @brief Détecte les transitions du personnage entre deux pas de simulation consécutifs.
 *
 * Fonction **pure** : lit @p previous et @p current, n'écrit rien, aucune dépendance Qt ni
 * périphérique (`EX-NFR-010`). À appeler **une fois par pas fixe** (jamais par image de rendu,
 * `EX-REN-021`) — le piège exact que le `LOT-33` a déjà traité pour les entrées : plusieurs
 * images de rendu dans le même pas ne doivent jamais produire l'événement deux fois.
 * @return La liste des événements survenus sur ce pas (généralement 0 ou 1 ; un dash au contact
 *         d'un mur peut en produire deux : `Dashed` et `WallContactEnter`).
 */
[[nodiscard]] std::vector<GameEvent> detectPlayerEvents(const PlayerEventState& previous,
                                                        const PlayerEventState& current);

/**
 * @brief Détecte les transitions des mécanismes entre deux pas de simulation consécutifs.
 *
 * Un même mécanisme (`isDoorOpen`) sert à la fois de déclencheur et de porte : une seule
 * transition produit **un seul** événement (celui du déclencheur), jamais un second pour la
 * porte — sans quoi une simple activation jouerait deux sons superposés.
 * @param previous État des mécanismes au pas précédent.
 * @param current État des mécanismes au pas courant.
 * @param isContinuous Pour chaque mécanisme (même index que @p previous / @p current) : true =
 *        plaque de pression (joue « pressée »/« relâchée »), false = interrupteur à bascule
 *        (joue « basculé »). Un désaccord de taille entre les trois vecteurs ignore l'index en
 *        trop plutôt que d'accéder hors bornes.
 */
[[nodiscard]] std::vector<GameEvent> detectMechanismEvents(const MechanismEventState& previous,
                                                           const MechanismEventState& current,
                                                           const std::vector<bool>& isContinuous);

/**
 * @brief Traduit l'issue d'un pas de simulation en événement.
 * @param outcome Issue renvoyée par `core::evaluateOutcome` / `core::GameSession::update`.
 * @return `Died` pour `Lost`, `LevelCompleted` pour `Won`, `std::nullopt` pour `Playing`.
 */
[[nodiscard]] std::optional<GameEvent> detectOutcomeEvent(core::LevelOutcome outcome);

}  // namespace hmi
