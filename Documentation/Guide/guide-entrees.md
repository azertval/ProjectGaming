# Entrées et actions logiques {#guide-entrees}

Cette page explique comment une touche physique devient un mouvement du personnage, et pourquoi ce
trajet passe par une étape intermédiaire qui, au premier abord, peut sembler superflue.

## Le principe : ne jamais coder « en dur » une touche dans le gameplay

Le gameplay ne dépend **jamais** directement d'une touche physique (`EX-CTRL-010`). Autrement dit,
`core::CharacterPhysicsSystem` ne contient **aucun** test du genre « si la touche flèche gauche est
enfoncée ». À la place, les entrées brutes (clavier, souris) sont d'abord traduites en une
**intention** neutre — « aller à gauche », « sauter » — que `Core` consomme sans jamais savoir
**quelle touche** a produit cette intention.

**Pourquoi cette indirection.** Sans elle, chaque système qui a besoin d'une entrée devrait
connaître les touches physiques, ce qui pose plusieurs problèmes concrets :

- **remappage impossible sans toucher la logique de jeu** — si un joueur veut réassigner « sauter »
  d'`Espace` à une autre touche, ou si le jeu ajoute le support d'une manette (`EX-CTRL-012`), il
  faudrait modifier chaque système qui teste directement les touches, au lieu d'un seul point de
  traduction ;
- **tests plus difficiles** — tester la physique du saut obligerait à simuler de vrais événements
  clavier Win32 plutôt qu'à construire directement une intention `{ jumpPressed: true }` ;
- **couplage entre `Core` et `HMI`** — `Core` n'a, par ailleurs, **aucune** dépendance à Win32
  (`EX-ARCH-010`, @ref guide-boucle) ; lui faire connaître des codes de touche briserait cette
  frontière architecturale.

Le trajet complet est donc : **touche physique → `hmi::InputState` (état brut) → `hmi::
toPlayerInput` (traduction) → `core::PlayerInput` (intention) → `Core` (logique de jeu)**. Les deux
premières étapes vivent dans `HMI` (dépendantes de la plateforme), la dernière dans `Core`
(indépendante).

## Échantillonner plutôt que réagir : `hmi::InputState`

Deux façons classiques d'observer les entrées existent :

- **piloté par les événements** : le code réagit immédiatement à chaque message du système
  d'exploitation (« touche enfoncée », « touche relâchée ») dès qu'il arrive, à un instant
  arbitraire ;
- **échantillonné** (*polling*) : le code lit un **état courant**, mis à jour une fois par frame, à
  un instant **prévisible** de la boucle de jeu.

Ce moteur choisit le second : `hmi::InputState` capture l'état clavier/souris **une fois par frame**
(`EX-CTRL-021`), pas à chaque événement Win32 individuel. **Pourquoi** : la logique de jeu tourne au
pas de temps fixe (@ref guide-boucle) — elle a besoin d'un état d'entrée **stable** pendant toute la
durée d'un pas, pas d'un flux d'événements arrivant de façon désynchronisée par rapport à ce
rythme. Échantillonner une fois par frame donne cette photographie stable.

### Détecter les fronts, pas seulement l'état

Savoir qu'une touche est enfoncée ne suffit pas : la physique a besoin de distinguer *plusieurs*
questions liées mais différentes, par exemple pour le saut (@ref guide-physique §3) :

- « le bouton **vient-il d'être pressé** cette frame ? » (le **front montant**) — déclenche le saut
  et alimente le *jump buffering* ;
- « le bouton est-il **maintenu** ? » — sert à la hauteur de saut variable (relâcher tôt écourte le
  saut) ;
- « le bouton **vient-il d'être relâché** ? » (le **front descendant**) — c'est ce relâchement,
  détecté précisément à la frame où il se produit, qui déclenche la coupe de vitesse ascendante.

`InputState` calcule ces trois fronts en gardant **deux instantanés** : l'état de la frame
**courante** et celui de la frame **précédente**. Une touche est « pressée » (front montant)
précisément quand elle est enfoncée maintenant mais ne l'était **pas** à la frame d'avant ; de même,
« relâchée » (front descendant) quand elle ne l'est plus mais l'était juste avant. Sans conserver
cet historique d'une frame, on ne pourrait connaître que l'état courant (`keyDown`), jamais le
moment précis de transition (`keyPressed`/`keyReleased`) — pourtant essentiel au *game feel* du
saut et du dash (des actions **ponctuelles**, à déclencher une seule fois par appui, pas à chaque
frame où la touche reste enfoncée).

### Le cycle d'une frame

1. `beginFrame()` recopie l'état **courant** vers l'état **précédent** — ouvre une nouvelle
   fenêtre d'observation ;
2. la couche fenêtre (`Window`, dans `HMI`) traite les messages Win32 de la frame et appelle
   `onKeyDown`/`onKeyUp`/`onMouseMove`/… sur l'`InputState`, qui met à jour l'état **courant** ;
3. la logique de jeu lit les fronts et l'état courant (`keyPressed`, `keyDown`, `keyReleased`, et
   leurs équivalents souris).

`InputState` est délibérément **indépendant de toute fenêtre** (aucune dépendance `<Windows.h>` dans
son en-tête) : les événements peuvent être injectés directement en test, sans ouvrir de fenêtre ni
passer par Win32, ce qui le rend testable en isolation (`EX-NFR-010`).

### Un détail d'implémentation qui simplifie tout : `Key` réutilise les codes Win32

`hmi::Key` est une énumération dont les valeurs coïncident **volontairement** avec les codes
virtuels (`VK_*`) de Win32. Ce choix n'a l'air que d'un détail, mais il évite une table de
correspondance entière : la couche de capture peut convertir un `WPARAM` Win32 en `Key` par un
simple `static_cast`, sans `switch` ni tableau de correspondance à maintenir. Ajouter le support
d'une nouvelle touche revient alors à ajouter un énumérateur nommé — aucune autre modification.

## Traduire l'état en intention : `hmi::toPlayerInput`

`hmi::toPlayerInput(input)` est la fonction qui **seule** connaît la correspondance entre touches et
intentions ; elle produit un `core::PlayerInput` — le **contrat** de données entre `HMI` et `Core` :

- `moveX` ∈ `[-1, 1]` : `-1` = gauche (`←` ou `Q`), `+1` = droite (`→` ou `D`), `0` = immobile.
  Appuyer sur les deux touches simultanément **neutralise** l'intention (`moveX == 0`) plutôt que de
  privilégier arbitrairement l'une ou l'autre ;
- `moveY` : intention de **visée verticale**, utilisée uniquement pour orienter le **dash**
  (`↑`/`↓`) — le déplacement au sol reste purement horizontal, ce champ ne sert qu'à choisir une
  direction de dash parmi les 8 possibles (@ref guide-physique §4) ;
- `jumpPressed` (front) / `jumpHeld` (maintenu) : `Espace` ou `W` ;
- `dashPressed` (front) : `Maj` (`EX-CTRL-013`), une action dédiée et distincte du saut.

`Core` ne reçoit **que** cette structure : il ignore totalement l'existence des touches physiques.
C'est précisément ce que cette dissociation a permis à la manette (`EX-CTRL-002`, ci-dessous) de
coûter **zéro** ligne dans `toPlayerInput` : la manette synthétise les **mêmes** `Key` que le
clavier, en amont, dans `InputState` lui-même — `toPlayerInput` continue de lire `Key::Space`,
`Key::Left`, etc. sans jamais savoir d'où ils viennent.

`toPlayerInput` est une fonction **pure** (aucun état interne, aucun effet de bord) : lui passer le
même `InputState` produit toujours le même `PlayerInput`, ce qui la rend testable sans fenêtre ni
minuteur, en construisant directement un `InputState` avec les touches voulues. Elle est appelée
**une fois par frame**, en amont de toute logique de jeu (`EX-CTRL-020`, `EX-CTRL-021`) — jamais à
l'intérieur du pas fixe lui-même, pour que tous les pas exécutés dans une même frame (@ref
guide-boucle) voient exactement la même intention.

## La manette : une seconde source, fusionnée en lecture (`EX-CTRL-002`, LOT-20)

`InputState` ne connaît qu'un seul `Key` par touche, mais **deux** sources indépendantes qui
peuvent l'enfoncer : le clavier (`onKeyDown`/`onKeyUp`) et la manette (`onGamepadKeyDown`/
`onGamepadKeyUp`), chacune avec sa propre paire courant/précédent. `keyDown`/`keyPressed`/
`keyReleased` **combinent** les deux (OU logique) au moment de la lecture — jamais à l'écriture.

**Pourquoi pas une seule table partagée ?** Parce que la manette est **sondée**, pas événementielle
: `Window::pollGamepad` interroge XInput une fois par frame et doit explicitement relâcher
(`onGamepadKeyUp`) chaque touche dont le bouton correspondant n'est plus enfoncé — y compris quand
la manette est débranchée. Si ce relâchement écrivait dans la **même** table que le clavier, il
effacerait une touche clavier réellement maintenue dès que la manette (absente ou relâchée) ne la
tient plus. Deux tables, combinées seulement en lecture, rendent ce bug structurellement
impossible plutôt que de compter sur la discipline du code appelant.

Chaque bouton/direction manette synthétise le **même** `Key` que son équivalent clavier (D-pad/
stick gauche → `Left`/`Right`/`Up`/`Down` ; **A** → `Enter` **et** `Space` ; **B**/**Start** →
`Escape` ; épaule droite → `Shift`, le dash). Conséquence directe de la fusion : **aucun**
consommateur de `Key` — `toPlayerInput` ci-dessus, `MenuModel`, `LevelPicker`, les raccourcis de
l'éditeur — n'a eu besoin d'être modifié pour « apprendre » la manette. C'est `EX-CTRL-010` (action
logique dissociée de la touche physique) satisfait par construction : le `Key` **est** déjà
l'action logique, sa source n'a jamais d'importance pour qui le lit.

Le sondage XInput lui-même (`<Xinput.h>`) vit dans `hmi::Window` (`Platform`), jamais dans
`InputState` : `InputState` reste indépendant de toute fenêtre (`EX-NFR-010`), y compris pour
tester la fusion manette — les tests appellent `onGamepadKeyDown`/`onGamepadKeyUp` directement,
sans manette réelle ni `<Windows.h>`.

## Le menu d'options : la fusion manette à l'œuvre (`hmi::OptionsModel`/`OptionsScreen`)

Le menu d'options (accessible depuis le menu principal) illustre concrètement la fusion
ci-dessus : ses deux entrées (bascule **V-Sync**, **Retour**) se naviguent identiquement au
clavier, à la souris et à la manette, sans un seul `if` dédié à cette dernière dans
`OptionsModel::update` — il lit `Key::Up`/`Key::Down`/`Key::Enter` comme `MenuModel` l'a toujours
fait. `OptionsModel` réutilise d'ailleurs directement les constantes de mise en page **publiques**
de `MenuModel` (`MARGIN_X`, `OPTIONS_TOP`, `OPTION_SPACING`, `OPTION_SCALE`) plutôt que d'en
dupliquer un second jeu : un seul style de liste verticale à chasse fixe pour tous les écrans qui
en ont besoin. `OptionsScreen` affiche aussi l'état de connexion de la manette
(`InputState::gamepadConnected()`, capturé à `update()` puisque `render()` ne reçoit pas
`InputState`) — purement informatif, non navigable.

## Voir aussi
- `hmi::InputState`, `hmi::Key`, `hmi::toPlayerInput`, `core::PlayerInput`.
- `hmi::Window::pollGamepad` — le sondage XInput et son intégration à `pumpMessages`.
- `hmi::MenuModel`, `hmi::OptionsModel`, `hmi::OptionsScreen`.
- @ref guide-physique — comment la physique consomme `PlayerInput` (saut, dash, mouvement).
- @ref guide-boucle — pourquoi l'entrée est échantillonnée une fois par frame et non par pas fixe.
