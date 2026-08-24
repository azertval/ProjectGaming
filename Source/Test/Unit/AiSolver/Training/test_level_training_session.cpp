// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_level_training_session.cpp
 * @brief Boucle d'entraînement pour un niveau et critère d'arrêt (LOT-ANNEXE-11, TACHE-01).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/LevelTrainingSession.h"
#include "AiSolver/Training/TrainingResult.h"
#include "TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::ObservationEncoder;
using aisolver::training::LevelTrainingSession;
using aisolver::training::StoppingConfig;
using aisolver::training::TrainingResult;
using aisolver::training::updateConsecutiveStableWins;
using aisolver::training::evolutionary::EvolutionaryConfig;
using aisolver::training::evolutionary::policyTopology;
using aisolver_test::TrivialLevelDirectory;

namespace {
// Budget de pas reduit (vs. 3000 par defaut) : le niveau trivial se resout en quelques pas,
// reduit tres largement le cout d'une population evaluee sur des dizaines de generations.
constexpr int kReducedMaxSteps = 50;
}  // namespace

/**
 * @brief Sur le niveau trivial (corridor de deux cases), la session s'arrête par résolution avant
 * d'atteindre un plafond de générations volontairement large.
 * \castest{<b>LevelTrainingSession : arrêt par résolution stable.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Session sur le niveau trivial, population 32, seed fixée, plafond large (200).<br/>
 * 2. `run()`.<br/>
 * \tattendu `TrainingResult::solved == true`, `generationsRun` strictement inférieur au plafond.}
 */
TEST(LevelTrainingSessionTest, ArretParResolutionStable) {
    const TrivialLevelDirectory level("resolution");
    const ObservationEncoder encoder;

    EvolutionaryConfig config;
    config.populationSize = 32;
    StoppingConfig stopping;
    stopping.requiredConsecutiveSuccesses = 3;
    stopping.maxGenerations = 200;

    LevelTrainingSession session(level.levelPath(), policyTopology(encoder.inputSize()), config,
                                 stopping, 4242, level.file("stats.csv"),
                                 EnvironmentConfig{.maxSteps = kReducedMaxSteps});

    const TrainingResult result = session.run();

    EXPECT_TRUE(result.solved);
    EXPECT_LT(result.generationsRun, static_cast<unsigned>(stopping.maxGenerations));
}

/**
 * @brief Avec un plafond de générations fixé artificiellement bas face à un seuil de stabilité
 * inatteignable dans cette fenêtre, la session s'arrête par le plafond.
 * \castest{<b>LevelTrainingSession : arrêt par plafond de générations.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Session sur le niveau trivial, seuil de stabilité (1000) inatteignable en 2
 * générations.<br/>2. `run()`.<br/>
 * \tattendu `TrainingResult::solved == false`, `generationsRun == 2`.}
 */
TEST(LevelTrainingSessionTest, ArretParPlafondDeGenerations) {
    const TrivialLevelDirectory level("plafond");
    const ObservationEncoder encoder;

    EvolutionaryConfig config;
    config.populationSize = 8;
    StoppingConfig stopping;
    stopping.requiredConsecutiveSuccesses = 1000;
    stopping.maxGenerations = 2;

    LevelTrainingSession session(level.levelPath(), policyTopology(encoder.inputSize()), config,
                                 stopping, 1, level.file("stats.csv"),
                                 EnvironmentConfig{.maxSteps = kReducedMaxSteps});

    const TrainingResult result = session.run();

    EXPECT_FALSE(result.solved);
    EXPECT_EQ(result.generationsRun, 2u);
}

/**
 * @brief Le compteur de générations consécutives est remis à zéro (puis relancé à `1` si la
 * génération courante résout tout de même) par tout changement de champion ou tout champion non
 * résolvant — jamais laissé progresser à tort.
 * \castest{<b>updateConsecutiveStableWins : réinitialisation du compteur.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Enchaîner des mises à jour avec rupture de champion ou de résolution.<br/>
 * \tattendu Le compteur retombe à `0` ou `1` selon que la génération courante résout ou non.}
 */
TEST(LevelTrainingSessionTest, ReinitialisationDuCompteurDeSuccesConsecutifs) {
    // Trois succès consecutifs du meme champion : le compteur progresse.
    int counter = 0;
    counter = updateConsecutiveStableWins(counter, /*sameChampionAsBefore=*/false,
                                          /*resolvingNow=*/true);
    EXPECT_EQ(counter, 1);
    counter = updateConsecutiveStableWins(counter, true, true);
    EXPECT_EQ(counter, 2);
    counter = updateConsecutiveStableWins(counter, true, true);
    EXPECT_EQ(counter, 3);

    // Un nouveau champion qui resout aussi : rupture, redemarre a 1 (pas de progression a tort).
    counter = updateConsecutiveStableWins(counter, /*sameChampionAsBefore=*/false,
                                          /*resolvingNow=*/true);
    EXPECT_EQ(counter, 1);

    // Le meme champion, mais qui ne resout plus (ne devrait jamais arriver en pratique puisque
    // deterministe, mais la fonction doit rester correcte) : remis a zero.
    counter = updateConsecutiveStableWins(counter, /*sameChampionAsBefore=*/true,
                                          /*resolvingNow=*/false);
    EXPECT_EQ(counter, 0);
}

/**
 * @brief `LevelTrainingSession` ne charge et ne joue jamais qu'un unique niveau du début à la fin
 * de la session (pas de progression automatique) : le niveau chargé reste identique après `run()`.
 * \castest{<b>LevelTrainingSession : un seul niveau pour toute la session.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Session sur le niveau trivial.<br/>2. `run()`.<br/>
 * \tattendu Le nom du niveau de l'environnement de la session reste celui du fichier fourni à la
 * construction.}
 */
TEST(LevelTrainingSessionTest, UnSeulNiveauPourTouteLaSession) {
    const TrivialLevelDirectory level("unique-niveau");
    const ObservationEncoder encoder;

    EvolutionaryConfig config;
    config.populationSize = 8;
    StoppingConfig stopping;
    stopping.requiredConsecutiveSuccesses = 1000;
    stopping.maxGenerations = 3;

    LevelTrainingSession session(level.levelPath(), policyTopology(encoder.inputSize()), config,
                                 stopping, 1, level.file("stats.csv"),
                                 EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    const TrainingResult result = session.run();
    static_cast<void>(result);

    ASSERT_TRUE(session.environment().loaded());
    EXPECT_EQ(session.environment().level().name(), "TrivialAI");
}

/**
 * @brief `shouldStop` renvoyant `true` dès le premier appel interrompt la session après une seule
 * génération, bien avant le plafond (`LOT-ANNEXE-21`).
 * \castest{<b>LevelTrainingSession : `shouldStop` interrompt avant le plafond.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Session avec un plafond large (200) et un critère de résolution jamais atteignable
 * (`requiredConsecutiveSuccesses` très élevé).<br/>2. `run(shouldStop)` avec un `shouldStop` qui
 * renvoie `true` dès le premier appel.<br/>
 * \tattendu `TrainingResult::solved == false`, `generationsRun` égal à `0` (interrompue avant la
 * première génération).}
 */
TEST(LevelTrainingSessionTest, ShouldStopInterromptAvantLePlafond) {
    const TrivialLevelDirectory level("shouldstop");
    const ObservationEncoder encoder;

    EvolutionaryConfig config;
    config.populationSize = 8;
    StoppingConfig stopping;
    stopping.requiredConsecutiveSuccesses = 1000;
    stopping.maxGenerations = 200;

    LevelTrainingSession session(level.levelPath(), policyTopology(encoder.inputSize()), config,
                                 stopping, 7, level.file("stats.csv"),
                                 EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    const TrainingResult result = session.run([] { return true; });

    EXPECT_FALSE(result.solved);
    EXPECT_EQ(result.generationsRun, 0u);
}

/**
 * @brief `onGenerationChampion` reçoit, à chaque génération, le champion réellement retenu par le
 * trainer interne (`LOT-ANNEXE-21`) : seul point d'accès externe à son réseau pendant la session.
 * \castest{<b>LevelTrainingSession : `onGenerationChampion` reçoit le vrai champion.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Session sur le niveau trivial, plafond de 3 générations.<br/>2. `run` avec un
 * `onGenerationChampion` qui compte ses appels.<br/>
 * \tattendu Le callback est appelé une fois par génération réellement exécutée (au moins une,
 * au plus le plafond).}
 */
TEST(LevelTrainingSessionTest, OnGenerationChampionRecoitLeVraiChampion) {
    const TrivialLevelDirectory level("ongenerationchampion");
    const ObservationEncoder encoder;

    EvolutionaryConfig config;
    config.populationSize = 8;
    StoppingConfig stopping;
    stopping.requiredConsecutiveSuccesses = 1000;
    stopping.maxGenerations = 3;

    LevelTrainingSession session(level.levelPath(), policyTopology(encoder.inputSize()), config,
                                 stopping, 3, level.file("stats.csv"),
                                 EnvironmentConfig{.maxSteps = kReducedMaxSteps});
    int callCount = 0;
    const TrainingResult result =
        session.run({}, [&callCount](const aisolver::training::evolutionary::Individual& champion) {
            ++callCount;
            EXPECT_TRUE(champion.network().parameters().size() > 0);
        });
    static_cast<void>(result);

    EXPECT_EQ(callCount, 3);
}
