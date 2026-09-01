// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_training_stats_recorder.cpp
 * @brief Tests unitaires de aisolver::TrainingStatsRecorder, bout en bout (LOT-ANNEXE-09,
 * TACHE-01/05).
 */

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Stats/MovingAverage.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"

namespace {

/// Répertoire temporaire du test courant, nettoyé à la destruction (RAII), même patron que
/// Source/Test/Unit/AiSolver/Replay/test_replay_file.cpp.
class TempDirectory {
public:
    TempDirectory()
        : _path(std::filesystem::temp_directory_path() / "aisolver_test_training_stats_recorder") {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
        std::filesystem::create_directories(_path);
    }
    ~TempDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }

    [[nodiscard]] std::filesystem::path file(const char* name) const {
        return _path / name;
    }

private:
    std::filesystem::path _path;
};

/// Analyseur CSV minimal dédié au test (jamais promu en code de production, TACHE-05).
std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char character = line[i];
        if (inQuotes) {
            if (character == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field += character;
            }
        } else {
            if (character == '"') {
                inQuotes = true;
            } else if (character == ',') {
                fields.push_back(field);
                field.clear();
            } else {
                field += character;
            }
        }
    }
    fields.push_back(field);
    return fields;
}

std::vector<std::string> readAllLines(const std::filesystem::path& path) {
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

aisolver::TrainingStatsRow rowAt(int index, float bestReward) {
    aisolver::TrainingStatsRow row;
    row.index = index;
    row.bestReward = bestReward;
    row.meanReward = bestReward - 1.0f;
    row.worstReward = bestReward - 2.0f;
    row.rewardStdDev = 0.5f;
    row.bestStepCount = 100 + index;
    row.successRate = 0.5f;
    row.seed = 42;
    row.levelName = "demo-deplacement";
    return row;
}

}  // namespace

/**
 * @brief Une séquence enregistrée est relue à l'identique, colonnes calculées incluses.
 * \castest{<b>Bout en bout : écriture puis relecture d'une séquence progression + plateau.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Enregistrer 8 lignes : 4 en progression stricte, 4 en plateau (même
 * `bestReward`).<br/>
 * 2. Relire le fichier avec l'analyseur CSV minimal du test.<br/>
 * \tattendu 9 lignes (en-tête + 8 données) ; en-tête conforme ; chaque colonne fournie correspond à
 * ce qui a été enregistré ; les colonnes de moyenne mobile/delta correspondent à un
 * `MovingAverageTracker` appliqué indépendamment à la même séquence de `bestReward`.}
 */
TEST(TrainingStatsRecorderTest, BoutEnBoutProgressionPuisPlateau) {
    TempDirectory tempDir;
    const std::filesystem::path csvPath = tempDir.file("stats.csv");

    const std::vector<float> bestRewards = {1.0f, 2.0f, 3.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f};
    constexpr int WINDOW = 3;

    // Oracle indépendant : même fenêtre qu'on passe explicitement à TrainingStatsRecorder
    // ci-dessous (une fenêtre plus petite que la séquence, pour que la moyenne mobile atteigne
    // réellement le plateau au lieu de rester une moyenne cumulative sur toute la séquence).
    aisolver::MovingAverageTracker oracle(WINDOW);
    std::vector<float> expectedMovingAverage;
    std::vector<float> expectedDelta;
    bool hasPrevious = false;
    float previous = 0.0f;
    for (const float reward : bestRewards) {
        const float average = oracle.push(reward);
        expectedMovingAverage.push_back(average);
        expectedDelta.push_back(hasPrevious ? (average - previous) : 0.0f);
        previous = average;
        hasPrevious = true;
    }

    {
        aisolver::TrainingStatsRecorder recorder(csvPath, WINDOW);
        for (std::size_t i = 0; i < bestRewards.size(); ++i) {
            recorder.record(rowAt(static_cast<int>(i), bestRewards[i]));
        }
    }

    const std::vector<std::string> lines = readAllLines(csvPath);
    ASSERT_EQ(lines.size(), bestRewards.size() + 1);
    EXPECT_EQ(lines[0],
              "index,bestReward,meanReward,worstReward,rewardStdDev,bestStepCount,successRate,"
              "seed,levelName,timestampIso8601,movingAverageReward,rewardDelta");

    for (std::size_t i = 0; i < bestRewards.size(); ++i) {
        const std::vector<std::string> fields = parseCsvLine(lines[i + 1]);
        ASSERT_EQ(fields.size(), 12u);
        EXPECT_EQ(std::stoi(fields[0]), static_cast<int>(i));
        EXPECT_FLOAT_EQ(std::stof(fields[1]), bestRewards[i]);
        EXPECT_EQ(fields[8], "demo-deplacement");
        // Tolerance liee a la precision par defaut du flux de serialisation (6 chiffres
        // significatifs), pas a la logique de MovingAverageTracker elle-meme.
        EXPECT_NEAR(std::stof(fields[10]), expectedMovingAverage[i], 1e-4f);
        EXPECT_NEAR(std::stof(fields[11]), expectedDelta[i], 1e-4f);
    }

    // Plateau (les 2 dernieres valeurs pousees pleinement dans une fenetre de 3) : le delta est
    // exactement nul, la fenetre ne contenant plus que la valeur constante du plateau.
    const std::vector<std::string> lastFields = parseCsvLine(lines.back());
    EXPECT_NEAR(std::stof(lastFields[11]), 0.0f, 1e-4f);
}

/**
 * @brief Un run interrompu à la ligne `k` laisse un fichier CSV valide de `k` lignes de données.
 * \castest{<b>Fichier interrompu simulé : destruction du recorder après `k` appels.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Créer un `TrainingStatsRecorder`, appeler `record` 3 fois sur une séquence prévue de
 * 10.<br/>2. Détruire le recorder (fin de portée) avant les 7 appels restants.<br/>
 * \tattendu Le fichier contient exactement 1 en-tête + 3 lignes de données, aucune ligne
 * partielle, aucune corruption.}
 */
TEST(TrainingStatsRecorderTest, FichierInterrompuResteExploitable) {
    TempDirectory tempDir;
    const std::filesystem::path csvPath = tempDir.file("stats_interrompu.csv");

    {
        aisolver::TrainingStatsRecorder recorder(csvPath);
        for (int i = 0; i < 3; ++i) {
            recorder.record(rowAt(i, static_cast<float>(i)));
        }
        // Le recorder est détruit ici, avant d'avoir atteint les 10 lignes prévues.
    }

    const std::vector<std::string> lines = readAllLines(csvPath);
    ASSERT_EQ(lines.size(), 4u);  // en-tête + 3 lignes.
    for (std::size_t i = 1; i < lines.size(); ++i) {
        const std::vector<std::string> fields = parseCsvLine(lines[i]);
        ASSERT_EQ(fields.size(), 12u);
        EXPECT_EQ(std::stoi(fields[0]), static_cast<int>(i - 1));
    }
}

/**
 * @brief Un appel à `record` ajoute exactement une ligne, sans altérer les lignes déjà écrites.
 * \castest{<b>`record` ajoute une ligne sans altérer les précédentes.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `record` une première fois, lire le fichier.<br/>2. `record` une deuxième fois, lire
 * de nouveau.<br/>
 * \tattendu La deuxième lecture contient exactement une ligne de données de plus, la première
 * ligne restant strictement identique.}
 */
TEST(TrainingStatsRecorderTest, RecordAjouteUneLigneSansAlterer) {
    TempDirectory tempDir;
    const std::filesystem::path csvPath = tempDir.file("stats_incremental.csv");

    aisolver::TrainingStatsRecorder recorder(csvPath);
    recorder.record(rowAt(0, 1.0f));
    const std::vector<std::string> afterFirst = readAllLines(csvPath);
    ASSERT_EQ(afterFirst.size(), 2u);

    recorder.record(rowAt(1, 2.0f));
    const std::vector<std::string> afterSecond = readAllLines(csvPath);
    ASSERT_EQ(afterSecond.size(), 3u);
    EXPECT_EQ(afterSecond[0], afterFirst[0]);
    EXPECT_EQ(afterSecond[1], afterFirst[1]);
}

/**
 * @brief `setOnRecord` observe exactement une ligne par appel à `record`, sans effet sur le CSV
 * produit (LOT-ANNEXE-21) : ni son nombre de lignes, ni ses champs autres que l'horodatage
 * (naturellement variable d'un appel à l'autre, jamais comparé ici).
 * \castest{<b>`setOnRecord` observe sans effet de bord sur le CSV.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Enregistrer un observateur qui copie l'index de chaque ligne reçue.<br/>2. `record`
 * trois lignes.<br/>
 * \tattendu Trois appels à l'observateur, un index par génération dans l'ordre ; le CSV compte
 * exactement quatre lignes (en-tête + trois), comme sans observateur.}
 */
TEST(TrainingStatsRecorderTest, SetOnRecordObserveSansEffetDeBord) {
    TempDirectory tempDir;
    const std::filesystem::path csvPath = tempDir.file("stats_observed.csv");

    std::vector<int> observedIndices;
    aisolver::TrainingStatsRecorder recorder(csvPath);
    recorder.setOnRecord([&observedIndices](const aisolver::TrainingStatsRow& row,
                                            const aisolver::TrainingStatsDerived&) {
        observedIndices.push_back(row.index);
    });

    for (int i = 0; i < 3; ++i) {
        recorder.record(rowAt(i, static_cast<float>(i)));
    }

    ASSERT_EQ(observedIndices.size(), 3u);
    EXPECT_EQ(observedIndices[0], 0);
    EXPECT_EQ(observedIndices[1], 1);
    EXPECT_EQ(observedIndices[2], 2);
    EXPECT_EQ(readAllLines(csvPath).size(), 4u);
}

/**
 * @brief L'observateur reçoit les grandeurs **dérivées** qui accompagnent la ligne dans le CSV, et
 *        non la seule ligne brute (`LOT-73`).
 *
 * La moyenne mobile et la variation étaient calculées, écrites dans le fichier, puis perdues : un
 * appelant qui voulait tracer une courbe lissée devait relire le fichier que l'enregistreur venait
 * d'écrire, ou refaire le calcul — deux façons de dupliquer ce qui existait déjà.
 * \castest{<b>L'observateur recoit la moyenne mobile et la variation, pas seulement la
 * ligne.</b><br/>
 * \tcat Unitaire · Statistiques d'entrainement<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Enregistrer trois lignes de recompenses croissantes.<br/>2. Capturer les grandeurs
 * derivees recues par l'observateur.<br/>
 * \tattendu La moyenne mobile suit les recompenses ; la variation est nulle sur la premiere ligne
 * puis strictement positive.
 * }
 */
TEST(TrainingStatsRecorderTest, ObservateurRecoitLesGrandeursDerivees) {
    TempDirectory tempDir;
    const std::filesystem::path csvPath = tempDir.file("stats_derived.csv");

    std::vector<aisolver::TrainingStatsDerived> observed;
    aisolver::TrainingStatsRecorder recorder(csvPath);
    recorder.setOnRecord([&observed](const aisolver::TrainingStatsRow&,
                                     const aisolver::TrainingStatsDerived& derived) {
        observed.push_back(derived);
    });

    for (int i = 0; i < 3; ++i) {
        recorder.record(rowAt(i, static_cast<float>(i + 1)));
    }

    ASSERT_EQ(observed.size(), 3u);
    // Premiere ligne : aucune ligne precedente, donc aucune variation a rapporter.
    EXPECT_FLOAT_EQ(observed[0].rewardDelta, 0.0f);
    EXPECT_FLOAT_EQ(observed[0].movingAverageReward, 1.0f);
    // Recompenses croissantes : la moyenne mobile monte, la variation reste strictement positive.
    EXPECT_GT(observed[1].movingAverageReward, observed[0].movingAverageReward);
    EXPECT_GT(observed[2].movingAverageReward, observed[1].movingAverageReward);
    EXPECT_GT(observed[1].rewardDelta, 0.0f);
    EXPECT_GT(observed[2].rewardDelta, 0.0f);
}
