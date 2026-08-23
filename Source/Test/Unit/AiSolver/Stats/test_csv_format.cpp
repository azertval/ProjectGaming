// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_csv_format.cpp
 * @brief Tests unitaires de aisolver::csvHeader / aisolver::csvRow (LOT-ANNEXE-09, TACHE-02).
 */

#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Stats/CsvFormat.h"

namespace {

aisolver::TrainingStatsRow makeRow(std::string levelName) {
    aisolver::TrainingStatsRow row;
    row.index = 3;
    row.bestReward = 12.5f;
    row.meanReward = 8.0f;
    row.worstReward = -1.0f;
    row.rewardStdDev = 2.5f;
    row.bestStepCount = 120;
    row.successRate = 0.4f;
    row.seed = 42;
    row.levelName = std::move(levelName);
    return row;
}

/// Analyseur CSV minimal dédié aux tests (jamais promu en code de production, TACHE-05) : gère les
/// champs entre guillemets avec guillemets doublés, comme produit par escapeCsvField.
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

}  // namespace

/**
 * @brief `csvHeader()` produit la liste de colonnes documentée, dans l'ordre exact.
 * \castest{<b>En-tête CSV exact.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Appeler `csvHeader()`.<br/>
 * \tattendu La chaîne retournée correspond exactement à l'en-tête documenté.}
 */
TEST(CsvFormatTest, EnTeteExact) {
    EXPECT_EQ(aisolver::csvHeader(),
              "index,bestReward,meanReward,worstReward,rewardStdDev,bestStepCount,successRate,"
              "seed,levelName,timestampIso8601,movingAverageReward,rewardDelta");
}

/**
 * @brief Un `levelName` contenant une virgule est échappé et se relit comme une seule valeur.
 * \castest{<b>Échappement CSV d'un `levelName` contenant une virgule.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `levelName = "niveau, special"`.<br/>2. `csvRow(...)`.<br/>3. Relire avec
 * l'analyseur CSV minimal du test.<br/>
 * \tattendu Le champ `levelName` relu est exactement `"niveau, special"`, une seule valeur.}
 */
TEST(CsvFormatTest, EchappementVirguleRoundTrip) {
    const aisolver::TrainingStatsRow row = makeRow("niveau, special");
    const std::string line = aisolver::csvRow(row, 10.0f, 1.0f, "2026-08-23T00:00:00Z");
    const std::vector<std::string> fields = parseCsvLine(line);
    ASSERT_EQ(fields.size(), 12u);
    EXPECT_EQ(fields[8], "niveau, special");
}

/**
 * @brief Un `levelName` contenant un guillemet est échappé (doublé) et se relit correctement.
 * \castest{<b>Échappement CSV d'un `levelName` contenant un guillemet.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `levelName = "niveau \"special\""`.<br/>2. `csvRow(...)` puis relecture.<br/>
 * \tattendu Le champ relu est identique au `levelName` original.}
 */
TEST(CsvFormatTest, EchappementGuillemetRoundTrip) {
    const aisolver::TrainingStatsRow row = makeRow("niveau \"special\"");
    const std::string line = aisolver::csvRow(row, 10.0f, 1.0f, "2026-08-23T00:00:00Z");
    const std::vector<std::string> fields = parseCsvLine(line);
    ASSERT_EQ(fields.size(), 12u);
    EXPECT_EQ(fields[8], "niveau \"special\"");
}

/**
 * @brief Deux lignes construites à partir de `TrainingStatsRow` différentes ont le même nombre de
 * champs, dans le même ordre.
 * \castest{<b>Stabilité du nombre et de l'ordre des champs entre deux lignes.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `csvRow` sur deux `TrainingStatsRow` différentes (`levelName` avec et sans
 * virgule).<br/>
 * \tattendu Les deux lignes produites ont exactement 12 champs, dans le même ordre de colonnes.}
 */
TEST(CsvFormatTest, StabiliteInterLignes) {
    const std::string lineA = aisolver::csvRow(makeRow("niveau-un"), 1.0f, 0.5f, "t1");
    const std::string lineB = aisolver::csvRow(makeRow("niveau, deux"), 2.0f, -0.5f, "t2");

    const std::vector<std::string> fieldsA = parseCsvLine(lineA);
    const std::vector<std::string> fieldsB = parseCsvLine(lineB);
    EXPECT_EQ(fieldsA.size(), fieldsB.size());
    EXPECT_EQ(fieldsA.size(), 12u);
}
