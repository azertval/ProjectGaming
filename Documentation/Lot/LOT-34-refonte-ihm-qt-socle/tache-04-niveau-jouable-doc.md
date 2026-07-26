# TACHE-04 — Niveau chargé/affiché/jouable dans le viewport ; documentation & vérification {#lot-34-tache-04-niveau-jouable-doc}

**Lot :** [LOT-34](epic.md) · **Emplacement :** `Source/Editor`, `Documentation` · **Statut :** non commencé

## Contexte
TACHE-02 fournit un viewport qui s'efface, TACHE-03 la boucle et les entrées. Cette tâche **branche le
rendu du jeu existant** dans le viewport pour atteindre l'objectif du lot : un niveau se **charge,
s'affiche et se joue** dans la fenêtre Qt, au rendu identique à l'exécutable historique. C'est le
critère qui **dérisque** l'intégration Qt ↔ D3D11 avant d'y bâtir l'IHM (LOT-35+). Elle clôt le lot
par la documentation et la vérification.

## Travail à réaliser
- **Assemblage de la scène de jeu** dans le viewport : charger un niveau via
  `core::LevelLoader::loadFromFile`, construire la scène (`core::LevelScene`/ECS `World`) et rendre via
  `hmi::SpriteRenderer` + `SpriteBatch` + `Camera2D` + `TextureAtlas` — **exactement** le chemin de
  rendu du jeu actuel (`GameScreen`). Réutiliser au maximum la logique de `GameScreen` (cadrage caméra
  par salle `RoomGrid`, interpolation `PreviousPosition`) : soit en instanciant `GameScreen` derrière
  le viewport, soit en extrayant sa boucle de rendu dans un composant réutilisable sans
  `IScreen`/`ScreenManager`.
- **Snapshot d'interpolation** : appeler `snapshotPreviousPositions` au début de chaque pas
  (parité `LOT-33`).
- **Entrées de jeu** : relier `InputState` (TACHE-03) + `GameKeyBindings`/`GamepadBindings` au
  contrôle du personnage (mêmes bindings, chargés depuis `keybindings.json`).
- Choisir un niveau de démonstration existant (`Source/Elements/Levels/*.json`) comme contenu de
  validation du lot (aucun nouveau niveau requis).
- **Documentation** :
  - Nouveau guide `Documentation/Guide/guide-ihm-qt.md` (ou section) : architecture de la cible Qt,
    pont viewport/HWND, boucle, entrées, coexistence avec l'exe legacy.
  - Mise à jour de `Documentation/Specification/architecture.md` : introduction de la frontière
    « UI Qt / rendu D3D11 embarqué / `Core` intact » ; déclaration des exigences `EX-IHM-001`,
    `EX-IHM-002`, `EX-BUILD-010`.
  - `Documentation/Lot/lots.md` déjà pourvu de `@subpage lot-34` ; l'epic référence ses tâches.

## Fichiers impactés
- `Source/Editor/` (assemblage scène/rendu de jeu ; éventuelle extraction d'un
  `GameView`/`GameRenderer` réutilisable depuis `HMI/Interface/GameScreen`).
- `Documentation/Guide/guide-ihm-qt.md` (nouveau), `Documentation/Specification/architecture.md`,
  et les fichiers d'exigences concernés.

## Tests (obligatoires)
- **Non-régression du jeu** : la suite existante (unitaires/intégration/système, dont
  `test_parcours_complet`) reste **verte et déterministe** — le rendu Qt ne touche pas `Core`.
- **Vérification manuelle** (dépendance GPU) : le niveau s'affiche identiquement à l'exe historique
  (tuiles, caméra par salle, interpolation) et se **joue** (déplacement/saut/dash clavier + manette).
- Toute logique extraite/réutilisée (p. ex. un `GameRenderer` sans `IScreen`) reste couverte par les
  tests existants ou de nouveaux tests si du comportement pur est déplacé.

## Points d'attention
- **Ne pas dupliquer la logique de `GameScreen`** : préférer l'extraction d'un composant de rendu
  réutilisable (partagé entre legacy et Qt le temps de la coexistence) plutôt qu'une copie, pour
  éviter la divergence.
- **Parité stricte** : mêmes couleurs de fond, même cadrage, même interpolation — toute différence
  visible signale une régression d'intégration à corriger avant de clôturer le lot.
- **Doxygen 1.9.8** : construire la doc localement avec la version CI avant push (piège connu du
  projet).

## Définition de fait (DoD)
- Un niveau est jouable dans la fenêtre Qt, rendu identique au legacy ; documentation d'architecture et
  guide Qt à jour ; tests verts et déterministes ; `/W4 /WX`, Doxygen, lint verts. Critères
  d'acceptation du [LOT-34](epic.md) satisfaits.

## Exigences
`EX-IHM-001`, `EX-IHM-002` (concrétisées), `EX-NFR-002` (déterminisme préservé) ; réutilise
`EX-REN-015`/`EX-ARCH-031` (caméra par salle, interpolation, `LOT-32`/`33`).
