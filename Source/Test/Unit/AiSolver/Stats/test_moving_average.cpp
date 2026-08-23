// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_moving_average.cpp
 * @brief Tests unitaires de aisolver::MovingAverageTracker (LOT-ANNEXE-09, TACHE-03).
 */

#include <cmath>
#include <tuple>

#include <gtest/gtest.h>

#include "AiSolver/Stats/MovingAverage.h"

/**
 * @brief Avant que la fenêtre soit pleine, la moyenne porte sur les valeurs disponibles.
 * \castest{<b>Moyenne mobile correcte avant que la fenêtre (`N=20`) soit pleine.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Fenêtre de taille 20.<br/>2. Pousser `10`, `20`, `30`.<br/>
 * \tattendu Chaque appel retourne la moyenne des valeurs poussées jusque-là (10, 15, 20), pas une
 * valeur biaisée par des zéros implicites.}
 */
TEST(MovingAverageTest, MoyenneAvantFenetrePleine) {
    aisolver::MovingAverageTracker tracker(20);
    EXPECT_FLOAT_EQ(tracker.push(10.0f), 10.0f);
    EXPECT_FLOAT_EQ(tracker.push(20.0f), 15.0f);
    EXPECT_FLOAT_EQ(tracker.push(30.0f), 20.0f);
}

/**
 * @brief Une fois la fenêtre pleine, les valeurs les plus anciennes sortent de la moyenne.
 * \castest{<b>Moyenne mobile correcte une fois la fenêtre pleine.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Fenêtre de taille 3.<br/>2. Pousser 1, 2, 3, puis 4.<br/>
 * \tattendu Après le 4e push, la moyenne ne porte que sur (2, 3, 4) = 3, la valeur 1 est sortie de
 * la fenêtre.}
 */
TEST(MovingAverageTest, MoyenneApresFenetrePleine) {
    aisolver::MovingAverageTracker tracker(3);
    std::ignore = tracker.push(1.0f);
    std::ignore = tracker.push(2.0f);
    EXPECT_FLOAT_EQ(tracker.push(3.0f), 2.0f);
    EXPECT_FLOAT_EQ(tracker.push(4.0f), 3.0f);
}

/**
 * @brief Un plateau synthétique (valeurs constantes) fait tendre le delta vers zéro.
 * \castest{<b>Delta proche de zéro sur un plateau synthétique.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Fenêtre de taille 5.<br/>2. Pousser 10 valeurs constantes égales à 42.<br/>
 * \tattendu La différence entre deux moyennes mobiles consécutives, une fois la fenêtre pleine et
 * stable, est nulle (exactement, valeurs constantes).}
 */
TEST(MovingAverageTest, DeltaNulSurPlateau) {
    aisolver::MovingAverageTracker tracker(5);
    float previous = tracker.push(42.0f);
    for (int i = 0; i < 9; ++i) {
        const float current = tracker.push(42.0f);
        if (i >= 4) {
            // Fenêtre pleine (5 valeurs identiques déjà poussées) : le delta doit être nul.
            EXPECT_NEAR(current - previous, 0.0f, 1e-6f);
        }
        previous = current;
    }
}

/**
 * @brief Une séquence strictement croissante produit un delta significativement positif.
 * \castest{<b>Delta significativement positif sur une séquence strictement croissante.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Fenêtre de taille 5.<br/>2. Pousser 1, 2, 3, ..., jusqu'à 20.<br/>
 * \tattendu Une fois la fenêtre pleine, le delta entre deux moyennes mobiles consécutives reste
 * strictement positif — distinct d'un plateau.}
 */
TEST(MovingAverageTest, DeltaPositifSurSequenceCroissante) {
    aisolver::MovingAverageTracker tracker(5);
    float previous = tracker.push(1.0f);
    for (int i = 2; i <= 20; ++i) {
        const float current = tracker.push(static_cast<float>(i));
        if (i > 5) {
            EXPECT_GT(current - previous, 0.0f);
        }
        previous = current;
    }
}
