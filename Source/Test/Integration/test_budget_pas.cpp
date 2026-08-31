// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_budget_pas.cpp
 * @brief Calibration du budget de pas et du seuil de blocage dérivés du niveau
 * (`AiSolver/Env/StepBudget.h`).
 *
 * Ces deux valeurs bornaient auparavant l'apprentissage bien avant la fin d'un niveau long : un
 * budget fixe de `3 000` pas contre les ~`4 000` que demande le tracé de `demo-final.json`, et un
 * seuil de blocage de `200` pas mesuré à vol d'oiseau vers la sortie — sur un niveau dont la
 * solution s'éloigne de la sortie, tout épisode était déclaré bloqué après trois secondes de jeu
 * simulé. Les tests ci-dessous **fixent** les constantes de `StepBudget.h` : ce ne sont pas des
 * valeurs estimées à la lecture, ce sont celles que les tracés livrés valident.
 */

#include <algorithm>
#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/Reward.h"
#include "AiSolver/Env/StepBudget.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"
#include "Test/Systeme/ScriptedLevelSequence.h"

namespace {

std::filesystem::path levelPath(const char* file) {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
}

/// Ce que coûte réellement le tracé de référence d'un niveau.
struct ScriptedRun {
    int stepCount = 0;
    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    /// Plus longue série de pas consécutifs sans amélioration de l'objectif immédiat.
    int longestStagnation = 0;
};

/// Rejoue @p scripted sous un budget volontairement large et sans seuil de blocage : la seule
/// façon de mesurer ce que coûte le tracé de référence sans que la mesure ne dépende de la valeur
/// qu'elle sert justement à calibrer.
ScriptedRun playScripted(const ScriptedLevel& scripted) {
    constexpr int GENEROUS_BUDGET = 30000;
    aisolver::HeadlessLevelEnvironment environment(aisolver::EnvironmentConfig{
        .maxSteps = GENEROUS_BUDGET, .stuckThreshold = GENEROUS_BUDGET});
    ScriptedRun run;
    if (!environment.reset(levelPath(scripted.file))) {
        return run;
    }

    const core::GridPosition entry = environment.level().entry();
    const core::Aabb spawnBox = core::Aabb::fromTopLeftSize(
        core::playerSpawnPosition(entry.column, entry.row), core::playerSize());
    core::Player playerState;
    float x = spawnBox.min.x;
    float y = spawnBox.min.y;

    while (run.outcome == core::LevelOutcome::Playing && !environment.budgetExhausted()) {
        const core::PlayerInput input = scripted.input(run.stepCount, playerState, x, y);
        const aisolver::StepObservation observation = environment.step(input);
        run.outcome = observation.outcome;
        playerState = observation.playerState;
        x = observation.playerBox.min.x;
        y = observation.playerBox.min.y;
        ++run.stepCount;
        run.longestStagnation = (std::max)(run.longestStagnation, environment.stepsSinceProgress());
    }
    return run;
}

}  // namespace

/**
 * @brief Le budget dérivé d'un niveau couvre le tracé de référence de ce niveau, pour tous les
 *        niveaux livrés.
 * \castest{<b>Le budget dérivé couvre chaque tracé scripté.</b><br/>
 * \tcat Intégration · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Pour chaque niveau de `scriptedSequence()`, rejouer son tracé sous un budget large
 * et relever le nombre de pas réellement consommés.<br/>2. Comparer à
 * `estimateStepBudget(niveau)`.<br/>
 * \tattendu Le tracé atteint `Won`, et `estimateStepBudget` est supérieur ou égal au nombre de pas
 * consommés — sinon une politique parfaite expirerait avant la fin du niveau.}
 */
TEST(BudgetPasTest, BudgetCouvreChaqueTraceScripte) {
    const std::vector<ScriptedLevel> sequence = scriptedSequence();
    ASSERT_FALSE(sequence.empty());

    for (const ScriptedLevel& scripted : sequence) {
        const ScriptedRun run = playScripted(scripted);
        ASSERT_EQ(run.outcome, core::LevelOutcome::Won) << "niveau : " << scripted.file;

        const core::LevelLoadResult loaded =
            core::LevelLoader::loadFromFile(levelPath(scripted.file));
        ASSERT_TRUE(loaded.ok()) << "niveau : " << scripted.file;
        const int budget = aisolver::estimateStepBudget(*loaded.level);

        // Marge de moitie, pas une simple inegalite : le tracé de reference est PARFAIT, et un
        // agent qui apprend a besoin de place pour se tromper avant de trouver la meme route.
        EXPECT_GE(budget, run.stepCount + run.stepCount / 2)
            << "niveau : " << scripted.file << " -- budget derive " << budget
            << " trop juste face aux " << run.stepCount << " pas du trace de reference";
    }
}

/**
 * @brief Un joueur qui résout réellement le niveau n'est jamais déclaré bloqué : c'est la
 *        régression exacte qui rendait `demo-final.json` inapprenable.
 * \castest{<b>Un tracé résolvant n'est jamais classé bloqué sous le seuil dérivé.</b><br/>
 * \tcat Intégration · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Pour chaque niveau de `scriptedSequence()`, rejouer son tracé et relever la plus
 * longue série de pas consécutifs sans amélioration de l'objectif immédiat.<br/>2. Comparer au
 * seuil de blocage dérivé du budget du niveau.<br/>
 * \tattendu La stagnation la plus longue reste strictement inférieure au seuil : aucun tracé
 * gagnant ne serait coupé en cours de route.}
 */
TEST(BudgetPasTest, AucunTraceResolvantNEstDeclareBloque) {
    const std::vector<ScriptedLevel> sequence = scriptedSequence();
    ASSERT_FALSE(sequence.empty());

    for (const ScriptedLevel& scripted : sequence) {
        const ScriptedRun run = playScripted(scripted);
        ASSERT_EQ(run.outcome, core::LevelOutcome::Won) << "niveau : " << scripted.file;

        const core::LevelLoadResult loaded =
            core::LevelLoader::loadFromFile(levelPath(scripted.file));
        ASSERT_TRUE(loaded.ok()) << "niveau : " << scripted.file;
        const int threshold =
            aisolver::stuckThresholdForBudget(aisolver::estimateStepBudget(*loaded.level));

        // Meme marge de moitie, et pour la meme raison : `demo-final.json` stagne deja `807` pas
        // en jouant parfaitement (attentes devant les dangers temporises, trajets en plateforme).
        EXPECT_GT(threshold, run.longestStagnation + run.longestStagnation / 2)
            << "niveau : " << scripted.file << " -- seuil derive " << threshold
            << " trop juste face aux " << run.longestStagnation
            << " pas de stagnation du trace de reference";
    }
}

/**
 * @brief La chaîne d'objectifs croît avec la complexité du niveau, et le budget reste borné.
 * \castest{<b>Chaîne d'objectifs et bornes du budget dérivé.</b><br/>
 * \tcat Intégration · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mesurer la chaîne d'objectifs de `demo-final.json` et de
 * `demo-deplacement.json`.<br/>2. Vérifier les bornes des budgets dérivés.<br/>
 * \tattendu La chaîne de `demo-final.json` est strictement plus longue ; les deux budgets restent
 * dans `[MIN_STEP_BUDGET, MAX_STEP_BUDGET]`.}
 */
TEST(BudgetPasTest, ChaineObjectifsCroitAvecLaComplexite) {
    const core::LevelLoadResult complexLevel =
        core::LevelLoader::loadFromFile(levelPath("demo-final.json"));
    const core::LevelLoadResult simpleLevel =
        core::LevelLoader::loadFromFile(levelPath("demo-deplacement.json"));
    ASSERT_TRUE(complexLevel.ok());
    ASSERT_TRUE(simpleLevel.ok());

    EXPECT_GT(aisolver::objectiveChainLength(*complexLevel.level),
              aisolver::objectiveChainLength(*simpleLevel.level));

    for (const core::LevelLoadResult* level : {&complexLevel, &simpleLevel}) {
        const int budget = aisolver::estimateStepBudget(*level->level);
        EXPECT_GE(budget, aisolver::MIN_STEP_BUDGET);
        EXPECT_LE(budget, aisolver::MAX_STEP_BUDGET);
    }
}

/**
 * @brief Les poids de la récompense restent cohérents avec le plafond de budget.
 * \castest{<b>Pénalité de temps cumulée bornée par la pénalité de mort, elle-même par le
 * bonus.</b><br/>
 * \tcat Intégration · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire les constantes par défaut de `RewardConfig`.<br/>2. Les confronter à
 * `MAX_STEP_BUDGET`.<br/>
 * \tattendu `MAX_STEP_BUDGET × timePenalty` reste inférieur à la pénalité de mort, elle-même
 * inférieure au bonus de complétion : sans quoi mourir tôt coûterait moins cher que d'explorer un
 * épisode entier, et l'agent apprendrait à se jeter dans le premier danger venu.}
 */
TEST(BudgetPasTest, PoidsDeRecompenseCoherentsAvecLePlafondDeBudget) {
    const aisolver::RewardConfig reward;
    const float worstTimePenalty =
        static_cast<float>(aisolver::MAX_STEP_BUDGET) * reward.timePenalty;

    EXPECT_LT(worstTimePenalty, -reward.deathPenalty);
    EXPECT_LT(-reward.deathPenalty, reward.completionBonus);
}
