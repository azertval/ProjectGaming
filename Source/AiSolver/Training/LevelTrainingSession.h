// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryConfig.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryTrainer.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/TrainingResult.h"

/**
 * @file AiSolver/Training/LevelTrainingSession.h
 * @brief Boucle d'entraînement pour un unique niveau et critère d'arrêt (`LOT-ANNEXE-11`,
 * `EX-IA-012`).
 */

namespace aisolver::training {

/**
 * @brief Critère d'arrêt d'une session d'entraînement : résolution stable, ou plafond.
 *
 * « Résolu N fois d'affilée » n'est pas une répétition de tirages aléatoires (l'évaluation et le
 * rejeu sont déterministes, `LOT-ANNEXE-10`) : c'est une exigence de stabilité
 * inter-générationnelle — le champion reste invaincu (aucun individu ne le dépasse) et résolvant
 * (`EpisodeStatus::Won`) pendant `requiredConsecutiveSuccesses` générations consécutives (voir
 * décision de cadrage de l'épic).
 */
struct StoppingConfig {
    /// Nombre de générations consécutives où le champion doit rester invaincu et résolvant pour
    /// que la session s'arrête par résolution.
    int requiredConsecutiveSuccesses = 5;
    /// Plafond dur de générations : borne le coût d'un entraînement qui ne convergerait jamais.
    int maxGenerations = 200;
};

/**
 * @brief Met à jour le compteur de générations consécutives où le champion reste invaincu et
 * résolvant, fonction pure isolée de toute simulation pour rester testable indépendamment de la
 * dynamique réelle de l'algorithme évolutionniste (`LevelTrainingSession::run`).
 *
 * Toute rupture (nouveau champion, ou champion non résolvant) remet le compteur à zéro puis, si la
 * génération courante résout tout de même le niveau, redémarre immédiatement une nouvelle série à
 * `1` (décision de cadrage de l'épic) — pas une réinitialisation muette qui perdrait cette
 * information.
 * @param previousCount        Compteur avant cette génération.
 * @param sameChampionAsBefore Le champion de cette génération est-il, poids pour poids, celui de la
 *                             génération précédente (aucun individu ne l'a dépassé) ?
 * @param resolvingNow         Le champion de cette génération résout-il le niveau
 *                             (`EpisodeStatus::Won`) ?
 * @return Le nouveau compteur.
 */
[[nodiscard]] int updateConsecutiveStableWins(int previousCount, bool sameChampionAsBefore,
                                              bool resolvingNow) noexcept;

/**
 * @brief Entraîne un algorithme évolutionniste (`LOT-ANNEXE-10`) sur **un seul** fichier de niveau,
 * jusqu'à résolution stable ou plafond de générations.
 *
 * Un run = un niveau, strictement, sans exception (décision de cadrage transverse du programme
 * Lot-Annexe) : cette classe ne connaît, nulle part, de liste de niveaux ni de mécanisme de
 * progression automatique. Un seul `HeadlessLevelEnvironment` est construit à la construction de la
 * session, réutilisé (`reset()`, via `EvolutionaryTrainer`/`FitnessEvaluator`) pour toute la durée
 * de la session — jamais reconstruit ni changé de niveau.
 */
class LevelTrainingSession {
public:
    /**
     * @param levelPath    Chemin du fichier de niveau **unique** joué par toute la session.
     * @param topology     Topologie du réseau de chaque individu (`policyTopology`, typiquement).
     * @param config       Paramètres de l'algorithme évolutionniste (`LOT-ANNEXE-10`).
     * @param stopping     Critère d'arrêt (résolution stable ou plafond).
     * @param seed         Graine explicite de tout l'aléatoire de l'entraînement.
     * @param statsCsvPath Chemin du fichier CSV de journalisation (`TrainingStatsRecorder`,
     *                     `LOT-ANNEXE-09`) ; les dossiers parents manquants sont créés.
     * @param environmentConfig Configuration de l'environnement (budget de pas dur, seuil de
     *                     progression) ; valeur par défaut de `LOT-ANNEXE-05` si omise.
     */
    LevelTrainingSession(std::filesystem::path levelPath, evolutionary::NetworkTopology topology,
                         evolutionary::EvolutionaryConfig config, StoppingConfig stopping,
                         std::uint64_t seed, const std::filesystem::path& statsCsvPath,
                         EnvironmentConfig environmentConfig = {});

    /**
     * @brief Exécute des générations jusqu'à ce que le champion reste invaincu et résolvant
     * pendant `StoppingConfig::requiredConsecutiveSuccesses` générations consécutives, ou jusqu'à
     * `StoppingConfig::maxGenerations`, selon ce qui survient en premier.
     *
     * Toute rupture (nouveau champion, ou champion non résolvant) remet le compteur de stabilité à
     * zéro — voir la décision de cadrage de l'épic.
     * @param shouldStop Vérifié au début de chaque génération (`LOT-ANNEXE-21`) ; si présent et
     *        renvoie `true`, la session s'arrête avant cette génération, comme si le plafond
     *        avait été atteint (résultat partiel, jamais une exception). `nullptr` (défaut) :
     *        comportement inchangé, jamais interrompu avant la résolution ou le plafond.
     * @param onGenerationChampion Appelé après chaque génération avec le champion courant
     *        (`LOT-ANNEXE-21`) : seul point d'accès de l'appelant au réseau du champion pendant
     *        la session (`_trainer` reste privé) — sert à l'aperçu en direct de l'IHM (rejeu du
     *        champion courant), jamais à une décision d'arrêt (voir `shouldStop`). `nullptr`
     *        (défaut) : aucun effet.
     */
    [[nodiscard]] TrainingResult run(
        const std::function<bool()>& shouldStop = {},
        const std::function<void(const evolutionary::Individual&)>& onGenerationChampion = {});

    /// @return L'environnement de la session, en lecture seule (diagnostic/tests).
    [[nodiscard]] const HeadlessLevelEnvironment& environment() const noexcept {
        return _environment;
    }

private:
    std::filesystem::path _levelPath;
    evolutionary::NetworkTopology _topology;
    StoppingConfig _stopping;
    HeadlessLevelEnvironment _environment;
    TrainingStatsRecorder _recorder;
    evolutionary::EvolutionaryTrainer _trainer;
};

}  // namespace aisolver::training
