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
  d'`Espace` à une autre touche (`EX-CTRL-012`, concrétisé par `LOT-29`, voir plus bas), ou si le
  jeu ajoute le support d'une manette, il faudrait modifier chaque système qui teste directement les
  touches, au lieu d'un seul point de traduction ;
- **tests plus difficiles** — tester la physique du saut obligerait à simuler de vrais événements
  clavier Win32 plutôt qu'à construire directement une intention `{ jumpPressed: true }` ;
- **couplage entre `Core` et `HMI`** — `Core` n'a, par ailleurs, **aucune** dépendance à Win32
  (`EX-ARCH-010`, @ref guide-boucle) ; lui faire connaître des codes de touche briserait cette
  frontière architecturale.

Le trajet complet est donc : **touche physique → `hmi::InputState` (état brut) → `hmi::toPlayerInput` (traduction) → `core::PlayerInput` (intention) → `Core` (logique de jeu)**. Les deux
premières étapes vivent dans `HMI` (dépendantes de la plateforme), la dernière dans `Core`
(indépendante).

## Échantillonner plutôt que réagir : \ref hmi::InputState "hmi::InputState"

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

```cpp
// keyDown : vraie si enfoncée maintenant, quelle que soit la source (clavier OU manette).
bool InputState::keyDown(Key key) const noexcept {
    return _keysCurrent[index] || _gamepadCurrent[index];
}

// keyPressed (front montant) : enfoncée maintenant, mais ne l'était sur AUCUNE source
// à la frame précédente.
bool InputState::keyPressed(Key key) const noexcept {
    const bool keyboardEdge = _keysCurrent[index] && !_keysPrevious[index];
    const bool gamepadEdge = _gamepadCurrent[index] && !_gamepadPrevious[index];
    return keyboardEdge || gamepadEdge;
}
```

Remarque sur la fusion manette (détaillée plus bas) : `keyPressed` calcule un front **par source**
puis les combine par OU logique — un bouton manette pressé alors que la touche clavier équivalente
était déjà maintenue produit bien un nouveau front (celui de la manette), sans que le clavier
« masque » cette pression. C'est cette même logique de combinaison en lecture, jamais en écriture,
qui protège de tout effacement accidentel d'une touche (voir « La manette » ci-dessous).

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

### Un détail d'implémentation qui simplifie tout : \ref hmi::Key "Key" réutilise les codes Win32

`hmi::Key` est une énumération dont les valeurs coïncident **volontairement** avec les codes
virtuels (`VK_*`) de Win32. Ce choix n'a l'air que d'un détail, mais il évite une table de
correspondance entière : la couche de capture peut convertir un `WPARAM` Win32 en `Key` par un
simple `static_cast`, sans `switch` ni tableau de correspondance à maintenir. Ajouter le support
d'une nouvelle touche revient alors à ajouter un énumérateur nommé — aucune autre modification.

## Traduire l'état en intention : \ref hmi::toPlayerInput "hmi::toPlayerInput"

`hmi::toPlayerInput(input, gameKeyBindings, gamepadBindings)` est la fonction qui **seule** connaît
la correspondance entre touches/boutons et intentions ; elle produit un `core::PlayerInput` — le
**contrat** de données entre `HMI` et `Core` :

- `moveX` ∈ `[-1, 1]` : `-1` = gauche, `+1` = droite, `0` = immobile. Appuyer sur les deux touches
  simultanément **neutralise** l'intention (`moveX == 0`) plutôt que de privilégier arbitrairement
  l'une ou l'autre ;
- `moveY` : intention de **visée verticale**, utilisée uniquement pour orienter le **dash** — le
  déplacement au sol reste purement horizontal, ce champ ne sert qu'à choisir une direction de dash
  parmi les 8 possibles (@ref guide-physique §4) ;
- `jumpPressed` (front) / `jumpHeld` (maintenu) ;
- `dashPressed` (front, `EX-CTRL-013`), une action dédiée et distincte du saut.

`Core` ne reçoit **que** cette structure : il ignore totalement l'existence des touches/boutons
physiques.

**Deux sources indépendantes par action.** `hmi::GameAction` (Gauche/Droite/Sauter/Dash/Viser
haut/Viser bas) est l'action logique commune ; `gameKeyBindings` (`hmi::GameKeyBindings`) l'associe
à une touche clavier, `gamepadBindings` (`hmi::GamepadBindings`) à un bouton manette — chacune
remappable **indépendamment** de l'autre depuis Options (Touches de jeu / Touches de la manette),
persistées dans le même `Settings/keybindings.json` (sections distinctes). Pour chaque action,
`toPlayerInput` vérifie simplement `input.keyDown(gameKeyBindings.key(action)) ||
input.gamepadButtonDown(gamepadBindings.button(action))` : l'une des deux sources suffit, remapper
l'une n'affecte jamais l'autre — deux touches/boutons ne sont d'ailleurs jamais partagés par deux
actions du **même** ensemble (`GameKeyBindings`/`GamepadBindings::setKey` échangent plutôt que de
dupliquer une touche), ce qui évite par construction qu'une touche ne déclenche deux actions à la
fois.

`toPlayerInput` est une fonction **pure** (aucun état interne, aucun effet de bord) : lui passer les
mêmes `InputState`/bindings produit toujours le même `PlayerInput`, ce qui la rend testable sans
fenêtre ni minuteur, en construisant directement un `InputState` avec les touches/boutons voulus.
Elle est appelée **une fois par frame**, en amont de toute logique de jeu (`EX-CTRL-020`,
`EX-CTRL-021`) — jamais à l'intérieur du pas fixe lui-même, pour que tous les pas exécutés dans une
même frame (@ref guide-boucle) voient exactement la même intention.

## La manette : une seconde source, fusionnée en lecture (EX-CTRL-002, LOT-20)

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

Chaque bouton/direction manette synthétise le **même** `Key` fixe que son équivalent clavier par
défaut (D-pad/stick gauche → `Left`/`Right`/`Up`/`Down` ; **A** → `Enter` **et** `Space` ; **B**/
**Start** → `Escape` ; épaule droite → `Shift`, le dash) — câblage en dur dans
`Window::pollGamepad`, jamais remappé. Conséquence directe de la fusion : `MenuModel`,
`LevelPicker`, les raccourcis de l'éditeur n'ont jamais eu besoin d'être modifiés pour « apprendre »
la manette — ils lisent des `Key` fixes, non remappables (comme `Échap`/`Entrée` côté clavier), ce
qui satisfait déjà `EX-CTRL-010` par construction.

**Une seconde piste, brute et indépendante, pour les actions de jeu remappables** (`LOT-30`) :
`Window::pollGamepad` alimente aussi, à partir du **même** relevé XInput, un état brut par
`hmi::GamepadButton` (`onGamepadButtonDown`/`onGamepadButtonUp`, `InputState::gamepadButtonDown`/
`gamepadButtonPressed`) — dix boutons/directions (D-pad et stick gauche fusionnés en une seule
notion logique par direction, comme la piste `Key`). C'est cette piste, et non la fusion `Key`
ci-dessus, que consulte `toPlayerInput` via `gamepadBindings` : la navigation de menu reste sur des
`Key` fixes, la manette côté gameplay devient pleinement configurable sans toucher à la première
piste.

Le sondage XInput lui-même (`<Xinput.h>`) vit dans `hmi::Window` (`Platform`), jamais dans
`InputState` : `InputState` reste indépendant de toute fenêtre (`EX-NFR-010`), y compris pour
tester la fusion manette ou la piste brute — les tests appellent `onGamepadKeyDown`/
`onGamepadKeyUp`/`onGamepadButtonDown`/`onGamepadButtonUp` directement, sans manette réelle ni
`<Windows.h>`.

## Le menu d'options : la fusion manette à l'œuvre (\ref hmi::OptionsModel "hmi::OptionsModel"/\ref hmi::OptionsScreen "OptionsScreen")

Le menu d'options (accessible depuis le menu principal) illustre concrètement la fusion
ci-dessus : ses cinq entrées (bascule **V-Sync**, **Touches de jeu**, **Touches de l'éditeur**,
**Touches de la manette**, **Retour**) se naviguent identiquement au clavier, à la souris et à la
manette, sans un seul `if` dédié à cette dernière dans `OptionsModel::update` — il lit
`Key::Up`/`Key::Down`/`Key::Enter` comme `MenuModel` l'a toujours fait. `OptionsModel` réutilise
d'ailleurs directement les constantes de mise en page **publiques** de `MenuModel` (`MARGIN_X`,
`OPTIONS_TOP`, `OPTION_SPACING`, `OPTION_SCALE`) plutôt que d'en dupliquer un second jeu : un seul
style de liste verticale à chasse fixe pour tous les écrans qui en ont besoin.

**Défilement** (`LOT-30`) : à cinq entrées, `MenuModel::OPTION_SPACING` (pensé pour 2 à 4) ne tient
plus dans une fenêtre 720p — `OptionsModel` a gagné le même mécanisme que `hmi::LevelPicker`
(liste plate, pas l'accordéon de `hmi::TilePalette`) : `visibleOptionCount(viewportHeight)` borne
combien de lignes s'affichent sans défilement, le clavier fait suivre la sélection dans cette
fenêtre (`followSelection`), la molette défile sans la changer. `OptionsScreen` dessine une barre
de défilement (piste + curseur, texture d'atlas teintée) uniquement quand toutes les entrées ne
tiennent pas à l'écran — même patron visuel que `EditorScreen::renderPicker`.

`OptionsScreen` affiche aussi l'état de connexion de la manette (`InputState::gamepadConnected()`,
capturé à `update()` puisque `render()` ne reçoit pas `InputState`) — purement informatif, non
navigable.

## Remapper les touches et boutons : \ref hmi::GameKeyBindings "GameKeyBindings"/\ref hmi::EditorKeyBindings "EditorKeyBindings" (LOT-29), \ref hmi::GamepadBindings "GamepadBindings" (LOT-30)

Les entrées « Touches de jeu »/« Touches de l'éditeur »/« Touches de la manette » ouvrent chacune un
sous-menu (même patron de liste qu'`OptionsModel`, avec des constantes de mise en page **plus
compactes** — `GameKeybindingsModel::ROWS_TOP`/`ROW_SPACING`/`ROW_SCALE` — pour faire tenir 8 à 11
lignes là où `MenuModel` n'en prévoyait que 2 à 4). Chaque ligne affiche une action et sa touche ou
son bouton **actuel** (`hmi::keyDisplayName`/`hmi::gamepadButtonDisplayName`) ; confirmer une ligne
d'action entre en **capture** : la frame suivante qui voit une touche ou un bouton assignable pressé
(`hmi::capturedKey`, qui scrute les 256 codes suivis par `InputState` ; `hmi::capturedGamepadButton`,
qui scrute les dix `GamepadButton` — `Échap`/`Entrée` et `B`/`Start` exclus, réservés à la
navigation) la lie via `setKey` (`(Game|Editor)KeyBindings` ou `GamepadBindings`), qui **échange**
avec toute autre action déjà sur cette touche/ce bouton plutôt que de rejeter le conflit — jamais
deux actions du même sous-menu sur la même touche ou le même bouton. `Échap` pendant la capture
l'annule sans effet (même convention que `hmi::TextInputField`). Le sous-menu manette exige en plus
une manette connectée (`InputState::gamepadConnected()`) pour entrer en capture — sans quoi la
ligne sélectionnée affiche une invite dédiée au lieu d'attendre indéfiniment un bouton qui ne
viendra jamais.

`GameKeyBindings`/`EditorKeyBindings`/`GamepadBindings` sont trois classes **séparées** (pas de
généricité commune) : même mécanique (`key`/`button`, `setKey`, `resetToDefaults`, `load`/`save`),
mais des cas concrets distincts, pas de base commune anticipée au-delà. Elles partagent un **seul**
fichier de persistance (`Settings/keybindings.json`, sections `"jeu"`/`"editeur"`/`"manette"`) —
chaque `save` relit le fichier existant pour préserver les sections des deux autres classes plutôt
que de les écraser. `GameKeyBindings`/`EditorKeyBindings` stockent le code VK **brut** (`Key` en est
déjà un, voir plus haut) ; `GamepadBindings`, elle, stocke un **nom symbolique** (`"gauche"`, `"a"`,
`"rb"`, …) — `GamepadButton` n'a pas d'équivalent numérique naturel comme le VK du clavier, et un
nom reste lisible/stable si l'ordre de l'enum change. Un fichier absent, corrompu, ou une entrée
invalide (bouton inconnu compris) retombe sur les valeurs par défaut pour l'entrée concernée
(`EX-NFR-040`), jamais bloquant.

Le remappage éditeur ne couvre qu'un **sous-ensemble significatif** (Sauvegarder/Annuler/Refaire/
Copier/Coller/Test rapide/Grille/Aide/Renommer), pas l'intégralité des raccourcis d'`EditorScreen`
— navigation de menu, redimensionnement par flèches, `Ctrl+R`, `"0"`, `Tab`, Maj+clic restent câblés
en dur (décision de cadrage `LOT-29`, portée volontairement limitée). Le panneau d'aide de l'éditeur
(`F1` par défaut, `EditorScreen::renderHelp`) interpole les touches réellement liées via
`keyDisplayName` plutôt que des libellés fixes, pour ne jamais afficher un raccourci obsolète après
un remap.

## La langue de l'interface : \ref hmi::Localization "hmi::Localization" et \ref hmi::LanguageSelector "hmi::LanguageSelector"

Tous les textes d'interface (menus, options) passent par une **clé** stable (`EX-REN-033`) plutôt
que par un libellé en dur : `hmi::Localization` résout une clé (« menu.jouer ») vers la chaîne de
la **langue active**, chargée depuis un fichier `<langue>.lang` (format `clé = valeur`, un fichier
par langue dans `Source/Elements/Localization/`). La résolution suit un **repli déterministe** —
langue active, puis langue par défaut, puis la clé elle-même — pour ne jamais planter ni afficher
un vide si une traduction manque (`EX-NFR-040`), au prix, dans ce dernier cas, d'un texte
visiblement « brut » (la clé) plutôt qu'un texte manquant silencieux.

Le **bouton de langue** (drapeau, ancré en bas à droite de l'écran) suit la même séparation
logique/rendu que le reste de l'entrée : `hmi::LanguageSelector` est une géométrie pure (rectangle
cliquable, bascule français ↔ anglais) testable sans fenêtre, tandis que le dessin de l'icône
(`hmi::FlagIcons`, @ref guide-rendu) est une préoccupation strictement graphique. `MenuScreen`
et `OptionsScreen` interrogent tous deux `LanguageSelector::update` et **rechargent** le catalogue
(`Localization::loadLanguage`) au clic — un échec de chargement (fichier absent) est **récupérable** :
la langue courante est simplement conservée plutôt que de planter.

## Voir aussi
- `hmi::InputState`, `hmi::Key`, `hmi::GamepadButton`, `hmi::toPlayerInput`, `core::PlayerInput`.
- `hmi::Window::pollGamepad` — le sondage XInput et son intégration à `pumpMessages`.
- `hmi::MenuModel`, `hmi::OptionsModel`, `hmi::OptionsScreen`.
- `hmi::GameKeyBindings`, `hmi::EditorKeyBindings`, `hmi::keyDisplayName`, `hmi::capturedKey`,
  `hmi::GameKeybindingsModel`, `hmi::EditorKeybindingsModel` — remappage des touches (`LOT-29`).
- `hmi::GamepadBindings`, `hmi::gamepadButtonDisplayName`, `hmi::capturedGamepadButton`,
  `hmi::GamepadBindingsModel`, `hmi::GamepadBindingsScreen` — remappage manette (`LOT-30`).
- `hmi::Localization`, `hmi::LanguageSelector`.
- @ref guide-physique — comment la physique consomme `PlayerInput` (saut, dash, mouvement).
- @ref guide-boucle — pourquoi l'entrée est échantillonnée une fois par frame et non par pas fixe.
