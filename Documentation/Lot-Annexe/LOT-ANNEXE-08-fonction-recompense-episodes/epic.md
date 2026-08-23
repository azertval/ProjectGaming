# LOT-ANNEXE-08 — Fonction de récompense et critères d'épisode {#lot-annexe-08}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-05](@ref lot-annexe-05) (`HeadlessLevelEnvironment`,
> budget de pas et mesure de progression). Quatrième lot de la génération 1 : définit le signal
> d'apprentissage **unique**, partagé par tous les algorithmes des générations 2 et 3 — évite qu'un
> algorithme évolutionniste et un algorithme de policy gradient optimisent, sans le savoir, deux
> définitions différentes de « bien jouer ».

## Objectif
`HeadlessLevelEnvironment` (`LOT-ANNEXE-05`) expose une issue brute (`core::LevelOutcome` :
`Playing`/`Won`/`Lost`) et des compteurs de progression, mais aucune notion de **récompense** — le
signal scalaire qu'un algorithme d'apprentissage cherche à maximiser. Sans une définition unique et
partagée, chaque lot de génération 2/3 (`LOT-ANNEXE-10`, `12`, `13`, `14`) risquerait d'inventer sa
propre fonction de récompense, rendant toute comparaison entre algorithmes (génération 4) invalide
par construction — on ne pourrait plus distinguer « cet algorithme apprend mieux » de « cet
algorithme optimise un problème légèrement différent ». Ce lot fixe une fois pour toutes ce que
« bien jouer un niveau » signifie numériquement.

## Périmètre

### Inclus
- **Récompense de progression** : proportionnelle à la diminution de la distance restante jusqu'à la
  sortie du niveau (`core::Level::exit()`), à chaque pas — dense (non nulle à chaque pas), pour
  donner un signal d'apprentissage y compris avant qu'un épisode complet ne se termine.
- **Bonus de complétion** : récompense positive fixe et significativement plus grande que la somme
  de récompenses de progression plausible sur un épisode, accordée uniquement à `core::
  LevelOutcome::Won`.
- **Pénalité de mort** : récompense négative fixe, accordée à `core::LevelOutcome::Lost`.
- **Pénalité de temps** : petite pénalité constante à chaque pas, pour favoriser, à réussite égale,
  une résolution plus rapide du niveau.
- **Détection de fin d'épisode** (`aisolver::EpisodeStatus` : `Ongoing`/`Won`/`Lost`/`TimedOut`/
  `Stuck`), réutilisant directement l'issue de `HeadlessLevelEnvironment::step` et son budget de pas/
  mesure de progression (`LOT-ANNEXE-05`) pour les deux derniers cas.
- Tests : cas limites (mort immédiate, complétion immédiate, stagnation sans progression),
  non-régression sur les niveaux `demo-*.json`.

### Exclus (hors périmètre de ce lot)
- **Façonnage de récompense spécifique à un niveau** (bonus ad hoc pour un mécanisme particulier,
  ex. bonus dédié à l'activation d'un interrupteur) : la récompense reste **générique**, dérivée
  uniquement de la distance à la sortie et de l'issue — un façonnage par mécanisme romprait la
  comparabilité inter-niveaux visée par la génération 4.
- **Normalisation de la récompense inter-niveaux** (mise à l'échelle pour que tous les niveaux aient
  une récompense maximale comparable) : hors périmètre tant que l'entraînement reste niveau par
  niveau (décision transverse) — chaque entraînement optimise sa propre échelle, sans besoin de
  comparer des valeurs absolues de récompense entre niveaux différents.
- **Curiosité ou récompense intrinsèque** (bonus d'exploration au-delà de la progression vers la
  sortie) : aucun niveau de la séquence `demo-*.json` n'a été identifié comme nécessitant une
  exploration au-delà de ce que la récompense de progression encourage déjà.
- **La politique de reprise après un échec** (relance automatique du niveau) : `HeadlessLevelEnvironment`
  (`LOT-ANNEXE-05`) ne relance jamais lui-même ; c'est la boucle d'entraînement (génération 2/3) qui
  appelle `reset()` après un épisode terminé — ce lot ne fait que **qualifier** la fin d'épisode, pas
  décider de la suite.

## Décisions de cadrage
- **Récompense dense (par pas), jamais uniquement sparse (fin d'épisode seule).** Un signal
  uniquement délivré à la fin (victoire/échec) serait extrêmement difficile à exploiter par un
  algorithme de gradient (génération 3) sur des niveaux de plusieurs centaines de pas — la
  récompense de progression donne un signal à **chaque** pas, y compris pour un épisode qui échoue
  finalement, ce qui informe quand même la direction d'amélioration.
- **La distance à la sortie est calculée en ligne droite (norme euclidienne) dans l'espace monde,
  jamais en distance de plus court chemin à travers le niveau.** Une distance de plus court chemin
  supposerait un pathfinding sur la grille (hors périmètre du programme, cf. absence de toute
  logique de recherche de chemin dans `Core`) ; la distance euclidienne reste un signal imparfait
  mais dense et bon marché à calculer à chaque pas, laissant à l'algorithme d'apprentissage la charge
  de découvrir le chemin réel (contournement d'obstacles, mécanismes) — cohérent avec l'esprit
  « apprentissage », pas « recherche de chemin déjà connue ».
- **La pénalité de mort et le bonus de complétion sont des constantes fixes, pas apprises ni
  calibrées automatiquement par niveau.** Valeurs documentées comme des paramètres de
  `aisolver::RewardConfig`, ajustables manuellement si l'expérience (génération 2/3) révèle un
  déséquilibre (ex. bonus de complétion trop faible face à la somme de pénalités de temps sur un
  niveau très long) — pas un objectif d'auto-calibration de ce lot.
- **Le blocage (`Stuck`, absence de progression pendant N pas) est un critère de fin d'épisode
  distinct d'un timeout dur** (`TimedOut`, plafond absolu de pas) : un agent qui progresse lentement
  mais sûrement ne doit pas être coupé aussi tôt qu'un agent qui stagne complètement — les deux
  bornent néanmoins la durée d'un épisode, condition nécessaire pour qu'un entraînement automatisé
  (génération 2/3) ne boucle jamais indéfiniment sur un seul épisode.
- **`Stuck` et `TimedOut` sont traités comme un échec du point de vue de la récompense** (même
  pénalité que `Lost`, pas une valeur neutre) : un épisode qui n'aboutit à rien ne doit pas être
  récompensé plus qu'un échec explicite, sous peine qu'un algorithme apprenne à « ne rien risquer »
  plutôt qu'à progresser.

## Notions abordées
Voir @ref guide-annexe-apprentissage-renforcement, sections « Récompense » et « Épisode, horizon »
(récompense dense vs sparse, retour, fin d'épisode). Sources directes : Sutton & Barto (2018,
référence de base pour la notion de récompense et de retour) — bibliographie complète dans le
chapitre.

## Exigences couvertes
- Nouvelle : \anchor EX-IA-009 **EX-IA-009** — Un signal de récompense unique et partagé (progression
  vers la sortie, bonus de complétion, pénalité de mort, pénalité de temps) et une classification de
  fin d'épisode (victoire, échec, timeout, blocage) doivent être définis une seule fois et réutilisés
  par tout algorithme d'apprentissage du programme, pour garantir la comparabilité de leurs résultats.
- Réutilisées : `EX-IA-005` (`HeadlessLevelEnvironment`, budget de pas et mesure de progression),
  `EX-GP-030`/`EX-GP-031` (conditions de victoire/échec du jeu, base de la classification d'épisode).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-recompense-progression-bonus-penalites.md) | Récompense de progression, bonus de complétion, pénalités | `Source/AiSolver/Env` | ✅ |
| [TACHE-02](tache-02-detection-fin-episode.md) | Détection de fin d'épisode | `Source/AiSolver/Env` | ✅ |
| [TACHE-03](tache-03-tests-cas-limites.md) | Tests : cas limites et non-régression sur `demo-*.json` | `Source/Test/Unit/AiSolver/Env` | ✅ |

## Critères d'acceptation du lot
1. La récompense de progression est strictement positive lorsque la distance à la sortie diminue
   d'un pas au suivant, nulle ou négative si elle stagne/augmente.
2. Un épisode se terminant en `Won` reçoit une récompense cumulée strictement supérieure à tout
   épisode se terminant en `Lost`/`TimedOut`/`Stuck` sur le même niveau, quelle que soit la
   progression partielle accomplie avant l'échec (le bonus de complétion domine).
3. `Stuck` est détecté après un nombre configurable de pas sans amélioration de la meilleure
   distance atteinte, indépendamment du plafond dur de pas (`TimedOut`), qui reste une borne
   distincte et toujours active.
4. Sur chaque niveau de `Source/Elements/Levels/demo-*.json`, un rejeu du script existant de
   `test_parcours_complet.cpp` (via `HeadlessLevelEnvironment`) classe l'épisode en `Won` avec une
   récompense cumulée positive et dominée par le bonus de complétion.
5. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Bâtit sur [LOT-ANNEXE-05](@ref lot-annexe-05) (`HeadlessLevelEnvironment`, `core::LevelOutcome`,
budget de pas). [LOT-ANNEXE-10](@ref lot-annexe-10) à [LOT-ANNEXE-14](@ref lot-annexe-14)
(génération 2 et 3) en dépendent directement comme signal d'apprentissage unique.

## Navigation des tâches
- @subpage lot-annexe-08-tache-01-recompense-progression-bonus-penalites
- @subpage lot-annexe-08-tache-02-detection-fin-episode
- @subpage lot-annexe-08-tache-03-tests-cas-limites
