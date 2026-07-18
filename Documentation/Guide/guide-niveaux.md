# Niveaux : modèle, chargement, mécanismes, budgets {#guide-niveaux}

Un niveau est une **grille de tuiles typées** plus des métadonnées. Tout vit dans
`Source/Core/Levels` et `Source/Core/Gameplay` (comportement des mécanismes).

## Modèle

- `core::TileType` : `Empty`, `Solid`, `Danger`, `Entry`, `Exit`, `Switch`, `Door`.
- `core::TileMap` : grille dense `width × height` de `TileType`, origine haut-gauche, accès
  `tile(col, row)` et **solidité** `isSolid(col, row)`.
- `core::GridPosition` : coordonnées **entières** (colonne, ligne) — à distinguer de `Vector2`
  (continu).
- `core::Level` : nom + `TileMap` + **entrée** + **sortie** + liste de `core::Mechanism` (liaisons
  interrupteur↔porte) + **budgets** de mouvements (`jumpBudget`/`dashBudget`, `-1` = illimité).

## Chargement JSON

`core::LevelLoader::loadFromFile` / `loadFromString` lit un objet **JSON** (via **nlohmann/json**,
confiné au `.cpp`) : `{ name, width, height, jumpBudget?, dashBudget?, tiles: [ {x, y, type, …} ] }`.
Les liaisons de mécanismes sont exprimées par **identifiant** (`switch.id` ↔ `door.opensWith`) et
résolues en positions.

Le chargement **valide** (`EX-LVL-004`) : dimensions cohérentes, tuiles dans les bornes, pas de
doublon de position, exactement **une** entrée et **une** sortie, liaisons de mécanismes valides. En
cas d'erreur, il renvoie un **résultat récupérable** (`core::LevelLoadResult` : `optional<Level>` +
message), **jamais** d'exception vers l'appelant (`EX-NFR-040`).

## De la grille aux entités : `buildLevelScene`

`core::buildLevelScene(world, level, régionParType)` projette chaque tuile **non vide** en une
entité ECS (`Transform` + `Sprite`), prête à être rendue. Le `TileMap` reste la **source de vérité**
pour la collision.

## Mécanismes interrupteur ↔ porte

`core::MechanismController` (pur, `Core/Gameplay`) donne un **comportement** aux liaisons
(`EX-GP-020`, `EX-GP-021`) :

- il détient une **copie mutable** du `TileMap` — la **grille de collision** — où chaque porte est
  posée **fermée = `Solid`** au départ ;
- `update(boîteDuPersonnage)` : au **contact** d'un interrupteur (**front** : première frame
  seulement), l'état de la porte liée **bascule** ; la grille est mise à jour (`Door` ouverte
  franchissable / `Solid` fermée) ;
- la physique consomme `collisionMap()` — c'est ainsi qu'une porte fermée **bloque** réellement,
  sans muter la carte du niveau.

## Budget de mouvements

Un tableau peut **limiter** les sauts et/ou dashs (`EX-GP-024`). Les compteurs vivent dans
`core::Player` (`jumpsRemaining`/`dashesRemaining`), initialisés au spawn depuis le `Level`,
**décomptés** par la physique, **refusant** l'action à zéro, et **réinitialisés** au (re)chargement
du niveau.

## Issue et enchaînement

`core::evaluateOutcome(boîte, level)` classe l'état : **gagné** (recouvre la sortie), **perdu**
(danger ou chute sous le niveau), **en cours** — échec prioritaire, déterministe. Côté écran,
`hmi::LevelSequence` gère l'**ordre** des niveaux et l'enchaînement (réussite → suivant ; après le
dernier → titre, `EX-LVL-010`/`EX-LVL-011`).

## Voir aussi
- `core::Level`, `core::TileMap`, `core::TileType`, `core::LevelLoader`, `core::LevelLoadResult`.
- `core::buildLevelScene`, `core::MechanismController`, `core::evaluateOutcome`, `hmi::LevelSequence`.
