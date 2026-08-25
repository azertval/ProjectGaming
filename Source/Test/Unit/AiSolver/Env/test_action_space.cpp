// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_action_space.cpp
 * @brief Tests unitaires de aisolver::ActionSpace (LOT-ANNEXE-07, TACHE-01).
 */

#include <set>

#include <gtest/gtest.h>

#include "AiSolver/Env/ActionSpace.h"

/**
 * @brief `actionCount()` vaut exactement 48 (3 directions x 2 x 2 x 2 x 2), verifie explicitement.
 * \castest{<b>`actionCount()` vaut exactement 48.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Lire `aisolver::actionCount()`.<br/>
 * \tattendu La valeur est 48.}
 */
TEST(ActionSpaceTest, NombreTotalDeCombinaisons) {
    EXPECT_EQ(aisolver::actionCount(), 48u);
}

/**
 * @brief Pour tout indice valide, `indexOf(actionAt(indice)) == indice` (bijection indice -> action
 * -> indice).
 * \castest{<b>Bijection indice -> action -> indice sur tout l'espace d'action.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Pour chaque indice de `[0, actionCount())`, appeler `actionAt` puis `indexOf`.<br/>
 * \tattendu L'indice obtenu est identique a l'indice de depart, pour tout indice.}
 */
TEST(ActionSpaceTest, BijectionIndiceVersActionVersIndice) {
    for (std::size_t index = 0; index < aisolver::actionCount(); ++index) {
        const aisolver::Action action = aisolver::actionAt(index);
        EXPECT_EQ(aisolver::indexOf(action), index) << "indice " << index;
    }
}

/**
 * @brief Aucune collision entre deux indices differents : les 48 actions sont deux a deux
 * distinctes.
 * \castest{<b>Les 48 actions sont deux a deux distinctes.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Convertir chaque action de `[0, actionCount())` en `core::PlayerInput` via
 * `toPlayerInput`.<br/>2. Comparer chaque paire.<br/>
 * \tattendu Aucune paire de `core::PlayerInput` produite n'est identique sur les cinq champs
 * pilotes par l'espace d'action (`moveX`/`jumpPressed`/`jumpHeld`/`dashPressed`/
 * `interactPressed`).}
 */
TEST(ActionSpaceTest, AucuneCollisionEntreActions) {
    std::set<std::tuple<float, bool, bool, bool, bool>> seen;
    for (std::size_t index = 0; index < aisolver::actionCount(); ++index) {
        const core::PlayerInput input = aisolver::toPlayerInput(aisolver::actionAt(index));
        const auto key = std::make_tuple(input.moveX, input.jumpPressed, input.jumpHeld,
                                         input.dashPressed, input.interactPressed);
        EXPECT_TRUE(seen.insert(key).second) << "collision a l'indice " << index;
    }
    EXPECT_EQ(seen.size(), aisolver::actionCount());
}

/**
 * @brief `toPlayerInput` traduit chaque `Direction` en `moveX` attendu, `moveY` toujours 0.
 * \castest{<b>Traduction correcte de la direction vers `moveX`, `moveY` toujours nul.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire une action pour chaque `Direction`.<br/>2. Traduire via
 * `toPlayerInput`.<br/>
 * \tattendu `moveX` vaut -1/0/1 selon `Left`/`None`/`Right` ; `moveY` vaut 0 dans tous les cas.}
 */
TEST(ActionSpaceTest, TraductionDirectionVersMoveX) {
    const core::PlayerInput left =
        aisolver::toPlayerInput(aisolver::Action{aisolver::Direction::Left});
    const core::PlayerInput none =
        aisolver::toPlayerInput(aisolver::Action{aisolver::Direction::None});
    const core::PlayerInput right =
        aisolver::toPlayerInput(aisolver::Action{aisolver::Direction::Right});

    EXPECT_FLOAT_EQ(left.moveX, -1.0f);
    EXPECT_FLOAT_EQ(none.moveX, 0.0f);
    EXPECT_FLOAT_EQ(right.moveX, 1.0f);
    EXPECT_FLOAT_EQ(left.moveY, 0.0f);
    EXPECT_FLOAT_EQ(none.moveY, 0.0f);
    EXPECT_FLOAT_EQ(right.moveY, 0.0f);
}

/**
 * @brief `toPlayerInput` reporte fidelement `jumpPressed`/`jumpHeld`/`dashPressed`/
 * `interactPressed`.
 * \castest{<b>Traduction fidele des quatre booleens de l'action.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une action avec les quatre booleens a `true`.<br/>2. Traduire via
 * `toPlayerInput`.<br/>
 * \tattendu Les quatre champs correspondants de `core::PlayerInput` valent `true`.}
 */
TEST(ActionSpaceTest, TraductionBooleensFidele) {
    const aisolver::Action action{aisolver::Direction::None, true, true, true, true};
    const core::PlayerInput input = aisolver::toPlayerInput(action);
    EXPECT_TRUE(input.jumpPressed);
    EXPECT_TRUE(input.jumpHeld);
    EXPECT_TRUE(input.dashPressed);
    EXPECT_TRUE(input.interactPressed);
}
