#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Core/Levels/Decor.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileMap.h"

/**
 * @file Core/Levels/Level.h
 * @brief Niveau chargé : grille de tuiles, entrée/sortie et mécanismes.
 */

namespace core {

/**
 * @brief Liaison d'un **interrupteur** à une **porte**, par positions résolues.
 *
 * Dans le fichier, la liaison est exprimée par identifiant (`switch.id` ↔ `door.opensWith`) ;
 * le chargeur la résout en positions de grille. Le **comportement** (l'interrupteur ou la plaque
 * de pression ouvre la porte) est résolu chaque pas fixe par `core::MechanismController`.
 */
struct Mechanism {
    GridPosition switchPosition;
    GridPosition doorPosition;
};

/**
 * @brief Liaison d'un **interrupteur/plaque de pression** à un **danger commuté**
 *        (`TileType::DangerSwitched`, `EX-GP-052`), par positions résolues.
 *
 * Miroir de `Mechanism`, dupliqué plutôt que généralisé : `Mechanism::doorPosition` est consommé
 * tel quel par une trentaine de sites (`MechanismController`, l'éditeur, les tests aux trois
 * niveaux) pour lesquels la cible est **toujours** une porte — le renommer en un champ générique
 * n'aurait profité qu'à ce second cas d'usage, pour un coût de renommage sans rapport. Dans le
 * fichier, la liaison est exprimée exactement comme `Mechanism` (`switch.id` ↔ `dangerSwitched.
 * opensWith`) ; seul le type de tuile à la position cible distingue les deux résolutions au
 * chargement (`LevelLoader`). Un danger commuté est mortel quand son déclencheur est **actif** —
 * l'inverse d'une porte, qui devient franchissable quand actif.
 */
struct DangerLink {
    GridPosition triggerPosition;
    GridPosition dangerPosition;
};

/// Axe de l'aller-retour d'un danger mobile (`TileType::DangerMover`, `EX-GP-051`).
enum class DangerMoverAxis {
    Horizontal,
    Vertical,
};

/**
 * @brief Paramètres d'un danger mobile (`TileType::DangerMover`, `EX-GP-051`) : axe et portée de
 *        son aller-retour **linéaire**, autour de sa position de départ dans le fichier.
 *
 * `TileMap` ne porte qu'un `TileType` par case (pas de métadonnée numérique, limite déjà actée en
 * `LOT-19`) : ces paramètres vivent dans un vecteur annexe de `Level`, keyé par position, même
 * patron que `Mechanism`/`DangerLink` plutôt qu'une extension de `TileMap`.
 */
struct DangerMoverConfig {
    GridPosition startPosition;
    DangerMoverAxis axis = DangerMoverAxis::Horizontal;
    /// Distance (cases entières) parcourue depuis `startPosition` dans le sens **positif** de
    /// l'axe avant de revenir — pas un aller-retour symétrique de part et d'autre du départ.
    int range = 2;
};

/**
 * @brief Paramètres d'un danger temporisé (`TileType::DangerBlink`, `EX-GP-053`) : période,
 *        déphasage et durée active, en **pas fixes** (`EX-NFR-002`, déterministe).
 *
 * Un déphasage différent par tuile permet des motifs désynchronisés dans un même niveau. Même
 * remarque que `DangerMoverConfig` : vit dans un vecteur annexe de `Level`, pas dans `TileMap`.
 */
struct DangerBlinkConfig {
    GridPosition position;
    /// Longueur totale du cycle, en pas fixes.
    int period = 120;
    /// Décalage initial dans le cycle, en pas fixes.
    int phase = 0;
    /// Portion du cycle (à partir de `phase`) pendant laquelle la tuile est mortelle.
    int activeDuration = 60;
};

/**
 * @brief Texture assignée explicitement à **une case précise**, prioritaire sur le skin de son
 *        type (`EX-EDIT-043`, LOT-42).
 *
 * Même patron que `Mechanism`/`DangerLink`/`DangerMoverConfig`/`DangerBlinkConfig` : vecteur
 * annexe de `Level`, keyé par position, `TileMap` ne portant qu'un `TileType` par case. Le nom
 * d'asset est une simple chaîne : `Core` ne vérifie pas son existence (`EX-NFR-011`), un override
 * pointant un fichier absent reste un niveau valide.
 */
struct TileTextureOverride {
    GridPosition position;
    std::string assetName;
};

/**
 * @brief Niveau complet en mémoire : nom, grille de tuiles, entrée/sortie et mécanismes.
 *
 * Assemblé par le chargeur (après parsing et validation) puis lu par le rendu et, à terme, le
 * gameplay. Donnée pure (`EX-ARCH-011`, `EX-LVL-002`) : aucune dépendance rendu ni fichier.
 */
class Level {
public:
    /**
     * @brief Construit un niveau à partir de ses composantes.
     * @param name         Nom du niveau.
     * @param tileMap      Grille de tuiles typées (déplacée).
     * @param entry        Position d'apparition (case `Entry`).
     * @param exit         Position de sortie (case `Exit`).
     * @param mechanisms   Liaisons interrupteur↔porte résolues.
     * @param jumpBudget   Budget de sauts du tableau (`EX-GP-024`) ; -1 = illimité.
     * @param dashBudget   Budget de dashs du tableau (`EX-GP-024`) ; -1 = illimité.
     * @param dangerLinks  Liaisons interrupteur/plaque ↔ danger commuté résolues (`EX-GP-052`).
     * @param moverConfigs Paramètres des dangers mobiles (`EX-GP-051`), un par tuile `DangerMover`.
     * @param blinkConfigs Paramètres des dangers temporisés (`EX-GP-053`), un par tuile
     *                     `DangerBlink`.
     * @param background   Nom de l'asset de fond du niveau (`EX-REN-044`), vide si aucun. Une
     *                     chaîne, jamais un handle de texture : `Core` ignore tout du rendu.
     * @param skinSet      Nom du jeu de skins du niveau (`EX-EDIT-024`), vide pour le jeu par
     *                     défaut.
     * @param textureOverrides Textures assignées par instance (`EX-EDIT-043`), prioritaires sur
     *                     le skin de leur type.
     * @param decors       Décors libres du niveau (`EX-DEC-001`, LOT-49), dans leur ordre de
     *                     superposition intra-couche.
     */
    Level(std::string name, TileMap tileMap, GridPosition entry, GridPosition exit,
          std::vector<Mechanism> mechanisms, int jumpBudget = -1, int dashBudget = -1,
          std::vector<DangerLink> dangerLinks = {},
          std::vector<DangerMoverConfig> moverConfigs = {},
          std::vector<DangerBlinkConfig> blinkConfigs = {},
          std::optional<std::string> background = std::nullopt,
          std::optional<std::string> skinSet = std::nullopt,
          std::vector<TileTextureOverride> textureOverrides = {},
          std::vector<Decor> decors = {})
        : _name(std::move(name)),
          _tileMap(std::move(tileMap)),
          _entry(entry),
          _exit(exit),
          _mechanisms(std::move(mechanisms)),
          _jumpBudget(jumpBudget),
          _dashBudget(dashBudget),
          _dangerLinks(std::move(dangerLinks)),
          _moverConfigs(std::move(moverConfigs)),
          _blinkConfigs(std::move(blinkConfigs)),
          _background(std::move(background)),
          _skinSet(std::move(skinSet)),
          _textureOverrides(std::move(textureOverrides)),
          _decors(std::move(decors)) {}

    /// @return Le nom du niveau.
    [[nodiscard]] const std::string& name() const noexcept {
        return _name;
    }

    /// @return La grille de tuiles du niveau.
    [[nodiscard]] const TileMap& tileMap() const noexcept {
        return _tileMap;
    }

    /// @return La position d'apparition.
    [[nodiscard]] GridPosition entry() const noexcept {
        return _entry;
    }

    /// @return La position de sortie.
    [[nodiscard]] GridPosition exit() const noexcept {
        return _exit;
    }

    /// @return Les liaisons de mécanismes du niveau.
    [[nodiscard]] const std::vector<Mechanism>& mechanisms() const noexcept {
        return _mechanisms;
    }

    /// @return Budget de **sauts** du tableau (`EX-GP-024`) ; **-1 = illimité**.
    [[nodiscard]] int jumpBudget() const noexcept {
        return _jumpBudget;
    }

    /// @return Budget de **dashs** du tableau (`EX-GP-024`) ; **-1 = illimité**.
    [[nodiscard]] int dashBudget() const noexcept {
        return _dashBudget;
    }

    /// @return Les liaisons interrupteur/plaque ↔ danger commuté du niveau (`EX-GP-052`).
    [[nodiscard]] const std::vector<DangerLink>& dangerLinks() const noexcept {
        return _dangerLinks;
    }

    /// @return Les paramètres des dangers mobiles du niveau (`EX-GP-051`).
    [[nodiscard]] const std::vector<DangerMoverConfig>& moverConfigs() const noexcept {
        return _moverConfigs;
    }

    /// @return Les paramètres des dangers temporisés du niveau (`EX-GP-053`).
    [[nodiscard]] const std::vector<DangerBlinkConfig>& blinkConfigs() const noexcept {
        return _blinkConfigs;
    }

    /// @return Le nom de l'asset de fond du niveau (`EX-REN-044`), absent si aucun n'est
    /// configuré. Une chaîne, jamais un handle : `Core` n'a pas accès au dossier d'assets.
    [[nodiscard]] const std::optional<std::string>& background() const noexcept {
        return _background;
    }

    /// @return Le nom du jeu de skins du niveau (`EX-EDIT-024`), absent si le niveau utilise le
    /// jeu par défaut.
    [[nodiscard]] const std::optional<std::string>& skinSet() const noexcept {
        return _skinSet;
    }

    /// @return Les textures assignées par instance du niveau (`EX-EDIT-043`).
    [[nodiscard]] const std::vector<TileTextureOverride>& textureOverrides() const noexcept {
        return _textureOverrides;
    }

    /// @return Les décors libres du niveau (`EX-DEC-001`), dans leur ordre de superposition
    /// intra-couche.
    [[nodiscard]] const std::vector<Decor>& decors() const noexcept {
        return _decors;
    }

private:
    std::string _name;
    TileMap _tileMap;
    GridPosition _entry;
    GridPosition _exit;
    std::vector<Mechanism> _mechanisms;
    int _jumpBudget = -1;
    int _dashBudget = -1;
    std::vector<DangerLink> _dangerLinks;
    std::vector<DangerMoverConfig> _moverConfigs;
    std::vector<DangerBlinkConfig> _blinkConfigs;
    std::optional<std::string> _background;
    std::optional<std::string> _skinSet;
    std::vector<TileTextureOverride> _textureOverrides;
    std::vector<Decor> _decors;
};

}  // namespace core
