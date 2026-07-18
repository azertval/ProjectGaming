# Boucle de jeu et pas de temps fixe {#guide-boucle}

La boucle principale (`Source/HMI/main.cpp`) enchaîne, à chaque tour : **pompage des événements**
(fenêtre, entrées), **mise à jour** de la logique, **rendu**, **présentation**.

## Pourquoi un pas de temps fixe

Si la logique avançait de `Δt` = durée réelle de la frame, la simulation dépendrait de la vitesse de
la machine : mêmes entrées → résultats différents. Le moteur utilise donc un **pas de temps fixe**
(`core::FixedTimestep`) : la logique avance par incréments **constants** (1/60 s), découplés du
rendu. Cela garantit le **déterminisme** (`EX-NFR-002`, `EX-ARCH-030`) — indispensable aux tests
reproductibles et à un *game feel* stable.

## Fonctionnement

`core::FixedTimestep` accumule le temps réel écoulé et libère un **nombre entier** de pas fixes
(borné pour éviter la « spirale de la mort » si une frame est très longue). Chaque pas appelle la
mise à jour de l'écran courant (physique, mécanismes, règles) avec `fixedDelta` constant ; le rendu,
lui, se fait une fois par frame réelle.

Conséquence pratique : **toute** la simulation (`core::CharacterPhysicsSystem`, mécanismes) reçoit
`fixedDelta` et **jamais** le temps réel. Les tests exploitent cela en appelant `update(...,
1.0f/60.0f)` en boucle : c'est exactement ce que fait le jeu.

## Voir aussi
- `core::FixedTimestep`.
- `hmi::ScreenManager` (navigation entre écrans), `hmi::IScreen`.
- @ref guide-ecs, @ref guide-physique.
