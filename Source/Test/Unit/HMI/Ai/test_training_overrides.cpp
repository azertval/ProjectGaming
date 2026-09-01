// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_training_overrides.cpp
 * @brief Tests unitaires de la traduction requête d'écran vers surcharges `aisolver-cli`
 *        (`LOT-73`, `EX-IHM-083`).
 *
 * Ce fichier existe à cause d'un défaut précis : neuf réglages de l'onglet Entraînement étaient lus
 * par l'écran puis **jetés**, faute de champ où atterrir. Les régler ne changeait rien, et le
 * `config.json` du run décrivait pourtant un run qui n'avait pas tourné — de sorte que « Reprendre
 * les réglages de ce run » rechargeait des valeurs fausses. Rien n'échouait : ni la compilation, ni
 * un test, ni l'exécution.
 *
 * Le premier test ci-dessous est écrit pour échouer dans ce cas-là.
 */

#include <gtest/gtest.h>

#include "HMI/Ai/TrainingOverrides.h"

namespace {

/// Requête dont **chaque** champ facultatif porte une valeur distincte et reconnaissable : un champ
/// oublié par la traduction se voit alors comme un `std::nullopt`, jamais comme une coïncidence de
/// valeurs.
[[nodiscard]] hmi::TrainingRequest fullyPopulatedRequest() {
    hmi::TrainingRequest request;
    request.levelPath = "demo.json";
    request.algorithmId = "ac";
    request.seed = 42U;
    request.populationSize = std::size_t{11};
    request.mutationRate = 0.11F;
    request.episodes = std::size_t{12};
    request.learningRate = 0.12F;
    request.criticLearningRate = 0.13F;
    request.gamma = 0.14F;
    request.hiddenSize = std::size_t{15};
    request.tournamentSize = 16;
    request.mutationStrength = 0.17F;
    request.maxGenerations = 18;
    request.requiredConsecutiveSuccesses = 19;
    request.dqnReplayCapacity = std::size_t{20};
    request.dqnBatchSize = std::size_t{21};
    request.dqnWarmupSize = std::size_t{22};
    request.dqnUpdatePeriodSteps = std::size_t{23};
    request.dqnTargetSyncPeriodSteps = std::size_t{24};
    request.dqnEpsilonStart = 0.25F;
    request.dqnEpsilonEnd = 0.26F;
    request.dqnEpsilonDecaySteps = std::size_t{27};
    request.batchEpisodes = std::size_t{28};
    request.entropyCoefficient = 0.29F;
    request.gradientClipNorm = 0.30F;
    request.actionRepeat = 31;
    request.explorationFloor = 0.32F;
    request.crossoverRate = 0.33F;
    request.maxSteps = 34;
    request.stuckThreshold = 35;
    return request;
}

}  // namespace

/**
 * @brief Chaque réglage porté par la requête atteint les surcharges. C'est le test qui aurait
 *        signalé les neuf réglages inertes : ils étaient lus, puis perdus entre l'écran et le
 *        moteur, sans que rien n'échoue.
 * \castest{<b>Chaque reglage de la requete atteint les surcharges de ligne de commande.</b><br/>
 * \tcat Unitaire · Mode IA<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire une requete dont chaque champ facultatif porte une valeur distincte.<br/>
 * 2. Traduire en surcharges de ligne de commande.<br/>
 * \tattendu Chaque champ est renseigne et porte la valeur de la requete.
 * }
 */
TEST(TrainingOverridesTest, ChaqueReglageAtteintLesSurcharges) {
    const aisolver::cli::CommandLineOverrides overrides =
        hmi::overridesFor(fullyPopulatedRequest());

    ASSERT_TRUE(overrides.populationSize.has_value());
    EXPECT_EQ(*overrides.populationSize, std::size_t{11});
    ASSERT_TRUE(overrides.mutationRate.has_value());
    EXPECT_FLOAT_EQ(*overrides.mutationRate, 0.11F);
    ASSERT_TRUE(overrides.episodes.has_value());
    EXPECT_EQ(*overrides.episodes, std::size_t{12});
    ASSERT_TRUE(overrides.learningRate.has_value());
    EXPECT_FLOAT_EQ(*overrides.learningRate, 0.12F);
    ASSERT_TRUE(overrides.criticLearningRate.has_value());
    EXPECT_FLOAT_EQ(*overrides.criticLearningRate, 0.13F);
    ASSERT_TRUE(overrides.gamma.has_value());
    EXPECT_FLOAT_EQ(*overrides.gamma, 0.14F);
    ASSERT_TRUE(overrides.hiddenSize.has_value());
    EXPECT_EQ(*overrides.hiddenSize, std::size_t{15});
    ASSERT_TRUE(overrides.tournamentSize.has_value());
    EXPECT_EQ(*overrides.tournamentSize, 16);
    ASSERT_TRUE(overrides.mutationStrength.has_value());
    EXPECT_FLOAT_EQ(*overrides.mutationStrength, 0.17F);
    ASSERT_TRUE(overrides.maxGenerations.has_value());
    EXPECT_EQ(*overrides.maxGenerations, 18);
    ASSERT_TRUE(overrides.requiredConsecutiveSuccesses.has_value());
    EXPECT_EQ(*overrides.requiredConsecutiveSuccesses, 19);
    ASSERT_TRUE(overrides.dqnReplayCapacity.has_value());
    EXPECT_EQ(*overrides.dqnReplayCapacity, std::size_t{20});
    ASSERT_TRUE(overrides.dqnBatchSize.has_value());
    EXPECT_EQ(*overrides.dqnBatchSize, std::size_t{21});
    ASSERT_TRUE(overrides.dqnWarmupSize.has_value());
    EXPECT_EQ(*overrides.dqnWarmupSize, std::size_t{22});
    ASSERT_TRUE(overrides.dqnUpdatePeriodSteps.has_value());
    EXPECT_EQ(*overrides.dqnUpdatePeriodSteps, std::size_t{23});
    ASSERT_TRUE(overrides.dqnTargetSyncPeriodSteps.has_value());
    EXPECT_EQ(*overrides.dqnTargetSyncPeriodSteps, std::size_t{24});
    ASSERT_TRUE(overrides.dqnEpsilonStart.has_value());
    EXPECT_FLOAT_EQ(*overrides.dqnEpsilonStart, 0.25F);
    ASSERT_TRUE(overrides.dqnEpsilonEnd.has_value());
    EXPECT_FLOAT_EQ(*overrides.dqnEpsilonEnd, 0.26F);
    ASSERT_TRUE(overrides.dqnEpsilonDecaySteps.has_value());
    EXPECT_EQ(*overrides.dqnEpsilonDecaySteps, std::size_t{27});

    // Les neuf reglages qui se perdaient. Les assertions ci-dessous sont la raison d'etre de ce
    // fichier : avant le LOT-73, toutes echouaient sur un std::nullopt.
    ASSERT_TRUE(overrides.batchEpisodes.has_value()) << "batchEpisodes n'atteint pas le moteur";
    EXPECT_EQ(*overrides.batchEpisodes, std::size_t{28});
    ASSERT_TRUE(overrides.entropyCoefficient.has_value())
        << "entropyCoefficient n'atteint pas le moteur";
    EXPECT_FLOAT_EQ(*overrides.entropyCoefficient, 0.29F);
    ASSERT_TRUE(overrides.gradientClipNorm.has_value())
        << "gradientClipNorm n'atteint pas le moteur";
    EXPECT_FLOAT_EQ(*overrides.gradientClipNorm, 0.30F);
    ASSERT_TRUE(overrides.actionRepeat.has_value()) << "actionRepeat n'atteint pas le moteur";
    EXPECT_EQ(*overrides.actionRepeat, 31);
    ASSERT_TRUE(overrides.explorationFloor.has_value())
        << "explorationFloor n'atteint pas le moteur";
    EXPECT_FLOAT_EQ(*overrides.explorationFloor, 0.32F);
    ASSERT_TRUE(overrides.crossoverRate.has_value()) << "crossoverRate n'atteint pas le moteur";
    EXPECT_FLOAT_EQ(*overrides.crossoverRate, 0.33F);
    ASSERT_TRUE(overrides.maxSteps.has_value()) << "maxSteps n'atteint pas le moteur";
    EXPECT_EQ(*overrides.maxSteps, 34);
    ASSERT_TRUE(overrides.stuckThreshold.has_value()) << "stuckThreshold n'atteint pas le moteur";
    EXPECT_EQ(*overrides.stuckThreshold, 35);
}

/**
 * @brief Les surcharges se propagent réellement jusqu'à la configuration résolue : la traduction
 *        seule ne prouverait rien si `loadTrainingConfig` n'en tenait pas compte. Ce test relie les
 *        deux bouts, c'est-à-dire ce que l'utilisateur constate — le run tourne avec ce qu'il a
 *        réglé.
 * \castest{<b>Les reglages de la requete se retrouvent dans la configuration resolue.</b><br/>
 * \tcat Unitaire · Mode IA<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Traduire une requete completement renseignee.<br/>2. Resoudre la configuration sans
 * fichier, avec ces seules surcharges.<br/>
 * \tattendu La configuration porte les valeurs reglees, y compris les neuf communes.
 * }
 */
TEST(TrainingOverridesTest, LesReglagesSeRetrouventDansLaConfigurationResolue) {
    const aisolver::cli::TrainingConfig config =
        aisolver::cli::loadTrainingConfig(std::nullopt, hmi::overridesFor(fullyPopulatedRequest()));

    EXPECT_FLOAT_EQ(config.criticLearningRate, 0.13F);
    EXPECT_FLOAT_EQ(config.evolutionary.crossoverRate, 0.33F);
    EXPECT_EQ(config.tuning.batchEpisodes, std::size_t{28});
    EXPECT_FLOAT_EQ(config.tuning.entropyCoefficient, 0.29F);
    EXPECT_FLOAT_EQ(config.tuning.explorationFloor, 0.32F);
    EXPECT_FLOAT_EQ(config.tuning.gradientClipNorm, 0.30F);
    EXPECT_EQ(config.tuning.actionRepeat, 31);
    EXPECT_EQ(config.maxSteps, 34);
    EXPECT_EQ(config.stuckThreshold, 35);
}

/**
 * @brief Une requête vierge ne pose **aucune** surcharge : l'écran qui ne touche à rien doit
 *        décrire le même run que `aisolver-cli train` sans option — corollaire de cadrage du
 *        `LOT-ANNEXE-22`, que ce test empêche de dériver.
 * \castest{<b>Une requete vierge ne pose aucune surcharge.</b><br/>
 * \tcat Unitaire · Mode IA<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Traduire une requete par defaut.<br/>2. Comparer la configuration resolue aux
 * defauts du moteur.<br/>
 * \tattendu Aucune valeur ne s'ecarte des defauts de TrainingConfig.
 * }
 */
TEST(TrainingOverridesTest, UneRequeteViergeNePoseAucuneSurcharge) {
    const aisolver::cli::CommandLineOverrides overrides = hmi::overridesFor(hmi::TrainingRequest{});
    EXPECT_FALSE(overrides.populationSize.has_value());
    EXPECT_FALSE(overrides.criticLearningRate.has_value());
    EXPECT_FALSE(overrides.batchEpisodes.has_value());
    EXPECT_FALSE(overrides.maxSteps.has_value());
    EXPECT_FALSE(overrides.stuckThreshold.has_value());

    const aisolver::cli::TrainingConfig resolved =
        aisolver::cli::loadTrainingConfig(std::nullopt, overrides);
    const aisolver::cli::TrainingConfig defaults;
    EXPECT_FLOAT_EQ(resolved.learningRate, defaults.learningRate);
    EXPECT_FLOAT_EQ(resolved.gamma, defaults.gamma);
    EXPECT_EQ(resolved.optimizer, defaults.optimizer);
    EXPECT_EQ(resolved.hiddenSize, defaults.hiddenSize);
}
