// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reward.cpp
 * @brief Tests unitaires de aisolver::computeReward (LOT-ANNEXE-08, TACHE-01 ; amendement
 * distance de grille, EX-IA-023).
 */

#include <cmath>

#include <gtest/gtest.h>

#include "AiSolver/Env/GridDistanceField.h"
#include "AiSolver/Env/Reward.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

namespace {

core::Aabb boxAt(float x, float y) {
    return core::Aabb::fromTopLeftSize(core::Vector2{x, y}, core::Vector2{1.0f, 1.0f});
}

/// Grille carrée entièrement vide (aucune case solide) : la distance de grille se réduit alors au
/// même comportement que l'ancienne distance euclidienne pour les tests qui ne visent pas
/// spécifiquement un mur.
core::TileMap openMap(int size) {
    return core::TileMap(size, size);
}

}  // namespace

/**
 * @brief Se rapprocher de la sortie produit une récompense de progression strictement positive.
 * \castest{<b>Se rapprocher de la sortie produit une récompense strictement positive.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `previousBox` a distance 10 de la sortie, `currentBox` a distance 9, grille
 * ouverte.<br/>
 * \tattendu La récompense est strictement positive.}
 */
TEST(RewardTest, ProgressionPositive) {
    const aisolver::RewardConfig config;
    const core::TileMap map = openMap(30);
    const core::GridPosition exit{0, 0};
    const aisolver::GridDistanceField distanceField(map, exit);
    const float reward = aisolver::computeReward(config, distanceField, boxAt(10.0f, 0.0f),
                                                 boxAt(9.0f, 0.0f), core::LevelOutcome::Playing);
    EXPECT_GT(reward, 0.0f);
}

/**
 * @brief Reculer ou stagner ne produit jamais une récompense de progression positive.
 * \castest{<b>Reculer ou stagner ne produit jamais une récompense positive.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Recul : `previousBox` plus proche que `currentBox`, grille ouverte.<br/>2. Stagnation
 * : boîtes identiques.<br/>
 * \tattendu La récompense est négative ou nulle dans les deux cas.}
 */
TEST(RewardTest, RegressionOuStagnationJamaisPositive) {
    const aisolver::RewardConfig config;
    const core::TileMap map = openMap(30);
    const core::GridPosition exit{0, 0};
    const aisolver::GridDistanceField distanceField(map, exit);

    const float regression = aisolver::computeReward(
        config, distanceField, boxAt(9.0f, 0.0f), boxAt(10.0f, 0.0f), core::LevelOutcome::Playing);
    EXPECT_LE(regression, 0.0f);

    const float stagnation = aisolver::computeReward(
        config, distanceField, boxAt(9.0f, 0.0f), boxAt(9.0f, 0.0f), core::LevelOutcome::Playing);
    EXPECT_LE(stagnation, 0.0f);
}

/**
 * @brief Le bonus de complétion domine la pénalité de temps cumulée sur un épisode long.
 * \castest{<b>Le bonus de complétion domine sur un épisode `Won` face à un `Lost`/`Playing` de
 * même progression partielle.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Cumule la récompense de plusieurs centaines de pas identiques, le dernier en
 * `Won`.<br/>2. Meme cumul, dernier pas en `Lost`.<br/>3. Meme cumul, dernier pas en
 * `Playing` (interrompu).<br/>
 * \tattendu La récompense cumulée de l'épisode `Won` est strictement supérieure aux deux
 * autres.}
 */
TEST(RewardTest, BonusDeCompletionDomine) {
    const aisolver::RewardConfig config;
    const core::TileMap map = openMap(30);
    const core::GridPosition exit{0, 0};
    const aisolver::GridDistanceField distanceField(map, exit);
    constexpr int STEPS = 500;

    auto cumulative = [&](core::LevelOutcome finalOutcome) {
        float total = 0.0f;
        for (int step = 0; step < STEPS; ++step) {
            const bool last = (step == STEPS - 1);
            total +=
                aisolver::computeReward(config, distanceField, boxAt(0.0f, 0.0f), boxAt(0.0f, 0.0f),
                                        last ? finalOutcome : core::LevelOutcome::Playing);
        }
        return total;
    };

    const float won = cumulative(core::LevelOutcome::Won);
    const float lost = cumulative(core::LevelOutcome::Lost);
    const float interrupted = cumulative(core::LevelOutcome::Playing);

    EXPECT_GT(won, lost);
    EXPECT_GT(won, interrupted);
}

/**
 * @brief Les valeurs par défaut de `RewardConfig` ne produisent ni `NaN` ni valeur aberrante.
 * \castest{<b>Les valeurs par défaut de `RewardConfig` restent finies sur des distances/durées
 * typiques.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `computeReward` avec `RewardConfig{}` sur des boîtes et une sortie plausibles pour
 * `Playing`, `Won` et `Lost`.<br/>
 * \tattendu Chaque récompense est finie (ni `NaN` ni infinie).}
 */
TEST(RewardTest, ValeursParDefautPlausibles) {
    const aisolver::RewardConfig config;
    const core::TileMap map = openMap(30);
    const core::GridPosition exit{20, 10};
    const aisolver::GridDistanceField distanceField(map, exit);
    for (const core::LevelOutcome outcome :
         {core::LevelOutcome::Playing, core::LevelOutcome::Won, core::LevelOutcome::Lost}) {
        const float reward = aisolver::computeReward(config, distanceField, boxAt(0.0f, 0.0f),
                                                     boxAt(1.0f, 0.5f), outcome);
        EXPECT_TRUE(std::isfinite(reward));
    }
}

/**
 * @brief Un pas de détour nécessaire autour d'un mur reçoit une récompense de progression positive,
 * même s'il éloigne le personnage de la sortie en ligne droite.
 * \castest{<b>Un détour forcé par un mur reçoit une récompense positive malgré un éloignement en
 * ligne droite (amendement LOT-ANNEXE-08, EX-IA-023).</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Grille 3x3, sortie en `(1,0)`, case `(1,1)` solide (mur direct entre le personnage en
 * `(1,2)` et la sortie).<br/>2. `previousBox` en `(1,2)`, `currentBox` en `(0,2)` (pas latéral de
 * contournement).<br/>3. Vérifie qu'en ligne droite (euclidienne) ce pas éloignerait le personnage
 * de la sortie (`(1.5,2.5)`→dist `2.0` vs `(0.5,2.5)`→dist `~2.236`).<br/>
 * \tattendu La récompense de progression est strictement positive : la distance de grille (BFS,
 * respectant le mur) diminue (`4`→`3`) alors que la distance euclidienne aurait augmenté.}
 */
TEST(RewardTest, DetourAutourDunMurRecompensePositivement) {
    const aisolver::RewardConfig config;
    core::TileMap map(3, 3);
    map.setTile(1, 1, core::TileType::Solid);
    const core::GridPosition exit{1, 0};
    const aisolver::GridDistanceField distanceField(map, exit);

    // Confirme le piège : en ligne droite, s'écarter de la colonne de la sortie *augmente* la
    // distance euclidienne pour ce pas (2.0 -> ~2.236), ce que l'ancienne récompense (LOT-ANNEXE-08
    // avant amendement) aurait pénalisé.
    ASSERT_EQ(distanceField.distance(core::GridPosition{1, 2}), 4);
    ASSERT_EQ(distanceField.distance(core::GridPosition{0, 2}), 3);

    const float reward = aisolver::computeReward(config, distanceField, boxAt(1.0f, 2.0f),
                                                 boxAt(0.0f, 2.0f), core::LevelOutcome::Playing);
    EXPECT_GT(reward, 0.0f);
}

/**
 * @brief Tant qu'une porte verrouillée reste fermée, `buildObjectiveDistanceField` cible sa clé en
 * plus de la sortie -- s'approcher de la clé produit une progression positive même quand la sortie
 * est inatteignable derrière la porte (amendement LOT-ANNEXE-21, `EX-IA-023` déplacé des murs
 * statiques aux mécanismes).
 * \castest{<b>Porte verrouillée fermée : approcher la clé progresse malgré une sortie
 * inatteignable.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Couloir 5x1 : clé en `(1,0)`, porte verrouillée fermée en `(2,0)`, sortie en
 * `(4,0)` -- inatteignable tant que la porte est fermée.<br/>2. Vérifie que le champ à cible unique
 * (sortie seule, ancien comportement) renvoie la sentinelle pour la case d'entrée.<br/>3. Vérifie
 * que `buildObjectiveDistanceField` renvoie une distance finie et décroissante à mesure qu'on
 * s'approche de la clé, et que le pas correspondant reçoit une récompense de progression
 * positive.<br/>
 * \tattendu Distance finie/décroissante vers la clé via le champ objectif, alors que le champ
 * à cible unique (sortie) reste inatteignable ; récompense de progression strictement positive.}
 */
TEST(RewardTest, PorteVerrouilleeFermeeCibleLaCleTantQueLaSortieEstInatteignable) {
    core::TileMap map(5, 1);
    map.setTile(1, 0, core::TileType::Key);
    map.setTile(2, 0, core::TileType::LockedDoor);
    const core::GridPosition entry{0, 0};
    const core::GridPosition keyPosition{1, 0};
    const core::GridPosition exit{4, 0};
    const std::vector<core::Mechanism> mechanisms{
        core::Mechanism{keyPosition, core::GridPosition{2, 0}}};
    const core::Level level("test", map, entry, exit, mechanisms);
    const core::MechanismController mechanismController(level);

    // Ancien comportement (cible unique = sortie) : inatteignable depuis l'entrée, la porte bloque
    // tout le couloir.
    const aisolver::GridDistanceField exitOnlyField(mechanismController.collisionMap(), exit);
    EXPECT_EQ(exitOnlyField.distance(entry), map.width() * map.height());

    // Nouveau champ objectif : la clé (porte encore fermée) est une cible concurrente atteignable.
    const aisolver::GridDistanceField objectiveField =
        aisolver::buildObjectiveDistanceField(level, mechanismController);
    EXPECT_EQ(objectiveField.distance(entry), 1);
    EXPECT_EQ(objectiveField.distance(keyPosition), 0);

    const aisolver::RewardConfig config;
    const float reward = aisolver::computeReward(config, objectiveField, boxAt(0.0f, 0.0f),
                                                 boxAt(1.0f, 0.0f), core::LevelOutcome::Playing);
    EXPECT_GT(reward, 0.0f);
}

namespace {

/// Couloir 5x1 du test de porte verrouillée ci-dessus, réutilisé par les tests du cache : clé en
/// `(1,0)`, porte verrouillée en `(2,0)`, sortie en `(4,0)` — inatteignable porte fermée.
core::Level lockedCorridor() {
    core::TileMap map(5, 1);
    map.setTile(1, 0, core::TileType::Key);
    map.setTile(2, 0, core::TileType::LockedDoor);
    const std::vector<core::Mechanism> mechanisms{
        core::Mechanism{core::GridPosition{1, 0}, core::GridPosition{2, 0}}};
    return core::Level("test", map, core::GridPosition{0, 0}, core::GridPosition{4, 0}, mechanisms);
}

/// Compare deux champs case par case sur toute la grille : c'est la seule façon d'affirmer une
/// identité *numérique*, et non la seule égalité des quelques cases qu'un test regarderait.
void expectSameField(const aisolver::GridDistanceField& actual,
                     const aisolver::GridDistanceField& expected, const core::TileMap& map) {
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            const core::GridPosition cell{column, row};
            EXPECT_EQ(actual.distance(cell), expected.distance(cell))
                << "case (" << column << ", " << row << ")";
        }
    }
}

}  // namespace

/**
 * @brief Le champ rendu par `ObjectiveDistanceFieldCache` est numériquement identique à celui
 * d'une reconstruction systématique, à état de portes constant comme après ouverture.
 *
 * C'est l'invariant que le cache promet : il supprime un recalcul, il ne change aucune récompense.
 * Les tests de reproductibilité ne l'attraperaient pas — deux exécutions également périmées
 * restent identiques l'une à l'autre.
 * \castest{<b>Le champ mis en cache est identique à une reconstruction, avant et après ouverture
 * d'une porte.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Couloir verrouillé : comparer `cache.field()` à `buildObjectiveDistanceField()` case
 * par case.<br/>2. Rappeler `cache.field()` sans rien changer, comparer à nouveau.<br/>3. Ouvrir la
 * porte (clé ramassée), comparer une troisième fois.<br/>
 * \tattendu Les trois champs coïncident case par case avec la reconstruction correspondante.}
 */
TEST(ObjectiveDistanceFieldCacheTest, ChampMisEnCacheIdentiqueAUneReconstruction) {
    const core::Level level = lockedCorridor();
    core::MechanismController mechanisms(level);
    aisolver::ObjectiveDistanceFieldCache cache;

    expectSameField(cache.field(level, mechanisms),
                    aisolver::buildObjectiveDistanceField(level, mechanisms), level.tileMap());
    // Second appel a etat inchange : c'est le chemin ou le cache REND sa copie sans reconstruire.
    expectSameField(cache.field(level, mechanisms),
                    aisolver::buildObjectiveDistanceField(level, mechanisms), level.tileMap());

    // Cle ramassee (contact + « Interagir ») : la porte s'ouvre definitivement.
    mechanisms.update(boxAt(1.0f, 0.0f), 1.0f, true);
    ASSERT_TRUE(mechanisms.isDoorOpen(0));
    expectSameField(cache.field(level, mechanisms),
                    aisolver::buildObjectiveDistanceField(level, mechanisms), level.tileMap());
}

/**
 * @brief Le cache se reconstruit à la bascule d'une porte : le champ rendu après ouverture n'est
 * plus celui d'avant.
 *
 * La signature de validité est le vecteur des états de portes. Un cache qui ne la relirait pas
 * continuerait de viser la clé — déjà ramassée — au lieu de la sortie devenue atteignable, et la
 * récompense guiderait l'agent vers une case sans objet pour le reste de l'épisode.
 * \castest{<b>Ouvrir une porte invalide le champ mis en cache.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Porte fermée : relever la distance de l'entrée (cible = la clé).<br/>2. Ramasser la
 * clé pour ouvrir la porte.<br/>3. Relever à nouveau la distance de l'entrée via le MÊME
 * cache.<br/>
 * \tattendu La distance passe de 1 (clé) à 4 (sortie devenue atteignable) : le champ a bien été
 * reconstruit.}
 */
TEST(ObjectiveDistanceFieldCacheTest, ReconstruitLeChampQuandUnePorteChangeDEtat) {
    const core::Level level = lockedCorridor();
    core::MechanismController mechanisms(level);
    aisolver::ObjectiveDistanceFieldCache cache;

    const core::GridPosition entry = level.entry();
    // Porte fermee : la sortie est murée, seule la cle est atteignable -- elle est a une case.
    EXPECT_EQ(cache.field(level, mechanisms).distance(entry), 1);

    mechanisms.update(boxAt(1.0f, 0.0f), 1.0f, true);
    ASSERT_TRUE(mechanisms.isDoorOpen(0));

    // Porte ouverte : plus aucune porte fermee, donc la sortie est la seule cible -- quatre cases.
    // Un cache qui ne s'invaliderait pas rendrait encore 1.
    EXPECT_EQ(cache.field(level, mechanisms).distance(entry), 4);
}
