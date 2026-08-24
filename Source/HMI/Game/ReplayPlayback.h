// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Core/Physics/PlayerInput.h"

/**
 * @file HMI/Game/ReplayPlayback.h
 * @brief Lecture d'un fichier de rejeu produit par le programme Lot-Annexe, comme source de
 * `core::PlayerInput` pour `hmi::GameSession` (`LOT-ANNEXE-18`, TACHE-01, `EX-IA-019`).
 *
 * Ne connaît jamais `hmi::InputState` ni les bindings clavier/manette (un rejeu **est** déjà
 * l'intention enregistrée, il n'y a rien à traduire) et ne référence **jamais**
 * `Source/AiSolver/Nn`, `Autodiff`, `Optim` ni aucun module de `Source/AiSolver/Training` — décision
 * transverse du programme, rappelée par l'épic de ce lot : `HMI` rejoue une séquence déjà figée,
 * aucune inférence de réseau de neurones n'a lieu ici ni ailleurs dans `HMI`/`Core`.
 */

namespace hmi {

/**
 * @brief Charge et valide un fichier de rejeu, puis fournit sa séquence de `core::PlayerInput`, un
 *        pas fixe à la fois, en lecture seule stricte (ne modifie jamais le fichier de rejeu).
 *
 * La validation (`aisolver::validateReplay`, `LOT-ANNEXE-17`) s'exécute une fois, à la construction
 * — jamais contournable : un rejeu dont le niveau référencé a changé depuis l'export est refusé
 * avant qu'aucun pas ne soit joué (`valid()` renvoie `false`, `error()` porte le message).
 */
class ReplayPlayback {
public:
    /**
     * @brief Charge le rejeu à @p replayPath et le valide contre @p levelsDir.
     *
     * Ne lève jamais d'exception (`EX-NFR-040`) : un échec de lecture ou de validation laisse
     * l'objet dans un état neutre (`valid()` faux, `nextInput()` renvoie toujours `std::nullopt`),
     * interrogeable par l'appelant avant tout usage.
     * @param replayPath Chemin du fichier de rejeu à jouer.
     * @param levelsDir  Répertoire sous lequel résoudre le niveau référencé par le rejeu.
     */
    ReplayPlayback(const std::filesystem::path& replayPath, const std::filesystem::path& levelsDir);

    /// @return `true` si le rejeu a été chargé et validé avec succès.
    [[nodiscard]] bool valid() const noexcept {
        return _valid;
    }

    /// @return Message d'erreur explicite si `!valid()`, chaîne vide sinon.
    [[nodiscard]] const std::string& error() const noexcept {
        return _error;
    }

    /// @return Chemin du niveau référencé (relatif à `levelsDir`), vide si `!valid()`.
    [[nodiscard]] const std::string& levelPath() const noexcept {
        return _levelPath;
    }

    /**
     * @brief Renvoie le prochain `core::PlayerInput` de la séquence enregistrée.
     * @return Le prochain pas, ou `std::nullopt` une fois la séquence épuisée (signal de fin de
     *         rejeu, distinct d'une fin de partie par victoire/défaite) — toujours `std::nullopt`
     *         si `!valid()`.
     */
    [[nodiscard]] std::optional<core::PlayerInput> nextInput();

private:
    bool _valid = false;
    std::string _error;
    std::string _levelPath;
    std::vector<core::PlayerInput> _steps;
    std::size_t _nextIndex = 0;
};

}  // namespace hmi
