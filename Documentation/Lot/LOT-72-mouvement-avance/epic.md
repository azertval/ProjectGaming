# LOT-72 — Mouvement avancé (dash chargé, poussée, pentes, ground pound, combo) {#lot-72}

> Statut : **à faire**.
> Prérequis : [LOT-10](@ref lot-10) (mécaniques avancées : dash, wall jump), [LOT-67](@ref lot-67)
> (charges de dash et budgets par tableau).

## Objectif

Enrichir le nuancier de mouvement du personnage en complexifiant le **dash** existant et en le
faisant composer avec les autres mécaniques (poussée de bloc, pentes, saut, mur), sans ajouter de
nouvelle touche.

Le dash (`EX-GP-017`, LOT-10) est déjà en 8 directions et dispose de charges par saut et d'un budget
par tableau (`EX-GP-055`/`EX-GP-024`, LOT-67). Mais il reste isolé des autres systèmes : pendant un
dash, la gravité est suspendue et la résolution passe par un balayage axis-aligned classique
(`sweepAabb`), **sans** la passe de suivi de pente (`resolveSlopeFollow`/`resolveCeilingSlopeFollow`)
qu'utilise le déplacement normal — un dash traversant une pente ne la suit pas. La poussée de bloc
(`EX-GP-022`) ne déplace qu'une case par contact, identique en marche ou en dash. Et sauter pendant
un dash, ou juste après une poussée, ne conserve aucune vitesse : le personnage retombe à sa physique
normale sans transition.

Ce lot comble ces trous par six mécaniques qui **réutilisent l'existant** (directions du dash,
`wallDirection`, suivi de pente, charges/budget de dash) plutôt que d'en dupliquer l'infrastructure.

## Périmètre

### Inclus
- **Dash chargé** (`EX-GP-056`) : maintenir la direction opposée avant de dasher charge un boost de
  vitesse/durée.
- **Poussée renforcée par le dash** (`EX-GP-057`) : un bloc percuté pendant un dash est repoussé de
  plusieurs cases d'un coup.
- **Ground pound** (`EX-GP-058`) : chute accélérée dirigée en l'air, front d'atterrissage exploitable
  par l'IHM.
- **Dash et pentes** (`EX-GP-060`) : suivi de pente pendant le dash, glissade de sortie contre une
  pente.
- **Combo dash + saut maximisé** (`EX-GP-061`) : jump-cancel du dash (conservation de vitesse),
  wall-jump en sortie de dash, momentum hérité après poussée renforcée, bonus cumulatif plafonné.
- Validation explicite du **dash diagonal** (déjà livré en LOT-10) avec chacune des nouvelles
  mécaniques (non-régression).

### Déjà livré (relecture de code, pas de nouveau travail)
- **Wall slide** : `EX-GP-016` (LOT-10) couvre déjà le clamp de chute au contact d'un mur en l'air
  (`PhysicsConfig::wallSlideSpeed`, `CharacterPhysicsSystem::resolveVelocity`). Aucune exigence
  supplémentaire créée ; seule la non-régression avec les nouvelles mécaniques est testée
  (TACHE-05/TACHE-07).

### Exclus
- **Casse de blocs fragiles** au ground pound : nécessiterait un nouveau `TileType` (aucun bloc
  fragile n'existe dans `Source/Core/Levels/TileType.h`), avec support éditeur/rendu — hors périmètre
  d'un lot de mouvement.
- Aucune nouvelle action de contrôle (`Documentation/Specification/controles.md` inchangé), aucun
  redimensionnement de bloc.
- Poussée/knockback sur des ennemis ou dangers : seuls les blocs poussables sont concernés.
- Réimplémentation du dash diagonal lui-même : déjà livré, seule sa non-régression est garantie.

## Décisions de cadrage

- **Charge de dash** = maintien de la direction opposée à `Player::facing` pendant un seuil
  configurable (`PhysicsConfig::dashChargeHoldTime`) avant l'appui sur dash ; relâcher ou changer de
  direction avant le seuil annule la charge en cours.
- **Le boost ne crée pas de charge de dash supplémentaire** : il modifie seulement vitesse/durée du
  dash consommé normalement (`dashChargesRemaining`/`dashesRemaining` de LOT-67 inchangés dans leur
  fonctionnement), pour rester composable sans dupliquer cette infrastructure.
- **Poussée renforcée** = déplacement multi-cases dans le même pas fixe (balayage case par case
  jusqu'au premier obstacle ou une distance maximale configurable), en réutilisant `isFree` de
  `BlockController`, pas une nouvelle simulation physique.
- **Ground pound** = nouvel état `Player` (`groundPounding`), déclenché par une direction verticale
  « bas » soutenue en l'air (entrée déjà mappée, pas de nouvelle touche), vitesse de chute imposée
  (`PhysicsConfig::groundPoundSpeed`), effet cosmétique côté `Source/HMI` à l'atterrissage (hors
  `Source/Core`) ; aucune règle spéciale pour les plaques de pression (`EX-GP-025`), déjà sensibles
  au poids sur tout contact.
- **Dash et pentes** : pendant le dash, appliquer aussi la passe de suivi de pente/plafond en plus du
  balayage classique ; à la fin d'un dash dont la trajectoire touche une pente, convertir la vitesse
  résiduelle en vitesse le long de la pente (glissade) au lieu de la couper net.
- **Combo dash + saut**, quatre briques qui composent avec l'existant sans nouvelle touche :
  - **Jump-cancel** : un saut pendant `dashTimer > 0` met fin au dash immédiatement et conserve sa
    vitesse horizontale au lieu de revenir à la physique normale.
  - **Wall-jump en sortie de dash** : si le jump-cancel a lieu au contact d'un mur, c'est un
    wall-jump qui se déclenche (réutilise `EX-GP-016` tel quel), pas un saut simple.
  - **Momentum hérité après poussée** : un saut dans une courte fenêtre après une poussée renforcée
    hérite d'un pourcentage configurable (`PhysicsConfig::momentumCarryRatio`) de la vitesse
    horizontale du bloc.
  - **Enchaînement plafonné** : chaque jump-cancel réussi dans une fenêtre courte après le précédent
    ajoute un bonus de vitesse cumulatif (`PhysicsConfig::comboSpeedBonus`), plafonné
    (`PhysicsConfig::comboSpeedCap`) ; le compteur de combo retombe à zéro au contact du sol ou hors
    fenêtre. Le nombre d'enchaînements reste borné par `dashChargesRemaining`/`dashesRemaining`
    existants — aucune charge supplémentaire créée.
- **Déterminisme** : tout nouvel état/minuterie est décompté au pas fixe comme `coyoteTimer`/
  `dashTimer`, conforme à `EX-NFR-002`.

## Exigences couvertes

`EX-GP-056`, `EX-GP-057`, `EX-GP-058`, `EX-GP-060`, `EX-GP-061` (nouvelles). Réutilisées :
`EX-GP-003`, `EX-GP-004`, `EX-GP-006`, `EX-GP-007` (pentes/arrondis et suivi), `EX-GP-016`
(`wallDirection`, wall jump), `EX-GP-017` (dash), `EX-GP-022` (blocs poussables), `EX-GP-024`,
`EX-GP-025` (plaques de pression), `EX-GP-055` (charges de dash par tableau), `EX-ARCH-011`,
`EX-NFR-002`.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | Charge de dash et dash boosté | `Source/Core/Ecs/{Components,Systems}`, `Source/Core/Physics` | ⬜ |
| TACHE-02 | Poussée renforcée pendant un dash | `Source/Core/Gameplay/BlockController.*`, `Source/HMI/Game` | ⬜ |
| TACHE-03 | Dash et pentes (suivi + glissade de sortie) | `Source/Core/Ecs/Systems`, `Source/Core/Physics` | ⬜ |
| TACHE-04 | Ground pound | `Source/Core/Ecs/{Components,Systems}`, `Source/HMI` | ⬜ |
| TACHE-05 | Wall slide (déjà livré, validation croisée) | `Source/Core/Ecs/Systems` | ✅ |
| TACHE-06 | Combo dash + saut maximisé | `Source/Core/Ecs/Systems` | ⬜ |
| TACHE-07 | Validation croisée et tests | `Source/Core/Ecs/Systems` (tests) | ⬜ |
| TACHE-08 | Documentation, exigences, CHANGELOG | `Documentation/Specification`, `CHANGELOG.md` | 🔄 |

## Critères d'acceptation du lot

1. Sans déclencher aucune des nouvelles mécaniques, le personnage se comporte **exactement** comme
   avant le lot (tests existants inchangés, y compris dash, poussée simple, saut, wall-jump).
2. Dash chargé : maintenir la direction opposée au-delà du seuil puis dasher produit un dash
   mesurablement plus rapide/long, dans les 8 directions y compris diagonales.
3. Poussée renforcée : un dash (normal ou boosté) contre un bloc le déplace de plusieurs cases en un
   pas fixe, sans traversée d'obstacle, sans dupliquer/casser les budgets/charges de LOT-67.
4. Dash sur pente : dasher le long d'une pente suit sa surface (ni clip, ni arrêt net non voulu) ;
   sortir d'un dash contre une pente déclenche une glissade cohérente plutôt qu'un arrêt brutal.
5. Ground pound : déclenché uniquement en l'air, vitesse de chute imposée, front d'atterrissage
   exploitable par `Source/HMI`, ne casse pas l'interaction existante avec les plaques de pression.
6. Combo dash + saut : un saut pendant un dash le coupe et conserve sa vitesse horizontale
   (jump-cancel) ; s'il a lieu contre un mur, c'est un wall-jump qui se déclenche ; un saut dans la
   fenêtre suivant une poussée renforcée hérite du ratio de vitesse configuré ; un enchaînement de
   jump-cancels rapprochés cumule un bonus de vitesse plafonné et reste borné par les charges/budget
   de dash existants.
7. `ctest` à 100 %, lint d'exigences vert, build `/W4 /WX`, déterminisme conservé (`EX-NFR-002`).

## Dépendances

Aucun lot ne dépend de celui-ci. Il s'appuie sur le dash et le wall jump du `LOT-10`, sur les charges
de dash par tableau du `LOT-67`, et sur le suivi de pente introduit au `LOT-08`/`LOT-19`.

## Navigation des tâches

- @subpage lot-72-tache-01-dash-charge
- @subpage lot-72-tache-02-poussee-renforcee
- @subpage lot-72-tache-03-dash-pentes
- @subpage lot-72-tache-04-ground-pound
- @subpage lot-72-tache-05-wall-slide
- @subpage lot-72-tache-06-combo-dash-saut
- @subpage lot-72-tache-07-validation-croisee
- @subpage lot-72-tache-08-documentation
