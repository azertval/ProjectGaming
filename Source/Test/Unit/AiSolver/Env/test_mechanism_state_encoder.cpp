// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mechanism_state_encoder.cpp
 * @brief Tests unitaires de aisolver::MechanismStateEncoder (LOT-ANNEXE-06, TACHE-03).
 */

#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/MechanismStateEncoder.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Physics/Aabb.h"

namespace {

std::filesystem::path levelPath(const char* file) {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
}

// Somme des deux canaux sur toute la fenetre : pratique pour verifier "rien d'actif".
float sumAllChannels(const aisolver::Tensor<float>& tensor) {
    float total = 0.0f;
    for (std::size_t index = 0; index < tensor.size(); ++index) {
        total += tensor.data()[index];
    }
    return total;
}

}  // namespace

/**
 * @brief Le canal 0 (porte) vaut 0.0f avant contact avec l'interrupteur, 1.0f apres.
 * \castest{<b>Le canal porte reflete l'etat ouvert/ferme apres contact interrupteur.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Charger `demo-interrupteur.json`, encoder avant tout contact.<br/>2. Appeler
 * `mechanisms.update` avec une boite recouvrant l'interrupteur, encoder a nouveau.<br/>
 * \tattendu Le canal 0 a la position de la porte vaut `0.0f` avant, `1.0f` apres.}
 */
TEST(MechanismStateEncoderTest, PorteFermeePuisOuverte) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromFile(levelPath("demo-interrupteur.json"));
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;

    core::MechanismController mechanisms(level);
    core::DangerController dangers(level);
    core::PlatformController platforms(level);
    ASSERT_FALSE(level.mechanisms().empty());
    const core::GridPosition doorPosition = level.mechanisms()[0].doorPosition;
    const core::GridPosition switchPosition = level.mechanisms()[0].switchPosition;

    const aisolver::MechanismStateEncoder encoder;
    const int radius = 6;

    const aisolver::Tensor<float> before =
        encoder.encode(mechanisms, dangers, platforms, level, doorPosition, radius);
    EXPECT_FLOAT_EQ(
        before.at({0, static_cast<std::size_t>(radius), static_cast<std::size_t>(radius)}), 0.0f);

    const core::Aabb overSwitch =
        core::Aabb::fromTopLeftSize(core::Vector2{static_cast<float>(switchPosition.column),
                                                  static_cast<float>(switchPosition.row)},
                                    core::Vector2{1.0f, 1.0f});
    mechanisms.update(overSwitch, 1.0f, false, {});

    const aisolver::Tensor<float> after =
        encoder.encode(mechanisms, dangers, platforms, level, doorPosition, radius);
    EXPECT_FLOAT_EQ(
        after.at({0, static_cast<std::size_t>(radius), static_cast<std::size_t>(radius)}), 1.0f);
}

/**
 * @brief Le canal 0 (porte) revient a 0.0f des que le personnage quitte la plaque de pression.
 * \castest{<b>La plaque de pression est une activation continue, pas un front.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Charger `demo-plaque-pression.json`.<br/>2. `mechanisms.update` avec une boite sur la
 * plaque, encoder.<br/>3. `mechanisms.update` avec une boite hors de la plaque, encoder a
 * nouveau.<br/>
 * \tattendu Le canal 0 vaut `1.0f` pendant que la boite recouvre la plaque, `0.0f` des qu'elle la
 * quitte.}
 */
TEST(MechanismStateEncoderTest, PlaqueDePressionRevientAZeroApresDepart) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromFile(levelPath("demo-plaque-pression.json"));
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;

    core::MechanismController mechanisms(level);
    core::DangerController dangers(level);
    core::PlatformController platforms(level);
    ASSERT_FALSE(level.mechanisms().empty());
    const core::GridPosition doorPosition = level.mechanisms()[0].doorPosition;
    const core::GridPosition switchPosition = level.mechanisms()[0].switchPosition;

    const aisolver::MechanismStateEncoder encoder;
    const int radius = 6;

    const core::Aabb overPlate =
        core::Aabb::fromTopLeftSize(core::Vector2{static_cast<float>(switchPosition.column),
                                                  static_cast<float>(switchPosition.row)},
                                    core::Vector2{1.0f, 1.0f});
    mechanisms.update(overPlate, 1.0f, false, {});
    const aisolver::Tensor<float> onPlate =
        encoder.encode(mechanisms, dangers, platforms, level, doorPosition, radius);
    EXPECT_FLOAT_EQ(
        onPlate.at({0, static_cast<std::size_t>(radius), static_cast<std::size_t>(radius)}), 1.0f);

    const core::Aabb farAway =
        core::Aabb::fromTopLeftSize(core::Vector2{0.0f, 0.0f}, core::Vector2{1.0f, 1.0f});
    mechanisms.update(farAway, 1.0f, false, {});
    const aisolver::Tensor<float> offPlate =
        encoder.encode(mechanisms, dangers, platforms, level, doorPosition, radius);
    EXPECT_FLOAT_EQ(
        offPlate.at({0, static_cast<std::size_t>(radius), static_cast<std::size_t>(radius)}), 0.0f);
}

/**
 * @brief Le danger temporise (`DangerBlink`) alterne 0.0f/1.0f au rythme de son cycle.
 * \castest{<b>Le canal danger suit le cycle actif/inactif d'un `DangerBlink`.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Charger `demo-dangers-avances.json` (danger temporise `period=180, phase=0,
 * activeDuration=45` a la case `(14, 8)`).<br/>2. Encoder au pas `0` (actif), puis apres 45 pas
 * fixes (inactif).<br/>
 * \tattendu Le canal 1 a la case du danger vaut `1.0f` au pas `0`, `0.0f` apres 45 pas.}
 */
TEST(MechanismStateEncoderTest, DangerTemporiseAlterneSelonLeCycle) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromFile(levelPath("demo-dangers-avances.json"));
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;
    ASSERT_FALSE(level.blinkConfigs().empty());
    const core::GridPosition dangerPosition = level.blinkConfigs()[0].position;

    core::MechanismController mechanisms(level);
    core::DangerController dangers(level);
    core::PlatformController platforms(level);
    const aisolver::MechanismStateEncoder encoder;
    const int radius = 2;

    const aisolver::Tensor<float> active =
        encoder.encode(mechanisms, dangers, platforms, level, dangerPosition, radius);
    EXPECT_FLOAT_EQ(
        active.at({1, static_cast<std::size_t>(radius), static_cast<std::size_t>(radius)}), 1.0f);

    for (int step = 0; step < 45; ++step) {
        dangers.update();
    }
    const aisolver::Tensor<float> inactive =
        encoder.encode(mechanisms, dangers, platforms, level, dangerPosition, radius);
    EXPECT_FLOAT_EQ(
        inactive.at({1, static_cast<std::size_t>(radius), static_cast<std::size_t>(radius)}), 0.0f);
}

/**
 * @brief Le danger mobile est rasterise sur la case qu'il recouvre initialement.
 * \castest{<b>Le canal danger suit la boite courante d'un danger mobile.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger `demo-dangers-avances.json` (danger mobile a `(8, 6)`).<br/>2. Encoder autour
 * de sa case de depart, avant tout `update`.<br/>
 * \tattendu Le canal 1 a la case de depart du danger mobile vaut `1.0f`.}
 */
TEST(MechanismStateEncoderTest, DangerMobileRasteriseSurSaBoiteInitiale) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromFile(levelPath("demo-dangers-avances.json"));
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;
    ASSERT_FALSE(level.moverConfigs().empty());
    const core::GridPosition moverStart = level.moverConfigs()[0].startPosition;

    core::MechanismController mechanisms(level);
    core::DangerController dangers(level);
    core::PlatformController platforms(level);
    const aisolver::MechanismStateEncoder encoder;
    const int radius = 2;

    const aisolver::Tensor<float> encoded =
        encoder.encode(mechanisms, dangers, platforms, level, moverStart, radius);
    EXPECT_FLOAT_EQ(
        encoded.at({1, static_cast<std::size_t>(radius), static_cast<std::size_t>(radius)}), 1.0f);
}

/**
 * @brief Un niveau sans aucun mecanisme ni danger avance produit un tenseur entierement nul.
 * \castest{<b>Absence de mecanisme produit un tenseur entierement nul sur les deux
 * canaux.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger `demo-pente.json` (aucun interrupteur ni danger avance).<br/>2. Encoder une
 * fenetre large.<br/>
 * \tattendu La somme de tous les elements du tenseur vaut `0.0f`.}
 */
TEST(MechanismStateEncoderTest, FenetreSansMecanismeEstEntierementNulle) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromFile(levelPath("demo-pente.json"));
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;

    core::MechanismController mechanisms(level);
    core::DangerController dangers(level);
    core::PlatformController platforms(level);
    const aisolver::MechanismStateEncoder encoder;

    const aisolver::Tensor<float> encoded =
        encoder.encode(mechanisms, dangers, platforms, level, level.entry(), 5);
    EXPECT_FLOAT_EQ(sumAllChannels(encoded), 0.0f);
}

/**
 * @brief Deux appels `encode` sur le meme etat produisent des tenseurs identiques.
 * \castest{<b>`encode` est deterministe.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Encoder deux fois le meme etat `mechanisms`/`dangers`/`center`.<br/>
 * \tattendu Les deux tenseurs sont bit-a-bit identiques.}
 */
TEST(MechanismStateEncoderTest, EncodageDeterministe) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromFile(levelPath("demo-interrupteur.json"));
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;

    core::MechanismController mechanisms(level);
    core::DangerController dangers(level);
    core::PlatformController platforms(level);
    const aisolver::MechanismStateEncoder encoder;

    const aisolver::Tensor<float> first =
        encoder.encode(mechanisms, dangers, platforms, level, level.entry(), 4);
    const aisolver::Tensor<float> second =
        encoder.encode(mechanisms, dangers, platforms, level, level.entry(), 4);

    ASSERT_EQ(first.size(), second.size());
    for (std::size_t index = 0; index < first.size(); ++index) {
        EXPECT_EQ(first.data()[index], second.data()[index]);
    }
}
