// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_training_config.cpp
 * @brief Tests unitaires de la résolution/traçabilité de configuration `aisolver-cli`
 * (LOT-ANNEXE-19, TACHE-02, `EX-IA-020`).
 */

#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "AiSolver/Cli/TrainingConfig.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

using aisolver::cli::CommandLineOverrides;
using aisolver::cli::loadTrainingConfig;
using aisolver::cli::TrainingConfig;
using aisolver::cli::writeTrainingConfigJson;

namespace {

std::filesystem::path scratchFile(const char* suffix) {
    return std::filesystem::temp_directory_path() /
           (std::string("aisolver_test_training_config_") + suffix);
}

}  // namespace

/**
 * @brief Sans fichier ni surcharge, `loadTrainingConfig` produit les valeurs par defaut
 * documentees.
 * \castest{Aucune source -> defauts.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. `loadTrainingConfig(std::nullopt, CommandLineOverrides{})`.<br/>
 * \tattendu Chaque champ vaut sa valeur par defaut documentee (population, gamma,
 * optimiseur, episodes).}
 */
TEST(LoadTrainingConfigTest, SansFichierNiSurchargeProduitLesDefauts) {
    const TrainingConfig config = loadTrainingConfig(std::nullopt, CommandLineOverrides{});
    EXPECT_EQ(config.evolutionary.populationSize,
              aisolver::training::evolutionary::DEFAULT_POPULATION_SIZE);
    EXPECT_FLOAT_EQ(config.gamma, 0.99f);
    EXPECT_EQ(config.optimizer, "sgd");
    EXPECT_EQ(config.episodes, 300u);
}

/**
 * @brief Un chemin de fichier de configuration inexistant est ignore sans erreur.
 * \castest{Fichier de configuration absent -> ignore, defauts appliques.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `loadTrainingConfig` avec un chemin vers un fichier inexistant.<br/>
 * \tattendu Aucune exception, configuration egale aux defauts documentes.}
 */
TEST(LoadTrainingConfigTest, UnFichierAbsentEstIgnoreSansErreur) {
    const TrainingConfig config =
        loadTrainingConfig(scratchFile("fichier_inexistant.json"), CommandLineOverrides{});
    EXPECT_EQ(config.evolutionary.populationSize,
              aisolver::training::evolutionary::DEFAULT_POPULATION_SIZE);
}

/**
 * @brief Un fichier de configuration surcharge les champs qu'il precise, laisse les autres au
 * defaut.
 * \castest{Fichier partiel -> champs precises surcharges, autres au defaut.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Fichier JSON `{"populationSize": 16, "gamma": 0.9}`.<br/>2.
 * `loadTrainingConfig`.<br/>
 * \tattendu `populationSize == 16`, `gamma == 0.9`, `learningRate` reste au defaut
 * documente (champ absent du fichier).}
 */
TEST(LoadTrainingConfigTest, LeFichierSurchargeLesDefauts) {
    const std::filesystem::path path = scratchFile("fichier.json");
    {
        std::ofstream file(path);
        file << R"({"populationSize": 16, "gamma": 0.9})";
    }
    const TrainingConfig config = loadTrainingConfig(path, CommandLineOverrides{});
    EXPECT_EQ(config.evolutionary.populationSize, 16u);
    EXPECT_FLOAT_EQ(config.gamma, 0.9f);
    // Champ absent du fichier : reste au defaut documente, pas ecrase silencieusement.
    EXPECT_FLOAT_EQ(config.learningRate, 0.01f);
    std::filesystem::remove(path);
}

/**
 * @brief Une surcharge d'argument individuel prime sur la valeur du fichier de configuration.
 * \castest{Argument individuel vs fichier -> priorite a l'argument.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Fichier JSON `{"gamma": 0.9}`.<br/>2. `CommandLineOverrides` avec
 * `gamma = 0.5`.<br/>3. `loadTrainingConfig`.<br/>
 * \tattendu `gamma == 0.5` (argument > fichier > defaut).}
 */
TEST(LoadTrainingConfigTest, LArgumentIndividuelPrimeSurLeFichier) {
    const std::filesystem::path path = scratchFile("priorite.json");
    {
        std::ofstream file(path);
        file << R"({"gamma": 0.9})";
    }
    CommandLineOverrides overrides;
    overrides.gamma = 0.5f;
    const TrainingConfig config = loadTrainingConfig(path, overrides);
    EXPECT_FLOAT_EQ(config.gamma, 0.5f);  // Argument > fichier > defaut.
    std::filesystem::remove(path);
}

/**
 * @brief Une configuration ecrite par `writeTrainingConfigJson` se relit a l'identique.
 * \castest{Ecriture puis relecture -> configuration identique.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `TrainingConfig` avec des valeurs non-defaut.<br/>2.
 * `writeTrainingConfigJson`.<br/>3. `loadTrainingConfig` sur le fichier ecrit.<br/>
 * \tattendu Les champs relus correspondent exactement aux valeurs ecrites — condition de
 * tracabilite d'un run passe.}
 */
TEST(WriteTrainingConfigJsonTest, EcritUneConfigurationRelueEtIdentique) {
    TrainingConfig config;
    config.evolutionary.populationSize = 12;
    config.gamma = 0.87f;
    config.optimizer = "adam";
    const std::filesystem::path path = scratchFile("ecrite.json");

    ASSERT_TRUE(writeTrainingConfigJson(config, path));

    const TrainingConfig reloaded = loadTrainingConfig(path, CommandLineOverrides{});
    EXPECT_EQ(reloaded.evolutionary.populationSize, 12u);
    EXPECT_FLOAT_EQ(reloaded.gamma, 0.87f);
    EXPECT_EQ(reloaded.optimizer, "adam");
    std::filesystem::remove(path);
}

/**
 * @brief Une valeur du mauvais type dans le fichier laisse le defaut en place, sans lever.
 * \castest{Valeur JSON du mauvais type -> defaut conserve, aucune exception.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Ecrire un fichier ou `gamma` est une chaine et `episodes` un booleen.<br/>2.
 * `loadTrainingConfig` sur ce fichier.<br/>
 * \tattendu Les deux champs gardent leur valeur par defaut, le champ bien type du meme fichier est
 * pris en compte, et aucune exception ne franchit la frontiere du module (`EX-NFR-040`).}
 */
TEST(LoadTrainingConfigTest, ValeurDuMauvaisTypeIgnoreeSansLever) {
    const std::filesystem::path path = scratchFile("mauvais_type.json");
    {
        std::ofstream file(path);
        file << R"({"gamma": "abc", "episodes": true, "optimizer": "adam"})";
    }
    const TrainingConfig defaults;

    const TrainingConfig loaded = loadTrainingConfig(path, CommandLineOverrides{});

    EXPECT_FLOAT_EQ(loaded.gamma, defaults.gamma);
    EXPECT_EQ(loaded.episodes, defaults.episodes);
    EXPECT_EQ(loaded.optimizer, "adam");  // le champ bien type du meme fichier reste lu
    std::filesystem::remove(path);
}

/**
 * @brief La taille de couche cachee d'un modele est relue dans le `config.json` de son run.
 * \castest{Taille de couche cachee resolue depuis le config.json voisin du modele.<br/>
 * \tcat Unitaire · AiSolver Cli<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Ecrire un `config.json` portant une `hiddenSize` differente du defaut, dans un
 * dossier de run.<br/>2. Appeler `hiddenSizeForModel` sur le chemin d'un modele de ce
 * dossier.<br/>3. Recommencer sans `config.json`.<br/>
 * \tattendu La taille du fichier est renvoyee, et le defaut documente lorsque le fichier est
 * absent — sans quoi tout modele entraine avec une autre taille serait irrecuperable.}
 */
TEST(HiddenSizeForModelTest, ResolueDepuisLeConfigDuRunSinonDefaut) {
    const std::filesystem::path runDir = scratchFile("run_hidden_size");
    std::filesystem::create_directories(runDir);
    const std::filesystem::path model = runDir / "model.bin";

    TrainingConfig config;
    config.hiddenSize = aisolver::training::evolutionary::DEFAULT_HIDDEN_SIZE + 9;
    ASSERT_TRUE(writeTrainingConfigJson(config, runDir / "config.json"));
    EXPECT_EQ(aisolver::cli::hiddenSizeForModel(model), config.hiddenSize);

    std::filesystem::remove(runDir / "config.json");
    EXPECT_EQ(aisolver::cli::hiddenSizeForModel(model),
              aisolver::training::evolutionary::DEFAULT_HIDDEN_SIZE);
    std::filesystem::remove_all(runDir);
}
