# Écrans et navigation {#guide-ecrans}

Cette page explique comment l'application passe du menu au jeu, à l'éditeur ou aux options — le
squelette de navigation qui relie les écrans décrits dans les autres pages (@ref guide-entrees pour
le menu et les options, @ref guide-niveaux et @ref guide-editeur pour ce que le jeu et l'éditeur
affichent une fois actifs). Depuis le `LOT-38`, l'application est une IHM **Qt** (@ref guide-ihm-qt) :
la navigation n'est plus une machine à états rejouée à chaque pas de simulation, mais un mécanisme
**événementiel** classique de Qt (widgets empilés + signaux/slots).

## Le principe : des pages empilées, pilotées par signaux

Il y a quatre « écrans » mutuellement exclusifs — **menu**, **options**, **éditeur**, **jeu** — et un
seul est visible à la fois. `hmi::MainWindow` (un `QMainWindow`) les héberge dans un
**`QStackedWidget`** central : une pile de widgets dont un seul est affiché. Trois pages y vivent :

- le **menu principal** (`hmi::MainMenu`) ;
- la **page Options** (`hmi::OptionsPage`) ;
- le **conteneur du viewport** (`hmi::GameViewport`), partagé par l'**éditeur** et le **jeu**.

Changer d'écran, c'est appeler l'une des méthodes privées de `MainWindow` qui sélectionne la bonne
page et ajuste l'habillage (barre de menus, panneaux dockables) :

| Méthode | Effet |
|---|---|
| `showMenu()`    | Affiche le menu ; masque docks et barre de menus. |
| `showOptions()` | Affiche la page Options (onglets). |
| `showEditor()`  | Affiche le viewport en **mode édition** + docks (Palette, Outils, Niveaux) + barre de menus. |
| `showGame()`    | Lance la **séquence de niveaux** dans le viewport, docks masqués. |

## Qui déclenche les transitions : les signaux

Aucun écran ne bascule lui-même vers un autre : chaque écran **émet un signal** d'intention, et
`MainWindow` le **connecte** au `show…` correspondant. C'est l'équivalent Qt du découplage
d'autrefois (un écran ne connaît pas les autres), mais porté par le mécanisme natif signaux/slots au
lieu d'une valeur de retour rejouée par un gestionnaire.

Le menu principal expose quatre signaux, connectés à la construction de `MainWindow` :

```cpp
connect(_menu, &MainMenu::playRequested,    this, &MainWindow::showGame);
connect(_menu, &MainMenu::editorRequested,  this, &MainWindow::showEditor);
connect(_menu, &MainMenu::optionsRequested, this, &MainWindow::showOptions);
connect(_menu, &MainMenu::quitRequested,    this, &QWidget::close);
```

Un écran est ainsi **testable/éditable en isolation** (le menu émet ses signaux sans rien connaître
du jeu), et ajouter un écran revient à ajouter une page à la pile et un `connect` — sans toucher aux
écrans existants.

## Le viewport partagé : éditeur **et** jeu

Le même `hmi::GameViewport` sert de canevas à l'éditeur et de surface de jeu (une seule intégration
Direct3D 11 à maintenir, @ref guide-ihm-qt) :

- **`startGame(levels)`** enchaîne une **séquence** de niveaux via `hmi::GameSession` (@ref
  guide-niveaux) : à chaque issue `Won` (`core::evaluateOutcome`), la session passe au niveau suivant
  **sans** quitter le viewport ; après le dernier niveau — ou sur `Échap` — le viewport émet
  **`exitToMenuRequested`**, que `MainWindow` connecte à `showMenu()`.
- en **mode édition**, le viewport peint sur `core::LevelDraft` et propose l'**essai immédiat** :
  jouer le brouillon courant puis y revenir exactement où l'édition en était — la même `GameSession`
  est réutilisée, sans duplication (@ref guide-editeur).

Le viewport signale aussi ses messages d'état via **`statusMessage`** (barre de statut de la
fenêtre) — par exemple l'échec d'un enregistrement de brouillon invalide.

## La séquence de niveaux démo

`MainWindow::startGame` fournit la **liste ordonnée** des niveaux démo
(`demo-deplacement.json` → `demo-saut.json` → … → `demo-final.json` → `demo-salles.json`) : un par
mécanique plus un niveau final combiné et un niveau à salles, dans un ordre de difficulté maîtrisé
(`EX-LVL-010`). Cette liste **doit** rester identique, dans le même ordre, à celle rejouée par le
test système `Source/Test/Systeme/test_parcours_complet.cpp` — un décalage entre les deux est
précisément le défaut qui a déclenché `LOT-25`. `scripts/check_demo_sequence.py` (CI) compare les
deux automatiquement.

## Où ça s'insère dans la boucle

L'**event loop Qt** (`QApplication::exec`) possède la navigation. La **boucle de jeu à pas fixe**
(déterminisme `EX-NFR-002`, @ref guide-boucle) ne tourne que **dans** le viewport, tant qu'il est en
mode jeu ou édition : elle est cadencée par `QEvent::UpdateRequest` (`QWindow::requestUpdate`) et
rejoue exactement la discipline historique — sonder la manette, convertir le temps réel en pas
fixes, mettre à jour puis `hmi::InputState::beginFrame` **par pas**, rendre **une fois** par frame
avec interpolation (`EX-ARCH-031`). Le détail est dans @ref guide-ihm-qt.

## Voir aussi
- `hmi::MainWindow`, `hmi::MainMenu`, `hmi::OptionsPage`, `hmi::GameViewport`, `hmi::GameSession`.
- @ref guide-ihm-qt — le socle Qt : viewport Direct3D 11 embarqué, boucle et entrées Qt.
- @ref guide-entrees — `hmi::MainMenu` et `hmi::OptionsPage`, deux écrans concrets et le remappage.
- @ref guide-niveaux, @ref guide-editeur — ce que le jeu et l'éditeur font une fois actifs, et
  comment l'éditeur réutilise `hmi::GameSession` pour l'essai immédiat.
- @ref guide-boucle — l'accumulateur à pas fixe qui cadence le viewport en mode jeu/édition.
