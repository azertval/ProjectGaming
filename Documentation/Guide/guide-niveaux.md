# Niveaux : modèle, chargement, mécanismes, budgets {#guide-niveaux}

Un niveau est une **grille de tuiles typées** plus des métadonnées (nom, entrée, sortie,
mécanismes, budgets de mouvements). Tout vit dans `Source/Core/Levels` (le modèle en mémoire et son
chargement) et `Source/Core/Gameplay` (le comportement des mécanismes).

## Le modèle en mémoire

### Deux systèmes de coordonnées à ne pas confondre

Le moteur manipule **deux** représentations de position différentes, et le confondre est une source
d'erreurs fréquente :

- `core::GridPosition` : une paire d'**entiers** `(column, row)` — désigne une **case** de la
  grille, sans unité de mesure continue. C'est ce que manipule tout ce qui parle du niveau comme
  d'un plateau discret (entrée, sortie, position d'un interrupteur).
- `core::Vector2` (@ref guide-maths) : une paire de **flottants** — une position **continue** dans
  le monde physique, utilisée par la simulation (position du personnage, résultat d'un balayage).

Comme une tuile = 1 unité monde, convertir l'un vers l'autre est une simple conversion de type
(`column` devient `x`, `row` devient `y`) — mais les deux ne sont **jamais** interchangeables dans
le code : une case n'a pas de position « à mi-chemin », un personnage si.

### \ref core::TileType "core::TileType" : le vocabulaire des cases

Chaque case de la grille a l'un de ces types :

| Type | Rôle |
|------|------|
| `Empty` | Case traversable, par défaut (aucun contenu). |
| `Solid` | Bloque le déplacement en toute circonstance — un mur ou le sol. |
| `Danger` | Traversable, mais son contact déclenche l'échec du niveau (`EX-GP-031`). |
| `Entry` | Position d'apparition du personnage au chargement du niveau. |
| `Exit` | Recouvrir cette case déclenche la réussite du niveau (`EX-GP-030`). |
| `Switch` | Interrupteur : son activation **bascule** l'état d'une `Door` liée (voir §« Mécanismes »). |
| `PressurePlate` | Plaque de pression : ouvre une `Door` liée **tant qu'un poids y repose** (`EX-GP-025`) — activation **continue**, pas de bascule. |
| `Door` | Porte : solide **fermée**, franchissable **ouverte** — son état dépend du `Switch`/`PressurePlate` lié. |
| `Block` | Bloc poussable (`EX-GP-022`) : solide comme un mur tant qu'il n'a pas bougé, mais peut être **déplacé** par le personnage et **retombe** sous gravité (voir §« Blocs poussables »). |
| `SlopeUpRight` | Pente à 45° (`EX-GP-003`) montant de **gauche à droite** : `Empty` pour toute autre logique (jamais solide), mais sa surface **inclinée** est suivie par la physique (@ref guide-physique). |
| `SlopeUpLeft` | Symétrique de `SlopeUpRight` : pente montant de **droite à gauche**. |
| `RoundedUpRight` | Variante **courbe** (quart de cercle, `EX-GP-004`) de `SlopeUpRight` : même orientation, même suivi de surface, formule de hauteur différente. |
| `RoundedUpLeft` | Symétrique de `RoundedUpRight` : arrondi montant de **droite à gauche**. |

Notez que `Door` n'est **pas** statiquement solide au sens de `core::isSolid(TileType)` — sa
solidité dépend de son **état**, calculé par le `MechanismController` (voir plus bas), pas du type
de tuile seul. Les pentes et arrondis ne sont, eux, **jamais** solides (`core::isSolid` renvoie
toujours faux) — voir @ref guide-physique pour le mécanisme de suivi qui les rend praticables.

### \ref core::TileMap "core::TileMap" : la grille

`core::TileMap` est une grille dense `width × height` de `TileType`, origine **haut-gauche** (même
convention que tout le moteur, @ref guide-maths). Elle expose `tile(colonne, ligne)` (lecture) et
`isSolid(colonne, ligne)` (utilisée directement par le balayage de collision, @ref guide-physique
§1). C'est une donnée pure, sans dépendance à un fichier ou à un rendu — testable isolément.

### \ref core::Level "core::Level" : le niveau assemblé

`core::Level` regroupe : un nom, une `TileMap`, une position d'**entrée** et de **sortie**
(`GridPosition`), une liste de `core::Mechanism` (liaisons interrupteur↔porte, résolues en
positions) et des **budgets** de mouvements (`jumpBudget`/`dashBudget`, décrits plus bas). C'est
l'objet que le chargeur produit et que le reste du moteur (rendu, gameplay) consomme en lecture
seule — la `TileMap` d'un `Level` ne change **jamais** après le chargement ; c'est le contrôleur de
mécanismes (voir plus loin) qui maintient sa **propre** copie mutable pour représenter les portes en
cours de partie.

## Chargement JSON

Un niveau est décrit dans un fichier texte au format [JSON](https://www.json.org/json-fr.html) ⧉,
parsé par la bibliothèque **nlohmann/json**, dont l'usage est confiné au fichier `.cpp` du
chargeur — le reste du moteur ne dépend jamais directement de cette bibliothèque. La classe
`core::LevelLoader` expose deux points d'entrée statiques : `loadFromFile` (depuis un chemin) et
`loadFromString` (depuis du texte déjà en mémoire, pratique pour les tests).

### Exemple concret

```json
{
  "name": "Demonstration 4 (puzzle)",
  "width": 14,
  "height": 8,
  "jumpBudget": 2,
  "dashBudget": 1,
  "tiles": [
    { "x": 0, "y": 0, "type": "solid" },
    { "x": 0, "y": 1, "type": "solid" },
    { "x": 1, "y": 6, "type": "entry" },
    { "x": 3, "y": 6, "type": "switch", "id": "s1" },
    { "x": 7, "y": 6, "type": "door", "opensWith": "s1" },
    { "x": 11, "y": 6, "type": "exit" }
  ]
}
```

(fichier réel, tronqué ici pour la lisibilité — voir `Source/Elements/Levels/demo4.json` pour la
version complète). À lire ainsi :

- `name`, `width`, `height` décrivent le niveau et les dimensions de sa grille ;
- `jumpBudget`/`dashBudget` sont **optionnels** — absents, le budget correspondant est **illimité**
  (voir plus bas) ;
- `tiles` est une **liste éparse** : seules les cases **non vides** (différentes de `Empty`) sont
  listées, chacune par ses coordonnées `x`/`y` (colonne/ligne) et son `type`. Une case absente de la
  liste est implicitement `Empty` ;
- les tuiles `switch` et `door` portent en plus un identifiant textuel : `id` **nomme**
  l'interrupteur, `opensWith` d'une porte **référence** cet identifiant. C'est ainsi qu'un
  interrupteur et sa porte sont **liés** dans le fichier — par un nom partagé, pas par leur position.
  Le chargeur résout ensuite cette liaison en un `core::Mechanism` reliant les deux
  `GridPosition` (l'identifiant textuel `"s1"` n'existe que dans le fichier ; en mémoire, le lien
  est purement géométrique).

### Validation

Le chargement **valide** le contenu (`EX-LVL-004`) avant de produire un `Level` utilisable :
dimensions cohérentes avec les tuiles listées, tuiles toutes dans les bornes de la grille, aucune
case dupliquée dans la liste, **exactement une** tuile `Entry` et **exactement une** tuile `Exit`
(un niveau sans sortie, ou avec deux entrées, est une erreur de contenu, pas une situation ambiguë à
tolérer), et chaque liaison de mécanisme **résolue** (un `opensWith` qui référence un `id`
inexistant est une erreur).

En cas d'échec — JSON malformé, champ manquant, type de tuile inconnu, échec d'une des validations
ci-dessus — le chargeur ne lève **jamais d'exception** vers l'appelant (`EX-NFR-040`) : il renvoie
un `core::LevelLoadResult`, une paire `{ optional<Level> level, std::string error }`. `ok()` indique
le succès ; en cas d'échec, `level` est vide et `error` décrit le problème de façon exploitable
(affichable au joueur ou au développeur). Ce choix — résultat récupérable plutôt qu'exception —
garde la gestion d'erreur explicite à chaque site d'appel, cohérent avec le reste du moteur qui ne
s'appuie pas sur les exceptions pour son flux de contrôle normal.

## De la grille aux entités : \ref core::buildLevelScene "buildLevelScene"

Le `TileMap` d'un niveau n'est, en lui-même, qu'un tableau de types — il ne peut pas être
**affiché**. `core::buildLevelScene(world, level, régionParType)` fait le pont vers le rendu, en
projetant chaque tuile **non vide** en une **entité ECS** (@ref guide-ecs) portant un `Transform`
(position = colonne, ligne, converties en unités monde) et un `Sprite` (l'image à afficher, choisie
par la correspondance `régionParType` **injectée** en paramètre plutôt que codée en dur — ce qui
permet de tester la projection sans dépendre du rendu réel ou du GPU).

Point important : cette projection est **à sens unique** et n'a lieu **qu'une fois**, au chargement
du niveau. Le `TileMap` reste la **source de vérité** pour tout ce qui touche à la **collision** —
la physique (@ref guide-physique) interroge directement `isSolid(colonne, ligne)`, jamais les
entités-sprites générées. Modifier une entité-sprite ne changerait donc rien à la collision ; c'est
le mécanisme de portes ci-dessous qui montre comment un changement d'état du niveau doit réellement
se propager.

## Mécanismes déclencheur ↔ porte

`core::MechanismController` (logique **pure**, dans `Core/Gameplay`, sans dépendance rendu) donne
un **comportement** aux liaisons déclencheur↔porte que le modèle de niveau ne fait que
**représenter** (`EX-GP-020`, `EX-GP-021`, `EX-GP-025`). Deux types de déclencheur, deux
comportements — la nature de chacun est figée **une fois pour toutes** à la construction, d'après
la tuile d'origine (`TileType::Switch` ou `TileType::PressurePlate` à sa position) :

- à sa construction, le contrôleur **copie** la `TileMap` du niveau dans une grille de collision
  **mutable** qui lui est propre (`_collision`) — le `Level` d'origine, lui, ne change jamais.
  Chaque porte y est posée **fermée**, c'est-à-dire remplacée par `TileType::Solid`, au départ ;
- `update(playerBox, playerMass)` est appelé chaque pas fixe avec la boîte englobante du
  personnage et sa masse (`core::Player::mass`, `EX-GP-019`) :
  - **interrupteur** (`Switch`) : si la boîte **recouvre** la case, et que ce recouvrement vient de
    **commencer** ce pas-ci (un **front** — pas un contact déjà en cours, sinon rester debout sur
    l'interrupteur ferait osciller la porte à chaque pas), l'état de la porte **liée** **bascule**
    (ouverte ↔ fermée) — inchangé depuis LOT-12 ;
  - **plaque de pression** (`PressurePlate`) : la porte liée est ouverte **si et seulement si** la
    boîte recouvre la plaque **et** que la masse du personnage atteint le seuil `MIN_TRIGGER_MASS`
    (constante interne, calée par défaut sur la masse par défaut du personnage) — réévalué à
    **chaque** pas, sans notion de front : rester dessus la garde ouverte, en partir la referme
    **immédiatement**.
- à chaque changement d'état, la grille `_collision` est mise à jour en conséquence : porte
  ouverte → `TileType::Door` (franchissable), porte fermée → `TileType::Solid` (bloquante) ;
- la **physique** ne consomme jamais directement la `TileMap` du `Level` : elle reçoit
  `collisionMap()`, la grille du contrôleur. C'est ce fil qui fait qu'une porte fermée **bloque**
  réellement le personnage, sans jamais muter la carte d'origine du niveau — le `Level` reste la
  donnée statique de référence, le `MechanismController` porte l'état **dynamique** de la partie en
  cours.

Concrètement, avec l'exemple JSON ci-dessus : la porte en `(7, 6)` est solide tant que
l'interrupteur `s1` en `(3, 6)` n'a pas été touché ; dès que le personnage marche dessus, elle
devient franchissable — et se refermerait si l'interrupteur était retouché après être sorti du
contact (nouveau front). Une **plaque de pression** liée à la même porte, elle, la garderait
ouverte pas à pas tant que le personnage y resterait, sans qu'un second passage soit nécessaire
pour la refermer.

Le seuil de poids ne **discrimine** encore rien tant qu'un seul acteur (le personnage) pèse sur une
plaque — toute plaque s'active « prête à l'emploi ». Les blocs poussables (ci-dessous) existent
désormais, mais `MechanismController` ne les interroge pas encore : un bloc posé sur une plaque de
pression ne l'active pas — l'infrastructure de comparaison de poids est prête à l'accueillir, mais
le câblage bloc → plaque reste une évolution à venir.

## Blocs poussables

`core::BlockController` (logique **pure**, dans `Core/Gameplay`, sans dépendance rendu) fait vivre
les tuiles `TileType::Block` (`EX-GP-022`) : contrairement aux mécanismes ci-dessus, dont seul
**l'état** change, un bloc change de **position**. Chaque bloc occupe exactement **une case**,
jamais à mi-chemin — pousser ou tomber le déplace d'une case entière, comme une porte bascule d'un
état à l'autre sans étape intermédiaire.

- **Poussée** : `update(playerBox, moveIntentX, baseCollision)` teste si la boîte du personnage
  touche un bloc du côté vers lequel `moveIntentX` (@ref guide-entrees) l'entraîne ; si la case
  suivante dans cette direction est libre (ni solide, ni un autre bloc), le bloc avance d'une case.
  Appelé **avant** la physique du personnage (`hmi::GameScreen::update`), avec la boîte **laissée
  par le pas précédent** : un bloc qui vient de se dégager ne bloque donc jamais le personnage sur
  ce même pas — la case est déjà libre quand le balayage de collision (@ref guide-physique) s'exécute.
- **Chute** : un bloc dont la case du dessous est libre tombe d'une case, au rythme d'une case
  toutes les `BlockController::FALL_INTERVAL_STEPS` pas fixes — une chute **discrète** (case par
  case), pas une intégration continue comme celle du personnage (@ref guide-physique §2) :
  cohérent avec le reste des mécanismes de ce moteur, tous résolus case par case plutôt qu'en
  unités monde continues.

`collisionMap(base)` complète une grille déjà résolue par `core::MechanismController` (portes) avec
la position **courante** des blocs : chaque case marquée `Block` dans `base` — qui ne porte que la
position de **départ**, jamais mise à jour — est d'abord effacée, puis chaque position **courante**
est reposée comme solide. Sans cet effacement, une case quittée par un bloc resterait perçue comme
un mur, indéfiniment (un bug réellement rencontré en écrivant ce contrôleur, corrigé avant
livraison).

## Budget de mouvements

Un tableau **puzzle** peut vouloir limiter délibérément le nombre de sauts et/ou de dashs
disponibles, pour forcer le joueur à les utiliser avec parcimonie plutôt que librement
(`EX-GP-024`). Les compteurs eux-mêmes vivent dans le composant `core::Player`
(`jumpsRemaining`/`dashesRemaining`, @ref guide-ecs) — pas dans le `Level` : le niveau ne fait que
fournir la **valeur initiale** du budget (`jumpBudget`/`dashBudget`), copiée dans le composant au
moment où le personnage apparaît (spawn). Ensuite :

- chaque saut ou dash effectué **décrémente** le compteur correspondant (voir @ref guide-physique
  §3 et §4) ;
- une action dont le compteur est tombé à **zéro** est **refusée** par la physique, exactement comme
  si les autres conditions (au sol, dash disponible, …) n'étaient pas remplies ;
- la valeur spéciale **`-1`** signifie « illimité » — aucun décompte n'a lieu, et c'est la valeur
  par défaut d'un niveau qui ne précise pas de budget (voir l'exemple JSON : un niveau **sans**
  `jumpBudget`/`dashBudget` a des sauts et dashs illimités) ;
- les compteurs sont **réinitialisés** à chaque (re)chargement du niveau — un échec (retour au menu
  ou nouvelle tentative) rend donc le budget complet à nouveau, jamais définitivement épuisé d'une
  tentative à l'autre.

## Issue et enchaînement

`core::evaluateOutcome(playerBox, level)` est une fonction **pure d'observation** (elle ne modifie
rien, ne déclenche aucune transition) qui **classe** l'état courant du niveau à partir de la seule
position du personnage, en trois issues possibles (`core::LevelOutcome`) :

- `Won` : la boîte du personnage recouvre la case `Exit` ;
- `Lost` : contact avec une tuile `Danger`, **ou** le personnage est tombé sous la limite basse de la
  grille (une chute hors du niveau, typique d'un plateformer, sans mur invisible artificiel à poser
  en bas de chaque niveau) ;
- `Playing` : aucun des deux cas précédents — la partie continue.

L'ordre de classement est **déterministe et volontaire** : si, au même pas, le personnage se trouve
à la fois sur la sortie et au contact d'un danger (situation limite mais possible géométriquement),
l'**échec l'emporte sur le succès** — une règle simple et prévisible plutôt que dépendante de
l'ordre de test interne.

Cette fonction ne fait que **classer** l'état ; c'est côté écran que la transition a réellement
lieu. `hmi::LevelSequence` gère l'**ordre** des niveaux d'une session et l'enchaînement qui en
découle : une issue `Won` avance vers le niveau suivant de la séquence ; après le **dernier** niveau,
elle ramène à l'écran-titre plutôt que de tenter un niveau inexistant (`EX-LVL-010`/`EX-LVL-011`).

## Voir aussi
- `core::Level`, `core::TileMap`, `core::TileType`, `core::LevelLoader`, `core::LevelLoadResult`.
- `core::buildLevelScene`, `core::MechanismController`, `core::BlockController`, `core::evaluateOutcome`, `hmi::LevelSequence`.
- @ref guide-physique — comment le balayage consomme `isSolid`/`collisionMap()`.
- @ref guide-ecs — le composant `core::Player` qui porte les compteurs de budget.
