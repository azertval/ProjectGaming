/**
 * @file test_level_run_stats.cpp
 * @brief Tests unitaires du bilan d'un tableau joué (`LOT-68`, `EX-IHM-070`).
 */

#include <vector>

#include <gtest/gtest.h>

#include "HMI/Game/LevelRunStats.h"

/**
 * @brief Les compteurs n'avancent que sur les évènements qui les concernent, et le nombre de pas
 *        avance à chaque appel — y compris pour un pas sans aucun évènement, qui reste du temps
 *        écoulé.
 * \castest{<b>Le bilan compte les pas, les morts et les sauts, et rien d'autre.</b><br/>
 * \tcat Unitaire · Bilan de tableau<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Accumuler un pas vide, un pas avec un saut, un pas avec une mort, un pas melangeant
 * un saut et des evenements sans rapport.<br/>
 * \tattendu Quatre pas comptes, deux sauts, une mort ; les autres evenements sont ignores.
 * }
 */
TEST(LevelRunStatsTest, CompteLesPasLesMortsEtLesSauts) {
    hmi::LevelRunStats stats;

    hmi::accumulateStep(stats, {});
    hmi::accumulateStep(stats, {hmi::GameEvent::Jumped});
    hmi::accumulateStep(stats, {hmi::GameEvent::Died});
    hmi::accumulateStep(stats,
                        {hmi::GameEvent::Jumped, hmi::GameEvent::MenuNavigate,
                         hmi::GameEvent::LevelCompleted});

    EXPECT_EQ(stats.simulationSteps, 4);
    EXPECT_EQ(stats.jumps, 2);
    EXPECT_EQ(stats.deaths, 1);
}

/**
 * @brief La durée dérive des **pas** et du pas fixe, jamais d'une horloge : c'est ce qui rend le
 *        temps comparable d'une machine à l'autre. Un pas fixe nul ou négatif — état impossible en
 *        service, mais pas en test — donne zéro plutôt qu'une valeur absurde.
 * \castest{<b>La duree derive du nombre de pas et du pas fixe.</b><br/>
 * \tcat Unitaire · Bilan de tableau<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Convertir 120 pas a 1/60 s.<br/>2. Convertir un bilan vide, puis avec un pas fixe
 * nul et negatif.<br/>
 * \tattendu 120 pas donnent 2 secondes ; les cas degeneres donnent zero.
 * }
 */
TEST(LevelRunStatsTest, DureeDeriveeDesPas) {
    hmi::LevelRunStats stats;
    stats.simulationSteps = 120;
    EXPECT_NEAR(hmi::elapsedSeconds(stats, 1.0f / 60.0f), 2.0f, 1e-4);

    EXPECT_EQ(hmi::elapsedSeconds(hmi::LevelRunStats{}, 1.0f / 60.0f), 0.0f);
    EXPECT_EQ(hmi::elapsedSeconds(stats, 0.0f), 0.0f);
    EXPECT_EQ(hmi::elapsedSeconds(stats, -1.0f), 0.0f);
}

/**
 * @brief La mise en forme tronque les secondes plutôt que de les arrondir : afficher `2:00` pour
 *        1 min 59,7 s laisserait croire qu'on a atteint la minute ronde. Au-delà de l'heure, le
 *        format gagne un champ plutôt que d'accumuler les minutes.
 * \castest{<b>La duree est mise en forme en tronquant les secondes.</b><br/>
 * \tcat Unitaire · Bilan de tableau<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en forme des durees courtes, une duree juste sous la minute ronde, une duree
 * de plus d'une heure et une duree negative.<br/>
 * \tattendu Les secondes sont tronquees, le format passe a h:mm:ss au-dela de l'heure, et une
 * duree negative vaut zero.
 * }
 */
TEST(LevelRunStatsTest, MiseEnFormeTronqueLesSecondes) {
    EXPECT_EQ(hmi::formatElapsed(0.0f), "0:00");
    EXPECT_EQ(hmi::formatElapsed(9.9f), "0:09");
    EXPECT_EQ(hmi::formatElapsed(119.7f), "1:59");
    EXPECT_EQ(hmi::formatElapsed(102.0f), "1:42");
    EXPECT_EQ(hmi::formatElapsed(3661.0f), "1:01:01");
    EXPECT_EQ(hmi::formatElapsed(-5.0f), "0:00");
}
