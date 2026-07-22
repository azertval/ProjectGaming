#pragma once

/**
 * @file Core/Ecs/Components/Animation.h
 * @brief Composant d'état d'animation par séquence d'images (données pures).
 */

namespace core {

/**
 * @brief Clip d'animation du personnage (`EX-REN-012`).
 */
enum class AnimationClip {
    /// Immobile, au sol.
    Idle,
    /// Déplacement horizontal, au sol.
    Run,
    /// En l'air (saut ou chute) — pose unique, pas un cycle.
    Jump,
};

/// Nombre d'images du clip `Idle`. Seule source de vérité (`Core` et `HMI`, cf. LOT-18).
constexpr int IDLE_FRAME_COUNT = 2;
/// Nombre d'images du clip `Run`.
constexpr int RUN_FRAME_COUNT = 4;
/// Nombre d'images du clip `Jump` (pose unique, jamais animée).
constexpr int JUMP_FRAME_COUNT = 1;

/**
 * @brief État d'animation courant d'une entité : quel clip, quelle image, depuis combien de temps.
 *
 * Donnée pure sans logique (`EX-ARCH-011`), mise à jour par `AnimationSystem` et lue par le rendu
 * (`HMI`) pour choisir la région d'atlas à afficher. Le clip est une **projection** de l'état
 * physique du personnage (`Player::grounded`, `Velocity`) — il ne s'agit pas d'un état piloté par
 * une entrée du joueur, ni d'une source de vérité supplémentaire.
 */
struct Animation {
    /// Clip actuellement joué.
    AnimationClip clip = AnimationClip::Idle;
    /// Index de l'image courante dans le clip (0-based, borné au nombre d'images du clip).
    int frameIndex = 0;
    /// Temps écoulé (secondes) depuis le passage à `frameIndex`.
    float elapsed = 0.0f;
};

}  // namespace core
