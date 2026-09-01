// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_headless_level_environment.cpp
 * @brief Tests unitaires de aisolver::HeadlessLevelEnvironment (LOT-ANNEXE-05).
 */

#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/StepBudget.h"
#include "Core/Diagnostics/Assert.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/PlayerInput.h"

namespace {

std::filesystem::path levelPath(const char* file) {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
}

// Script constant : avancer a droite, rien d'autre -- meme patron que rightOnly() dans
// Source/Test/Systeme/test_parcours_complet.cpp.
core::PlayerInput rightOnly() {
    return core::PlayerInput{1.0f};
}

}  // namespace

/**
 * @brief `reset` sur un niveau valide charge le niveau et positionne le personnage sur l'entree.
 * \castest{<b>`reset` charge un niveau valide et positionne le personnage sur l'entree.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `reset("demo-deplacement.json")`.<br/>2. Appeler `step` une fois et lire la boite du
 * personnage.<br/>
 * \tattendu `loaded()` est vrai, `loadError()` est vide, le centre de la boite du personnage
 * coincide avec le centre de la case d'entree du niveau.}
 */
TEST(HeadlessLevelEnvironmentTest, ResetReussitEtPositionneLePersonnage) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));
    EXPECT_TRUE(env.loaded());
    EXPECT_TRUE(env.loadError().empty());

    const core::GridPosition entry = env.level().entry();
    const aisolver::StepObservation observation = env.step(core::PlayerInput{});
    const core::Vector2 center = (observation.playerBox.min + observation.playerBox.max) * 0.5f;
    EXPECT_NEAR(center.x, static_cast<float>(entry.column) + 0.5f, 1e-3f);
    EXPECT_NEAR(center.y, static_cast<float>(entry.row) + 0.5f,
                1e-1f);  // un pas de chute deja ecoule
}

/**
 * @brief `reset` sur un chemin inexistant echoue proprement, sans exception.
 * \castest{<b>`reset` sur un chemin inexistant echoue proprement, sans exception.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `reset("demo-fichier-inexistant.json")`.<br/>
 * \tattendu `reset` renvoie `false`, `loaded()` reste faux, `loadError()` est non vide.}
 */
TEST(HeadlessLevelEnvironmentTest, ResetEchoueSansException) {
    aisolver::HeadlessLevelEnvironment env;
    EXPECT_FALSE(env.reset(levelPath("demo-fichier-inexistant.json")));
    EXPECT_FALSE(env.loaded());
    EXPECT_FALSE(env.loadError().empty());
}

/**
 * @brief Un budget de sauts/dashs illimite (`-1`) du niveau se retrouve tel quel sur le personnage.
 * \castest{<b>Un budget de sauts/dashs illimite (`-1`) se retrouve identique sur le
 * personnage.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `reset("demo-deplacement.json")` (budgets illimites).<br/>2. `step` une fois.<br/>
 * \tattendu `playerState.jumpsRemaining`/`dashesRemaining` valent `-1`.}
 */
TEST(HeadlessLevelEnvironmentTest, BudgetsInitiauxIllimites) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));
    ASSERT_EQ(env.level().jumpBudget(), -1);
    ASSERT_EQ(env.level().dashBudget(), -1);

    const aisolver::StepObservation observation = env.step(core::PlayerInput{});
    EXPECT_EQ(observation.playerState.jumpsRemaining, -1);
    EXPECT_EQ(observation.playerState.dashesRemaining, -1);
}

/**
 * @brief Un budget fini du niveau se retrouve identique sur le personnage apres `reset`.
 * \castest{<b>Un budget fini du niveau se retrouve identique sur le personnage apres
 * `reset`.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `reset("demo-budget.json")` (budget fini).<br/>2. `step` une fois, sans saut ni
 * dash.<br/>
 * \tattendu `playerState.jumpsRemaining`/`dashesRemaining` valent exactement
 * `level().jumpBudget()`/`dashBudget()`.}
 */
TEST(HeadlessLevelEnvironmentTest, BudgetsInitiauxFinis) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath("demo-budget.json")));
    const int jumpBudget = env.level().jumpBudget();
    const int dashBudget = env.level().dashBudget();
    ASSERT_NE(jumpBudget, -1);

    // Un pas sans saut ni dash ne consomme aucun budget.
    const aisolver::StepObservation observation = env.step(core::PlayerInput{});
    EXPECT_EQ(observation.playerState.jumpsRemaining, jumpBudget);
    EXPECT_EQ(observation.playerState.dashesRemaining, dashBudget);
}

/**
 * @brief `reset` appele deux fois sur le meme chemin produit un etat strictement identique
 *        (determinisme, `EX-NFR-002`).
 * \castest{<b>`reset` repete sur le meme chemin produit un etat strictement identique
 * (determinisme).</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `reset` puis un `step`, releve position/vitesse/`stepIndex`.<br/>2. `reset` a nouveau
 * sur le meme chemin puis un `step` identique.<br/>
 * \tattendu Position, vitesse et `stepIndex` sont identiques entre les deux rejeux ;
 * `stepsSinceProgress()` est remis a zero par le second `reset`.}
 */
TEST(HeadlessLevelEnvironmentTest, ResetRepeteProduitUnEtatIdentique) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));
    const aisolver::StepObservation first = env.step(rightOnly());
    EXPECT_EQ(first.stepIndex, 1);

    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));
    EXPECT_EQ(env.stepsSinceProgress(), 0);
    const aisolver::StepObservation second = env.step(rightOnly());

    EXPECT_EQ(second.stepIndex, 1);
    EXPECT_NEAR(first.playerBox.min.x, second.playerBox.min.x, 1e-6f);
    EXPECT_NEAR(first.playerBox.min.y, second.playerBox.min.y, 1e-6f);
    EXPECT_NEAR(first.playerVelocity.value.x, second.playerVelocity.value.x, 1e-6f);
    EXPECT_NEAR(first.playerVelocity.value.y, second.playerVelocity.value.y, 1e-6f);
}

/**
 * @brief Un pas de marche fait avancer la position `x` du personnage, issue `Playing`.
 * \castest{<b>Un pas de marche fait avancer la position `x` du personnage.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `reset("demo-deplacement.json")`.<br/>2. Deux `step` successifs avec `moveX =
 * 1`.<br/>
 * \tattendu La position `x` du second pas est strictement superieure a celle du premier, l'issue
 * reste `Playing`.}
 */
TEST(HeadlessLevelEnvironmentTest, PasDeMarcheAvanceLaPosition) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));
    const aisolver::StepObservation first = env.step(rightOnly());
    const aisolver::StepObservation second = env.step(rightOnly());

    EXPECT_GT(second.playerBox.min.x, first.playerBox.min.x);
    EXPECT_EQ(second.outcome, core::LevelOutcome::Playing);
}

/**
 * @brief Rejouer `demo-deplacement.json` en marchant a droite atteint `Won` (aucun saut requis).
 * \castest{<b>Rejouer `demo-deplacement.json` en marchant a droite atteint `Won`.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `reset("demo-deplacement.json")`.<br/>2. `step` en boucle avec `moveX = 1` jusqu'a
 * une issue ou au budget de pas.<br/>
 * \tattendu L'issue finale est `Won`.}
 */
TEST(HeadlessLevelEnvironmentTest, SequenceCompleteDeplacementAtteintLaSortie) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    while (outcome == core::LevelOutcome::Playing && !env.budgetExhausted()) {
        outcome = env.step(rightOnly()).outcome;
    }
    EXPECT_EQ(outcome, core::LevelOutcome::Won);
}

/**
 * @brief Un personnage qui saute en continu dans le couloir de `demo-dangers-avances.json` finit
 *        par toucher un danger mobile (`DangerController` actif, a la difference de l'ancien
 *        `playLevel()` sans dangers -- voir la decision de cadrage de l'epic).
 * \castest{<b>Un personnage qui saute en continu dans `demo-dangers-avances.json` finit par
 * toucher un danger mobile.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `reset("demo-dangers-avances.json")`.<br/>2. `step` en boucle, `moveX = 1` et saut
 * maintenu en continu, jusqu'a une issue ou au budget de pas.<br/>
 * \tattendu L'issue finale est `Lost` -- preuve que `core::DangerController` est bien actif dans
 * `HeadlessLevelEnvironment`.}
 */
TEST(HeadlessLevelEnvironmentTest, DangerAvanceProvoqueUnePerte) {
    aisolver::HeadlessLevelEnvironment env(aisolver::EnvironmentConfig{.maxSteps = 3000});
    ASSERT_TRUE(env.reset(levelPath("demo-dangers-avances.json")));

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    while (outcome == core::LevelOutcome::Playing && !env.budgetExhausted()) {
        core::PlayerInput in{1.0f};
        in.jumpHeld = true;
        in.jumpPressed = true;  // saut en continu : traverse la rangee des dangers mobiles
        outcome = env.step(in).outcome;
    }
    EXPECT_EQ(outcome, core::LevelOutcome::Lost);
}

/**
 * @brief `budgetExhausted` devient vrai exactement au pas `maxSteps`, jamais avant.
 * \castest{<b>`budgetExhausted` devient vrai exactement au pas `maxSteps`, jamais avant.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `reset` avec `maxSteps = 10`.<br/>2. `step` dix fois, verifiant `budgetExhausted()`
 * faux avant chacun.<br/>
 * \tattendu `budgetExhausted()` est faux aux dix premiers pas, vrai apres le dixieme.}
 */
TEST(HeadlessLevelEnvironmentTest, BudgetDePasAtteintExactementAMaxSteps) {
    constexpr int MAX_STEPS = 10;
    aisolver::HeadlessLevelEnvironment env(aisolver::EnvironmentConfig{.maxSteps = MAX_STEPS});
    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));

    for (int step = 0; step < MAX_STEPS; ++step) {
        EXPECT_FALSE(env.budgetExhausted()) << "pas " << step;
        (void)env.step(core::PlayerInput{});
    }
    EXPECT_TRUE(env.budgetExhausted());
}

/**
 * @brief Appeler `step` au-dela du budget de pas declenche l'assertion de programmation.
 * \castest{<b>Appeler `step` au-dela du budget de pas declenche l'assertion de
 * programmation.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `reset` avec `maxSteps = 2`, consommer les deux pas.<br/>2. Poser un gestionnaire
 * d'assertion de test, appeler `step` une fois de plus.<br/>
 * \tattendu Le gestionnaire d'assertion est invoque exactement une fois (Debug uniquement).}
 */
TEST(HeadlessLevelEnvironmentTest, AppelAuDelaDuBudgetDeclencheAssertion) {
#ifdef NDEBUG
    GTEST_SKIP() << "Assertions desactivees en Release";
#else
    constexpr int MAX_STEPS = 2;
    aisolver::HeadlessLevelEnvironment env(aisolver::EnvironmentConfig{.maxSteps = MAX_STEPS});
    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));
    for (int step = 0; step < MAX_STEPS; ++step) {
        (void)env.step(core::PlayerInput{});
    }
    ASSERT_TRUE(env.budgetExhausted());

    int assertionCount = 0;
    core::setAssertionHandler(
        [&](const char*, const char*, const char*, int) { ++assertionCount; });
    (void)env.step(core::PlayerInput{});
    EXPECT_EQ(assertionCount, 1);
    core::setAssertionHandler(nullptr);
#endif
}

/**
 * @brief En avancant vers la sortie, `stepsSinceProgress` croit moins vite qu'a l'arret : la
 * distance de grille n'ameliore pas `bestObjectiveDistance` a CHAQUE pas (une case demande vingt
 * pas de marche), mais bien plus souvent qu'immobile.
 * \castest{<b>En avancant vers la sortie, `stepsSinceProgress` croit moins vite qu'a
 * l'arret.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Rejouer `demo-deplacement.json` 60 pas en marchant a droite.<br/>2. Rejouer le meme
 * niveau 60 pas, immobile.<br/>
 * \tattendu `stepsSinceProgress()` du personnage qui avance est strictement inferieur a celui du
 * personnage immobile.}
 */
TEST(HeadlessLevelEnvironmentTest, ProgressionDetecteeEnAvancant) {
    aisolver::HeadlessLevelEnvironment moving;
    ASSERT_TRUE(moving.reset(levelPath("demo-deplacement.json")));
    for (int step = 0; step < 60; ++step) {
        (void)moving.step(rightOnly());
    }

    aisolver::HeadlessLevelEnvironment still;
    ASSERT_TRUE(still.reset(levelPath("demo-deplacement.json")));
    for (int step = 0; step < 60; ++step) {
        (void)still.step(core::PlayerInput{});
    }

    EXPECT_LT(moving.stepsSinceProgress(), still.stepsSinceProgress());
}

/**
 * @brief Sans deplacement, `stepsSinceProgress` croit de facon monotone, sans se reinitialiser.
 * \castest{<b>Sans deplacement, `stepsSinceProgress` croit de facon monotone.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `reset("demo-deplacement.json")`, 30 pas immobile (chute puis pose au sol), releve
 * `stepsSinceProgress()`.<br/>2. 30 pas immobile supplementaires.<br/>
 * \tattendu `stepsSinceProgress()` a strictement augmente entre les deux releves.}
 */
TEST(HeadlessLevelEnvironmentTest, BlocageDetecteSansMouvement) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));

    // Laisse le personnage se poser au sol (chute initiale, ne compte pas comme un blocage).
    for (int step = 0; step < 30; ++step) {
        (void)env.step(core::PlayerInput{});
    }
    const int settled = env.stepsSinceProgress();

    for (int step = 0; step < 30; ++step) {
        (void)env.step(core::PlayerInput{});
    }
    EXPECT_GT(env.stepsSinceProgress(), settled);
}

/**
 * @brief `reset` remet `bestObjectiveDistance`/`stepsSinceProgress` a leur etat initial, meme apres
 *        un run precedent sur le meme niveau.
 * \castest{<b>`reset` remet `bestObjectiveDistance`/`stepsSinceProgress` a leur etat
 * initial.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `reset` puis 30 pas en marchant a droite, releve `bestObjectiveDistance()`.<br/>2.
 * `reset` a nouveau sur le meme niveau (sans pas), releve les compteurs.<br/>3. `reset` une
 * troisieme fois, compare a l'etat frais du second `reset`.<br/>
 * \tattendu `stepsSinceProgress()` revient a zero apres chaque `reset` ; `bestObjectiveDistance()`
 * fraichement reinitialise est identique d'un `reset` a l'autre sur le meme niveau.}
 */
TEST(HeadlessLevelEnvironmentTest, ResetRemetLesCompteursAZero) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));
    for (int step = 0; step < 30; ++step) {
        (void)env.step(rightOnly());
    }
    EXPECT_GT(env.bestObjectiveDistance(), 0);

    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));
    EXPECT_EQ(env.stepsSinceProgress(), 0);
    const int freshDistance = env.bestObjectiveDistance();

    ASSERT_TRUE(env.reset(levelPath("demo-deplacement.json")));
    EXPECT_EQ(env.bestObjectiveDistance(), freshDistance);
}

/**
 * @brief Sur un niveau dont la solution s'eloigne de la sortie, un joueur qui progresse vraiment
 *        n'est jamais declare bloque -- la regression qui rendait `demo-final.json` inapprenable.
 * \castest{<b>Progression mesuree sur l'objectif immediat, pas a vol d'oiseau vers la
 * sortie.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `reset("demo-final.json")` : l'entree (1,12) est a trois unites de la sortie (5,15),
 * mais la sortie est scellee par trois portes verrouillees dont la cle est a l'oppose du
 * niveau.<br/>2. Marcher a droite jusqu'a la cle `s1` (5,12), puis revenir vers l'entree et monter
 * le puits -- un itineraire qui EL0IGNE de la sortie a vol d'oiseau tout en rapprochant de
 * l'objectif immediat.<br/>
 * \tattendu `stepsSinceProgress()` est remis a zero pendant la montee : la distance mesuree est
 * celle du champ d'objectif, jamais la distance euclidienne a la sortie.}
 */
TEST(HeadlessLevelEnvironmentTest, ProgressionMesureeSurObjectifImmediatPasSurLaSortie) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath("demo-final.json")));

    // Marche vers la cle s1 (a droite de l'entree) : rapproche a la fois de la sortie et de
    // l'objectif immediat, les deux mesures sont d'accord.
    for (int step = 0; step < 120; ++step) {
        core::PlayerInput input = rightOnly();
        input.interactPressed = true;
        (void)env.step(input);
    }

    // Retour vers la gauche puis montee : s'EL0IGNE de la sortie, se rapproche de l'objectif
    // suivant. L'ancienne mesure euclidienne ne pouvait plus battre son record ici.
    int resetsDuringClimb = 0;
    int previousStepsSinceProgress = env.stepsSinceProgress();
    for (int step = 0; step < 400 && !env.budgetExhausted(); ++step) {
        core::PlayerInput input;
        input.moveX = -1.0f;
        input.jumpPressed = step % 20 == 0;
        input.jumpHeld = true;
        (void)env.step(input);
        if (env.stepsSinceProgress() < previousStepsSinceProgress) {
            ++resetsDuringClimb;
        }
        previousStepsSinceProgress = env.stepsSinceProgress();
    }
    EXPECT_GT(resetsDuringClimb, 0);
}

/**
 * @brief Le budget de pas et le seuil de blocage sont derives du niveau, et une valeur explicite
 *        les remplace.
 * \castest{<b>Budget et seuil derives du niveau, surchargeables.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `reset("demo-final.json")` avec une configuration par defaut, relever
 * `stepBudget()`.<br/>2. `reset("demo-deplacement.json")` sur un environnement neuf, meme
 * relevé.<br/>3. Construire un environnement a `maxSteps`/`stuckThreshold` explicites.<br/>
 * \tattendu Le budget de `demo-final.json` depasse celui de `demo-deplacement.json` ; le seuil
 * vaut `stuckThresholdForBudget(budget)` ; une valeur explicite est respectee telle quelle.}
 */
TEST(HeadlessLevelEnvironmentTest, BudgetEtSeuilDerivesDuNiveau) {
    aisolver::HeadlessLevelEnvironment complexLevel;
    ASSERT_TRUE(complexLevel.reset(levelPath("demo-final.json")));
    aisolver::HeadlessLevelEnvironment simpleLevel;
    ASSERT_TRUE(simpleLevel.reset(levelPath("demo-deplacement.json")));

    EXPECT_GT(complexLevel.stepBudget(), simpleLevel.stepBudget());
    EXPECT_EQ(complexLevel.stuckThreshold(),
              aisolver::stuckThresholdForBudget(complexLevel.stepBudget()));

    aisolver::HeadlessLevelEnvironment explicitBudget(
        aisolver::EnvironmentConfig{.maxSteps = 1234, .stuckThreshold = 77});
    ASSERT_TRUE(explicitBudget.reset(levelPath("demo-final.json")));
    EXPECT_EQ(explicitBudget.stepBudget(), 1234);
    EXPECT_EQ(explicitBudget.stuckThreshold(), 77);
}
