// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recompense_demo_niveaux.cpp
 * @brief Non-régression (LOT-ANNEXE-08, TACHE-03, critère d'acceptation 4 de l'épic) : sur chaque
 * niveau `demo-*.json`, un rejeu du script d'entrées existant (`test_parcours_complet.cpp`) via
 * `aisolver::HeadlessLevelEnvironment` classe l'épisode en `Won` avec une récompense cumulée
 * positive dominée par le bonus de complétion.
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/Reward.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"
#include "Test/Systeme/ScriptedLevelSequence.h"

namespace {

std::filesystem::path levelPath(const char* file) {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
}

}  // namespace

/**
 * @brief Chaque niveau `demo-*.json` de la séquence livrée atteint `Won` avec une récompense
 *        cumulée positive et dominée par le bonus de complétion.
 * \castest{<b>Chaque niveau `demo-*.json` atteint `Won` avec une récompense cumulée positive
 * dominée par le bonus de complétion.</b><br/>
 * \tcat Intégration · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Pour chaque niveau de `scriptedSequence()`, rejoue son script d'entrées via
 * `HeadlessLevelEnvironment`, cumulant la récompense de chaque pas.<br/>2. Classe l'épisode via
 * `classifyEpisode` à l'issue du rejeu.<br/>
 * \tattendu `classifyEpisode(...) == Won` et la récompense cumulée est strictement positive et
 * strictement supérieure à `completionBonus / 2` (dominée par le bonus, pas seulement portée par
 * lui).}
 */
TEST(RecompenseDemoNiveauxTest, ChaqueNiveauDemoAtteintWonAvecRecompenseDomineeParLeBonus) {
    const std::vector<ScriptedLevel> sequence = scriptedSequence();
    ASSERT_FALSE(sequence.empty());

    const aisolver::RewardConfig rewardConfig;
    constexpr int HARD_STEP_BUDGET = 3000;
    // Seuil de blocage desactive de fait (egal au plafond dur) : ce test verifie l'issue finale
    // (Won, recompense dominee par le bonus), pas la sensibilite du seuil de blocage (deja
    // couverte par test_episode.cpp/test_reward_episode.cpp) -- un niveau multi-salles comme
    // demo-final.json traverse legitimement de longues sections sans ameliorer la distance a la
    // sortie (detour par une autre salle, attente devant un danger temporise).
    constexpr int STUCK_THRESHOLD = HARD_STEP_BUDGET;

    for (const ScriptedLevel& scripted : sequence) {
        aisolver::HeadlessLevelEnvironment env;
        ASSERT_TRUE(env.reset(levelPath(scripted.file))) << "niveau : " << scripted.file;

        float cumulativeReward = 0.0f;
        aisolver::EpisodeStatus status = aisolver::EpisodeStatus::Ongoing;

        // Boîte de départ, même formule que HeadlessLevelEnvironment::reset (spawn à l'entrée) :
        // point de comparaison du premier pas, avant tout mouvement.
        const core::GridPosition entry = env.level().entry();
        core::Aabb previousBox = core::Aabb::fromTopLeftSize(
            core::playerSpawnPosition(entry.column, entry.row), core::playerSize());
        core::Player playerState;
        // Coin haut-gauche (`Transform::position`), pas le centre de la boîte : même convention
        // que playLevelTraced()/expectStepByStepFidelity (test_parcours_complet.cpp), dont les
        // fenêtres de déclenchement (`atLedge`, seuils `y`) sont calibrées dessus.
        float x = previousBox.min.x;
        float y = previousBox.min.y;
        int step = 0;

        while (status == aisolver::EpisodeStatus::Ongoing && !env.budgetExhausted()) {
            const core::PlayerInput input = scripted.input(step, playerState, x, y);
            const aisolver::StepObservation observation = env.step(input);

            cumulativeReward +=
                aisolver::computeReward(rewardConfig, previousBox, observation.playerBox,
                                        env.level().exit(), observation.outcome);
            previousBox = observation.playerBox;
            playerState = observation.playerState;
            x = observation.playerBox.min.x;
            y = observation.playerBox.min.y;
            ++step;
            status = aisolver::classifyEpisode(observation.outcome, observation.stepIndex,
                                               env.stepsSinceProgress(), HARD_STEP_BUDGET,
                                               STUCK_THRESHOLD);
        }

        EXPECT_EQ(status, aisolver::EpisodeStatus::Won) << "niveau : " << scripted.file;
        EXPECT_GT(cumulativeReward, rewardConfig.completionBonus / 2.0f)
            << "niveau : " << scripted.file;
    }
}
