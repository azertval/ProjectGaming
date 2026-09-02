// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlatformSample.h"

/**
 * @file Core/Gameplay/SinkingBlockController.h
 * @brief Comportement des blocs descendants : armement au contact puis descente continue
 *        (`EX-GP-027`).
 */

namespace core {

/**
 * @brief Fait vivre les **blocs descendants** d'un niveau (`TileType::SinkingBlock`,
 *        `EX-GP-027`).
 *
 * Logique **pure** (aucun rendu ni fenêtre), calquée sur `core::PlatformController` plutôt que sur
 * `core::BlockController` : la position d'un bloc descendant est **continue** (jamais alignée sur
 * la grille), et il **porte** ce qui repose dessus.
 *
 * Ce contrôleur ne résout **aucune** collision lui-même. Il n'expose que des
 * `core::PlatformSample` — exactement le type que consomment déjà la physique du personnage
 * (`core::CharacterPhysicsSystem`) et les blocs poussables (`core::BlockController`) pour les
 * plateformes mobiles. L'appelant (`hmi::GameSession`) concatène ces échantillons à ceux des
 * plateformes, et le bloc descendant hérite alors **sans une ligne de code de collision** du
 * portage du personnage, de la collision continue boîte-contre-boîte (`core::sweepAabbVsAabb`),
 * du portage des blocs poussables posés dessus et de l'interpolation d'affichage. L'écrasement
 * mortel contre le sol (`EX-GP-031`) suit le même chemin que celui d'une plateforme montante
 * contre un plafond (`Player::squished`, `EX-GP-026`), sans traitement dédié.
 *
 * Trois règles, toutes résolues au pas fixe :
 * - **Armement** : le premier contact avec le personnage, par n'importe quelle face (dessus, côté
 *   ou dessous — jamais un test de portage, qui obligerait à définir un seuil de « vraiment posé
 *   dessus »), arme le bloc. L'armement est un **aller simple** : s'éloigner ne l'annule pas.
 * - **Descente** : à vitesse constante (`SINK_SPEED_CELLS_PER_SECOND`), position fonction du seul
 *   nombre de pas écoulés **depuis l'armement** — jamais d'une accumulation (`EX-NFR-002`).
 * - **Arrêt et sortie** : le bloc s'arrête définitivement contre la première case pleine sous lui
 *   (il ne repart pas si elle se libère), et il est **retiré** s'il franchit le bord bas du
 *   tableau — il cesse alors d'émettre tout échantillon, donc de porter et de bloquer quoi que ce
 *   soit.
 *
 * Un bloc **non armé** émet lui aussi son échantillon, immobile : sans quoi il ne serait pas
 * solide (`core::isSolid` vaut `false` pour ce type, comme pour `MovingPlatform`) et le personnage
 * lui passerait au travers avant d'avoir pu l'armer.
 */
class SinkingBlockController {
public:
    /// Vitesse de descente, en cases par seconde. Constante du **moteur** et non donnée de niveau
    /// (`EX-GP-027`) : le constructeur de `core::Level` atteint déjà 19 paramètres et sa surface
    /// est actée comme maximale. L'ouvrir par tuile plus tard n'invalidera aucun fichier existant.
    static constexpr float SINK_SPEED_CELLS_PER_SECOND = 2.5F;

    /// Tolérance de contact pour l'armement : deux boîtes qui se **touchent** (bords jointifs)
    /// arment, sans exiger un chevauchement d'aire strictement positive. Sans elle, se poser sur
    /// un bloc — le cas le plus courant — ne l'armerait jamais, les bords étant exactement
    /// alignés. Même ordre de grandeur que `BlockController::PUSH_TOUCH_TOLERANCE`.
    static constexpr float CONTACT_TOLERANCE = 0.05F;

    /// Construit le contrôleur : repère chaque tuile `SinkingBlock` de @p level comme un bloc
    /// descendant, à sa position de fichier, non armé.
    explicit SinkingBlockController(const Level& level);

    /**
     * @brief Met à jour les blocs pour un pas : arme ceux que le personnage touche, puis fait
     *        descendre ceux qui sont armés.
     * @param playerBox     Boîte du personnage **avant** le pas physique courant (même convention
     *                      que `core::BlockController::update`).
     * @param baseCollision Grille de collision déjà résolue par les mécanismes et les blocs
     *                      poussables, utilisée pour savoir où un bloc doit s'arrêter.
     */
    void update(const Aabb& playerBox, const TileMap& baseCollision);

    /// @return Le nombre de blocs descendants du niveau (retirés compris : les indices sont
    ///         stables sur toute la durée d'un tableau, pour que l'affichage puisse s'y référer).
    [[nodiscard]] std::size_t count() const noexcept {
        return _blocks.size();
    }

    /// @return La position de départ, telle qu'écrite dans le fichier, du bloc @p index.
    [[nodiscard]] GridPosition startPositionAt(std::size_t index) const noexcept {
        return _blocks[index].start;
    }

    /// @return La boîte **courante** du bloc @p index, en unités monde.
    [[nodiscard]] Aabb boxAt(std::size_t index) const noexcept;

    /// @return `true` si le bloc @p index a franchi le bord bas du tableau et n'existe plus.
    [[nodiscard]] bool isRemovedAt(std::size_t index) const noexcept {
        return _blocks[index].removed;
    }

    /// @return `true` si le bloc @p index a été touché par le personnage et descend (ou est
    ///         arrêté contre la matière).
    [[nodiscard]] bool isArmedAt(std::size_t index) const noexcept {
        return _blocks[index].armed;
    }

    /// @return Les échantillons des blocs **encore présents**, dans l'ordre des indices — à
    ///         concaténer aux échantillons des plateformes mobiles par l'appelant. Un bloc retiré
    ///         n'y figure pas : un échantillon fantôme continuerait de porter le personnage.
    ///
    /// La liste est recalculée par `update()`, jamais à la lecture. Référence valable jusqu'au
    /// prochain `update()`.
    [[nodiscard]] const std::vector<PlatformSample>& samples() const noexcept {
        return _samples;
    }

private:
    /// État d'un bloc descendant : sa case de départ et sa course.
    struct SinkingBlock {
        GridPosition start;       ///< Position de la tuile dans le fichier (point de départ).
        float previousY = 0.0F;   ///< Ordonnée du coin haut-gauche au pas précédent.
        float currentY = 0.0F;    ///< Ordonnée du coin haut-gauche au pas courant.
        long long armedStep = 0;  ///< Numéro de pas de l'armement ; sens seulement si `armed`.
        bool armed = false;       ///< Le personnage l'a touché au moins une fois.
        bool stopped = false;     ///< Arrivé contre la matière : figé définitivement.
        bool removed = false;     ///< Sorti par le bas du tableau : n'existe plus.
    };

    /// Ordonnée maximale que le bloc @p index peut atteindre sans entrer dans la matière de
    /// @p baseCollision ; `std::nullopt` s'il n'y a aucune case pleine sous lui (le bloc sortira
    /// alors du tableau).
    [[nodiscard]] float stopLimitAt(std::size_t index, const TileMap& baseCollision,
                                    bool& hasLimit) const noexcept;

    /// Recalcule `_samples` pour le pas courant.
    void refreshSamples();

    std::vector<SinkingBlock> _blocks;
    std::vector<PlatformSample> _samples;
    int _mapHeight = 0;
    long long _stepCount = 0;
};

}  // namespace core
