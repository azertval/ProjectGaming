// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_argmax_rollout.cpp
 * @brief Tests unitaires de `aisolver::training::argmaxRollout` (`LOT-ANNEXE-19`, extrait au
 * `LOT-ANNEXE-21`).
 *
 * C'est le chemin qui produit le rejeu **réellement regardé par le joueur** : `aisolver-cli`
 * (`runEvaluate`/`runExportReplay`) et `hmi::TrainingWorker` y passent tous deux. Les politiques
 * utilisées ici sont des doublures — l'objet du test est le rejeu lui-même (déterminisme, issue,
 * chemins d'échec documentés), pas la qualité d'un modèle entraîné.
 */

#include <filesystem>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Eval/ActionDecodingMode.h"
#include "AiSolver/Eval/TrainedPolicy.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Training/ArgmaxRollout.h"
#include "Core/Physics/PlayerInput.h"
#include "TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::EpisodeStatus;
using aisolver::HeadlessLevelEnvironment;
using aisolver::Rng;
using aisolver::Tensor;
using aisolver::eval::ActionDecodingMode;
using aisolver::eval::TrainedPolicy;
using aisolver::training::argmaxRollout;
using aisolver::training::DeterministicReplayResult;
using aisolver_test::TrivialLevelDirectory;

namespace {

/// Politique factice rendant toujours la même entrée, quelle que soit l'observation : le rejeu
/// devient alors entièrement prévisible, ce qui permet d'affirmer son issue plutôt que de se
/// contenter de constater qu'il se termine.
class ConstantPolicy : public TrainedPolicy {
public:
    explicit ConstantPolicy(core::PlayerInput input) : _input(input) {}

    std::optional<core::PlayerInput> selectAction(const Tensor<float>& observation,
                                                  ActionDecodingMode mode, Rng& rng) override {
        (void)observation;
        (void)mode;
        (void)rng;
        ++_calls;
        return _input;
    }

    [[nodiscard]] int calls() const noexcept {
        return _calls;
    }

private:
    core::PlayerInput _input;
    int _calls = 0;
};

/// Politique refusant `Argmax` : reproduit la garde défensive de `argmaxRollout`, seul autre
/// chemin d'échec documenté avec le niveau illisible.
class ArgmaxRefusingPolicy : public TrainedPolicy {
public:
    std::optional<core::PlayerInput> selectAction(const Tensor<float>& observation,
                                                  ActionDecodingMode mode, Rng& rng) override {
        (void)observation;
        (void)rng;
        if (mode == ActionDecodingMode::Argmax) {
            return std::nullopt;
        }
        return core::PlayerInput{};
    }

    [[nodiscard]] bool supportsMode(ActionDecodingMode mode) const noexcept override {
        return mode != ActionDecodingMode::Argmax;
    }
};

core::PlayerInput moveRight() {
    return core::PlayerInput{1.0f};
}

}  // namespace

/**
 * @brief Un niveau introuvable rend `nullopt` : erreur récupérable, jamais de plantage.
 * \castest{<b>argmaxRollout : niveau introuvable rend nullopt.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `argmaxRollout` sur un chemin de niveau qui n'existe pas.<br/>
 * \tattendu Aucun résultat, aucune exception, et la politique n'est jamais interrogée.}
 */
TEST(ArgmaxRolloutTest, NiveauIntrouvableRendNullopt) {
    HeadlessLevelEnvironment environment{EnvironmentConfig{}};
    ConstantPolicy policy(moveRight());

    const std::optional<DeterministicReplayResult> result =
        argmaxRollout(policy, environment, std::filesystem::path("niveau_inexistant.json"));

    EXPECT_FALSE(result.has_value());
    // Le niveau est refuse AVANT tout appel a la politique : rien n'est joue a moitie.
    EXPECT_EQ(policy.calls(), 0);
}

/**
 * @brief Une politique qui refuse le mode `Argmax` rend `nullopt` plutôt que de rejouer une
 * séquence arbitraire.
 *
 * Aucun des quatre adaptateurs livrés ne refuse `Argmax` : c'est une garde défensive, et c'est
 * précisément pour cela qu'aucun entraînement réel ne l'exercerait jamais.
 * \castest{<b>argmaxRollout : politique refusant Argmax rend nullopt.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Politique dont `selectAction` rend `nullopt` en mode `Argmax`.<br/>2.
 * `argmaxRollout` sur le niveau trivial.<br/>
 * \tattendu Aucun résultat, aucune exception.}
 */
TEST(ArgmaxRolloutTest, PolitiqueRefusantArgmaxRendNullopt) {
    const TrivialLevelDirectory levelDirectory("argmax_refus");
    HeadlessLevelEnvironment environment{EnvironmentConfig{}};
    ArgmaxRefusingPolicy policy;

    EXPECT_FALSE(argmaxRollout(policy, environment, levelDirectory.levelPath()).has_value());
}

/**
 * @brief Sur le niveau trivial, une politique constante « vers la droite » franchit la sortie :
 * le rejeu rend `Won` et une séquence d'entrées non vide.
 * \castest{<b>argmaxRollout : politique constante vers la droite termine le niveau
 * trivial.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Politique constante rendant un déplacement vers la droite.<br/>2. `argmaxRollout`
 * sur le niveau trivial (entrée et sortie adjacentes).<br/>
 * \tattendu Le rejeu se termine sur `Won`, avec au moins une entrée enregistrée.}
 */
TEST(ArgmaxRolloutTest, PolitiqueConstanteVersLaDroiteTermineLeNiveauTrivial) {
    const TrivialLevelDirectory levelDirectory("argmax_gagne");
    HeadlessLevelEnvironment environment{EnvironmentConfig{}};
    ConstantPolicy policy(moveRight());

    const std::optional<DeterministicReplayResult> result =
        argmaxRollout(policy, environment, levelDirectory.levelPath());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, EpisodeStatus::Won);
    EXPECT_FALSE(result->steps.empty());
    // Une entree enregistree par appel a la politique : le rejeu ne perd ni n'invente de pas.
    EXPECT_EQ(result->steps.size(), static_cast<std::size_t>(policy.calls()));
}

/**
 * @brief Deux rejeux d'une même politique sur un même niveau produisent exactement la même
 * séquence d'entrées et la même récompense.
 *
 * C'est l'invariant qui rend un rejeu exportable : ce que l'entraînement rapporte au jeu est une
 * séquence d'actions, rejouée à l'identique par le moteur. Un rejeu qui varierait d'une exécution
 * à l'autre ne serait pas exportable du tout.
 * \castest{<b>argmaxRollout : deux rejeux identiques à politique et niveau identiques.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux `argmaxRollout` successifs, même politique constante, même niveau.<br/>
 * \tattendu Même issue, même nombre de pas, mêmes entrées terme à terme, même récompense.}
 */
TEST(ArgmaxRolloutTest, DeuxRejeuxIdentiquesDonnentLaMemeSequence) {
    const TrivialLevelDirectory levelDirectory("argmax_determinisme");
    HeadlessLevelEnvironment environment{EnvironmentConfig{}};
    ConstantPolicy policy(moveRight());

    const std::optional<DeterministicReplayResult> first =
        argmaxRollout(policy, environment, levelDirectory.levelPath());
    const std::optional<DeterministicReplayResult> second =
        argmaxRollout(policy, environment, levelDirectory.levelPath());

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(first->status, second->status);
    EXPECT_FLOAT_EQ(first->finalReward, second->finalReward);
    ASSERT_EQ(first->steps.size(), second->steps.size());
    for (std::size_t index = 0; index < first->steps.size(); ++index) {
        EXPECT_FLOAT_EQ(first->steps[index].moveX, second->steps[index].moveX) << "pas " << index;
        EXPECT_EQ(first->steps[index].jumpPressed, second->steps[index].jumpPressed)
            << "pas " << index;
    }
}

/**
 * @brief Une politique qui n'avance jamais s'arrête sur le budget de pas, sans jamais dépasser
 * celui-ci.
 *
 * Le budget est une borne de **terminaison** : c'est lui qui garantit qu'un rejeu se termine
 * toujours, y compris avec une politique qui ne progresse pas.
 * \castest{<b>argmaxRollout : politique immobile bornée par le budget de pas.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Politique constante rendant une entrée neutre (aucun déplacement).<br/>2.
 * `argmaxRollout` avec un budget de pas réduit.<br/>
 * \tattendu Le rejeu se termine (jamais `Ongoing`), sans dépasser le budget de pas.}
 */
TEST(ArgmaxRolloutTest, PolitiqueImmobileBorneeParLeBudgetDePas) {
    constexpr int STEP_BUDGET = 12;
    const TrivialLevelDirectory levelDirectory("argmax_budget");
    HeadlessLevelEnvironment environment{EnvironmentConfig{.maxSteps = STEP_BUDGET}};
    ConstantPolicy policy(core::PlayerInput{});

    const std::optional<DeterministicReplayResult> result =
        argmaxRollout(policy, environment, levelDirectory.levelPath());

    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->status, EpisodeStatus::Ongoing);
    EXPECT_LE(result->steps.size(), static_cast<std::size_t>(STEP_BUDGET));
}
