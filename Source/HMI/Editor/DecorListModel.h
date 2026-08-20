// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Levels/Decor.h"

/**
 * @file HMI/Editor/DecorListModel.h
 * @brief Construction des lignes de la section « Décors » du panneau « Textures » (`LOT-50`
 *        TACHE-04), pure et testable.
 */

namespace hmi {

/// Une ligne de la section « Décors » : un décor du niveau courant, avec son rang d'origine
/// (identité stable pour la sélection croisée et les actions, `core::LevelDraft::decors()`).
struct DecorListRow {
    /// Rang dans `core::LevelDraft::decors()` — jamais recalculé après tri : c'est l'identité de
    /// la ligne, utilisée par les actions (réordonner, changer de couche, supprimer, centrer).
    std::size_t index = 0;
    std::string assetName;
    core::DecorLayer layer = core::DecorLayer::Decor;
    /// `true` si `assetName` ne figure pas parmi les assets connus (`Assets/Decors/`) — signale
    /// dans la liste ce que le damier magenta signale déjà dans le canevas (`EX-NFR-040`).
    bool assetMissing = false;
};

/**
 * @brief Construit les lignes de la section « Décors », groupées par couche puis dans l'ordre de
 *        superposition.
 *
 * Fonction **pure**, testable sans Qt (`EX-NFR-010`) : trie @p decors par couche
 * (`core::DecorLayer::Background` puis `Decor` puis `Foreground`, l'ordre de déclaration de
 * l'énumération) et, à couche égale, préserve l'ordre d'origine du vecteur — c'est l'ordre de
 * superposition (`LOT-49` TACHE-01, rang croissant = dessiné par-dessus). C'est ce regroupement
 * qui rend le réordonnancement compréhensible : sans lui, la liste mélangerait des décors sans
 * rapport de superposition entre eux.
 * @param decors         Décors du niveau courant, dans leur ordre (`core::LevelDraft::decors()`).
 * @param availableAssets Noms des assets effectivement présents dans `Assets/Decors/` (balayage du
 *                        dossier, résolu par l'appelant — cette fonction reste pure) ; un décor
 *                        dont l'asset n'y figure pas est marqué `assetMissing`.
 * @return Les lignes, groupées par couche puis triées par ordre de superposition.
 */
[[nodiscard]] std::vector<DecorListRow> buildDecorListRows(
    const std::vector<core::Decor>& decors, const std::vector<std::string>& availableAssets);

}  // namespace hmi
