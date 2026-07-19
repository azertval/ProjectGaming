# Éditeur de niveaux intégré {#guide-editeur}

Cette page explique comment le mode éditeur (menu « Mode Édition ») transforme le personnage
jouable, la caméra et le rendu déjà vus dans les pages précédentes en un **outil de création de
contenu**, sans écrire une seule ligne de nouveau moteur de rendu. Tout vit dans
`Source/HMI/Editor` et `Source/HMI/Interface/EditorScreen.*`, avec sa brique de données dans
`Source/Core/Levels/LevelDraft.*`/`LevelWriter.*`.

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

### La palette : une simple bande de rectangles cliquables

`hmi::TilePalette` ne dépend d'aucune ressource de rendu : c'est de la géométrie pure (des
rectangles en pixels écran) et un test d'appartenance point-dans-rectangle, exactement le motif
déjà vu pour `hmi::MenuModel` (@ref guide-entrees). Elle expose le type actuellement sélectionné ;
c'est `EditorScreen` qui, en réponse à un clic sur la grille, appelle
`draft.paintTile(colonne, ligne, palette.selected())`.

### Un clic, deux significations possibles

Un même bouton de souris doit soit peindre la grille, soit interagir avec la palette qui est
dessinée **par-dessus**. `EditorScreen::update` tranche une fois pour toutes, **au moment du front
de pression** (@ref guide-entrees), quelle intention ce geste sert :

```cpp
if (input.mouseButtonPressed(MouseButton::Left)) {
    if (palette.handleClick(mouseX, mouseY)) {
        paintingDrag = false;             // la palette a consomme le clic
    } else if (input.keyDown(Key::Shift)) {
        handleLinkClick(mouseX, mouseY);  // Maj+clic : liaison de mecanisme
        paintingDrag = false;
    } else {
        paintingDrag = true;              // ce geste peindra tant que le bouton reste enfonce
    }
}
```

Décider **une seule fois**, à la pression, évite qu'un glisser qui commence sur la palette et finit
sur la grille ne peigne accidentellement les cases survolées en cours de route.

### Lier deux tuiles sans dessiner de trait

`Maj` + clic sur un `Switch` puis (Maj toujours enfoncé) sur une `Door` les lie ; répéter la même
paire les délie — un simple bascule, mémorisé le temps d'un clic dans `_pendingLink`. Le choix de
ne **pas** dessiner de ligne reliant les deux tuiles est une contrainte du moteur de rendu, pas un
choix arbitraire : `hmi::SpriteQuad` (@ref guide-rendu) ne porte ni rotation ni épaisseur — un quad
est toujours un rectangle aligné aux axes. Tracer un trait entre deux cases quelconques demanderait
un quad **incliné**, que le pipeline actuel ne sait pas dessiner. L'éditeur associe donc les deux
tuiles liées par une **même teinte** (superposée en transparence, comme la surbrillance de survol) —
une solution qui reste dans les capacités du pipeline existant plutôt que d'en réclamer un nouveau.

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
optionnel : `P`, sur un brouillon valide, l'instancie (après avoir écrit le brouillon dans un
fichier temporaire — `GameScreen` ne sait charger qu'un chemin de fichier, @ref guide-niveaux) ;
tant qu'il existe, `update`/`render` lui **délèguent** entièrement la frame ; dès qu'il signale une
transition (Échap, ou niveau terminé), l'éditeur le détruit et reprend la main — le `LevelDraft` et
son historique n'ont, à aucun moment, été touchés.

## Enregistrer : valider avant d'écrire, jamais l'inverse

`Ctrl+S` appelle `draft.toLevel()` en premier. Si la validation échoue, **aucun fichier n'est
écrit** — le brouillon invalide reste en mémoire, avec un message d'erreur traduit en langage
compréhensible par un non-développeur (« il manque une sortie », plutôt que le texte technique du
validateur). Si elle réussit, `LevelWriter::saveToFile` écrit le JSON dans le dossier `Levels` de
l'application — le **même** dossier que `core::LevelLoader` lit au démarrage du jeu (@ref
guide-niveaux), garantissant qu'un niveau enregistré est immédiatement chargeable sans étape
supplémentaire.

## Choisir un niveau à éditer : `hmi::LevelPicker`

Avant d'entrer réellement en édition, `EditorScreen` affiche une liste — « Nouveau niveau » suivi
des fichiers `.json` déjà présents — navigable au clavier (`↑`/`↓`/`Entrée`), sur le même modèle
que `hmi::MenuModel` (@ref guide-entrees). `LevelPicker::forDirectory` fait le pont avec le disque
(scan du dossier `Levels`) ; son constructeur public, lui, prend une liste déjà résolue — une
séparation délibérée entre la **logique de navigation** (pure, testable sans système de fichiers)
et l'**accès disque** (non testé unitairement, comme le reste des E/S de `HMI`), le même principe
de séparation que `LevelLoader`/`Core` appliquent déjà à la validation.

## Voir aussi
- `core::LevelDraft`, `core::LevelWriter`, `core::LevelLoader`, `core::Mechanism`.
- `hmi::EditorScreen`, `hmi::TilePalette`, `hmi::LevelPicker`.
- @ref guide-niveaux — le modèle de niveau immuable, la validation et le format JSON réutilisés
  sans duplication.
- @ref guide-rendu — `SpriteBatch`/`Camera2D`/`TextureAtlas`, réutilisés tels quels pour dessiner
  la grille et la palette.
- @ref guide-entrees — la détection de fronts (`keyPressed`) et le motif de navigation clavier
  repris de `MenuModel`.
- @ref guide-physique — la simulation rejouée telle quelle pendant l'essai immédiat.
