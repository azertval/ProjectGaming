# IHM Qt (refonte) — socle applicatif & viewport Direct3D 11 {#guide-ihm-qt}

> Statut : **en cours** (`LOT-34`). Cette page décrit le socle de l'application Qt qui remplace
> progressivement l'IHM « maison ». Le rendu **du jeu** reste Direct3D 11 ; seule l'**interface
> hors-jeu** passe à Qt (voir [spécification IHM](@ref spec-interface-ihm)).

## Pourquoi Qt

L'IHM historique est dessinée quad par quad (police bitmap, aucune primitive de ligne, aucune
bibliothèque d'UI) : illisible pour les liaisons d'interrupteurs, fragile pour la liste des niveaux,
coûteuse à maintenir. La refonte (`LOT-34` → `LOT-39`) porte **toute l'UI hors-jeu** (éditeur, menus,
options) sur **Qt** — fenêtres dockables réglables hors code — tandis que le **rendu in-game reste
Direct3D 11**, embarqué dans un viewport Qt. `Core` n'est pas touché.

## Deux cibles, une bibliothèque de rendu

Le module `Source/HMI` est scindé (`LOT-34`) :

- **`HmiRuntime`** (bibliothèque statique) : rendu Direct3D 11 (`hmi::GraphicsDevice`,
  `hmi::SpriteBatch`, `hmi::SpriteRenderer`, `hmi::TextureAtlas`, `hmi::Camera2D`…), entrées
  (`hmi::InputState`, `hmi::GamepadPoller`, `*KeyBindings`), localisation, et la **session de jeu**
  (`hmi::GameSession`). Aucune UI « maison ». Réutilisée par les deux exécutables.
- **`ProjectGaming`** (exécutable historique) : fenêtre Win32, boucle, écrans et éditeur « maison ».
  Conservé **en parallèle** pendant toute la migration pour ne jamais casser le jeu (retrait au
  `LOT-38`).
- **`ProjectGamingEditor`** (`Source/Editor`, nouvel exécutable Qt) : `QApplication` +
  `QMainWindow`, dont le widget central est le **viewport Direct3D 11**.

La cible Qt est **optionnelle** côté build : sans Qt, la configuration CMake n'échoue pas (l'exe
historique se construit). Voir `External/README.md` pour le provisionnement de Qt (local, CI,
release).

## Le viewport : Direct3D 11 dans une fenêtre Qt

`editor::GameViewport` dérive de `QWindow` et est inséré dans la hiérarchie de widgets par
`QWidget::createWindowContainer`. Le principe :

1. Qt fournit un **`HWND` natif** (`winId()`), passé à `hmi::GraphicsDevice` qui y crée sa swap
   chain et y présente — Direct3D 11 dessine **directement** sur cette surface (aucun `QBackingStore`
   Qt). Le device est construit **paresseusement** au premier `exposeEvent` (handle natif valide).
2. Le redimensionnement (`resizeEvent`) répercute la nouvelle taille **en pixels physiques** (DPI :
   taille logique × `devicePixelRatio`) sur la swap chain.

## La boucle : Qt pilote, le pas fixe est préservé

Qt possède la boucle d'événements (`QApplication::exec`). Le rendu est cadencé par
`QWindow::requestUpdate` : chaque `QEvent::UpdateRequest` déclenche un tick qui **rejoue exactement**
la discipline de la boucle historique (voir @ref guide-boucle) :

1. sonde la manette (`hmi::GamepadPoller`) ;
2. convertit le temps réel écoulé en un nombre entier de **pas fixes** (`core::FixedTimestep`,
   déterminisme `EX-NFR-002`) ;
3. pour **chaque pas** : met à jour la simulation puis appelle `hmi::InputState::beginFrame`
   (fronts avancés par pas consommé, jamais par frame de rendu — `LOT-33`, `EX-CTRL-020`/`021`) ;
4. rend **une fois** par frame réelle, avec le facteur d'**interpolation**
   (`FixedTimestep::interpolationAlpha`, `EX-ARCH-031`) ;
5. reprogramme le tick suivant.

## Les entrées : événements Qt vers l'état partagé

Le viewport traduit les événements Qt vers le **même** `hmi::InputState` que la fenêtre Win32 :

- **Clavier** : `keyPressEvent`/`keyReleaseEvent` → `hmi::Key`. Les lettres/chiffres/espace partagent
  déjà leur valeur entre Qt et Win32 (`Qt::Key_A` == code virtuel `VK 'A'` == `0x41`) ; seules les
  touches spéciales (flèches, Échap, Tab, Maj, Ctrl, F1/F2/F10) passent par une petite table. Les
  répétitions automatiques (`isAutoRepeat`) sont ignorées.
- **Souris/molette** : positions en **pixels physiques**, molette en unités Win32 (`WHEEL_DELTA`).
- **Perte de focus** (`QEvent::FocusOut`) : `hmi::InputState::releaseAll` — aucune touche « collée »
  au retour d'un `Alt+Tab`.
- **Manette** : XInput sondé par `hmi::GamepadPoller`, extrait de la fenêtre Win32 pour être partagé.

## Jouer un niveau : `hmi::GameSession` réutilisée

Toute la logique de jeu d'**un** niveau (scène ECS, physique, mécanismes, blocs, dangers, caméra par
salle, animation, interpolation) est isolée dans `hmi::GameSession` — **sans dépendance d'écran**.
`update()` avance d'un pas fixe et renvoie l'issue (`core::LevelOutcome`, rechargement interne sur
échec) ; `render()` dessine via le `SpriteRenderer`. La même session est utilisée **et** par l'écran
de jeu historique (`hmi::GameScreen`, désormais un mince adaptateur) **et** par le viewport Qt :
aucune duplication, comportement identique (voir @ref guide-niveaux, @ref guide-physique).

## Éditeur : docks, palette, peinture (LOT-35)

`editor::MainWindow` est un **poste de travail à panneaux dockables** (`QDockWidget` — Palette,
Outils, Statut) autour du viewport central. La **disposition** est persistée hors code
(`QSettings` : restaurée au lancement, sauvée à la fermeture, réinitialisable ; `EX-IHM-011`).

- **Palette** (`editor::PalettePanel`) : un `QTreeView` alimenté par la taxonomie **pure**
  `editor::tileTaxonomy` (catégories → sous-groupes → tuiles, tous les `core::TileType` couverts,
  testé sans GPU). Sélectionner une tuile définit le type peint.
- **Édition dans le viewport** : le viewport affiche le brouillon (`core::LevelDraft`) via
  `hmi::DraftRenderer` (scène ECS reconstruite à la demande, rendue par le `SpriteRenderer`), caméra
  cadrant le niveau entier. Le **clic/glisser gauche peint** le type actif à la case survolée
  (`Camera2D::screenToWorld` → `LevelDraft::paintTile`) ; `Ctrl+Z`/`Ctrl+Y` annulent/refont.
- **Enregistrer** (`Ctrl+S`) : `LevelDraft::toLevel` (validation) → `core::LevelWriter` ; un
  brouillon invalide n'écrit rien et l'erreur s'affiche en barre d'état.
- **Essai immédiat** (`P`) : rejoue le brouillon validé via `hmi::GameSession` dans le viewport ;
  `Échap` restitue l'éditeur, brouillon intact — **la même session que l'écran de jeu**, sans
  duplication.

## Voir aussi
- [Spécification IHM](@ref spec-interface-ihm) — le *quoi/pourquoi* de la refonte (`EX-IHM-*`).
- @ref guide-boucle — la boucle et le pas de temps fixe (repris à l'identique côté Qt).
- @ref guide-entrees — la traduction des entrées en actions logiques (partagée).
- @ref guide-rendu — le rendu Direct3D 11 embarqué (pipeline inchangé).
