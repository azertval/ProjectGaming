// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/CameraFraming.h"
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
     * @brief Écrit un niveau au format JSON dans un fichier (`EX-EDIT-006`).
     *
     * Écriture directe (`std::ofstream`), sans exception vers l'appelant : un échec (chemin
     * invalide, permissions) est signalé par la valeur de retour, jamais par une exception
     * (`EX-NFR-040`), symétrique à `LevelLoader::loadFromFile`.
     * @param level Niveau à écrire (déjà validé).
     * @param path  Chemin du fichier de destination ; le dossier parent doit exister.
     * @return `true` si l'écriture a réussi.
     */
    [[nodiscard]] static bool saveToFile(const Level& level, const std::filesystem::path& path);

    /**
     * @brief Construit le JSON à partir des composantes brutes d'un niveau (utilisé également
     *        par `LevelDraft::toLevel()`, qui n'a pas nécessairement de `Level` construit).
     * @param name         Nom du niveau.
     * @param tileMap      Grille de tuiles (source de vérité des positions entrée/sortie/
     *                     mécanismes : une tuile `Entry`/`Exit`/`Switch`/`Door` présente dans la
     *                     grille est émise, qu'elle soit ou non reliée/complète).
     * @param mechanisms   Liaisons interrupteur↔porte à réexprimer par identifiant.
     * @param jumpBudget   Budget de sauts (`-1` = illimité, omis du JSON dans ce cas).
     * @param dashBudget   Budget de dashs (`-1` = illimité, omis du JSON dans ce cas).
     * @param dangerLinks  Liaisons interrupteur↔danger commuté à réexprimer par identifiant
     *                     (`EX-GP-052`), même schéma que @p mechanisms.
     * @param moverConfigs Configurations explicites de dangers mobiles (`EX-GP-051`) ; une tuile
     *                     `DangerMover` sans entrée correspondante est émise sans champs
     *                     `axis`/`range` (valeurs de conception par défaut au rechargement).
     * @param blinkConfigs Configurations explicites de dangers temporisés (`EX-GP-053`), même
     *                     remarque que @p moverConfigs pour les champs `period`/`phase`/
     *                     `activeDuration`.
     * @param background   Nom de l'asset de fond (`EX-REN-044`), omis du JSON si absent.
     * @param skinSet      Nom du jeu de skins du niveau (`EX-EDIT-024`), omis du JSON si absent.
     * @param textureOverrides Textures assignées par instance (`EX-EDIT-043`) ; une tuile sans
     *                     override correspondant est émise sans champ `"texture"`.
     * @param planes       Plans picturaux du niveau (`EX-DEC-040`, LOT-69), émis dans le tableau
     *                     racine optionnel `"planes"`, omis si vide. Chaque champ à sa valeur par
     *                     défaut est omis (convention du `LOT-67`).
     * @param parallaxEnabled Drapeau de parallaxe du niveau (`EX-DEC-043`) ; omis quand il vaut
     *                     `true`, sa valeur par défaut.
     * @param platformConfigs Configurations explicites de plateformes mobiles (`EX-GP-026`), même
     *                     remarque que @p moverConfigs pour les champs `waypoints`/`mode`/`speed`/
     *                     `phase`. La route est toujours écrite en `waypoints` : le couple
     *                     `endX`/`endY` d'avant le multi-points reste **lu** par `LevelLoader`
     *                     mais n'est plus jamais produit (`EX-LVL-008`).
     * @param cameraFraming Cadrage de caméra résolu (`EX-LVL-006`) ; le champ `"cameraFraming"`
     *                     n'est émis que s'il **diverge** de ce que la règle de repli
     *                     (`resolveCameraFraming`) recalculerait pour ces dimensions -- un niveau
     *                     dont le cadrage résolu coïncide avec le repli reste sans le champ, comme
     *                     avant ce lot (`EX-LVL-006`, aucune régression de round-trip).
     * @param airJumps     Sauts aériens accordés par le tableau (`EX-GP-055`) ; champ omis si
     *                     absent (le niveau s'en remet alors au réglage du moteur).
     * @param dashCharges  Charges de dash accordées par le tableau (`EX-GP-055`) ; même règle.
     * @return Le contenu JSON correspondant.
     */
    [[nodiscard]] static std::string buildJson(
        const std::string& name, const TileMap& tileMap, const std::vector<Mechanism>& mechanisms,
        int jumpBudget, int dashBudget, const std::vector<DangerLink>& dangerLinks = {},
        const std::vector<DangerMoverConfig>& moverConfigs = {},
        const std::vector<DangerBlinkConfig>& blinkConfigs = {},
        const std::optional<std::string>& background = std::nullopt,
        const std::optional<std::string>& skinSet = std::nullopt,
        const std::vector<TileTextureOverride>& textureOverrides = {},
        const std::vector<MovingPlatformConfig>& platformConfigs = {},
        const CameraFramingConfig& cameraFraming = {},
        const std::optional<int>& airJumps = std::nullopt,
        const std::optional<int>& dashCharges = std::nullopt, const std::vector<Plane>& planes = {},
        bool parallaxEnabled = true);
};

}  // namespace core
