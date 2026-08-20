// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Levels/TileType.h
 * @brief Types de tuiles d'un niveau et utilitaires associés.
 */

namespace core {

/**
 * @brief Type d'une tuile de la grille d'un niveau (`EX-GP-001`).
 *
 * `Empty` est la case traversable par défaut ; `Solid` bloque le déplacement ; `Danger`
 * provoque l'échec au contact ; `Entry`/`Exit` sont l'apparition et la sortie ; `Switch`,
 * `PressurePlate` et `Door` sont les mécanismes de puzzle, résolus chaque pas fixe par
 * `core::MechanismController` (ce modèle ne fait que les représenter) : `Switch` bascule au
 * contact (front), `PressurePlate` (`EX-GP-025`) reste active tant qu'un poids suffisant y
 * repose — les deux partagent la même infrastructure de liaison à une `Door`. `Block` (`EX-GP-022`)
 * est un **bloc poussable** : sa position initiale est celle du fichier, mais
 * `core::BlockController` la fait évoluer chaque pas fixe (poussée par le personnage, chute si non
 * soutenu) — comme pour les mécanismes, ce modèle ne fait que représenter sa position de
 * **départ**. `SlopeUpRight`/ `SlopeUpLeft` (`EX-GP-003`) sont des **pentes** à 45° (montée sur
 * toute la largeur d'une case, respectivement vers la droite et vers la gauche) : leur surface est
 * **inclinée**, décrite par `core::slopeSurfaceHeight` (`Core/Physics/SlopeGeometry.h`) et suivie
 * par une passe de résolution dédiée (@ref guide-physique) — **pas** par `isSolid` (voir
 * ci-dessous). `RoundedUpRight`/ `RoundedUpLeft` (`EX-GP-004`) sont la variante **courbe** (quart
 * de cercle) des pentes : même orientation, même infrastructure de suivi
 * (`core::slopeSurfaceHeight`, non solides), seule la formule de hauteur diffère (linéaire pour une
 * pente, quart de cercle pour un arrondi). `BlockHalf`/`BlockQuarter` (`EX-GP-005`) sont des
 * **blocs poussables réduits** (facteurs `×0.5`/ `×0.25`, `core::BlockController`) : mêmes règles
 * de poussée/chute que `Block` (case par case), mais leur boîte de collision **réelle** (testée
 * contre le personnage) est plus petite que la case et **centrée** dedans — résolue par une routine
 * dédiée boîte-contre-boîte
 * (`core::sweepAabbVsAabb`, @ref guide-physique), pas par la grille classique. `SlopeDownRight`/
 * `SlopeDownLeft`/`RoundedDownRight`/`RoundedDownLeft` (`EX-GP-006`) sont les variantes de
 * **plafond** des pentes/arrondis ci-dessus : miroir vertical de la même silhouette (matière
 * pleine en haut de la case plutôt qu'en bas). Comme leurs équivalents de sol, elles ne sont
 * **jamais solides** pour la grille classique (`core::isSolid`) — leur collision est résolue par
 * deux passes de suivi symétriques : `core::resolveCeilingSlopeFollow` bloque un saut qui
 * franchirait leur silhouette **par en dessous** (miroir de celle des pentes de sol), tandis que
 * leur **face du haut** (toujours plate, au sommet de la case — `core::slopeSurfaceHeight` y
 * renvoie `0`) supporte normalement un personnage qui tombe dessus **par au-dessus**, via
 * `core::resolveSlopeFollow` réutilisé tel quel. Le personnage ne « marche » en revanche jamais
 * **latéralement** le long de leur silhouette inclinée (`core::isFollowableSurface` reste `false`
 * — pas de déplacement calé en suivant la pente, contrairement à une pente de sol).
 * `ConcaveUpRight`/ `ConcaveUpLeft`/`ConcaveDownRight`/`ConcaveDownLeft` (`EX-GP-007`) sont une
 * **seconde famille** de quart de cercle, **concave** plutôt que **convexe**
 * (`RoundedUpRight`/`RoundedUpLeft` et leurs variantes de plafond ci-dessus) : le centre du cercle
 * est du côté de la **matière** plutôt que du côté creux, ce qui inverse la courbure — tangente
 * **horizontale** du côté bas/creux, **verticale** du côté haut/plein (l'inverse exact de l'arrondi
 * convexe). Mêmes orientations (`Up`/`Down`, `Right`/`Left`) et même infrastructure de suivi que
 * les arrondis convexes (`core::slopeSurfaceHeight`, `core::resolveSlopeFollow` pour le sol ;
 * `core::ceilingSlopeHeight`, `core::resolveCeilingSlopeFollow` pour le plafond) — seule la formule
 * de hauteur change. `DangerUp`/`DangerDown`/`DangerLeft`/ `DangerRight` (`EX-GP-050`) sont un
 * **danger directionnel** : le suffixe décrit le bord **mortel** de la case (celui vers lequel les
 * pics pointent), pas un mouvement — le reste de la case est traversable sans risque. Non solides
 * (comme `Danger`), leur zone mortelle réelle (une bande étroite le long du bord désigné, pas la
 * case entière) est décrite par `core::dangerHitbox`
 * (`Core/Levels/DangerGeometry.h`), consommée par `core::evaluateOutcome`. `DangerMover`
 * (`EX-GP-051`) est un **danger mobile** : sa position initiale est celle du fichier, mais un
 * aller-retour linéaire déterministe (axe/portée, `core::DangerMoverConfig`) la fait évoluer chaque
 * pas fixe — comme pour `Block`, ce modèle ne représente que sa position de **départ**.
 * `DangerSwitched`
 * (`EX-GP-052`) est un **danger commuté** : mortel uniquement quand l'interrupteur/la plaque de
 * pression qui lui est lié (`core::DangerLink`) est actif — l'inverse de `Door`, qui devient
 * franchissable quand actif. `DangerBlink` (`EX-GP-053`) est un **danger temporisé** : alterne
 * mortel/inoffensif selon une période fixe et un déphasage propres à la tuile
 * (`core::DangerBlinkConfig`), indépendamment de tout interrupteur. `Key`/`LockedDoor`
 * (`EX-GP-023`) sont la seconde paire déclencheur↔cible, résolue par la **même** liaison
 * `core::Mechanism` que `Switch`/`PressurePlate`↔`Door` (aucune notion de liaison dupliquée) :
 * `core::MechanismController` distingue leur comportement à la construction, d'après le type de
 * la tuile déclencheur (comme il le fait déjà pour `_continuous`). Une clé ramassée **consomme**
 * son mécanisme et ouvre sa porte **définitivement** (jamais de re-fermeture, contrairement à
 * l'interrupteur) ; une porte verrouillée fermée est solide exactement comme `Door` (géré par la
 * grille de collision du contrôleur, jamais par `isSolid` ci-dessous). `MovingPlatform`
 * (`EX-GP-026`) est une **plateforme mobile** : sa position est **continue** (pas alignée sur la
 * grille comme `Block`), fonction déterministe du numéro de pas fixe
 * (`core::PlatformController`), qui porte le personnage et les blocs poussables posés dessus. Sa
 * position dans le fichier n'est que son point de **départ** — comme `DangerMover`, ce modèle ne
 * représente que cela. N'est jamais solide pour la grille classique (comme les blocs réduits,
 * `EX-GP-005`) : sa collision réelle est résolue par une passe dédiée boîte-contre-boîte
 * (`core::sweepAabbVsAabb`), pas par ce test statique.
 */
enum class TileType {
    Empty,
    Solid,
    Danger,
    Entry,
    Exit,
    Switch,
    Door,
    PressurePlate,
    Block,
    SlopeUpRight,
    SlopeUpLeft,
    RoundedUpRight,
    RoundedUpLeft,
    BlockHalf,
    BlockQuarter,
    SlopeDownRight,
    SlopeDownLeft,
    RoundedDownRight,
    RoundedDownLeft,
    ConcaveUpRight,
    ConcaveUpLeft,
    ConcaveDownRight,
    ConcaveDownLeft,
    DangerUp,
    DangerDown,
    DangerLeft,
    DangerRight,
    DangerMover,
    DangerSwitched,
    DangerBlink,
    Key,
    LockedDoor,
    MovingPlatform,
};

/**
 * @brief Indique si un type de tuile bloque le déplacement de manière **statique**.
 * @param type Type de tuile.
 * @return true pour `Solid` et les trois tailles de bloc (`Block`/`BlockHalf`/`BlockQuarter`, non
 *         encore déplacés bloquent comme un mur — `core::BlockController` gère leur position
 *         réelle et, pour les tailles réduites, leur boîte de collision **plus petite que la
 *         case** via `core::sweepAabbVsAabb`, jamais via ce test statique). `false` pour les
 *         pentes et arrondis, de **sol** (`SlopeUpRight`/`SlopeUpLeft`/`RoundedUpRight`/
 *         `RoundedUpLeft`) comme de **plafond** (`SlopeDownRight`/`SlopeDownLeft`/
 *         `RoundedDownRight`/`RoundedDownLeft`, `EX-GP-006`) : une surface suivie n'est **jamais**
 *         solide pour le balayage classique (`core::sweepAabb`), sous peine de transformer son
 *         bord (haut pour le sol, bas pour le plafond) en mur invisible — sa solidité est
 *         entièrement gérée par une passe de suivi dédiée (`core::slopeSurfaceHeight` +
 *         `core::resolveSlopeFollow` pour le sol, `core::ceilingSlopeHeight` +
 *         `core::resolveCeilingSlopeFollow` pour le plafond, miroir l'une de l'autre). La solidité
 *         d'une porte dépend de son **état** (ouverte/fermée) et la position d'un bloc évolue en
 *         jeu : les deux sont gérées par la simulation, pas par ce test statique.
 */
[[nodiscard]] constexpr bool isSolid(TileType type) noexcept {
    return type == TileType::Solid || type == TileType::Block || type == TileType::BlockHalf ||
           type == TileType::BlockQuarter;
}

/**
 * @brief Facteur de taille **visuel/collision** d'une tuile bloc réduite (`EX-GP-005`).
 *
 * `1.0f` pour tout type qui n'est pas un bloc réduit (y compris `Block`, plein) : un appelant qui
 * calcule systématiquement `margin = (1 - scale) * 0.5` et dessine/teste à `scale × scale` obtient
 * ainsi le comportement plein-case habituel sans cas particulier. Seule source de vérité pour ce
 * facteur, partagée par `core::BlockController` (collision réelle, `core::sweepAabbVsAabb`) et le
 * rendu (`hmi::TileVisuals`, éditeur **et** jeu) — la même formule des deux côtés garantit qu'un
 * bloc réduit s'affiche exactement comme sa hitbox, sans pouvoir diverger.
 * @param type Type de tuile.
 * @return `0.5f` pour `BlockHalf`, `0.25f` pour `BlockQuarter`, `1.0f` sinon.
 */
[[nodiscard]] constexpr float tileVisualScale(TileType type) noexcept {
    switch (type) {
        case TileType::BlockHalf:
            return 0.5f;
        case TileType::BlockQuarter:
            return 0.25f;
        default:
            return 1.0f;
    }
}

}  // namespace core
