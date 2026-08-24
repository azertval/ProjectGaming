// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileMap.h"

/**
 * @file AiSolver/Env/GridDistanceField.h
 * @brief Champ de distances de plus court chemin sur la grille, respectant les murs (amendement
 * `LOT-ANNEXE-08`, `EX-IA-023`).
 */

namespace aisolver {

/**
 * @brief Distance de plus court chemin en nombre de cases (4-connexité, respectant les cases
 * statiquement solides d'un `core::TileMap`), pré-calculée une fois par BFS multi-source depuis une
 * case cible.
 *
 * Remplace la distance euclidienne en ligne droite (décision de cadrage initiale de
 * `LOT-ANNEXE-08`) comme base de la récompense de progression : la distance euclidienne pousse un
 * agent à s'approcher en ligne droite de la sortie même quand un mur force un détour, ce qui donne
 * une récompense négative aux pas de détour pourtant nécessaires -- un signal qui combat activement
 * la bonne politique plutôt que de simplement l'ignorer. `GridDistanceField` corrige cela sans
 * introduire de recherche de chemin complète (pas de replanification par pas, pas de notion de coût
 * autre que le nombre de cases) : un unique BFS à la construction, puis des lectures `O(1)`.
 *
 * Cases dynamiques (portes, plateformes mobiles) non prises en compte : seule la solidité statique
 * (`core::TileMap::isSolid`) borne le BFS, cohérent avec la portée de la récompense partagée
 * (`LOT-ANNEXE-08`), qui ne modélise déjà aucun mécanisme spécifique à un niveau.
 */
class GridDistanceField {
public:
    /**
     * @brief Calcule le champ par BFS multi-source à partir de @p target.
     * @param tileMap Grille statique du niveau (cases solides/non-solides).
     * @param target  Case cible (typiquement `core::Level::exit()`), point de départ du BFS.
     */
    GridDistanceField(const core::TileMap& tileMap, const core::GridPosition& target);

    /**
     * @brief Distance de plus court chemin, en nombre de cases, depuis @p position jusqu'à la case
     * cible passée au constructeur.
     * @param position Case dont on veut la distance ; hors-grille traitée comme inatteignable.
     * @return La distance en cases, ou la sentinelle `largeur * hauteur` (borne supérieure stricte
     *         de toute distance atteignable sur la grille) si @p position est hors-grille ou ne
     * peut atteindre la cible par aucun chemin de cases non solides.
     */
    [[nodiscard]] int distance(const core::GridPosition& position) const noexcept;

private:
    int _width;
    int _height;
    std::vector<int> _distances;
};

}  // namespace aisolver
