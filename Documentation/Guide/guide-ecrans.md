# Écrans, navigation et boucle de jeu {#guide-ecrans}

Cette page explique comment l'application passe du menu au jeu, à l'éditeur ou aux options, ainsi
que la boucle de jeu complète ajoutée en `LOT-59` — pause, fin de niveau, sélection de niveau et
progression persistée. Elle relie les écrans décrits dans les autres pages (@ref guide-entrees pour
le menu et les options, @ref guide-niveaux et @ref guide-editeur pour ce que le jeu et l'éditeur
affichent une fois actifs). Depuis le `LOT-38`, l'application est une IHM **Qt** (@ref guide-ihm-qt) :
la navigation n'est pas une machine à états rejouée à chaque pas de simulation, mais un mécanisme
**événementiel** classique de Qt (widgets empilés + signaux/slots) piloté par une table pure.

## La machine à états : `hmi::ScreenFlow`

`hmi::ScreenFlow.h` (`Source/HMI/Interface/ScreenFlow.h`) porte la navigation comme une **table
pure**, sans dépendance Qt — testable hors instance d'application (`EX-NFR-010`), même patron que
`hmi::PanelFocus`/`hmi::ActionCatalog`. Deux fonctions :

- `hmi::resolveTransition(current, event)` : résout un `hmi::ScreenEvent` depuis l'état courant
  (`hmi::ScreenState`) vers le nouvel écran, ou `std::nullopt` si la transition est **interdite**
  depuis cet écran — jamais de bascule silencieuse (`EX-GP-041`).
- `hmi::dressingFor(screen)` : l'habillage attendu (docks visibles, barre de menus, barres
  d'outils, commandes d'édition, navigation manette) — ce que chaque ancien `showXxx()` répétait à
  la main.

`hmi::ScreenId` compte sept écrans : `Menu`, `Editor`, `Game`, `Options`, `Pause`, `NiveauTermine`,
`LevelSelect`. `MainWindow::transitionScreen(event)` est l'**unique** point d'entrée : il résout la
transition, puis applique l'habillage (`applyScreenDressing`) — aucun code ne bascule d'écran
autrement.

### Pages empilées et recouvrements : deux patrons distincts

`hmi::MainWindow` (un `QMainWindow`) héberge les écrans dans un **`QStackedWidget`** central, mais
tous n'y vivent pas de la même façon :

- **Pages normales** (une par écran hors jeu) : `Menu` (`hmi::MainMenu`), `Options`
  (`hmi::OptionsPage`), `LevelSelect` (`hmi::LevelSelectScreen`, `LOT-59` TACHE-06), et le
  **conteneur du viewport** partagé par `Editor`/`Game` (`hmi::GameViewport`). Ajoutées via
  `QStackedWidget::addWidget`, Qt gère leur taille.
- **Recouvrements** (`Pause`, `NiveauTermine`) : `hmi::PauseScreen` et `hmi::LevelCompleteScreen`
  ne sont **jamais** des pages de la pile — ce sont des widgets **frères** du conteneur du
  viewport (même parent, `_stack`), dont la géométrie est synchronisée à la main
  (`MainWindow::syncOverlayGeometry`, appelé à la construction et à chaque `resizeEvent`) et la
  visibilité pilotée par `applyScreenDressing` (`hide()`/`show()` + `raise()`). C'est le patron
  documenté par Qt pour superposer un widget à un `QWidget::createWindowContainer` : la fenêtre
  **native** qu'embarque le conteneur (le viewport Direct3D 11) peint toujours par-dessus ses
  propres **descendants** Qt — un widget enfant du conteneur ne s'afficherait donc jamais visible
  par-dessus lui. Un widget **frère**, en revanche, est un widget Qt ordinaire aux yeux du
  compositeur de fenêtres et se dessine normalement par-dessus. Conséquence directe :
  `Pause`/`NiveauTermine` ne basculent **aucune page** de `_stack` (`applyScreenDressing` laisse le
  conteneur du viewport affiché derrière eux) — c'est ce qui permet à la **scène de rester visible**
  derrière l'écran de pause ou de fin de niveau.

`LevelSelect`, à l'inverse, est une page normale : atteint depuis le **menu**, jamais en cours de
partie, il n'a aucune scène à laisser visible derrière lui.

## Qui déclenche les transitions : les signaux

Aucun écran ne bascule lui-même vers un autre : chaque écran **émet un signal** d'intention, et
`MainWindow` le **connecte** à la méthode qui appelle `transitionScreen` puis agit. Un écran est
ainsi **testable/éditable en isolation** (il émet ses signaux sans rien connaître des autres), et
ajouter un écran revient à ajouter une page/un recouvrement à la pile et un `connect` — sans
toucher aux écrans existants.

Le menu principal (`hmi::MainMenu`) expose six signaux depuis `LOT-59` (l'ancien `playRequested`
unique est devenu trois intentions distinctes, TACHE-06) :

```cpp
connect(_menu, &MainMenu::continueRequested,   this, &MainWindow::continueGame);
connect(_menu, &MainMenu::newGameRequested,    this, &MainWindow::newGame);
connect(_menu, &MainMenu::selectLevelRequested,this, &MainWindow::openLevelSelect);
connect(_menu, &MainMenu::editorRequested,     this, &MainWindow::showEditor);
connect(_menu, &MainMenu::optionsRequested,    this, &MainWindow::showOptions);
connect(_menu, &MainMenu::quitRequested,       this, &QWidget::close);
```

## Le viewport partagé : éditeur **et** jeu

Le même `hmi::GameViewport` sert de canevas à l'éditeur et de surface de jeu (une seule intégration
Direct3D 11 à maintenir, @ref guide-ihm-qt) :

- **`startGame(levels, startIndex)`** joue une **séquence** de niveaux via `hmi::GameSession` (@ref
  guide-niveaux), à partir de `startIndex` (0 = depuis le début ; « Continuer »/sélection de niveau
  reprennent plus loin, ci-dessous). `Échap`/bouton manette **B** ouvre désormais la **pause**
  plutôt que de quitter directement (`GameViewport::pauseRequested`, ancien comportement décrit par
  `EX-REN-031` avant `LOT-59`).
- en **mode édition**, le viewport peint sur `core::LevelDraft` et propose l'**essai immédiat** :
  jouer le brouillon courant puis y revenir exactement où l'édition en était — la même `GameSession`
  est réutilisée, sans duplication (@ref guide-editeur). L'essai (`stopPlaytest`) garde son propre
  chemin de retour, entièrement séparé de la pause/fin de niveau ci-dessous : `Échap` en essai
  revient directement à l'édition, jamais à un écran de pause.

Le viewport signale aussi ses messages d'état via **`statusMessage`** (barre de statut de la
fenêtre) — par exemple l'échec d'un enregistrement de brouillon invalide.

## Pause : suspendre sans perdre un pas

`Échap` (ou bouton **B** manette) en **partie réelle** ouvre l'écran de pause
(`hmi::PauseScreen`) : *Reprendre*, *Recommencer le niveau*, *Options*, *Quitter vers le menu*
(confirmation requise, la progression du tableau en cours serait perdue). L'exigence
(`EX-IHM-004`) est que la simulation soit **réellement suspendue**, pas ralentie — et c'est là
qu'un piège classique de l'accumulateur à pas fixe (@ref guide-boucle) apparaît.

**Suspendre, ce n'est pas multiplier `dt` par zéro.** Si `GameViewport::tick()` continuait
d'appeler `core::FixedTimestep::advance(0)` pendant la pause, la boucle consommerait quand même des
pas (zéro à chaque fois, mais des pas) — ou pire, laisserait l'illusion que le temps ne compte pas
alors que tout ce qui se mesure en **pas** (animations, dangers temporisés, budgets) resterait
figé sans que ce soit délibéré. La bonne suspension, c'est **ne pas appeler `advance()` du tout**
tant que `_paused` est vrai : l'accumulateur ne reçoit aucun temps réel à convertir, donc `steps`
vaut zéro par construction, sans même avoir à le vérifier.

**Le piège du réarmement d'horloge.** `_previousFrame` (l'instant de référence dont
`GameViewport::tick()` soustrait `now()` pour obtenir le temps écoulé) continue, lui, de s'écouler
pendant la pause — c'est une horloge murale, pas un compteur de pas. Si rien ne le corrige,
`GameViewport::resumeSimulation()` verrait au **premier** tick suivant un `elapsedSeconds` égal à
la **durée entière de la pause** (potentiellement plusieurs minutes) ; `FixedTimestep::advance`
rendrait alors des **dizaines de pas d'un coup** — le personnage traverserait le niveau. C'est
exactement la « spirale de la mort » que le plafond `maximumStepsPerCall` de `FixedTimestep`
atténue (@ref guide-boucle) sans jamais s'y substituer : un plafond limite les dégâts d'un
rattrapage, il ne l'empêche pas de commencer. La correction est de **réarmer** `_previousFrame` sur
l'instant courant au moment de la reprise :

```cpp
void GameViewport::resumeSimulation() {
    _paused = false;
    _previousFrame = Clock::now();  // sans ceci : rattrapage massif au premier tick suivant
}
```

**Le rendu continue** pendant la pause : la scène reste dessinée derrière l'écran (l'interpolation
d'affichage se fige naturellement, la position simulée n'avançant plus) — c'est ce que permet le
patron de recouvrement décrit plus haut.

**Piège annexe, la manette tenue.** La manette est *pollée*, pas événementielle : un bouton **B**
maintenu au moment précis où la pause s'ouvre laisserait, sans précaution, un front « bouton
pressé » périmé qui ferait immédiatement ressortir de la pause à la reprise (front fantôme). La
correction : `GameViewport::tick()` continue d'appeler `hmi::InputState::beginFrame()` à **chaque**
image même en pause (juste hors de la boucle de pas), pour que la ligne de base des fronts reste à
jour et qu'un bouton simplement *tenu* ne soit jamais relu comme *pressé* à la reprise.

## Fin de niveau et fin de séquence

Une issue `Won` (`core::evaluateOutcome`, @ref guide-niveaux) ne charge plus directement le niveau
suivant depuis `LOT-59` : `GameViewport::tick()` **fige** la simulation (`pauseSimulation` — même
suspension que la pause manuelle) et émet `levelSucceeded()`. `MainWindow::openLevelComplete()`
configure `hmi::LevelCompleteScreen` (même patron de recouvrement que la pause) avec le nom du
tableau et la variante — fin de **tableau** (*Continuer*/*Rejouer*/retour) si un tableau suivant
existe, fin de **séquence** (retour seul) sinon (`GameViewport::isLastGameLevel`) — puis ouvre
l'écran. *Continuer* et *Rejouer* **reprennent** la simulation (`resumeSimulation`, même réarmement
d'horloge que ci-dessus) avant de charger le tableau voulu — sans cette reprise, `_paused` resterait
vrai et la nouvelle session ne serait jamais avancée.

C'est aussi le point unique où la **progression** est marquée (ci-dessous).

## Sélection de niveau et progression persistée (`LOT-59` TACHE-05/06)

### Le modèle : `hmi::Progression`

`hmi::Progression` (`Source/HMI/Game/Progression.h`) est de la logique **pure**, sans Qt (comme
`core::LevelSequence`) : identifiant de séquence, tableau **atteint** (où reprendre), et
**ensemble** des tableaux terminés — le tout par **nom de fichier**, jamais par indice, pour
qu'un réordonnancement ou un ajout de tableau (`EX-LVL-013`) ne rende pas la progression fausse.
Persistée dans `Settings/progression.json` (à côté de `Settings/keybindings.json`), même patron
de lecture **tolérante** que `hmi::GameKeyBindings` (fichier absent/vide/malformé → partie neuve,
jamais bloquant, `EX-NFR-040`), mais avec une **écriture atomique** — fichier temporaire puis
`std::filesystem::rename`, comme `hmi::encodeImageFile` (`LOT-54`) — pour qu'une fermeture brutale
ne laisse jamais un fichier à demi écrit.

`MainWindow` la charge une fois à la construction et l'écrit à un **unique** point :
`openLevelComplete`, une fois par réussite, **avant** tout chargement du tableau suivant — jamais
par image ni par pas. Un tableau rejoué qui était **déjà** terminé (via *Rejouer*, ou en choisissant
un ancien tableau depuis l'écran de sélection) ne fait **pas** reculer le tableau atteint : seule
une **première** réussite l'avance.

### La règle de déverrouillage : `hmi::isLevelUnlocked`

Fonction **pure**, seule autorité sur ce qui est jouable — un écran ne fait que l'afficher, jamais
sa propre condition : un tableau est jouable s'il est déjà **terminé**, ou s'il est le **premier**
tableau **non terminé** de la séquence dans son ordre. Tous les suivants restent verrouillés.
`hmi::LevelSelectScreen` grise les entrées verrouillées (`Qt::ItemIsEnabled` retiré, ce qui les
exclut aussi de la navigation clavier/manette standard de Qt) ; `MainWindow::chooseSequenceLevel`
**revalide** quand même via cette fonction avant tout lancement — défense en profondeur, jamais un
tableau verrouillé lancé même par un chemin qui contournerait l'affichage.

### Le menu : trois intentions

*Continuer* (grisé sans progression, `MainMenu::setContinueEnabled`) reprend au tableau atteint.
*Nouvelle partie* efface la progression (confirmation si elle existe) et recommence au premier
tableau. *Choisir un niveau* ouvre `hmi::LevelSelectScreen` — une page normale (pas un
recouvrement, atteint depuis le menu) à deux onglets :

- **Séquence** : les tableaux, avec leur état (terminé/atteint/verrouillé).
- **Niveaux personnels** : tout le dossier de niveaux (`hmi::LevelFileOperations::list()`, déjà
  réutilisé par le panneau Niveaux de l'éditeur, @ref guide-editeur) **moins** les tableaux qui
  appartiennent à la séquence démo — sans ce second filtre, un tableau verrouillé serait apparu
  librement lançable dans cet onglet, contournant la règle de déverrouillage. Un niveau personnel
  se lance **seul** (`_gameTracksProgression = false` côté `MainWindow`) et ne touche **jamais** la
  progression de la séquence, même s'il est terminé.

## Où ça s'insère dans la boucle

L'**event loop Qt** (`QApplication::exec`) possède la navigation. La **boucle de jeu à pas fixe**
(déterminisme `EX-NFR-002`, @ref guide-boucle) ne tourne que **dans** le viewport, tant qu'il est en
mode jeu ou édition **et non en pause** : elle est cadencée par `QEvent::UpdateRequest`
(`QWindow::requestUpdate`) et rejoue exactement la discipline historique — sonder la manette,
convertir le temps réel en pas fixes, mettre à jour puis `hmi::InputState::beginFrame` **par pas**,
rendre **une fois** par frame avec interpolation (`EX-ARCH-031`). Le détail est dans @ref
guide-ihm-qt.

## Voir aussi
- `hmi::MainWindow`, `hmi::MainMenu`, `hmi::OptionsPage`, `hmi::GameViewport`, `hmi::GameSession`.
- `hmi::ScreenFlow`, `hmi::ScreenId`, `hmi::ScreenEvent`, `hmi::ScreenState`, `hmi::ScreenDressing`.
- `hmi::PauseScreen`, `hmi::LevelCompleteScreen`, `hmi::LevelSelectScreen`.
- `hmi::Progression`, `hmi::isLevelUnlocked`, `core::LevelSequence`, `core::LevelSequenceLoader`.
- @ref guide-ihm-qt — le socle Qt : viewport Direct3D 11 embarqué, boucle et entrées Qt.
- @ref guide-entrees — `hmi::MainMenu` et `hmi::OptionsPage`, deux écrans concrets et le remappage.
- @ref guide-niveaux, @ref guide-editeur — ce que le jeu et l'éditeur font une fois actifs, le
  format de la séquence de niveaux, et comment l'éditeur réutilise `hmi::GameSession` pour l'essai
  immédiat.
- @ref guide-boucle — l'accumulateur à pas fixe suspendu pendant la pause.
