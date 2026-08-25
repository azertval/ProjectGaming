// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/MechanismStateEncoder.h"
#include "AiSolver/Env/PlayerStateEncoder.h"
#include "AiSolver/Env/TileWindowEncoder.h"
#include "AiSolver/Math/Tensor.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Physics/Aabb.h"

/**
 * @file AiSolver/Env/ObservationEncoder.h
 * @brief Assemblage plat des trois encodeurs de `LOT-ANNEXE-06` en un unique vecteur d'entrée de
 * réseau (`Tensor<float>` de forme `[inputSize(), 1]`).
 *
 * `LOT-ANNEXE-06` livre trois encodeurs indépendants (`TileWindowEncoder`, `MechanismStateEncoder`,
 * `PlayerStateEncoder`) mais aucun assemblage plat consommable directement par un `nn::Network` —
 * seul un test (`test_observation_determinisme.cpp`) les combinait, localement, sans aplatissement.
 * `LOT-ANNEXE-10` (fitness évaluée par propagation avant d'un réseau) est le premier consommateur
 * réel de cet assemblage : introduit ici plutôt que dans `LOT-ANNEXE-06` pour rester au plus près
 * de son premier usage, sans rouvrir un lot déjà clos.
 */

namespace aisolver {

/**
 * @brief Concatène, dans un ordre fixe et documenté, les trois encodeurs d'observation en un seul
 * vecteur d'entrée de réseau.
 *
 * Ordre de concaténation (invariant stable, comme `ActionSpace::actionAt` : un réseau entraîné sur
 * cet ordre romprait sa correspondance entrée/poids si l'ordre changeait) : fenêtre de tuiles
 * (`TileWindowEncoder`), puis état des mécanismes (`MechanismStateEncoder`), puis état joueur
 * (`PlayerStateEncoder`).
 */
class ObservationEncoder {
public:
    /// Rayon de fenêtre par défaut (voir `TileWindowEncoder`) : compromis entre visibilité des
    /// obstacles proches et taille du vecteur d'entrée (donc coût de la propagation avant, répétée
    /// à chaque pas de chaque individu de chaque génération).
    static constexpr int DEFAULT_RADIUS = 2;

    explicit ObservationEncoder(int radius = DEFAULT_RADIUS);

    /**
     * @brief Encode l'observation complète pour un état donné du personnage.
     * @param environment   Environnement chargé, source de la fenêtre de tuiles et des mécanismes.
     * @param playerBox     Boîte courante du personnage (détermine la case centrale de la fenêtre).
     * @param playerState   État `core::Player` courant (minuteries, budgets, contact).
     * @param playerVelocity Vitesse courante du personnage.
     * @return Vecteur colonne de forme `[inputSize(), 1]`.
     */
    [[nodiscard]] Tensor<float> encode(const HeadlessLevelEnvironment& environment,
                                       const core::Aabb& playerBox, const core::Player& playerState,
                                       const core::Velocity& playerVelocity) const;

    /// @return Taille totale du vecteur d'entrée produit par `encode()`.
    [[nodiscard]] std::size_t inputSize() const noexcept;

private:
    int _radius;
    TileWindowEncoder _tileEncoder;
    MechanismStateEncoder _mechanismEncoder;
    PlayerStateEncoder _playerEncoder;
};

}  // namespace aisolver
