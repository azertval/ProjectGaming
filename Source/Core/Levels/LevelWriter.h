#pragma once

#include <string>
#include <vector>

#include "Core/Levels/Level.h"

/**
 * @file Core/Levels/LevelWriter.h
 * @brief Sérialisation d'un niveau vers le format JSON, symétrique à `LevelLoader`.
 */

namespace core {

class TileMap;

/**
 * @brief Sérialise un niveau vers le format JSON défini par `EX-LVL-003`.
 *
 * Fonction **pure**, symétrique à `LevelLoader::loadFromString` : recharger la chaîne produite
 * reconstruit un niveau équivalent (mêmes tuiles, entrée/sortie, mécanismes, budgets),
 * `EX-EDIT-011`. Les identifiants d'interrupteurs (`id`/`opensWith`) ne sont pas conservés du
 * fichier d'origine — ni `Level` ni `LevelDraft` ne les retiennent après chargement — ils sont
 * **régénérés** de façon déterministe (balayage de la grille ligne par ligne), sans effet sur la
 * sémantique du niveau rechargé.
 */
class LevelWriter {
public:
    /**
     * @brief Sérialise un niveau déjà construit et validé.
     * @param level Niveau à sérialiser.
     * @return Le contenu JSON du niveau.
     */
    [[nodiscard]] static std::string toJsonString(const Level& level);

    /**
     * @brief Construit le JSON à partir des composantes brutes d'un niveau (utilisé également
     *        par `LevelDraft::toLevel()`, qui n'a pas nécessairement de `Level` construit).
     * @param name       Nom du niveau.
     * @param tileMap    Grille de tuiles (source de vérité des positions entrée/sortie/
     *                   mécanismes : une tuile `Entry`/`Exit`/`Switch`/`Door` présente dans la
     *                   grille est émise, qu'elle soit ou non reliée/complète).
     * @param mechanisms Liaisons interrupteur↔porte à réexprimer par identifiant.
     * @param jumpBudget Budget de sauts (`-1` = illimité, omis du JSON dans ce cas).
     * @param dashBudget Budget de dashs (`-1` = illimité, omis du JSON dans ce cas).
     * @return Le contenu JSON correspondant.
     */
    [[nodiscard]] static std::string buildJson(const std::string& name, const TileMap& tileMap,
                                               const std::vector<Mechanism>& mechanisms,
                                               int jumpBudget, int dashBudget);
};

}  // namespace core
