#pragma once

#include <cstddef>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/PlatformSample.h"

/**
 * @file Core/Gameplay/BlockController.h
 * @brief Comportement des blocs poussables : position, poussée et chute (`EX-GP-022`).
 */

namespace core {

struct Aabb;

/**
 * @brief Fait vivre les **blocs poussables** d'un niveau (`TileType::Block`/`BlockHalf`/
 *        `BlockQuarter`, `EX-GP-022`/`EX-GP-005`).
 *
 * Logique **pure** (aucun rendu ni fenêtre), sur le modèle de `MechanismController` : chaque bloc
 * occupe exactement **une case**, sans position continue — pousser ou tomber le déplace d'une
 * case entière, jamais à mi-chemin, **quelle que soit sa taille** (`EX-GP-005` ne change que la
 * boîte de collision **testée contre le personnage**, jamais la logique de déplacement). Deux
 * comportements, résolus chaque pas fixe :
 * - **Poussée** : si la boîte (réelle, réduite pour `BlockHalf`/`BlockQuarter`) du bloc touche
 *   celle du personnage du côté vers lequel il se déplace et que la case suivante dans cette
 *   direction est libre (ni solide, ni un autre bloc, ni une pente/arrondi — voir `isFree`), le
 *   bloc avance d'une case.
 * - **Chute** : un bloc dont la case du dessous est libre tombe d'une case, au rythme de
 *   `FALL_INTERVAL_STEPS` pas fixes par case (chute discrète plutôt que continue — cohérent avec
 *   le reste des mécanismes de ce moteur, tous résolus case par case).
 *
 * **Pentes/arrondis** (`EX-GP-003`/`EX-GP-004`/`EX-GP-006`, sol ou plafond) : ce contrôleur n'a
 * **aucune** notion de suivi de surface (contrairement au personnage, `core::resolveSlopeFollow`)
 * — une case de pente/arrondi est donc traitée comme un **obstacle simple**, comme un `Solid`
 * ordinaire (ni poussable dedans, ni traversée en tombant), plutôt que comme une case libre. Sans
 * cette règle, un bloc glisserait au travers d'une pente au lieu de s'y arrêter.
 *
 * Le contrôleur ne connaît pas les mécanismes interrupteur/porte : `collisionMap()` complète une
 * grille de collision **déjà résolue** par `MechanismController` (portes ouvertes/fermées) avec
 * la position **courante** des blocs **pleins** (`Block`), sans jamais modifier la carte statique
 * du `Level`. Les blocs **réduits** (`BlockHalf`/`BlockQuarter`) ne sont **jamais** marqués solides
 * dans cette grille — leur case doit rester franchissable **autour** d'eux (`EX-GP-005`) ; leur
 * collision réelle avec le personnage est résolue à part, par `core::sweepAabbVsAabb`
 * (@ref guide-physique), composée par l'appelant (`hmi::GameSession`) après le balayage sur grille.
 *
 * **Support mouvant** (`TileType::MovingPlatform`, `EX-GP-026`, optionnel via `update`) : un bloc
 * qui repose sur une plateforme mobile ne tombe jamais (support valide comme le sol) et est
 * **porté** avec elle — le déplacement continu de la plateforme est accumulé (`core::Vector2` par
 * bloc) et converti en poussée d'une case entière dès que l'accumulateur atteint `1.0`, exactement
 * comme le reste de ce contrôleur (une case à la fois, jamais de position infra-case pour un
 * bloc). Le reliquat infra-case est conservé d'un pas à l'autre, pas perdu.
 */
class BlockController {
public:
    /// Nombre de pas fixes qu'un bloc non soutenu met à tomber d'une case (chute discrète).
    static constexpr int FALL_INTERVAL_STEPS = 6;

    /// Construit le contrôleur : repère chaque tuile de bloc (`Block`/`BlockHalf`/`BlockQuarter`)
    /// du niveau comme un bloc mobile, avec son facteur de taille (`1`/`0.5`/`0.25`).
    explicit BlockController(const Level& level);

    /**
     * @brief Met à jour les blocs pour un pas : tente une poussée, porte les blocs posés sur une
     *        plateforme mobile, puis fait avancer les chutes.
     * @param playerBox     Boîte englobante du personnage (position **avant** le pas physique
     *                      courant : la poussée doit libérer la case avant que la physique ne
     *                      résolve le déplacement du personnage sur ce même pas).
     * @param moveIntentX   Intention de déplacement horizontal du personnage (`PlayerInput::moveX`,
     *                      dans `[-1, 1]`) ; `0` ne pousse aucun bloc.
     * @param baseCollision Grille de collision déjà résolue par les mécanismes (portes), utilisée
     *                      pour savoir si la case visée par une poussée ou une chute est libre.
     * @param platforms     Échantillons de position des plateformes mobiles pour ce pas
     *                      (`core::PlatformController::samples`) ; vide par défaut (comportement
     *                      inchangé pour tout niveau sans plateforme).
     */
    void update(const Aabb& playerBox, float moveIntentX, const TileMap& baseCollision,
                const std::vector<PlatformSample>& platforms = {});

    /// @return Une copie de @p base où chaque case occupée par un bloc **plein** (`Block`) est
    ///         rendue solide. Les blocs réduits (`BlockHalf`/`BlockQuarter`) restent francs dans
    ///         cette grille (voir en-tête de la classe) — utiliser `boxAt`/`scales` pour leur
    ///         collision réelle.
    [[nodiscard]] TileMap collisionMap(const TileMap& base) const;

    /// @return Les positions courantes des blocs (pour le rendu : une entité-tuile par bloc).
    [[nodiscard]] const std::vector<GridPosition>& positions() const noexcept {
        return _positions;
    }

    /// @return Le facteur de taille de chaque bloc, même index que `positions()` (`1` = `Block`,
    ///         `0.5` = `BlockHalf`, `0.25` = `BlockQuarter`).
    [[nodiscard]] const std::vector<float>& scales() const noexcept {
        return _scales;
    }

    /// @return La boîte de collision **réelle** du bloc d'indice @p index à sa position courante :
    ///         pleine case si `scales()[index] == 1`, sinon centrée et réduite (`EX-GP-005`).
    [[nodiscard]] Aabb boxAt(std::size_t index) const;

    /**
     * @brief Masse du bloc d'indice @p index, pour les mécanismes à activation continue
     *        (`EX-GP-025`, `LOT-65` TACHE-06).
     *
     * Vaut le **facteur de taille** du bloc (`scales()`), et non son aire : un bloc plein pèse
     * `1`, un demi `0.5`, un quart `0.25`. Le seuil des plaques de pression étant calé sur la masse
     * par défaut du personnage, cela donne une règle lisible pour le level designer — **seul un
     * bloc plein enfonce une plaque** — et un levier d'énigme : distinguer les caisses par leur
     * taille plutôt que par une propriété invisible. Source unique de cette règle : les deux
     * appelants qui composent le pas fixe (`hmi::GameSession` et le test système) la lisent ici
     * plutôt que de la recalculer chacun de son côté.
     * @param index Indice du bloc, même ordre que `positions()`.
     * @return La masse du bloc, comparable au seuil des plaques.
     */
    [[nodiscard]] float massAt(std::size_t index) const {
        return _scales[index];
    }

private:
    /// @return true si @p target est dans les bornes, non solide et non pente/arrondi
    ///         (`core::isFollowableSurface`/`core::isCeilingSlope`, voir en-tête de la classe)
    ///         dans @p base, et non occupée par un bloc autre que celui d'indice @p excluding.
    [[nodiscard]] bool isFree(GridPosition target, const TileMap& base,
                              std::size_t excluding) const;
    /// Étape 1 de update() : pousse chaque bloc touché du côté du déplacement voulu si sa case
    /// suivante est libre.
    void pushBlocks(const Aabb& playerBox, float moveIntentX, const TileMap& base);
    /// Étape 2 de update() : porte chaque bloc reposant sur une plateforme mobile (accumulateur
    /// infra-case, converti en poussée d'une case entière à `1.0`) ; supprime aussi la chute pour
    /// ce pas (support valide).
    void carryBlocksOnPlatforms(const TileMap& base, const std::vector<PlatformSample>& platforms);
    /// Étape 3 de update() : fait tomber d'une case chaque bloc non soutenu depuis
    /// `FALL_INTERVAL_STEPS` pas.
    void dropBlocks(const TileMap& base);

    std::vector<GridPosition> _positions;
    std::vector<float> _scales;    ///< Facteur de taille (`1`/`0.5`/`0.25`), même index.
    std::vector<int> _fallTimers;  ///< Pas écoulés depuis que ce bloc est non soutenu.
    std::vector<Vector2> _platformCarryAccumulator;  ///< Déplacement infra-case accumulé par la
                                                     ///< plateforme qui porte ce bloc, même index ;
                                                     ///< remis à zéro dès que le bloc n'est plus
                                                     ///< porté.
};

}  // namespace core
