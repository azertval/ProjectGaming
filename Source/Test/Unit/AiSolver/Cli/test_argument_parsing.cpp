// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_argument_parsing.cpp
 * @brief Tests unitaires de l'analyse d'arguments de `aisolver-cli` (LOT-ANNEXE-19, TACHE-01,
 * `EX-IA-020`).
 */

#include <gtest/gtest.h>

#include "AiSolver/Cli/ArgParsing.h"
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
 * \tattendu Analyse reussie, `algo` egal a la valeur fournie, pour les quatre cas.}
 */
TEST(ParseTrainArgsTest, AccepteLesQuatreAlgorithmesConnus) {
    for (const char* algo : {"evo", "pg", "ac", "avance"}) {
        std::string error;
        const std::optional<TrainArgs> parsed =
            parseTrainArgs({"--level", "demo.json", "--algo", algo}, error);
        ASSERT_TRUE(parsed.has_value()) << algo;
        EXPECT_EQ(parsed->algo, algo);
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
        parseTrainArgs({"--level", "demo.json", "--algo", "evo", "--seed", "7",
                        "--population-size", "8", "--mutation-rate", "0.2", "--episodes", "50",
                        "--learning-rate", "0.01", "--gamma", "0.95", "--optimizer", "adam"},
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
    const std::optional<EvaluateArgs> parsed = parseEvaluateArgs(
        {"--model", "m.bin", "--algo", "pg", "--level", "l.json", "--repetitions", "10",
         "--report", "r.csv"},
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
    const std::optional<ExportReplayArgs> parsed = parseExportReplayArgs(
        {"--model", "m.bin", "--algo", "evo", "--level", "l.json"}, error);
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
 * \tattendu `algo == "avance"`, `seed == 3`.}
 */
TEST(ParseExportReplayArgsTest, AnalyseUnAppelValide) {
    std::string error;
    const std::optional<ExportReplayArgs> parsed =
        parseExportReplayArgs({"--model", "m.bin", "--algo", "avance", "--level", "l.json",
                               "--output", "o.json", "--seed", "3"},
                              error);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->algo, "avance");
    EXPECT_EQ(parsed->seed, 3u);
}
