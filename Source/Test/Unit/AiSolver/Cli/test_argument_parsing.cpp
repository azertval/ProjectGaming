// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_argument_parsing.cpp
 * @brief Tests unitaires de l'analyse d'arguments de `aisolver-cli` (LOT-ANNEXE-19, TACHE-01,
 * `EX-IA-020`).
 */

#include <gtest/gtest.h>

#include "AiSolver/Cli/ArgumentParsing.h"
#include "AiSolver/Cli/Commands.h"

using aisolver::cli::EvaluateArgs;
using aisolver::cli::ExportReplayArgs;
using aisolver::cli::findOption;
using aisolver::cli::hasFlag;
using aisolver::cli::parseEvaluateArgs;
using aisolver::cli::parseExportReplayArgs;
using aisolver::cli::parseTrainArgs;
using aisolver::cli::TrainArgs;

/**
 * @brief `findOption` retrouve la valeur associee a une option connue.
 * \castest{Option presente -> valeur retournee.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Liste d'arguments contenant `--level demo.json --seed 42`.<br/>2.
 * `findOption` pour chaque nom.<br/>
 * \tattendu La valeur associee a chaque option est retournee.}
 */
TEST(FindOptionTest, RetourneLaValeurQuandLOptionEstPresente) {
    const std::vector<std::string> args{"--level", "demo.json", "--seed", "42"};
    EXPECT_EQ(findOption(args, "--level"), "demo.json");
    EXPECT_EQ(findOption(args, "--seed"), "42");
}

/**
 * @brief `findOption` renvoie `std::nullopt` quand l'option est absente ou sans valeur.
 * \castest{Option absente ou sans valeur -> nullopt.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Liste d'arguments `--level` (sans valeur associee).<br/>2. `findOption`
 * pour une option absente et pour `--level`.<br/>
 * \tattendu `std::nullopt` dans les deux cas.}
 */
TEST(FindOptionTest, RetourneNulloptSiAbsenteOuSansValeur) {
    const std::vector<std::string> args{"--level"};
    EXPECT_FALSE(findOption(args, "--algo").has_value());
    EXPECT_FALSE(findOption(args, "--level").has_value());  // Valeur manquante en fin de liste.
}

/**
 * @brief `hasFlag` detecte la presence d'un drapeau sans valeur associee.
 * \castest{Drapeau present/absent -> detection correcte.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Liste d'arguments `--verbose`.<br/>2. `hasFlag` pour `--verbose` et
 * `--quiet`.<br/>
 * \tattendu `true` pour `--verbose`, `false` pour `--quiet`.}
 */
TEST(HasFlagTest, DetecteUnDrapeauSansValeur) {
    const std::vector<std::string> args{"--verbose"};
    EXPECT_TRUE(hasFlag(args, "--verbose"));
    EXPECT_FALSE(hasFlag(args, "--quiet"));
}

/**
 * @brief `parseTrainArgs` rejette un appel sans `--level`.
 * \castest{`--level` manquant -> rejet.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments `--algo evo` (sans `--level`).<br/>2. `parseTrainArgs`.<br/>
 * \tattendu `std::nullopt` et message d'erreur non vide.}
 */
TEST(ParseTrainArgsTest, RejetteUnNiveauManquant) {
    std::string error;
    const std::optional<TrainArgs> parsed = parseTrainArgs({"--algo", "evo"}, error);
    EXPECT_FALSE(parsed.has_value());
    EXPECT_FALSE(error.empty());
}

/**
 * @brief `parseTrainArgs` rejette un appel sans `--algo`.
 * \castest{`--algo` manquant -> rejet.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments `--level demo.json` (sans `--algo`).<br/>2. `parseTrainArgs`.<br/>
 * \tattendu `std::nullopt` et message d'erreur non vide.}
 */
TEST(ParseTrainArgsTest, RejetteUnAlgorithmeManquant) {
    std::string error;
    const std::optional<TrainArgs> parsed = parseTrainArgs({"--level", "demo.json"}, error);
    EXPECT_FALSE(parsed.has_value());
    EXPECT_FALSE(error.empty());
}

/**
 * @brief `parseTrainArgs` rejette une valeur `--algo` inconnue.
 * \castest{`--algo` invalide -> rejet.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments `--level demo.json --algo ppo` (`ppo` inconnu).<br/>2.
 * `parseTrainArgs`.<br/>
 * \tattendu `std::nullopt` et message d'erreur non vide.}
 */
TEST(ParseTrainArgsTest, RejetteUnAlgorithmeInvalide) {
    std::string error;
    const std::optional<TrainArgs> parsed =
        parseTrainArgs({"--level", "demo.json", "--algo", "ppo"}, error);
    EXPECT_FALSE(parsed.has_value());
    EXPECT_FALSE(error.empty());
}

/**
 * @brief `parseTrainArgs` accepte chacune des quatre valeurs connues de `--algo`.
 * \castest{Quatre algorithmes connus -> acceptation.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Arguments `--level demo.json --algo <evo|pg|ac|avance>` pour chaque
 * valeur.<br/>2. `parseTrainArgs`.<br/>
 * \tattendu Analyse reussie, `algorithmId` egal a la valeur fournie, pour les quatre cas.}
 */
TEST(ParseTrainArgsTest, AccepteLesQuatreAlgorithmesConnus) {
    for (const char* algorithmId : {"evo", "pg", "ac", "avance"}) {
        std::string error;
        const std::optional<TrainArgs> parsed =
            parseTrainArgs({"--level", "demo.json", "--algo", algorithmId}, error);
        ASSERT_TRUE(parsed.has_value()) << algorithmId;
        EXPECT_EQ(parsed->algorithmId, algorithmId);
    }
}

/**
 * @brief `parseTrainArgs` analyse toutes les surcharges d'hyperparametres individuelles.
 * \castest{Surcharges individuelles -> valeurs analysees.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments avec `--seed`/`--population-size`/`--mutation-rate`/`--episodes`/
 * `--learning-rate`/`--gamma`/`--optimizer`.<br/>2. `parseTrainArgs`.<br/>
 * \tattendu Chaque champ optionnel correspondant est rempli avec la valeur fournie.}
 */
TEST(ParseTrainArgsTest, AnalyseLesSurchargesIndividuelles) {
    std::string error;
    const std::optional<TrainArgs> parsed =
        parseTrainArgs({"--level", "demo.json", "--algo", "evo", "--seed", "7", "--population-size",
                        "8", "--mutation-rate", "0.2", "--episodes", "50", "--learning-rate",
                        "0.01", "--gamma", "0.95", "--optimizer", "adam"},
                       error);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->seed, 7u);
    ASSERT_TRUE(parsed->populationSize.has_value());
    EXPECT_EQ(*parsed->populationSize, 8u);
    ASSERT_TRUE(parsed->mutationRate.has_value());
    EXPECT_FLOAT_EQ(*parsed->mutationRate, 0.2f);
    ASSERT_TRUE(parsed->episodes.has_value());
    EXPECT_EQ(*parsed->episodes, 50u);
    ASSERT_TRUE(parsed->optimizer.has_value());
    EXPECT_EQ(*parsed->optimizer, "adam");
}

/**
 * @brief `parseEvaluateArgs` rejette un appel manquant `--model`, `--algo` ou `--level`.
 * \castest{Argument requis manquant -> rejet.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Trois appels, chacun omettant respectivement `--model`, `--algo` ou
 * `--level`.<br/>2. `parseEvaluateArgs`.<br/>
 * \tattendu `std::nullopt` dans les trois cas.}
 */
TEST(ParseEvaluateArgsTest, RejetteUnModeleUnAlgorithmeOuUnNiveauManquant) {
    std::string error;
    EXPECT_FALSE(parseEvaluateArgs({"--algo", "evo", "--level", "l.json"}, error).has_value());
    EXPECT_FALSE(parseEvaluateArgs({"--model", "m.bin", "--level", "l.json"}, error).has_value());
    EXPECT_FALSE(parseEvaluateArgs({"--model", "m.bin", "--algo", "evo"}, error).has_value());
}

/**
 * @brief `parseEvaluateArgs` analyse un appel complet avec `--repetitions` et `--report`.
 * \castest{Appel complet avec options -> analyse correcte.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments `--model m.bin --algo pg --level l.json --repetitions 10 --report
 * r.csv`.<br/>2. `parseEvaluateArgs`.<br/>
 * \tattendu `repetitions == 10`, `report == "r.csv"`.}
 */
TEST(ParseEvaluateArgsTest, AnalyseUnAppelValideAvecRapportOptionnel) {
    std::string error;
    const std::optional<EvaluateArgs> parsed =
        parseEvaluateArgs({"--model", "m.bin", "--algo", "pg", "--level", "l.json", "--repetitions",
                           "10", "--report", "r.csv"},
                          error);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->repetitions, 10);
    ASSERT_TRUE(parsed->report.has_value());
    EXPECT_EQ(*parsed->report, std::filesystem::path("r.csv"));
}

/**
 * @brief `parseExportReplayArgs` rejette un appel sans `--output`.
 * \castest{`--output` manquant -> rejet.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments `--model m.bin --algo evo --level l.json` (sans `--output`).<br/>2.
 * `parseExportReplayArgs`.<br/>
 * \tattendu `std::nullopt` et message d'erreur non vide.}
 */
TEST(ParseExportReplayArgsTest, RejetteUneSortieManquante) {
    std::string error;
    const std::optional<ExportReplayArgs> parsed =
        parseExportReplayArgs({"--model", "m.bin", "--algo", "evo", "--level", "l.json"}, error);
    EXPECT_FALSE(parsed.has_value());
    EXPECT_FALSE(error.empty());
}

/**
 * @brief `parseExportReplayArgs` analyse un appel complet avec `--seed`.
 * \castest{Appel complet -> analyse correcte.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments `--model m.bin --algo avance --level l.json --output o.json --seed
 * 3`.<br/>2. `parseExportReplayArgs`.<br/>
 * \tattendu `algorithmId == "avance"`, `seed == 3`.}
 */
TEST(ParseExportReplayArgsTest, AnalyseUnAppelValide) {
    std::string error;
    const std::optional<ExportReplayArgs> parsed =
        parseExportReplayArgs({"--model", "m.bin", "--algo", "avance", "--level", "l.json",
                               "--output", "o.json", "--seed", "3"},
                              error);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->algorithmId, "avance");
    EXPECT_EQ(parsed->seed, 3u);
}

/**
 * @brief Une valeur numerique invalide produit un message d'usage, jamais un plantage.
 * \castest{Argument numerique invalide -> refus explicite, aucune exception.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `parseTrainArgs` avec `--seed abc`.<br/>2. Recommencer avec `--seed 12abc`, qui
 * commence par un nombre sans en etre un.<br/>
 * \tattendu Les deux appels renvoient `std::nullopt` avec un message citant l'option et la valeur
 * recue — `std::stoull` levait une exception non rattrapee, qui terminait le processus.}
 */
TEST(ParseTrainArgsTest, ValeurNumeriqueInvalideRefuseeSansLever) {
    std::string error;
    const std::optional<TrainArgs> lettres =
        parseTrainArgs({"--level", "n.json", "--algo", "evo", "--seed", "abc"}, error);
    EXPECT_FALSE(lettres.has_value());
    EXPECT_NE(error.find("--seed"), std::string::npos) << error;
    EXPECT_NE(error.find("abc"), std::string::npos) << error;

    error.clear();
    const std::optional<TrainArgs> partiel =
        parseTrainArgs({"--level", "n.json", "--algo", "evo", "--seed", "12abc"}, error);
    EXPECT_FALSE(partiel.has_value()) << "une valeur partiellement numerique doit etre refusee";
    EXPECT_FALSE(error.empty());
}

/**
 * @brief `parseTrainArgs` lit les hyperparametres evolutionnistes et de topologie ajoutes par
 * LOT-ANNEXE-22, jusqu'ici accessibles par le seul fichier `--config`.
 * \castest{Drapeaux `train` de topologie et de critere d'arret.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Arguments `--level`/`--algo evo` plus `--hidden-size`, `--tournament-size`,
 * `--mutation-strength`, `--max-generations` et `--required-successes`.<br/>2.
 * `parseTrainArgs`.<br/>
 * \tattendu Chaque valeur est portee par le champ correspondant de `TrainArgs`, sans quoi le
 * plafond de generations resterait au defaut quelle que soit la demande.}
 */
TEST(ParseTrainArgsTest, LitLesHyperparametresEvolutionnistesEtLaTopologie) {
    std::string error;
    const std::optional<TrainArgs> parsed = parseTrainArgs(
        {"--level", "demo.json", "--algo", "evo", "--hidden-size", "48", "--tournament-size", "7",
         "--mutation-strength", "0.25", "--max-generations", "20", "--required-successes", "3"},
        error);

    ASSERT_TRUE(parsed.has_value()) << error;
    EXPECT_EQ(parsed->hiddenSize, 48u);
    EXPECT_EQ(parsed->tournamentSize, 7);
    ASSERT_TRUE(parsed->mutationStrength.has_value());
    EXPECT_FLOAT_EQ(*parsed->mutationStrength, 0.25f);
    EXPECT_EQ(parsed->maxGenerations, 20);
    EXPECT_EQ(parsed->requiredConsecutiveSuccesses, 3);
}

/**
 * @brief `parseTrainArgs` lit les huit hyperparametres DQN, que `CommandLineOverrides` declarait
 * sans qu'aucun drapeau ne les alimente.
 * \castest{Les huit drapeaux `--dqn-*` de `train`.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments `--algo avance` avec les huit drapeaux `--dqn-*`.<br/>2.
 * `parseTrainArgs`.<br/>
 * \tattendu Les huit champs sont renseignes : l'ecran Mode IA les exposait deja, la ligne de
 * commande ne les atteignait que par fichier de configuration.}
 */
TEST(ParseTrainArgsTest, LitLesHuitHyperparametresDqn) {
    std::string error;
    const std::optional<TrainArgs> parsed = parseTrainArgs({"--level",
                                                            "demo.json",
                                                            "--algo",
                                                            "avance",
                                                            "--dqn-replay-capacity",
                                                            "500",
                                                            "--dqn-batch-size",
                                                            "16",
                                                            "--dqn-warmup-size",
                                                            "64",
                                                            "--dqn-update-period",
                                                            "4",
                                                            "--dqn-target-sync-period",
                                                            "100",
                                                            "--dqn-epsilon-start",
                                                            "0.8",
                                                            "--dqn-epsilon-end",
                                                            "0.02",
                                                            "--dqn-epsilon-decay",
                                                            "3000"},
                                                           error);

    ASSERT_TRUE(parsed.has_value()) << error;
    EXPECT_EQ(parsed->dqnReplayCapacity, 500u);
    EXPECT_EQ(parsed->dqnBatchSize, 16u);
    EXPECT_EQ(parsed->dqnWarmupSize, 64u);
    EXPECT_EQ(parsed->dqnUpdatePeriodSteps, 4u);
    EXPECT_EQ(parsed->dqnTargetSyncPeriodSteps, 100u);
    ASSERT_TRUE(parsed->dqnEpsilonStart.has_value());
    EXPECT_FLOAT_EQ(*parsed->dqnEpsilonStart, 0.8f);
    ASSERT_TRUE(parsed->dqnEpsilonEnd.has_value());
    EXPECT_FLOAT_EQ(*parsed->dqnEpsilonEnd, 0.02f);
    EXPECT_EQ(parsed->dqnEpsilonDecaySteps, 3000u);
}

/**
 * @brief Un nouveau drapeau mal forme produit le message d'usage de la sous-commande, jamais une
 * exception ni une valeur tronquee.
 * \castest{Valeur non numerique sur un nouveau drapeau de `train`.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Arguments valides plus `--max-generations abc`.<br/>2. `parseTrainArgs`.<br/>
 * \tattendu Aucun resultat, et un message d'erreur nommant le drapeau fautif et la valeur recue
 * (`EX-NFR-040`).}
 */
TEST(ParseTrainArgsTest, RejetteUneValeurNonNumeriqueSurUnNouveauDrapeau) {
    std::string error;
    const std::optional<TrainArgs> parsed = parseTrainArgs(
        {"--level", "demo.json", "--algo", "evo", "--max-generations", "abc"}, error);

    EXPECT_FALSE(parsed.has_value());
    EXPECT_NE(error.find("--max-generations"), std::string::npos);
    EXPECT_NE(error.find("abc"), std::string::npos);
}

/**
 * @brief En l'absence des nouveaux drapeaux, aucune surcharge n'est posee : les defauts documentes
 * restent en place.
 * \castest{Nouveaux drapeaux absents -> aucune surcharge.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments reduits a `--level` et `--algo`.<br/>2. `parseTrainArgs`.<br/>
 * \tattendu Tous les champs optionnels ajoutes restent vides, sans quoi un run sans option
 * n'utiliserait plus les defauts documentes.}
 */
TEST(ParseTrainArgsTest, NouveauxDrapeauxAbsentsNePosentAucuneSurcharge) {
    std::string error;
    const std::optional<TrainArgs> parsed =
        parseTrainArgs({"--level", "demo.json", "--algo", "evo"}, error);

    ASSERT_TRUE(parsed.has_value()) << error;
    EXPECT_FALSE(parsed->hiddenSize.has_value());
    EXPECT_FALSE(parsed->tournamentSize.has_value());
    EXPECT_FALSE(parsed->mutationStrength.has_value());
    EXPECT_FALSE(parsed->maxGenerations.has_value());
    EXPECT_FALSE(parsed->requiredConsecutiveSuccesses.has_value());
    EXPECT_FALSE(parsed->dqnReplayCapacity.has_value());
}

/**
 * @brief `parseEvaluateArgs` couvre le reste de `BenchmarkConfig` : budget de pas, graine et mode
 * de decodage.
 * \castest{Drapeaux `--max-steps`, `--seed` et `--decoding` de `evaluate`.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arguments `evaluate` complets avec `--max-steps`, `--seed` et `--decoding
 * stochastic`.<br/>2. `parseEvaluateArgs`.<br/>3. Recommencer avec `--decoding inconnu`.<br/>
 * \tattendu Les trois valeurs sont lues ; un mode de decodage inconnu est refuse avec un message
 * nommant les valeurs attendues.}
 */
TEST(ParseEvaluateArgsTest, LitBudgetDePasGraineEtModeDeDecodage) {
    std::string error;
    const std::optional<EvaluateArgs> parsed =
        parseEvaluateArgs({"--model", "m.bin", "--algo", "evo", "--level", "demo.json",
                           "--max-steps", "750", "--seed", "11", "--decoding", "stochastic"},
                          error);

    ASSERT_TRUE(parsed.has_value()) << error;
    EXPECT_EQ(parsed->maxStepsPerEpisode, 750);
    EXPECT_EQ(parsed->seed, 11u);
    EXPECT_TRUE(parsed->stochasticDecoding);

    const std::optional<EvaluateArgs> rejected = parseEvaluateArgs(
        {"--model", "m.bin", "--algo", "evo", "--level", "demo.json", "--decoding", "inconnu"},
        error);
    EXPECT_FALSE(rejected.has_value());
    EXPECT_NE(error.find("argmax"), std::string::npos);
}
