# Entrées et actions logiques {#guide-entrees}

Le gameplay ne dépend **jamais** d'une touche physique (`EX-CTRL-010`) : les entrées brutes sont
traduites en une **intention** neutre que `Core` consomme.

## Échantillonnage : `InputState`

`hmi::InputState` (dans `Source/HMI/Input`) capture l'état clavier/souris **une fois par frame**
(`EX-CTRL-021`) et expose les **fronts** — *pressée* (`keyPressed`), *maintenue* (`keyDown`),
*relâchée* (`keyReleased`) — nécessaires au *jump buffering* et à la hauteur de saut variable
(`EX-CTRL-011`). Il est **indépendant de la fenêtre** (aucun `<Windows.h>`), donc testable en
isolation : les événements peuvent être injectés directement.

Le cycle d'une frame : `beginFrame()` recopie l'état courant vers l'état précédent, la couche fenêtre
applique les événements Win32, puis la logique lit les fronts.

## Traduction : `toPlayerInput`

`hmi::toPlayerInput(input)` produit un `core::PlayerInput` — le **contrat** entre `HMI` et `Core` :

- `moveX` ∈ [-1, 1] : gauche (`←`/`Q`) / droite (`→`/`D`) ;
- `moveY` : visée verticale du dash (`↑`/`↓`) ;
- `jumpPressed` (front) et `jumpHeld` (maintenu) : `Espace`/`W` ;
- `dashPressed` (front) : `Maj` (`EX-CTRL-013`).

`Core` ne connaît **que** `PlayerInput` : il ignore les touches. Un futur remappage ou une manette
(`EX-CTRL-012`) ne toucherait **que** cette fonction — c'est tout l'intérêt de la dissociation.

## Voir aussi
- `hmi::InputState`, `hmi::Key`, `hmi::toPlayerInput`, `core::PlayerInput`.
- @ref guide-physique (la physique consomme l'intention).
