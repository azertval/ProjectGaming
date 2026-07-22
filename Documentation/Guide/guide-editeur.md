# Éditeur de niveaux intégré {#guide-editeur}

Cette page explique comment le mode éditeur (menu « Mode Édition ») transforme le personnage
jouable, la caméra et le rendu déjà vus dans les pages précédentes en un **outil de création de
contenu**, sans écrire une seule ligne de nouveau moteur de rendu. Tout vit dans
`Source/HMI/Editor` et `Source/HMI/Interface/EditorScreen.*`, avec sa brique de données dans
`Source/Core/Levels/LevelDraft.*`/`LevelWriter.*`.

L'édition de tuiles de base (peindre, lier, redimensionner, annuler/refaire, enregistrer, essai
immédiat) date de LOT-14 ; la robustesse et le confort d'édition (nommage, garde-fous contre la
perte de travail, caméra manuelle, outils de zone, panneau latéral) datent de LOT-15 — cette page
couvre l'ensemble, sans distinguer les deux lots dans le texte.

## Le problème : éditer un niveau sans (re)coder le moteur

@ref guide-niveaux a montré que `core::Level` est **immuable** une fois construit : ses champs sont
posés au constructeur, sans mutateur. C'est un choix délibéré — un niveau **en cours de jeu** ne
doit jamais changer de forme sous les pieds du joueur. Mais un **éditeur**, par nature, fait
exactement l'inverse : poser une tuile, la retirer, déplacer l'entrée, doivent être des opérations
courantes, répétées des dizaines de fois par minute. Réutiliser `Level` tel quel pour l'édition
obligerait soit à le rendre mutable (fragilisant l'invariant « un niveau chargé est valide », dont
dépend tout le reste du moteur), soit à dupliquer sa logique dans un second type — exactement ce
que `EX-EDIT-010` interdit (« aucune duplication de la logique de niveau »).

La solution retenue : un type **distinct**, `core::LevelDraft`, qui porte toute la mutabilité, et
qui ne redevient un `Level` **validé** qu'au moment décisif (l'enregistrement ou l'essai), en
repassant par le chemin de validation déjà existant plutôt que d'en écrire un second.

## `core::LevelDraft` : un niveau qu'on peut défaire

`LevelDraft` reprend les mêmes données qu'un `Level` (nom, grille de tuiles, entrée, sortie,
mécanismes, budgets) mais expose des **mutateurs** : `paintTile`, `setEntry`, `setExit`,
`linkMechanism`, `unlinkMechanism`, `resize`. Deux invariants structurent tout le reste de la
page :

- **La grille de tuiles reste l'unique source de vérité.** Exactement comme pour `Level`
  (@ref guide-niveaux), une case `Entry`/`Exit`/`Switch`/`Door` dans `core::TileMap` **est** la
  donnée — `entry()`/`exit()` ne sont que des accès en cache, toujours resynchronisés par les
  mutateurs. Peindre autre chose par-dessus une entrée l'invalide automatiquement (et retire les
  liaisons de mécanismes qui la référençaient) : il ne peut jamais exister d'état où la grille dit
  une chose et le cache une autre.
- **`LevelDraft::toLevel()` ne réimplémente aucune règle de validation.** Plutôt que de vérifier
  « y a-t-il une entrée ? y a-t-il une sortie ? » une seconde fois, `toLevel()` **sérialise** le
  brouillon en JSON (via `core::LevelWriter`, ci-dessous) puis le fait passer par
  `core::LevelLoader::loadFromString` — le **même** chemin qu'un fichier chargé depuis le disque.
  Un brouillon incomplet produit donc exactement le même message d'erreur qu'un fichier de niveau
  mal formé (`EX-LVL-004`), sans qu'une seule règle de `LevelLoader` n'ait été dupliquée.

### Mécanismes : qui a le droit de se lier à qui

`linkMechanism(switchPosition, doorPosition)` exige (par assertion, @ref guide-journalisation) que
les deux cases portent déjà respectivement un `Switch` et une `Door` — lier ne **peint** rien, ça
n'associe que deux tuiles déjà posées. Relier une porte déjà liée **remplace** la liaison
précédente (une porte n'a qu'un seul interrupteur), alors qu'un même interrupteur peut ouvrir
plusieurs portes — cette asymétrie découle directement du format de fichier (@ref guide-niveaux) :
chaque tuile `door` porte un unique champ `opensWith`, mais plusieurs portes peuvent référencer le
même `switch.id`.

## `core::LevelWriter` : l'inverse du chargement, avec un piège

Écrire un niveau est presque l'inverse exact de `LevelLoader::loadFromString` (@ref guide-niveaux)
— parcourir la grille ligne par ligne, émettre un objet JSON par tuile non vide. Le piège tient
aux **identifiants d'interrupteurs** : le fichier source les porte (`"id": "s1"`), mais ni `Level`
ni `LevelDraft` ne les conservent après chargement — seules les **positions** sont retenues
(`core::Mechanism{switchPosition, doorPosition}`). `LevelWriter` doit donc **régénérer** des
identifiants à l'écriture, de façon déterministe (balayage de la grille, `"s0"`, `"s1"`, …). Cela
n'a aucune incidence sur le niveau rechargé : le nom exact d'un identifiant d'interrupteur n'est
qu'un détail du fichier texte, jamais une donnée de jeu — un interrupteur **non lié** à une porte
obtient tout de même un identifiant (le format l'exige), simplement absent de toute liaison.

Cette sérialisation sert `EX-EDIT-011` : sérialiser puis recharger un niveau produit un niveau
**équivalent**, jamais un niveau différent — la propriété qui rend `toLevel()` fiable.

## `EditorScreen` : peindre, c'est convertir un pixel en case

`hmi::EditorScreen` réutilise **exactement** l'infrastructure de rendu déjà vue en @ref
guide-rendu (`SpriteBatch`, `TextureAtlas`, `Camera2D`) — aucun nouveau pipeline graphique n'existe
pour l'éditeur. La seule nouveauté conceptuelle est l'**interaction** : convertir une position
souris en case de grille en composant deux briques déjà connues,
`Camera2D::screenToWorld` (@ref guide-rendu) puis `std::floor` (une position monde `4.7` désigne la
case `4`, pas la case `5`) :

```cpp
const core::Vector2 world = camera.screenToWorld(core::Vector2{mouseX, mouseY});
const int column = static_cast<int>(std::floor(world.x));
const int row = static_cast<int>(std::floor(world.y));
```

### La palette : une simple colonne de rectangles cliquables

`hmi::TilePalette` ne dépend d'aucune ressource de rendu : c'est de la géométrie pure (des
rectangles en pixels écran, empilés verticalement dans le panneau latéral — voir plus bas) et un
test d'appartenance point-dans-rectangle, exactement le motif déjà vu pour `hmi::MenuModel`
(@ref guide-entrees). Elle expose le type actuellement sélectionné ; c'est `EditorScreen` qui, en
réponse à un clic sur la grille, appelle `draft.paintTile(colonne, ligne, palette.selected())`.

### Un clic, plusieurs significations possibles

Un même bouton de souris doit soit peindre la grille, soit interagir avec le panneau latéral
(palette, barre d'outils) qui est dessiné **par-dessus**. `EditorScreen::update` tranche une fois
pour toutes, **au moment du front de pression** (@ref guide-entrees), quelle intention ce geste
sert — dans cet ordre de priorité :

```cpp
if (input.mouseButtonPressed(MouseButton::Left)) {
    if (palette.handleClick(mouseX, mouseY) || toolBar.handleClick(mouseX, mouseY)) {
        // panneau lateral consomme le clic
    } else if (mouseX < PANEL_WIDTH) {
        // reste du panneau (zone vide) : rien a faire, surtout pas peindre une case cachee dessous
    } else if (input.keyDown(Key::Shift)) {
        handleLinkClick(mouseX, mouseY);  // Maj+clic : liaison de mecanisme, quel que soit l'outil
    } else if (toolBar.selected() == EditorTool::Paint) {
        paintingDrag = true;              // peint tant que le bouton reste enfonce
    } else {
        areaDragActive = true;            // Rectangle/Selection : glisser definit une zone
    }
}
```

Décider **une seule fois**, à la pression, évite qu'un glisser qui commence sur le panneau et finit
sur la grille ne peigne accidentellement les cases survolées en cours de route. Le troisième cas
(`mouseX < PANEL_WIDTH`) mérite une explication : la caméra cadre la grille dans le **canevas**, à
droite du panneau (voir plus bas) — sans ce garde-fou, un clic dans une zone vide du panneau
pourrait, selon la position de la caméra, correspondre à une case de la grille **visuellement
cachée** sous le panneau, et la peindre à l'insu de l'utilisateur.

### Trois outils, une même grille : `EditorTool`

Au-delà du pinceau (peindre case par case), l'éditeur propose **Rectangle** (glisser définit un
rectangle, rempli du type sélectionné au relâchement) et **Sélection** (glisser mémorise une zone,
`Ctrl+C`/`Ctrl+V` la copient/collent ailleurs). Les trois sont un simple `enum class EditorTool`
porté par `hmi::ToolBar` — une classe de géométrie pure sur le modèle de `TilePalette`
(entrées cliquables, sélection courante), qui **ne connaît rien** au dessin ni à la grille. `Tab`
fait défiler les trois dans l'ordre (`ToolBar::selectNext`), indépendamment d'un clic sur la barre.

Rectangle et Sélection partagent la même mécanique de glisser (`areaDragActive`, case de départ
mémorisée à la pression, zone calculée au relâchement) mais des effets différents : Rectangle
mute le brouillon immédiatement, Sélection se contente de retenir des coordonnées. Changer d'outil
**pendant** un glisser en cours l'annule plutôt que de l'appliquer à moitié — un choix délibéré
pour qu'un changement d'avis ne produise jamais de mutation partielle et surprenante.

### Peindre par lot sans dupliquer la logique de peinture : `LevelDraft::paintRegion`

Remplissage rectangulaire et collage partagent le même besoin : appliquer un **bloc** de types de
tuiles en une seule fois, plutôt qu'une case. Une implémentation naïve dupliquerait la sémantique
de `paintTile` (déplacement d'entrée/sortie, nettoyage des liaisons) pour chaque case du bloc.
`LevelDraft::paintRegion` évite cela en factorisant cette sémantique dans une méthode privée sans
`pushUndo()` (`paintTileInternal`), appelée une fois par case du bloc ; `paintTile` devient
elle-même un appel à `paintRegion` avec un bloc `1×1`. Résultat : remplir un rectangle de 50 cases
ou peindre une seule case suivent **exactement** le même chemin de code, et ne poussent
**qu'un seul** instantané sur la pile d'annulation pour toute l'opération — cohérent avec le
principe « un geste = une mutation undoable » déjà en place pour le pinceau.

Le **copier** n'a besoin d'aucun ajout à `Core` : `EditorScreen` lit directement
`LevelDraft::tileMap()` (déjà publique) pour construire un presse-papiers local
(`std::vector<std::vector<TileType>>`). Seul le **coller** repasse par `paintRegion`.

### Lier deux tuiles sans dessiner de trait

`Maj` + clic sur un `Switch` puis (Maj toujours enfoncé) sur une `Door` les lie ; répéter la même
paire les délie — un simple bascule, mémorisé le temps d'un clic dans `_pendingLink`. Le choix de
ne **pas** dessiner de ligne reliant les deux tuiles est une contrainte du moteur de rendu, pas un
choix arbitraire : `hmi::SpriteQuad` (@ref guide-rendu) ne porte ni rotation ni épaisseur — un quad
est toujours un rectangle aligné aux axes. Tracer un trait entre deux cases quelconques demanderait
un quad **incliné**, que le pipeline actuel ne sait pas dessiner. L'éditeur associe donc les deux
tuiles liées par une **teinte partagée** (superposée en transparence, comme la surbrillance de
survol) — une solution qui reste dans les capacités du pipeline existant plutôt que d'en réclamer un
nouveau.

Une seule teinte pour **toutes** les liaisons (choix initial de LOT-14) devient ambiguë dès que
plusieurs mécanismes sont visibles à l'écran en même temps : impossible de savoir quelle porte va
avec quel interrupteur. LOT-15 assigne donc une teinte parmi un cycle fixe de six couleurs, **par
interrupteur** (selon son ordre d'apparition dans `LevelDraft::mechanisms()`) — une porte reprend
toujours la teinte de son interrupteur, y compris quand plusieurs portes partagent le même
interrupteur (elles héritent alors de la même couleur, ce qui reste cohérent : elles s'ouvrent
ensemble).

## Annuler/refaire : pourquoi des instantanés complets

Chaque mutateur de `LevelDraft` empile, **avant** de s'appliquer, une copie complète de l'état du
brouillon (`snapshot()`) sur une pile d'annulation ; `undo()` la restitue et bascule l'état courant
sur la pile de refaire, symétriquement pour `redo()`. Une nouvelle mutation après un `undo()` vide
la pile de refaire — l'historique reste **linéaire**, jamais arborescent, comme dans la plupart des
éditeurs grand public.

Le choix d'un **instantané complet** plutôt que d'un enregistrement différentiel (« quelle case a
changé ») est délibéré : un différentiel serait plus économe en mémoire, mais demande une logique
d'inversion propre à **chaque** type de mutation (annuler un redimensionnement n'est pas l'inverse
symétrique de le refaire, par exemple). Les niveaux de ce projet restent petits (une grille de
tuiles, quelques dizaines à quelques centaines de cases) : copier l'état entier à chaque étape est
largement assez rapide, et la garantie de correction (« l'état restitué est identique à l'octet
près ») est bien plus simple à établir qu'avec des deltas.

## Essai immédiat : jouer sans quitter l'éditeur

Appuyer sur `P` doit lancer une **vraie** partie sur le niveau en cours d'édition — avec le
personnage, la physique, les mécanismes (@ref guide-physique, @ref guide-niveaux) — puis, à
`Échap`, **revenir exactement où l'édition en était**. Deux architectures étaient possibles :

1. Utiliser `hmi::ScreenManager` (@ref guide-boucle évoque brièvement la navigation entre écrans) :
   transitionner vers `ScreenId::Game`, comme le fait le menu principal.
2. Faire vivre la session de jeu **à l'intérieur même** de `EditorScreen`.

La première option est **impossible sans perte** avec l'architecture actuelle : `ScreenManager`
**détruit** l'écran quitté à chaque transition (`_current = factory(target)` remplace le
`unique_ptr` précédent) — retourner à l'éditeur créerait une instance **neuve**, brouillon vierge,
historique perdu. C'est un excellent compromis pour naviguer entre le menu et le jeu (aucun état à
préserver), mais inadapté ici. `EditorScreen` embarque donc un `std::unique_ptr<GameScreen>`
optionnel : `P`, sur un brouillon valide, l'instancie ; tant qu'il existe, `update`/`render` lui
**délèguent** entièrement la frame ; dès qu'il signale une transition (Échap, ou niveau terminé),
l'éditeur le détruit et reprend la main — le `LevelDraft` et son historique n'ont, à aucun moment,
été touchés.

**Transmettre le niveau, pas un chemin de fichier.** `GameScreen` ne savait à l'origine charger
qu'une **séquence de fichiers** (`LevelSequence`, @ref guide-niveaux) — l'essai immédiat de LOT-14
écrivait donc le brouillon validé dans un fichier temporaire avant de le charger, pour réutiliser ce
seul chemin. LOT-15 ajoute un second constructeur, `GameScreen(batch, atlas, w, h, core::Level
level)`, qui accepte un niveau **déjà en mémoire** : la construction de scène (monde ECS, grille de
collision des mécanismes, personnage à l'entrée), auparavant enfouie dans
`loadLevel(chemin)`, est factorisée dans un `loadLevel(core::Level)` privé partagé par les deux
constructeurs — `loadLevel(chemin)` n'est plus qu'un chargement de fichier suivi d'un appel à cette
version en mémoire. `EditorScreen::startPlaytest` appelle directement ce second constructeur avec
`draft.toLevel()`, sans écrire ni lire aucun fichier. Bénéfice secondaire : un niveau qui **échoue**
pendant l'essai (danger, chute) se relance désormais via `loadLevel(*_level)` — le `Level` déjà tenu
en mémoire — plutôt que de relire le fichier source, aussi bien en mode séquence qu'en mode niveau
unique (`_sequence` devient un `std::optional<LevelSequence>`, absent dans ce second cas : atteindre
la sortie termine l'essai au lieu d'enchaîner, faute de niveau suivant).

## Enregistrer : valider avant d'écrire, jamais l'inverse

`Ctrl+S` appelle `draft.toLevel()` en premier. Si la validation échoue, **aucun fichier n'est
écrit** — le brouillon invalide reste en mémoire, avec un message d'erreur traduit en langage
compréhensible par un non-développeur (« il manque une sortie », plutôt que le texte technique du
validateur). Si elle réussit, `LevelWriter::saveToFile` écrit le JSON dans le dossier `Levels` de
l'application — le **même** dossier que `core::LevelLoader` lit au démarrage du jeu (@ref
guide-niveaux), garantissant qu'un niveau enregistré est immédiatement chargeable sans étape
supplémentaire.

**Traduire l'erreur sans deviner le texte.** La première version (LOT-14) devinait le message
non-développeur en cherchant des sous-chaînes françaises (`"entree"`, `"interrupteur"`…) dans le
message technique de `LevelLoader` — fragile : toute reformulation de ce message y aurait cassé la
traduction, silencieusement. `LevelLoadResult` porte désormais un `LevelValidationError` (énuméré,
`ParseError`/`InvalidEntryCount`/`UnresolvedMechanism`/…), rempli par `LevelLoader` en plus du
message technique (additif : aucune signature existante n'a changé, les tests et journaux qui
lisaient déjà `.error` continuent de fonctionner tels quels). `EditorScreen::describeValidationError`
bascule sur ce code, pas sur le texte.

**Écraser un fichier n'est jamais silencieux.** `saveDraft` compare le fichier `<nom>.json` qu'il
s'apprête à écrire au chemin d'origine du brouillon (`_loadedFrom`, absent pour un niveau tout juste
créé) : s'ils diffèrent et que le fichier cible existe déjà, une confirmation est posée avant
d'écrire — le même mécanisme générique que pour le redimensionnement destructeur (section
suivante).

## Garde-fous contre la perte de travail

Trois situations peuvent faire perdre du travail sans avertissement : redimensionner la grille de
façon à perdre l'entrée/la sortie/une liaison, écraser le fichier d'un **autre** niveau à
l'enregistrement, et quitter l'éditeur avec des modifications non enregistrées. Plutôt que trois
mécanismes séparés, `EditorScreen` porte un seul type interne, `PendingConfirmation` (un message et
une fonction `onConfirm` à exécuter si l'utilisateur accepte) : poser une confirmation **bloque**
le reste de l'interaction (peinture, liaison, redimensionnement supplémentaire) jusqu'à ce que
`Entrée` (exécute `onConfirm`) ou `Échap` (annule sans effet) la referme.

Détecter le redimensionnement destructeur ne duplique aucune règle de `resize` : une requête pure,
`LevelDraft::wouldResizeDropContent(largeur, hauteur)`, inspecte les positions actuelles de
l'entrée, de la sortie et des mécanismes contre les nouvelles bornes, **sans rien modifier** —
`EditorScreen` l'interroge avant d'appeler `resize`. Réimplémenter cette détection côté `HMI` aurait
demandé de rederiver la même logique de troncature que `resize` porte déjà ; l'exposer comme
requête pure évite cette duplication, dans le droit fil d'`EX-EDIT-010`.

Le drapeau `_dirty` (modifications non enregistrées) est mis à jour par **toute** mutation du
brouillon (peinture, liaison, redimensionnement, annuler/refaire, collage) et remis à `false` après
un enregistrement réussi — `Échap` avec `_dirty == true` pose une confirmation au lieu de quitter
directement.

## Nommer un niveau : un champ de saisie minimal

LOT-14 nommait tout nouveau brouillon `"Nouveau niveau"`, sans jamais appeler
`LevelDraft::setName()` — deux niveaux créés sans charger de fichier existant s'écrasaient donc
silencieusement au premier `Ctrl+S` (même nom de fichier). `hmi::TextInputField` corrige cela : un
champ de saisie **pur** (aucune dépendance rendu, comme `TilePalette`), qui consomme
`InputState::typedCharacters()` (les caractères tapés, capturés via `WM_CHAR` — distinct des codes
virtuels de `WM_KEYDOWN`, cf. @ref guide-entrees) et les touches `Retour arrière`/`Entrée`/`Échap`.

Un **validateur** optionnel (`std::function<bool(const std::string&)>`) conditionne la confirmation
: `hmi::isValidLevelName` refuse un nom vide ou contenant un caractère interdit par le système de
fichiers Windows (antislash, barre oblique, deux-points, astérisque, point d'interrogation,
guillemet droit, chevrons ouvrant/fermant, barre verticale) — une liste **noire** minimale, pas une
liste blanche
restrictive (les accents restent autorisés). Un nom refusé laisse le champ actif plutôt que de le
fermer, avec un message affiché par l'appelant (le champ lui-même ignore tout de la sémantique
« niveau », il ne fait que saisir et valider du texte).

Choisir « Nouveau niveau » dans le sélecteur ouvre ce champ **avant** de créer le brouillon — annuler
(`Échap`) à ce stade revient au sélecteur sans rien avoir créé, puisqu'aucun brouillon n'existe
encore. `F2`, en cours d'édition, ouvre le même champ pré-rempli du nom courant pour un renommage ;
annuler y laisse le nom inchangé.

## Un panneau plutôt que des bandes empilées

La palette et la barre d'outils vivaient à l'origine en bandes horizontales empilées dans le coin
haut-gauche de l'écran — au premier usage réel, cet empilement se recouvrait lui-même (aide,
palette, barre d'outils) et pouvait recouvrir la grille selon le cadrage. `Source/HMI/Editor/
EditorLayout.h` centralise désormais la disposition d'un **panneau latéral** vertical fixe
(constantes de marge, taille d'icône, pas de ligne), partagées par `TilePalette`, `ToolBar` et
`EditorScreen` — aucune des deux premières classes ne connaît l'autre, mais toutes deux s'alignent
sur les mêmes repères.

La caméra doit alors cadrer la grille dans le **canevas**, à droite du panneau, pas sur toute la
fenêtre : `renderGrid` calcule son zoom automatique sur `largeur_fenêtre - PANEL_WIDTH`, puis
décale le centre de la caméra vers la gauche (en unités monde, de la moitié de la largeur du
panneau convertie à l'échelle courante) pour que le milieu **apparent** de la grille tombe au milieu
du canevas plutôt qu'au milieu de la fenêtre entière. Un fond opaque, dessiné en premier, protège
en plus le panneau de tout débordement visuel — une seconde ligne de défense, pas une nécessité
stricte si le calcul de décalage est correct, mais bon marché et robuste aux cas limites.

**Caméra manuelle.** Molette (zoom) et glisser du bouton droit (pan) prennent le relais du cadrage
automatique dès la première interaction (`_manualCamera`) ; `0` y revient. Le zoom est borné : au
minimum le cadrage automatique lui-même (inutile de zoomer moins — il n'y a rien à voir au-delà du
niveau), au maximum la valeur qui laisse encore **4 cases visibles** sur le plus petit axe de
l'écran (une précision plus fine n'apporte rien pour poser un bloc). Ces deux bornes se recalculent
sur les dimensions **courantes** du brouillon à chaque molette, donc s'adaptent sans changement à
des niveaux plus grands (prévu pour un lot ultérieur).

**Grille de repère.** `F10` bascule l'affichage de fines lignes sur chaque bord de case,
par-dessus les tuiles déjà peintes — un repère visuel simple pour poser un bloc précisément, sans
bouton dédié dans le panneau (un raccourci clavier suffit, cohérent avec `F1`/`F2`/`Tab`). Capturer
`F10` a nécessité un ajout à la couche fenêtre : Win32 délivre cette touche (et les combinaisons
`Alt`+quelque-chose) via `WM_SYSKEYDOWN`/`WM_SYSKEYUP`, pas `WM_KEYDOWN`/`WM_KEYUP` — une convention
historique d'activation du menu, sans rapport avec l'absence de menu dans cette fenêtre.
`Window::handleMessage` enregistre désormais aussi ces deux messages dans `InputState` (comme les
autres touches), tout en laissant `DefWindowProcW` traiter le comportement système par défaut
(`Alt+F4`, `Alt+Tab`…) — sauf pour `F10` lui-même, absorbé (`return 0`) pour éviter l'activation
visuelle, inutile ici, du (non-)système de menu au relâchement.

**Découvrabilité.** Un libellé court accompagne désormais chaque entrée de la palette et de la
barre d'outils (texte dessiné par `EditorScreen`, la géométrie reste dans les classes pures). `F1`
bascule un aperçu compact de tous les raccourcis, dessiné dans le canevas plutôt que sous le
panneau — un indice discret (« F1 : aide ») le rappelle en haut à droite quand il est replié.

## Choisir un niveau à éditer : `hmi::LevelPicker`

Avant d'entrer réellement en édition, `EditorScreen` affiche une liste — « Nouveau niveau » suivi
des fichiers `.json` déjà présents — navigable au clavier (`↑`/`↓`/`Entrée`), sur le même modèle
que `hmi::MenuModel` (@ref guide-entrees). `LevelPicker::forDirectory` fait le pont avec le disque
(scan du dossier `Levels`) ; son constructeur public, lui, prend une liste déjà résolue — une
séparation délibérée entre la **logique de navigation** (pure, testable sans système de fichiers)
et l'**accès disque** (non testé unitairement, comme le reste des E/S de `HMI`), le même principe
de séparation que `LevelLoader`/`Core` appliquent déjà à la validation.

## Voir aussi
- `core::LevelDraft` (dont `paintRegion`, `wouldResizeDropContent`), `core::LevelWriter`,
  `core::LevelLoader` (dont `LevelValidationError`), `core::Mechanism`.
- `hmi::EditorScreen`, `hmi::TilePalette`, `hmi::ToolBar`, `hmi::EditorTool`, `hmi::LevelPicker`,
  `hmi::TextInputField`, `hmi::isValidLevelName`.
- @ref guide-niveaux — le modèle de niveau immuable, la validation et le format JSON réutilisés
  sans duplication.
- @ref guide-rendu — `SpriteBatch`/`Camera2D`/`TextureAtlas`, réutilisés tels quels pour dessiner
  la grille, la palette et le panneau latéral.
- @ref guide-entrees — la détection de fronts (`keyPressed`), le motif de navigation clavier repris
  de `MenuModel`, et la capture des entrées brutes (`WM_CHAR`, molette) sur laquelle s'appuient la
  saisie de texte et le zoom caméra.
- @ref guide-physique — la simulation rejouée telle quelle pendant l'essai immédiat.
