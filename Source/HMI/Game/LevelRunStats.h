#pragma once

#include <string>
#include <vector>

#include "HMI/Game/GameEvents.h"

/**
 * @file HMI/Game/LevelRunStats.h
 * @brief Bilan d'un tableau joué : durée, morts, sauts (`LOT-68`, `EX-IHM-070`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — même patron que `hmi::gameHudLines` et `hmi::editorStatusLines`.
 *
 * La durée se mesure en **pas de simulation**, jamais à l'horloge murale. Le pas est fixe
 * (`core::FixedTimestep`, `LOT-33`) : compter les pas donne un temps qui ne dépend ni de la
 * cadence de rendu ni de la charge de la machine, donc comparable d'une partie à l'autre et d'un
 * poste à l'autre — condition de la reproductibilité posée au `LOT-66`. Une mesure à l'horloge
 * murale récompenserait un ordinateur lent, qui laisse plus de temps réel pour le même nombre de
 * pas.
 */

namespace hmi {

/// Compteurs d'un tableau, remis à zéro à chaque entrée dans le tableau (y compris en rejouant).
struct LevelRunStats {
    int simulationSteps = 0;  ///< Pas de simulation consommés depuis l'entrée dans le tableau.
    int deaths = 0;           ///< Morts du personnage, la dernière comprise s'il y en a eu une.
    int jumps = 0;            ///< Sauts déclenchés, aériens compris.

    [[nodiscard]] friend bool operator==(const LevelRunStats&,
                                         const LevelRunStats&) noexcept = default;
};

/**
 * @brief Comptabilise **un** pas de simulation et les évènements qu'il a produits.
 * @param stats      Compteurs à mettre à jour.
 * @param stepEvents Évènements du pas qui vient de s'exécuter (`GameSession::lastStepEvents`).
 *
 * Appelée une fois par pas, jamais par image de rendu : les deux cadences divergent dès que le
 * rendu dépasse ou rattrape le pas fixe, et compter par image rendrait le bilan dépendant de la
 * machine.
 */
void accumulateStep(LevelRunStats& stats, const std::vector<GameEvent>& stepEvents);

/**
 * @brief Durée écoulée, en secondes.
 * @param stats             Compteurs du tableau.
 * @param fixedDeltaSeconds Durée d'un pas de simulation (`core::FixedTimestep`).
 */
[[nodiscard]] float elapsedSeconds(const LevelRunStats& stats, float fixedDeltaSeconds) noexcept;

/**
 * @brief Met une durée en forme `m:ss`, ou `h:mm:ss` au-delà de l'heure.
 *
 * Les secondes sont **tronquées** et non arrondies : afficher `2:00` pour une partie de 1 min 59,7
 * s laisserait croire qu'on a atteint la minute ronde. Une durée négative est traitée comme nulle.
 */
[[nodiscard]] std::string formatElapsed(float seconds);

}  // namespace hmi
