// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Env/GridDistanceField.h"
#include "AiSolver/Math/Tensor.h"
#include "Core/Levels/GridPosition.h"

/**
 * @file AiSolver/Env/ObjectiveEncoder.h
 * @brief Gradient local du champ d'objectif, rendu visible à l'agent.
 */

namespace aisolver {

/**
 * @brief Encode, autour de la case du personnage, le **champ de distances vers l'objectif
 *        immédiat** sur lequel la récompense est déjà construite.
 *
 * Sans ce vecteur, la récompense mesure une quantité que l'observation ne contient pas : l'agent
 * est payé pour se rapprocher d'un but dont rien ne lui dit la direction. Sur un niveau linéaire
 * « aller à droite » suffit à le retrouver ; sur un niveau à plusieurs salles, deux passages
 * opposés dans le **même** couloir produisent la même fenêtre de tuiles et la même cinématique,
 * et exigent des actions contraires — une politique sans mémoire ne peut pas les distinguer.
 * Le gradient du champ, lui, les distingue : il pointe vers la clé à l'aller, vers la porte au
 * retour. Il rend aussi l'inventaire implicite, le champ changeant de cible dès qu'une clé est
 * ramassée.
 *
 * Ce n'est pas une solution soufflée à l'agent : le champ ne connaît ni la physique, ni les
 * dangers, ni l'ordre de résolution attendu — il ignore jusqu'à la gravité. Il dit « par où le
 * chemin de cases est plus court », pas « quoi faire ».
 *
 * Composantes, dans cet ordre documenté (indices `0`..`size() - 1`) :
 *   0. `1.0f` si l'objectif est atteignable depuis la case du personnage, `0.0f` sinon (une
 *      distance sentinelle et une distance réelle ne se lisent pas de la même façon).
 *   1. Distance normalisée à l'objectif, `distance / (largeur × hauteur)`, `1.0f` si inatteignable.
 *   2. Gradient vers le **haut** : `d(centre) − d(voisin)`, écrêté à `[-1, 1]`, `0.0f` si l'un des
 *      deux est inatteignable — positif quand ce voisin rapproche du but.
 *   3. Gradient vers le **bas**, même convention.
 *   4. Gradient vers la **gauche**, même convention.
 *   5. Gradient vers la **droite**, même convention.
 *
 * L'écrêtage à `[-1, 1]` n'est pas cosmétique : sur une grille à quatre voisins, la distance entre
 * deux cases adjacentes ne varie jamais de plus de `1` — sauf en bordure d'une zone inatteignable,
 * où l'écart vaut la sentinelle entière. L'écrêtage borne cette seule discontinuité.
 */
class ObjectiveEncoder {
public:
    /**
     * @brief Encode le voisinage du champ d'objectif autour de @p center.
     * @param field  Champ de distances vers l'objectif immédiat
     *               (`HeadlessLevelEnvironment::objectiveField`).
     * @param center Case de grille où se trouve le personnage.
     * @return Vecteur (tenseur de rang 1) de taille `size()`.
     */
    [[nodiscard]] Tensor<float> encode(const GridDistanceField& field,
                                       core::GridPosition center) const;

    /// @return La taille fixe du vecteur produit par `encode` (`6`, voir l'ordre documenté
    /// ci-dessus).
    [[nodiscard]] static constexpr int size() noexcept {
        return OBJECTIVE_STATE_SIZE;
    }

    static constexpr int OBJECTIVE_STATE_SIZE = 6;
};

}  // namespace aisolver
