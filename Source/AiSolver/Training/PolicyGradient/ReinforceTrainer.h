// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Optim/IOptimizer.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/PolicyGradient/TrajectoryCollector.h"
#include "AiSolver/Training/PolicyGradientTuning.h"

/**
 * @file AiSolver/Training/PolicyGradient/ReinforceTrainer.h
 * @brief Boucle d'entraînement REINFORCE par épisodes, journalisée (`LOT-ANNEXE-12`, TACHE-04,
 * `EX-IA-013`).
 */

namespace aisolver::training {

/// Paramètres d'un run REINFORCE : `gamma` proche de `1` par défaut (voir décision de cadrage de
/// l'épic), `seedBase` d'où dérive déterministiquement la graine de chaque épisode, et les réglages
/// partagés avec l'acteur-critique (`PolicyGradientTuning`).
struct ReinforceConfig {
    float gamma = DEFAULT_GAMMA;
    std::uint64_t seedBase = 0;
    PolicyGradientTuning tuning{};
};

/**
 * @brief Assemble collecte de trajectoire (`TrajectoryCollector`), calcul de retour
 * (`ReturnCalculator`), perte REINFORCE (`ReinforceLoss`) et un optimiseur (`optim::IOptimizer`,
 * `LOT-ANNEXE-04`) en une boucle exécutable épisode par épisode, journalisée
 * (`TrainingStatsRecorder`, `LOT-ANNEXE-09`).
 *
 * Un seul niveau à la fois (décision de cadrage transverse du programme) : construit à partir d'un
 * `HeadlessLevelEnvironment&` et d'**un seul** chemin de niveau, jamais reconstruit ni changé en
 * cours de run. Ne possède ni le réseau de politique ni l'optimiseur (références) : l'appelant
 * reste seul propriétaire de leur durée de vie, pour pouvoir les réutiliser après le run (export,
 * rejeu).
 */
class ReinforceTrainer {
public:
    /**
     * @param policy      Réseau de politique, entraîné en place (poids mis à jour à chaque
     * épisode).
     * @param optimizer   Optimiseur appliquant la mise à jour de poids
     * (`optim::Sgd`/`optim::Adam`).
     * @param environment Environnement réutilisé à chaque épisode ; réinitialisé sur `levelPath` en
     *                    tout début de chaque épisode par cette classe.
     * @param levelPath   Chemin du niveau **unique** joué par tout le run.
     * @param config      Facteur d'actualisation et graine de base.
     * @param recorder    Enregistreur CSV, une ligne par épisode.
     * @param levelName   Nom du niveau, dupliqué dans chaque ligne journalisée.
     */
    ReinforceTrainer(nn::Network& policy, optim::IOptimizer& optimizer,
                     HeadlessLevelEnvironment& environment, std::filesystem::path levelPath,
                     ReinforceConfig config, TrainingStatsRecorder& recorder,
                     std::string levelName);

    /**
     * @brief Exécute `episodeCount` épisodes, par **lots** de `tuning.batchEpisodes`.
     *
     * Pour chaque lot : réinitialise l'environnement et collecte une trajectoire complète par
     * épisode (poids figés pendant tout le lot), calcule les retours, les centre-réduit sur le lot
     * entier (`normalizeWeights`), rétropropage la perte de chaque épisode en **accumulant** les
     * gradients, écrête leur norme globale, applique **un seul** pas d'optimiseur, journalise une
     * ligne par épisode, puis remet le gradient à zéro.
     *
     * Une seule passe d'optimisation par lot de données collecté (pas plusieurs époques sur le
     * même lot, cf. décision de cadrage de l'épic — la différence structurante avec PPO,
     * `LOT-ANNEXE-14`).
     * @param episodeCount Nombre d'épisodes à exécuter à la suite de ceux déjà joués.
     * @param shouldStop Vérifié au début de chaque épisode (`LOT-ANNEXE-21`) ; si présent et
     *        renvoie `true`, l'exécution s'arrête avant cet épisode (arrêt propre, pas une
     *        exception). `nullptr` (défaut) : comportement inchangé.
     */
    void run(std::size_t episodeCount, const std::function<bool()>& shouldStop = {});

    /// @return Le nombre d'épisodes déjà exécutés par ce trainer.
    [[nodiscard]] int episodeIndex() const noexcept {
        return _episodeIndex;
    }

private:
    nn::Network& _policy;
    optim::IOptimizer& _optimizer;
    HeadlessLevelEnvironment& _environment;
    std::filesystem::path _levelPath;
    ReinforceConfig _config;
    TrainingStatsRecorder& _recorder;
    std::string _levelName;
    TrajectoryCollector _collector;
    int _episodeIndex = 0;
};

}  // namespace aisolver::training
