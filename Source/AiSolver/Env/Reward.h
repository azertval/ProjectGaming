// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <vector>

#include "AiSolver/Env/GridDistanceField.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/Aabb.h"

/**
 * @file AiSolver/Env/Reward.h
 * @brief Signal de récompense unique et partagé de tous les algorithmes d'apprentissage du
 * programme (`LOT-ANNEXE-08`, `EX-IA-009`).
 */

namespace aisolver {

/**
 * @brief Constantes de la fonction de récompense (`EX-IA-009`).
 *
 * Deux inégalités déterminent ces valeurs par défaut, et elles sont vérifiées par un test
 * (`test_reward.cpp`) plutôt que laissées à la relecture :
 *
 * 1. `MAX_STEP_BUDGET × timePenalty < |deathPenalty|` — sinon **mourir tôt coûte moins cher que
 *    d'explorer jusqu'au bout**, et l'agent apprend à se jeter dans le premier danger venu. C'est
 *    l'inégalité que le budget de pas dérivé du niveau (`StepBudget.h`) a rendue mordante : à
 *    l'ancien `timePenalty = 0.01`, un épisode de `demo-final` perdait `70` en temps contre `10`
 *    de pénalité de mort.
 * 2. `|deathPenalty| < completionBonus` — terminer le niveau doit rester préférable à tout, et
 *    d'une marge qui dépasse la progression cumulée plausible (≈ `137` cases sur `demo-final`,
 *    le plus long des niveaux `demo-*.json`).
 *
 * @see `StepBudget.h` pour `MAX_STEP_BUDGET`, la borne haute du budget dérivé d'un niveau.
 */
struct RewardConfig {
    /// Multiplicateur de la récompense de progression (diminution de distance à l'objectif
    /// immédiat, en cases de grille).
    float progressScale = 1.0f;
    /// Récompense fixe accordée uniquement à `core::LevelOutcome::Won`.
    float completionBonus = 100.0f;
    /// Récompense (négative) accordée uniquement à `core::LevelOutcome::Lost`.
    float deathPenalty = -20.0f;
    /// Pénalité (positive, soustraite) appliquée à chaque pas, quelle que soit l'issue.
    float timePenalty = 0.001f;
};

/**
 * @brief Calcule la récompense d'un pas unique, fonction pure sans état interne ni effet de bord.
 *
 * La progression se mesure en distance de plus court chemin sur la grille, murs compris
 * (`GridDistanceField`, `EX-IA-023`) : c'est la seule mesure qui récompense un détour imposé par
 * la géométrie du niveau. Le centre de la boîte du personnage est converti en case de grille
 * (partie entière des coordonnées monde, 1 case = 1 unité monde, même convention que
 * `HMI::GameViewport::cellAt`).
 * @param config Constantes de la fonction de récompense.
 * @param distanceField Champ de distances vers l'objectif **immédiat** courant, pas vers la sortie
 *        seule (`buildObjectiveDistanceField`) : une porte verrouillée fermée n'est pas un chemin,
 *        et le détour par sa clé doit donc compter comme une progression.
 * @param previousBox Boîte du personnage avant ce pas.
 * @param currentBox Boîte du personnage après ce pas.
 * @param outcome Issue du niveau à l'issue de ce pas.
 * @return La récompense scalaire de ce pas.
 */
[[nodiscard]] float computeReward(const RewardConfig& config,
                                  const GridDistanceField& distanceField,
                                  const core::Aabb& previousBox, const core::Aabb& currentBox,
                                  core::LevelOutcome outcome);

/**
 * @brief Construit le champ de distances de l'**objectif immédiat** courant (`LOT-ANNEXE-21`).
 *
 * Cible la sortie **et** la position de chaque déclencheur (`core::Mechanism::switchPosition` --
 * clé, interrupteur ou plaque de pression) dont la porte liée n'est pas encore ouverte
 * (`!mechanisms.isDoorOpen(index)`), sur la grille de collision **courante**
 * (`mechanisms.collisionMap()`, portes fermées solides). `GridDistanceField::distance()` renvoie
 * alors la distance à la plus proche de ces cibles : tant qu'une porte reste fermée, s'approcher de
 * son déclencheur réduit la distance exactement comme s'approcher de la sortie le ferait une fois
 * le chemin dégagé -- sans connaître ni l'ordre de résolution attendu ni la nature du mécanisme
 * (générique, ne nécessite aucune modification quand un niveau ajoute de nouveaux mécanismes).
 * L'ensemble des cibles ne dépend que de l'état ouvert/fermé des portes : le champ reste valide
 * tant qu'aucune porte ne change d'état, et n'a donc à être reconstruit qu'à ce moment-là.
 * @param level Niveau chargé (sortie, liaisons de mécanismes).
 * @param mechanisms Contrôleur de mécanismes du pas courant (état ouvert/fermé, grille de
 *        collision à jour).
 */
[[nodiscard]] GridDistanceField buildObjectiveDistanceField(
    const core::Level& level, const core::MechanismController& mechanisms);

/**
 * @brief Même champ, sur une grille de collision fournie par l'appelant.
 *
 * `mechanisms.collisionMap()` ne connaît que les portes. Un **bloc poussable** déplacé depuis le
 * début du niveau y reste à sa case d'origine : le champ mesurerait alors la progression le long
 * d'un chemin que le bloc obstrue, ou refuserait celui qu'il vient de dégager. L'environnement
 * passe donc la grille composée qu'il vient de résoudre (`HeadlessLevelEnvironment::collisionMap`,
 * portes **et** blocs), et `mechanisms` ne sert plus qu'à savoir quelles portes restent fermées.
 * @param level Niveau chargé (sortie, liaisons de mécanismes).
 * @param mechanisms Contrôleur de mécanismes du pas courant (état ouvert/fermé).
 * @param collision Grille sur laquelle propager le BFS.
 */
[[nodiscard]] GridDistanceField buildObjectiveDistanceField(
    const core::Level& level, const core::MechanismController& mechanisms,
    const core::TileMap& collision);

/**
 * @brief Champ de distances vers l'objectif immédiat, reconstruit seulement lorsqu'il cesse
 * d'être valide.
 *
 * `buildObjectiveDistanceField` coûte un BFS sur toute la grille, là où la récompense en a besoin
 * à chaque pas de simulation. Or ce que le champ décrit -- l'ensemble des cibles **et** la grille
 * de collision -- ne dépend que de l'état ouvert/fermé des portes : `core::MechanismController`
 * n'écrit dans sa grille qu'à l'ouverture ou à la fermeture d'une porte. Le vecteur de ces états
 * est donc une signature complète du champ, et sa comparaison coûte le nombre de mécanismes du
 * niveau au lieu de sa surface.
 *
 * Le champ rendu est numériquement identique à celui d'une reconstruction systématique : ce cache
 * ne change aucune récompense, il supprime un recalcul.
 *
 * Une instance par boucle d'épisode, déclarée **hors** de la boucle des pas.
 */
class ObjectiveDistanceFieldCache {
public:
    /**
     * @brief Champ valide pour l'état courant, reconstruit s'il a cessé de l'être depuis l'appel
     *        précédent.
     * @param level Niveau chargé (sortie, liaisons de mécanismes).
     * @param mechanisms Contrôleur de mécanismes du pas courant.
     * @param collision Grille de collision composée du pas courant (portes **et** blocs).
     * @param blockPositions Positions courantes des blocs poussables : elles font partie de la
     *        signature au même titre que l'état des portes, un bloc déplacé changeant la grille
     *        sur laquelle le BFS se propage.
     * @return Référence sur le champ mémorisé, valable jusqu'au prochain appel.
     */
    [[nodiscard]] const GridDistanceField& field(
        const core::Level& level, const core::MechanismController& mechanisms,
        const core::TileMap& collision, const std::vector<core::GridPosition>& blockPositions);

    /**
     * @brief Surcharge « portes seules », sur `mechanisms.collisionMap()` et sans bloc poussable.
     *
     * Pendant du constructeur à deux arguments de `buildObjectiveDistanceField` : pour un appelant
     * qui n'a pas de `core::BlockController` sous la main (un niveau sans bloc, un test). Un
     * appelant qui en a un doit passer par la surcharge complète, sinon un bloc déplacé reste
     * invisible du champ.
     * @param level Niveau chargé (sortie, liaisons de mécanismes).
     * @param mechanisms Contrôleur de mécanismes du pas courant.
     */
    [[nodiscard]] const GridDistanceField& field(const core::Level& level,
                                                 const core::MechanismController& mechanisms);

    /// @return Le champ mémorisé, sans le revalider (`field()` doit avoir été appelé au moins une
    /// fois depuis la construction).
    [[nodiscard]] const GridDistanceField& lastField() const;

    /// @return Le numéro de version du champ, incrémenté à **chaque reconstruction**.
    ///
    /// C'est le seul moyen pour un appelant de savoir que l'ensemble des objectifs a changé — donc
    /// que les distances mesurées avant et après ne sont plus comparables, et qu'un record de
    /// progression accumulé sur l'objectif précédent doit être abandonné.
    [[nodiscard]] unsigned long long revision() const noexcept {
        return _revision;
    }

private:
    std::optional<GridDistanceField> _field;
    /// État ouvert/fermé de chaque porte au moment où `_field` a été construit.
    std::vector<bool> _doorsOpen;
    /// Positions des blocs poussables au moment où `_field` a été construit.
    std::vector<core::GridPosition> _blockPositions;
    unsigned long long _revision = 0;
};

}  // namespace aisolver
