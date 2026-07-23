# Changelog

Toutes les évolutions notables du projet sont consignées ici.
Format inspiré de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/) ;
le projet suit le [versionnage sémantique](https://semver.org/lang/fr/).

## [Non publié]

### Corrigé
- **Cahier de test illisible (page unique, 321 cas à plat)** : la page agrégeait tous les cas de
  test sur un seul niveau (mécanisme Doxygen `\xrefitem`, sans aucune section). Remplacée par
  `Documentation/CahierTest.md`, généré par le nouveau `scripts/generate_cahier_test.py` à partir
  des mêmes blocs `\castest{...}` (source de vérité inchangée), structuré selon l'arborescence de
  `Source/Test/` (Tests unitaires par module Core/HMI, Tests d'intégration et Système par fichier)
  — navigable via l'arbre latéral comme toute autre page. Vérifié en CI (`--check`).
- **Accumulateur à pas de temps fixe non utilisé dans `main`** (`EX-NFR-002`) : depuis
  l'intégration du menu principal (LOT-06), la boucle appelait `ScreenManager::update` une seule
  fois par frame réelle avec un delta constant, sans jamais mesurer le temps réel écoulé ni
  appeler `core::FixedTimestep::advance` — la simulation ne restait déterministe que par
  coïncidence, tant que le V-Sync cadençait l'affichage à 60 Hz. La mesure du temps réel
  (`std::chrono::steady_clock`) et la boucle d'accumulateur (rattrapage de plusieurs pas sur une
  frame lente, plafonné par `maximumStepsPerCall`) sont restaurées, conformément à
  `Documentation/Guide/guide-boucle.md`.
- **Sélection de niveau à la souris** (`hmi::LevelPicker`) : le sélecteur de niveau de l'éditeur
  (« Choisir un niveau ») ne répondait qu'au clavier (`↑`/`↓`/`Entrée`) — le survol et le clic
  gauche sélectionnent et confirment désormais un choix, comme le menu principal
  (`hmi::MenuModel`, même mise en page à chasse fixe).
- **Séparateur `*` accepté pour la taille de grille** (`hmi::parseLevelSize`, `Ctrl+R`) : en plus
  de `x`/`X`, un niveau peut être redimensionné en tapant `largeur*hauteur` (ex. `60*40`).

### Ajouté
- **Journalisation étendue pour le diagnostic** : trois nouvelles catégories de log
  (`Core/Gameplay/GameplayLog.h`, `Core/Levels/LevelsLog.h`, `HMI/Editor/EditorLog.h`), sur le
  modèle déjà établi (voir `Documentation/Guide/guide-journalisation.md`). Journalise désormais les
  bascules d'état des mécanismes (interrupteur/plaque de pression), le chargement d'un niveau et
  chaque raison d'échec de validation (`Core/Levels/LevelLoader`), les actions de l'éditeur
  (liaison/déliaison de mécanisme, annuler/refaire, redimensionnement), le nombre de niveaux trouvés
  par le sélecteur, et la connexion/déconnexion de la manette — toujours en dehors des chemins
  exécutés à chaque frame ou à chaque pas fixe (uniquement sur changement d'état réel).
- **LOT-20 — Manette et menu d'options** (`EX-CTRL-002`). Le jeu, le menu et l'éditeur sont
  désormais jouables/navigables à la **manette** (XInput) : D-pad/stick gauche pilotent les mêmes
  directions que les flèches/`Q`/`D`, **A** valide (menu) et saute (jeu), **B**/**Start**
  reviennent/échappent, l'épaule droite dashe. La manette est **fusionnée** dans `InputState` au
  niveau des mêmes touches déjà lues par tout le code existant (deux sources indépendantes
  combinées en lecture, jamais en écriture, pour ne jamais effacer une touche clavier réellement
  maintenue) : aucun consommateur (`MenuModel`, `PlayerInputMapper`, raccourcis de l'éditeur…) n'a
  été modifié. Nouveau **menu d'options** (4ᵉ entrée du menu principal) : bascule **V-Sync**
  (jusqu'ici fixée en dur), bouton de langue, état de connexion de la manette, retour au menu.
  **12 nouveaux tests** (321 au total, 309 au jalon précédent) ; vérifié visuellement dans
  l'application (menu, options, bascule V-Sync/langue).
- **LOT-19 — Physique newtonienne et plaque de pression** (`EX-GP-019`, `EX-GP-025`). Deux
  évolutions liées par le **poids** du personnage : la chute suit désormais un modèle
  **newtonien** — masse (`core::Player::mass`) et traînée proportionnelle à la vitesse, dont
  l'équilibre fait **émerger** une vitesse terminale progressive plutôt qu'un plafond arbitraire
  (`std::min`) ; calibrée pour retomber sur l'ancienne vitesse terminale (25 unités/s) à masse par
  défaut, seule la **courbe** change, pas le résultat final. La montée du saut reste inchangée
  (ressenti LOT-11 non affecté). Nouveau mécanisme de puzzle, la **plaque de pression**
  (`TileType::PressurePlate`) : ouvre la porte liée tant qu'un poids suffisant y repose,
  la referme dès qu'il en part — activation **continue**, à la différence de l'interrupteur à
  bascule (conservé tel quel). Réutilise l'infrastructure de liaison existante (identifiant/
  `opensWith`, éditeur Maj+clic généralisé aux deux types de déclencheur). Niveau de démonstration
  `demo5.json` ajouté à la séquence de jeu. **8 nouveaux tests** (305 au total, 297 au jalon
  précédent) ; vérifié visuellement dans l'application (courbe de chute, plaque de pression,
  éditeur).
- **LOT-18 — Animation du personnage** (`EX-REN-012`). Le personnage anime désormais trois
  **clips** dérivés de son état physique (`core::Player::grounded`, `core::Velocity`) : `Idle`
  (repos, 2 images, au sol et immobile), `Run` (course, 4 images, au sol en mouvement), `Jump`
  (saut, 1 pose fixe, en l'air) — un nouveau composant `core::Animation` et
  `core::AnimationSystem` (logique pure, `Core`, testée sans GPU) déterminent le clip et l'image
  courante chaque pas fixe, **après** `CharacterPhysicsSystem` (qui vient de calculer `grounded`
  pour ce même pas). Côté rendu, `TextureAtlas` expose une grille de 7 images (16×16, toujours
  **carrées** — non-régression du bug d'échelle de LOT-17) et `GameScreen` met à jour la région du
  sprite **à chaque frame** (plus seulement au spawn). Second des deux lots dédiés au personnage
  (`EX-REN-012`, après la silhouette statique de LOT-17). **5 nouveaux tests** d'intégration
  (297 au total, 292 au jalon précédent) ; vérifié visuellement dans l'application (repos, course,
  chute/saut).
- **LOT-17 — Sprite du personnage (statique)** (`EX-REN-011`). Le personnage n'est plus une tuile
  de couleur unie (`_atlas.tile(1, 1)`, cyan) : il affiche désormais une **silhouette humanoïde**
  (tête, cheveux, torse/manches, mains, jambes, chaussures) générée en code, comme le reste de
  l'atlas — aucun fichier image, aucune dépendance externe. La région reste **carrée** (16×16,
  comme une tuile) : c'est l'échelle du `Transform` (`core::playerSize()`, déjà non uniforme,
  0,4×0,8) qui donne au personnage sa proportion finale deux fois plus haute que large — une région
  déjà non carrée aurait doublé cet effet et fait déborder la silhouette de la boîte de collision.
  Elle vit dans la **même** texture que les tuiles (`TextureAtlas`, agrandie d'une bande sous la
  grille), plutôt que dans une classe séparée façon `SaveIcon`/`FlagIcons` : `SpriteRenderer` ne
  dessine qu'une seule texture
  par passe, étendre l'atlas existant évite toute restructuration du rendu. Premier de **deux
  lots** : ce lot livre une **pose statique unique** ; l'animation par séquence d'images
  (repos/course/saut, `EX-REN-012`) est explicitement un lot séparé à venir. Vérifié visuellement
  dans l'application ; `ctest` inchangé (292/292, aucune logique `Core` nouvelle — génération de
  texture non testable hors GPU).

## [0.0.2] - 2026-07-22

> Deuxième jalon : **éditeur de niveaux intégré**, du prototype (LOT-14) à l'outil de production
> (LOT-15 : nommage, garde-fous contre la perte de travail, caméra manuelle, outils de zone,
> panneau latéral, découvrabilité) puis aux grands niveaux (LOT-16 : saisie directe de taille,
> caméra qui cadre un niveau plus grand que la fenêtre). Un level designer peint, lie des
> mécanismes, redimensionne, annule/refait, enregistre et teste un niveau sans écrire de code —
> guide non-codeur inclus pour partager les niveaux via Git sans ligne de commande. **292 tests**
> (215 au jalon précédent), CI verte.
>
> Voir le [CHANGELOG](CHANGELOG.md) pour le détail par lot (LOT-14 → LOT-16).

### Ajouté
- **LOT-16 — Niveaux de grande taille** (`EX-EDIT-017`, `EX-REN-013` corrigée). L'éditeur permet
  désormais de **saisir directement** une taille cible (**Ctrl+R**, format `largeurxhauteur`, ex.
  `60x40`) plutôt que d'incrémenter case par case aux flèches (toujours disponibles pour l'ajustement
  fin) — un **plafond généreux** (100 cases par axe, garde-fou d'usage côté `HMI`, sans limite dans
  `Core`) s'applique aux deux voies, qui passent toutes deux par le même point de redimensionnement
  (`EditorScreen::requestResize`, même confirmation destructrice qu'avant si la nouvelle taille
  perdrait l'entrée/la sortie/une liaison). Le champ de saisie du nom (LOT-15) est généralisé pour
  porter aussi cet usage — un seul mécanisme de saisie de texte, pas un second construit en
  parallèle. **Un niveau plus grand que la fenêtre reste entièrement visible** : la caméra, qui ne
  descendait jamais sous le zoom ×1 (une partie de la grille restait hors champ, dans l'éditeur
  comme en jeu), zoome désormais **en dessous de ×1** quand c'est nécessaire pour cadrer le niveau
  entier — correction unique (`Camera2D::fitZoom`, testée) partagée par l'éditeur et `GameScreen`,
  sans dupliquer le calcul ; le zoom reste entier (netteté pixel art) pour tout niveau tenant déjà
  dans la fenêtre, aucune régression sur les niveaux livrés à ce jour. `EX-REN-013` reformulée pour
  refléter la stratégie réellement implémentée (caméra qui cadre le niveau, ne suit pas le
  personnage — la formulation d'origine ne correspondait déjà plus au code depuis LOT-08). Couvert
  par tests **unitaires** (`LevelSizeValidation`, `Camera2D::fitZoom`) ; vérifié manuellement dans
  l'application (grand niveau entièrement visible à l'ouverture et en essai immédiat, niveaux
  existants inchangés).
- **LOT-15 — Éditeur de niveaux : robustesse et confort d'édition** (`EX-EDIT-009`, `EX-EDIT-012` à
  `EX-EDIT-016`). L'éditeur intégré (LOT-14) se rapproche d'un outil de production : créer un
  niveau **demande un nom** (plus de collision silencieuse sur « Nouveau niveau.json ») et **F2**
  le renomme en cours d'édition ; enregistrer sous un nom qui **écraserait** un autre fichier, un
  **redimensionnement** qui supprimerait l'entrée/la sortie/une liaison, ou **Échap** avec des
  modifications **non enregistrées**, sont désormais **confirmés** avant d'agir. La caméra se
  **déplace** (glisser bouton droit) et **zoome** (molette) indépendamment du cadrage automatique
  (« 0 » y revient), bornée entre ce cadrage (rien à voir au-delà du niveau) et un maximum de
  **4 cases visibles** sur le plus petit axe (précision suffisante). Une **grille de repère**
  (lignes fines sur chaque bord de case) bascule au clavier (`F10`) pour simplifier le repérage
  d'une case avant d'y peindre. Deux nouveaux outils au-delà du pinceau : **Rectangle**
  (remplissage d'une zone glissée) et **Sélection** (`Ctrl+C`/`Ctrl+V`, copier/coller une zone de
  tuiles) — `Tab` fait défiler Pinceau/Rectangle/Sélection, la liaison de mécanismes (`Maj`+clic)
  restant disponible quel que soit l'outil actif. Découvrabilité : la palette et la barre d'outils
  rejoignent un **panneau latéral** vertical fixe (au lieu de bandes empilées pouvant se
  superposer entre elles ou avec la grille), avec un libellé à côté de chaque entrée ; un aperçu
  des raccourcis (`F1`) ; et des liaisons interrupteur↔porte teintées **différemment par
  interrupteur** (au lieu d'une seule teinte cyan partagée).
  Côté dette technique : l'essai immédiat (`P`) transmet désormais le niveau **directement en
  mémoire** à une session de jeu interne (plus de fichier temporaire partagé), et les messages
  d'erreur de validation s'appuient sur un **code d'erreur catégorisé** (`LevelValidationError`)
  plutôt que sur une recherche de sous-chaîne dans le message technique. `LevelDraft::paintRegion`
  (remplissage/collage) réutilise la sémantique cellule-par-cellule de `paintTile`, sans dupliquer
  de règle de niveau (`EX-EDIT-010`), en un seul snapshot undo par opération. Couvert par tests
  **unitaires** (`LevelDraft::paintRegion`/`wouldResizeDropContent`, `TextInputField`,
  `LevelNameValidation`, `ToolBar`, `InputState` — molette/texte tapé, `LevelLoader` — codes
  d'erreur) ; vérifié manuellement dans l'application pour la caméra et le glisser-déposer des
  outils de zone (logique non testable hors GPU, comme le reste d'`EditorScreen`/`GameScreen`).
- **LOT-14 — Éditeur de niveaux intégré** (`EX-EDIT-001` à `EX-EDIT-011`, `EX-EDIT-020` à
  `EX-EDIT-022`, `EX-EDIT-030`/`031`). Le menu **« Mode Édition »** ouvre désormais un véritable
  éditeur, intégré à l'application (même exécutable, même rendu Direct3D 11 que le jeu) : un
  sélecteur propose **« Nouveau niveau »** ou l'un des fichiers déjà enregistrés ; la grille se
  peint à la **souris** depuis une **palette** de tuiles (vide, solide, danger, entrée, sortie,
  interrupteur, porte) ; **Maj**+clic lie un interrupteur à une porte (répéter la paire la délie) ;
  les **flèches** redimensionnent la grille ; **Ctrl+Z**/**Ctrl+Y** annulent/refont n'importe
  quelle mutation (pile d'historique portée par `core::LevelDraft`, LOT-14) ; **Ctrl+S** valide
  puis enregistre le niveau (message clair, non technique, en cas d'échec — aucun fichier écrit) ;
  la touche **P** lance un **essai immédiat** du niveau en cours d'édition dans une session de jeu
  **intégrée** (Échap y met fin et restitue l'éditeur intact, brouillon et historique compris).
  Toute la logique de modèle/sérialisation/validation (`core::LevelWriter`, `core::LevelDraft`)
  vit dans `Core`, pure et testée sans GPU, et réutilise **sans duplication** la validation de
  `LevelLoader` (le brouillon se convertit en niveau en repassant par le même chemin que le
  chargement d'un fichier). Nouveau guide non-codeur (`Documentation/Manuel/partager-un-niveau.md`)
  expliquant, sans ligne de commande, comment créer un niveau et le partager via GitHub Desktop.
  Couvert par tests **unitaires** (`LevelWriter`, `LevelDraft` — mutateurs, undo/redo,
  `LevelPicker`, `TilePalette`) et un test **système** (parcours complet d'édition : peindre, lier
  un mécanisme, annuler, redimensionner, enregistrer, recharger, vérifier que le niveau produit est
  directement jouable). Vérifié manuellement dans l'application (peinture, palette, liaisons,
  redimensionnement, undo/redo, enregistrement sur disque, essai immédiat).

### Corrigé
- **`F10` (éditeur, LOT-15) ne déclenchait rien.** Win32 délivre cette touche (comme les
  combinaisons `Alt`+quelque-chose) via `WM_SYSKEYDOWN`/`WM_SYSKEYUP`, jamais
  `WM_KEYDOWN`/`WM_KEYUP` — une convention historique d'activation du menu, indépendante de
  l'absence de menu dans cette fenêtre. `Window::handleMessage` capture désormais aussi ces deux
  messages (comme les autres touches), en laissant `DefWindowProcW` traiter le comportement
  système par défaut (`Alt+F4`, `Alt+Tab`) — sauf pour `F10` lui-même, absorbé pour éviter
  l'activation visuelle, inutile ici, du (non-)système de menu au relâchement. Découvert lors d'un
  essai interactif par l'utilisateur.

## [0.0.1] - 2026-07-18

> Premier jalon : base stable du **moteur physique 2D** (personnage humanoïde, gravité asymétrique,
> saut/double-saut/wall-jump/dash, niveaux JSON avec mécanismes puzzle et budget de mouvements),
> **215 tests** verts, documentation consolidée (Guide du développeur + Cahier de test).

### Ajouté
- **LOT-13 — Consolidation de la documentation** : mise à jour des pages Doxygen et du `README.md`
  (section « Fonctionnalités du moteur », cibles de test unitaire/intégration/**système**). Nouvelle
  rubrique **Guide du développeur** (`Documentation/Guide/`) : accueil + 6 pages par domaine
  (physique, boucle de simulation, ECS, mathématiques, niveaux, entrées) expliquant chaque notion du
  moteur avec liens **code ↔ mathématiques** (balayage AABB, gravité, normalisation…). Nouvelle
  rubrique **Cahier de test** générée depuis les sources via une balise Doxygen avancée `\castest`
  (alias + `\xrefitem` agrégeant une page unique) : les **215 cas** (unitaires, intégration, système)
  sont annotés (titre, **catégorie**, sous-catégorie, **criticité**, **étapes** détaillées, **résultat
  attendu**), la documentation vivant **à côté du test** pour la maintenabilité. Aucun changement de
  comportement moteur.
- **LOT-12 — Niveau puzzle : mécanismes interrupteur/porte + budget de mouvements** (`EX-GP-020`, `EX-GP-021`, `EX-GP-024`). Les liaisons interrupteur↔porte (chargées depuis le LOT-07 mais inertes) deviennent **fonctionnelles** : un `MechanismController` (Core, pur) **bascule** l'état d'une porte au **contact** de son interrupteur (front) ; une porte **fermée** est **solide** (bloque), **ouverte** franchissable. La physique consomme une **grille de collision** reflétant l'état des portes (la carte du niveau reste la source de vérité). Nouveau **budget de mouvements par tableau** : un niveau peut limiter le nombre de **sauts** et/ou de **dashs** (`jumpBudget`/`dashBudget` dans le JSON, `-1` = illimité) ; à budget épuisé, l'action est **refusée**, et le budget est **réinitialisé** au (re)chargement. Le `GameScreen` exécute les mécanismes, initialise le budget au spawn, recharge le niveau à l'échec (mécanismes + budget remis) et **teinte** les portes selon leur état. Niveau puzzle `demo4.json` (interrupteur ouvrant la porte vers la sortie, budget serré) ajouté à la séquence. Couvert par tests **unitaires** (mécanismes, budget parsing), **intégration** (budget refusé au-delà, porte fermée bloque, `demo4` franchissable via l'interrupteur) et **système** (les 4 niveaux enchaînés).
- **LOT-11 — Ressenti avancé : personnage humanoïde, gravité asymétrique, finitions** (`EX-GP-018`). Le personnage n'est plus un carré 1×1 mais une **silhouette humanoïde** (0,4 × 0,8, collision **et** rendu), centrée dans la tuile d'entrée au spawn (`Core/Physics/PlayerSpawn.h`, source unique taille/placement). La **gravité** devient **asymétrique** : chute plus rapide que la montée (`fallGravityMultiplier`), **flottement à l'apex** (gravité réduite quand la vitesse verticale est faible) et **fast-fall** (maintenir « bas » accélère la chute, via `moveY`). La retombée reste à gravité **constante à multiplicateur près** (`EX-GP-011`). Toute la logique est **pure et déterministe au pas fixe**. Les preuves de franchissabilité (intégration) et le parcours **système** utilisent désormais la **vraie taille** du personnage. Couvert par tests **unitaires** (réglages, taille/spawn), **intégration** (chute > montée, apex hang, fast-fall) et **système** (les 3 niveaux franchis par l'humanoïde).
- **LOT-10 — Mécaniques aériennes avancées : double saut, wall jump, dash** (au-delà du MVP : nouvelles exigences `EX-GP-015`/`016`/`017`, `EX-CTRL-013` ; `EX-GP-013` assouplie). Le saut devient **contextuel** (sol/coyote, mur, ou saut aérien). **Double saut** (`EX-GP-015`) : N sauts aériens paramétrables, rechargés au contact du sol. **Wall jump + wall slide** (`EX-GP-016`) : contre un mur en l'air, le personnage glisse (chute plafonnée) et un saut l'éjecte **en diagonale opposée** au mur (avec verrou horizontal transitoire). **Dash** (`EX-GP-017`) : ruée à vitesse élevée dans l'une des **8 directions** (touches directionnelles, à défaut l'orientation), sur une courte durée, **gravité suspendue**, **une ruée par phase aérienne** rechargée au sol ; touche **Maj** (`EX-CTRL-013`). Toute la logique est **pure et déterministe au pas fixe** dans `Core`, câblée via `toPlayerInput`/`GameScreen`. Un troisième niveau `demo3.json` (couloir bas + fosse, **dash requis**) rejoint la séquence. Couvert par tests **unitaires** (mapping), **d'intégration** (double saut paramétrable, wall slide/jump, dash 8 directions, franchissabilité avec/sans mécanique) et **système** (parcours `demo → demo2 → demo3`).
- **LOT-09 — Saut, game feel et enchaînement de niveaux** : le personnage **saute** (`Espace`/`W`, `EX-GP-011`/`EX-GP-013`) — impulsion au sol, retombée sous **gravité constante** — avec un *game feel* de platformer : **hauteur de saut variable** (relâcher tôt = petit saut), **coyote time** (~80 ms, sauter juste après un bord) et **jump buffering** (~120 ms, saut pré-appuyé honoré à la pose). Les minuteries sont portées par le composant `Player` et décomptées au **pas fixe déterministe** (`EX-NFR-002`) ; l'intention de saut distingue *pressé*/*maintenu* (`EX-CTRL-011`, `toPlayerInput`). Le jeu enchaîne désormais une **séquence ordonnée de niveaux** (`LevelSequence`, `EX-LVL-010`) : à la réussite d'un niveau, le **suivant** se charge automatiquement ; après le **dernier**, retour au titre (`EX-LVL-011`). Deux niveaux livrés — `demo.json` (déplacement/chute) puis `demo2.json` (**marche ascendante, saut requis**). Couvert par tests **unitaires** (mapping saut, `LevelSequence`), **d'intégration** (saut au sol, pas de double saut, hauteur variable, coyote, buffering, franchissabilité avec/sans saut) et un premier test **système** (`Source/Test/Systeme/`) rejouant un **parcours complet** de la séquence sans la couche GPU.
- **LOT-08 — Gameplay personnage (jouable)** : le niveau statique devient **jouable**. Un **personnage** apparaît à l'entrée et est piloté au clavier — **actions logiques** dissociées des touches (`←`/`Q` gauche, `→`/`D` droite, `EX-CTRL-010`, `toPlayerInput`). La physique (`CharacterPhysicsSystem`) applique, **au pas fixe déterministe** (`EX-NFR-002`), le **déplacement horizontal** à vitesse constante (`EX-GP-010`), la **gravité continue** (`EX-GP-012`), et résout les **collisions** par **balayage continu** (`sweepAabb`) contre les tuiles solides — aucune traversée à vitesse élevée, glissement le long des surfaces (`EX-GP-014`). Atteindre la **sortie** revient au menu (`EX-GP-030`) ; toucher un **danger** ou tomber sous le niveau **redémarre** le personnage à l'entrée (`EX-GP-031`, `EX-GP-032`, `evaluateOutcome`). Jeu **par tableaux** : la caméra reste **fixe** et cadre le tableau (adaptation de `EX-REN-013`). Pas de **saut** ni de **mécanismes** (lots ultérieurs). Toute la simulation est **pure dans `Core`** (composants `Player`/`Collider`, `PhysicsConfig`, balayage `Aabb`) et **testée** (balayage, physique, règles, mapping) ; `HMI` ne fait que mapper les touches et afficher. Logique couverte par tests unitaires et d'intégration ; intégration `GameScreen` vérifiée visuellement.
- **Complétude des tests (audit)** : combler des trous de couverture — test unitaire de `approximatelyEqual` (tolérance relative-absolue, grandes magnitudes), test que le **catalogue français livré** (`fr.lang`) se charge et résout ses clés, et un test d'**intégration** « fichier de niveau livré (`demo.json`) → monde ECS » (une entité par tuile non vide). Rééquilibre unit/intégration.
- **LOT-07 — Niveaux (terminé)** : **« Charger niveau »** ouvre désormais un **vrai niveau** chargé depuis `Source/Elements/Levels/demo.json` (format JSON) au lieu de la scène codée en dur. `GameScreen` (TACHE-06) charge le fichier via `LevelLoader`, projette chaque tuile non vide en **entité ECS** (couleur d'atlas par type : solide, danger, entrée, sortie, interrupteur, porte) rendue par le `SpriteRenderer` (`EX-REN-010`, `EX-ARCH-012`), et ajuste la caméra pour cadrer le niveau. Échec de chargement **récupérable** (état neutre à l'écran, pas de plantage, `EX-NFR-040`) ; **Échap** revient au menu. Vérifié de bout en bout (grille de tuiles affichée conforme au fichier).
- **LOT-07 (TACHE-05)** : niveau de démonstration `Source/Elements/Levels/demo.json` (12×8, bords solides, entrée/sortie, danger, paire interrupteur/porte) au format JSON ; copié à côté de l'exécutable par CMake (comme les `.lang`). Un test unitaire vérifie que le niveau **livré se charge et se valide** via `LevelLoader`.
- **LOT-07 (TACHE-04)** : validation du niveau (`EX-LVL-004`) — le chargement rejette proprement, avec message exploitable, un niveau **incohérent** : tuile hors bornes, **positions en double**, entrée/sortie **absente** ou en **plusieurs exemplaires** (unicité), liaison de mécanisme non résolue. Couvert par tests unitaires.
- **LOT-07 (TACHE-03)** : chargement de niveau dans `Core/Levels` — `LevelLoader` lit le format **JSON** (objet `{name, width, height, tiles}`, `tiles` = liste d'objets `{x, y, type, …}`) via nlohmann/json et construit un `Level` ; liaisons interrupteur↔porte résolues par identifiant (`switch.id` ↔ `door.opensWith`). Renvoie un **résultat récupérable** (`LevelLoadResult`, jamais d'exception vers l'appelant, `EX-NFR-040`) : JSON malformé, champ manquant, type inconnu, tuile hors bornes, entrée/sortie absente, liaison non résolue. nlohmann/json reste **confiné au `.cpp`**. Couvert par tests unitaires (`EX-LVL-001`, `EX-LVL-003`).
- **LOT-07 (TACHE-02)** : modèle de niveau dans `Core/Levels` — `TileType` (`Empty`/`Solid`/`Danger`/`Entry`/`Exit`/`Switch`/`Door`), `TileMap` (grille dense typée, accès borné, `isSolid`), `GridPosition` et `Level` (nom + grille + entrée/sortie + mécanismes). Donnée pure, testable sans GPU (`EX-GP-001`, `EX-LVL-002`) ; couvert par tests unitaires.
- **LOT-07 (TACHE-01)** : dépendance **nlohmann/json** (v3.11.3, épinglée via FetchContent, *header-only*) ajoutée pour le parsing des fichiers de niveaux ; liée **en privé** à `Core` (`EX-NFR-031`).
- **Niveau de log configurable au lancement** : le niveau minimum du journaliseur (par défaut `Trace`) se règle via la variable d'environnement `PROJECTGAMING_LOG_LEVEL` ou l'argument `--log-level=<trace|info|warning|error>` (ce dernier prioritaire) ; une valeur non reconnue est ignorée et signalée. Un analyseur pur `core::parseLogLevel` (couvert par tests) fait la conversion. Permet de réduire le bruit en release ou d'augmenter le détail pour déboguer.
- **Bouton d'enregistrement des logs de la session** (**outil de développement, masqué en Release**) : un `MemoryLogSink` capture désormais tous les messages émis, et un **bouton** (icône « télécharger » générée en code) en bas à droite du menu, à gauche du drapeau, écrit la session dans `logs/session_AAAAMMJJ_HHMMSS.log` à côté de l'exécutable. Le bouton n'est présent que dans les builds de développement, via `core::kDeveloperBuild` (`if constexpr`, éliminé en Release). Logique du bouton (`SaveLogButton`) et sérialisation (`serializeSessionLog`) **pures et testées** ; l'action d'enregistrement est injectée dans `MenuScreen` (découplage de la journalisation/fichiers). Écriture **récupérable** (échec signalé, pas de plantage).
- **Journalisation des événements de cycle de vie sur tout le projet** (via l'infrastructure de log du LOT-02). Chaque module a sa **catégorie** de log (en-têtes dédiés, modèle `HMI/HmiLog.h`) : `Core`, `Ecs`, `Graphics`, `Platform`, `HMI`. Sont tracés : création et dimensions de la fenêtre (`Platform`), création des ressources de rendu — device/swap chain, `SpriteBatch`, atlas, police, drapeaux — et redimensionnement (`Graphics`), cadenceur à pas fixe et enregistrement des systèmes ECS (`Core`/`Ecs`), initialisation, **transitions d'écran** (Menu ↔ Jeu ↔ Éditeur), demande de fermeture et changement de langue (`HMI`). Les traces sont **pilotées par événement** — **aucune journalisation dans les chemins par frame** (rendu, mise à jour ECS) — et respectent les niveaux (`INFO` pour les jalons, `TRACE` pour le détail) ; les logs désactivés sont gratuits (garde `isEnabled` avant formatage). Objectif : rendre le déroulé de l'application observable pour le débogage plutôt qu'un fonctionnement silencieux.
- **LOT-06 (TACHE-08)** : sélecteur de langue au menu — un **bouton drapeau** en bas à droite affiche le drapeau de la **langue courante** et bascule entre **français** et **anglais** au clic (`EX-REN-033`). Ajoute le catalogue anglais (`en.lang`), des **icônes de drapeaux générées en code** (`FlagIcons` : France, Royaume-Uni) et une logique `LanguageSelector` **pure, testable sans GPU** (ancrage bas-droit, détection du clic, bascule) ; le `MenuScreen` recharge le catalogue au clic et redessine les libellés dans la nouvelle langue. Bascule **récupérable** (langue conservée si le fichier cible manque). Couvert par tests unitaires ; vérifié visuellement (menu fr ↔ en, drapeaux France / Royaume-Uni).
- **LOT-06 — Menu principal (terminé)** : au lancement, l'exécutable ouvre désormais un **menu principal** au lieu d'une scène codée en dur. `main` (TACHE-07) initialise les ressources partagées (device, `SpriteBatch`, atlas, `BitmapFont`, `Localization` chargée sur le français) et une boucle **pilotée par le `ScreenManager`** démarrant sur le `MenuScreen` : entrées échantillonnées **une fois par frame** (`EX-CTRL-021`), une mise à jour d'écran, puis effacement/rendu/présentation. « Charger niveau » ouvre la scène de démonstration, « Mode Edition » l'écran « à venir », « Quitter » ferme proprement ; **Échap** revient au menu (il ne ferme plus l'application). La scène de démonstration a quitté `main` pour le `GameScreen`. Vérifié de bout en bout (menu → jeu → menu → éditeur → fermeture).
- **LOT-06 (TACHE-06)** : écrans cibles dans `HMI/Interface` — `GameScreen` encapsule la **scène de démonstration** du LOT-05 (grille de tuiles + sprite mobile), avec son `core::World`+`MovementSystem` à pas fixe (`EX-ARCH-030`) et le rendu en lecture seule de l'ECS (`EX-ARCH-012`) ; `EditorScreen` affiche un placeholder « à venir » issu du catalogue de traduction (`EX-REN-032`). Depuis les deux écrans, **Échap** revient au menu. Chaque écran possède ses ressources (RAII), libérées à la transition.
- **LOT-06 (TACHE-05)** : écran de menu principal dans `HMI/Interface` — trois options (**Charger niveau**, **Mode Edition**, **Quitter**) navigables au **clavier** (flèches + Entrée, bouclage) **et à la souris** (survol + clic), l'option sélectionnée mise en évidence (`EX-REN-030`, `EX-CTRL-011`, `EX-CTRL-001`). Les libellés proviennent du **catalogue de traduction** par leur clé (aucun texte en dur). La logique de sélection/transition est isolée dans un `MenuModel` **pur, testable sans GPU** (`EX-NFR-010`), tandis que `MenuScreen` (`IScreen`) se limite au dessin via la police bitmap. Couvert par tests unitaires (navigation, bouclage, survol, clic, actions).
- **LOT-06 (TACHE-04)** : structure d'écrans dans `HMI/Interface` — interface `IScreen` (`update` renvoyant une intention de transition **rester / basculer / quitter**, `render`), `RenderContext` (ressources de rendu partagées) et `ScreenManager` qui détient l'écran courant, applique les transitions et fabrique l'écran suivant via une **fabrique injectée**. Découplé des écrans concrets (aucun écran n'en connaît un autre) et testable sans fenêtre ni GPU (`EX-NFR-010`) ; la demande « quitter » se propage à la boucle (`EX-REN-030`). Couvert par tests unitaires (exécution, basculement, fermeture).
- **LOT-06 (TACHE-03)** : catalogue de traduction (i18n) dans `HMI/Localization` — `Localization` résout tout texte d'interface par **clé** selon la **langue active**, chargée depuis un **fichier par langue** (`Source/Elements/Localization/<langue>.lang`, format `clé = valeur` UTF-8, commentaires `#`). **Repli déterministe** (langue active → langue par défaut → la clé elle-même) : une clé ou un fichier manquant est une erreur **récupérable**, jamais un plantage (`EX-REN-033`, `EX-NFR-040`). Logique pure, testable sans fenêtre ni GPU (`EX-NFR-010`) ; catalogue français fourni (`fr.lang`) et copié à côté de l'exécutable par CMake. Couvert par tests unitaires (analyse, repli, changement de langue, fichier absent).
- **LOT-06 (TACHE-02)** : rendu de texte dans `HMI/Graphics` — `BitmapFont`, police bitmap **monospace générée en code** (glyphes 5×7 intégrés couvrant ASCII imprimable, plus les lettres accentuées françaises composées d'une lettre de base et d'un diacritique). `drawText` dessine une chaîne **UTF-8** en **espace écran** (pixels) via le `SpriteBatch` : un quad par glyphe, teinte appliquée (glyphes blancs colorés par multiplication), échantillonnage *nearest* (`EX-REN-032`, `EX-ARCH-022`) ; les caractères non couverts sont ignorés sans plantage. Fournit aussi `textWidth`/`lineHeight` (centrage) et `screenProjection` (projection pixels → clip de l'interface). Ressources Direct3D en RAII. Vérifié visuellement (libellés du menu, jeu de caractères, accents, teintes).
- **LOT-06 (TACHE-01)** : entrées clavier/souris dans `HMI/Input` — `InputState` échantillonne l'état par frame et expose les fronts **pressée / maintenue / relâchée** (`EX-CTRL-011`) pour le clavier et les boutons souris, plus la position souris. Indépendant de toute fenêtre (aucun `<Windows.h>`), donc testable en isolation (`EX-NFR-010`) ; `hmi::Window` capture les messages Win32 (`WM_KEY*`, `WM_MOUSE*`) et échantillonne l'`InputState` une fois par frame en tête de `pumpMessages` (`EX-CTRL-021`), et expose `input()` + `requestClose()`. Couvert par tests unitaires (fronts déterministes).

### Modifié
- **Pas de console en Release** : l'exécutable est désormais bâti en **sous-système Windows** en Release (aucune fenêtre console — expérience utilisateur plus propre), tout en conservant la **console en Debug** (logs visibles). Le point d'entrée reste le `main()` standard (`/ENTRY:mainCRTStartup`). En conséquence, **aucun sink de log n'est ajouté en Release** (ni console ni capture mémoire) : pas de destination de log, pas de croissance mémoire.
- **LOT-06 (TACHE-01)** : la touche **Échap ne ferme plus** la fenêtre — elle devient une entrée normale (destinée au retour menu) ; la fermeture programmée passe désormais par `Window::requestClose()` (action « Quitter »), la croix continuant de fermer.

### Supprimé
- **Code mort** (audit projet) : retrait de fonctions publiques définies mais **jamais appelées ni testées** — `Camera2D::center()`/`zoom()`, `Logger::minimumLevel()`, `Localization::defaultLanguage()`. Le build `/W4 /WX` couvre déjà locals/paramètres/fonctions-fichier inutilisés ; l'API testée mais consommée au prochain lot (ex. `TileMap::isSolid`, `Level::entry/exit`, `MovementSystem`) et les macros de log uniformes (sans code généré) sont **conservées**.

### Corrigé
- La cible de tests `UnitTests` compile désormais aussi en **Release** : `test_assert.cpp` marquait `[[maybe_unused]]` manquant sur une variable non lue lorsque `PROJECTGAMING_ASSERT` est neutralisé en Release (`/W4 /WX`).

### Ajouté
- **LOT-05 — Rendu 2D (terminé)** : l'exécutable affiche désormais une **scène issue de l'ECS**. La boucle de `main` branche un `core::World` (avec le `MovementSystem`) cadencé à **pas fixe**, puis un **rendu découplé** en lecture seule. `main` construit une scène de démonstration (grille de tuiles + sprite mobile partiellement transparent) rendue via `SpriteRenderer` / `SpriteBatch` / `TextureAtlas` et la `Camera2D`. Première image réelle du jeu (tuiles, transparence, couches, déplacement déterministe).
- **LOT-05 (TACHE-05)** : système de rendu des sprites `SpriteRenderer` dans `HMI/Graphics` — parcourt `world.view<Transform, Sprite>()`, construit le quad monde de chaque entité, résout la région d'atlas en coordonnées de texture, trie par **couche** (tri stable) et dessine via le `SpriteBatch` en appliquant la projection de la caméra. Lecture seule de l'ECS (`EX-ARCH-012`) ; rendu découplé de la simulation.
- **LOT-05 (TACHE-04)** : caméra 2D `Camera2D` dans `HMI/Graphics` — projette le monde vers l'écran (16 px/unité, origine haut-gauche, Y-bas, zoom), fournit la matrice de projection pour le vertex shader et les conversions monde↔écran. Logique pure (DirectXMath), couverte par tests unitaires (conversions réciproques, projection).
- **LOT-05 (TACHE-03)** : atlas de textures procédural `TextureAtlas` dans `HMI/Graphics` — génère en mémoire une grille 4×4 de tuiles 16×16 (couleurs distinctes + une tuile à zones **transparentes** pour valider l'alpha), crée la `ID3D11Texture2D` et sa vue de ressource (RAII), et expose les régions (`tile(colonne, ligne)`). Génération déterministe, remplaçable plus tard par un chargement de fichier.
- **LOT-05 (TACHE-02)** : pipeline de rendu 2D `SpriteBatch` dans `HMI/Graphics` — dessine des quads texturés en Direct3D 11 (shaders HLSL compilés à l'exécution via `D3DCompile`), avec fusion **alpha** (transparence) et échantillonnage **nearest** (pixel art) ; API `begin`/`draw`/`end` avec *batching* (buffers dynamiques, RAII `ComPtr`). `GraphicsDevice` expose `device()`/`context()`.
- **LOT-05 (TACHE-01)** : composant `Sprite` (données pures) dans `Core/Ecs/Components` — région d'atlas (`AtlasRegion`, en pixels), couche de dessin et teinte (`Color` RVBA, blanc opaque par défaut). Agnostique du backend (aucun type DirectX) ; lu par le rendu de `HMI`. Couvert par tests unitaires.
- **LOT-04 — Documentation Doxygen (terminé)** : le site Doxygen est désormais une documentation **navigable** — page d'accueil (`index.md`), rubriques **Spécifications**, **Lots** et **Manuel utilisateur** (page « Télécharger et lancer »), en plus de la référence de code. Navigation hiérarchique (`@page`/`@subpage`) ; l'ordre des spécifications est porté par l'index (pas par des préfixes de fichiers). Garde-fou qualité `WARN_AS_ERROR` sur la génération. **Traçabilité des exigences** : chaque `EX-…` est une ancre Doxygen (`@ref`), avec un lint CI (`scripts/lint_exigences.py`) garantissant l'unicité des identifiants et l'absence de référence orpheline (mode `--next` pour le prochain numéro libre).

### Modifié
- **LOT-04 (TACHE-01)** : réorganisation de l'arborescence documentaire — `Specification/` et `Lot/` déplacés sous `Documentation/` ; le guide de conventions rejoint les spécifications (`Documentation/Specification/conventions.md`) ; les fichiers de spécification perdent leur préfixe numérique (`00-vision.md` → `vision.md`, …), l'ordre étant désormais porté par la navigation Doxygen. Références mises à jour dans tout le dépôt.

### Ajouté
- **Tests d'intégration** : première cible `IntegrationTests` (niveau *Integration*) — la démonstration de mouvement de l'ECS est reclassée depuis les tests unitaires (`test_ecs_mouvement.cpp`), et un test inter-lots vérifie que le cadenceur à pas fixe `FixedTimestep` (LOT-01) pilote correctement la simulation ECS (LOT-03) sans dérive et de façon déterministe (`test_boucle_simulation.cpp`).
- **LOT-03 (TACHE-06)** : premier composant réel et système de démonstration, bouclant l'ECS de bout en bout — composants données pures `Transform` (position, échelle, rotation, en unités monde) et `Velocity` (`Core/Ecs/Components`), et `MovementSystem` (`Core/Ecs/Systems`) qui applique `position += velocity * fixedDelta` aux entités possédant les deux composants. Déterministe, couvert par tests unitaires. **LOT-03 (fondation ECS & math `Core`) terminé.**
- **LOT-03 (TACHE-05)** : façade `World` et systèmes dans `Core/Ecs` — `World` possède les entités (`EntityManager`), les pools de composants (une par type, créée à la demande, via l'interface `IComponentPool`) et les systèmes ; API composants (`addComponent` / `getComponent` / `hasComponent` / `removeComponent`), vues (`view<...>()`) et cycle de vie (`destroyEntity` retire l'entité de **toutes** les pools). Interface `ISystem` (`update(World&, float)`) ; `World::update` exécute les systèmes **dans l'ordre d'enregistrement** au pas de temps fixe. Couvert par tests unitaires.
- **LOT-03 (TACHE-04)** : vue multi-composants dans `Core/Ecs` — `View<Components...>` itère exactement les entités possédant **tous** les composants demandés, pilotée par la plus petite pool (filtrage sur les autres). Deux formes d'usage : `for (auto [entity, ...] : view)` et `view.each([](Entity, A&, B&){ ... })`, avec accès en référence aux composants. Couvert par tests unitaires.
- **LOT-03 (TACHE-03)** : stockage de composants dans `Core/Ecs` — `ComponentPool<T>`, sparse set typé (tableau dense contigu + tableau creux indexé par entité) avec `add` / `get` / `has` / `remove` / `removeIfPresent` ; suppression par swap-and-pop préservant la densité ; `get`/`remove` sur entité absente traités par assertion de précondition. Couvert par tests unitaires.
- **LOT-03 (TACHE-02)** : entités de l'ECS dans `Core/Ecs` — `Entity` (handle générationnel `index`+`generation`, `INVALID_ENTITY`) et `EntityManager` (`create` / `destroy` / `isAlive`, recyclage des index par liste libre avec incrément de génération pour invalider les handles périmés). Couverts par tests unitaires.
- **LOT-03 (TACHE-01)** : types mathématiques de `Core` dans `Core/Math`, sans dépendance DirectX — `Vector2` (opérateurs, produit scalaire, longueur, normalisation, égalité approchée), `Rect` (bords, `contains`, `intersects`, origine haut-gauche / Y-bas) et `MathUtils.h` (`approximatelyEqual`, `kEpsilon`). Couverts par tests unitaires.
- **LOT-02** implémenté : journalisation & diagnostics dans `Core/Diagnostics` — niveaux de log, `Logger` (filtrage + sinks), `ConsoleLogSink` / `MemoryLogSink`, macros `PROJECTGAMING_LOG_*` (horodatage + fichier/ligne) et assertions `PROJECTGAMING_ASSERT` (handler surchargeable, actives en Debug). `main` journalise désormais son démarrage et ses erreurs.
- **LOT-01** implémenté : fenêtre Win32 (`hmi::Window`), initialisation Direct3D 11 en RAII (`hmi::GraphicsDevice`, effacement + présentation V-Sync + redimensionnement) et boucle de jeu à pas de temps fixe déterministe (`core::FixedTimestep`, testée). L'exécutable ouvre une fenêtre stable et se ferme proprement (croix / Échap).
- Arborescence du projet (`Specification/`, `Lot/`, `Documentation/`, `Source/`, `External/`).
- Découpage `Source/` : `Core`, `HMI`, `Elements`, `Test` (`Unit`, `Integration`, `Systeme`).
- Build **CMake** (C++20) avec presets Visual Studio et Ninja ; Visual Studio utilisé via son intégration CMake native.
- **GoogleTest** (FetchContent) et un premier test unitaire.
- **CI GitHub Actions** : configure, build et tests sur `windows-latest`.
- Documentation **Doxygen** (`Doxyfile`) et **guide de conventions** de code.
- Outillage qualité : `.clang-format`, `.clang-tidy`, `.editorconfig`, avertissements `/W4 /WX`, option AddressSanitizer.
- En-têtes précompilés (`Source/pch.h`, option `ENABLE_PCH`).
- CI : couverture de code (OpenCppCoverage, artefact Cobertura + HTML) et génération de la documentation Doxygen (artefact HTML).
- Gouvernance : `CONTRIBUTING.md` (Conventional Commits, trunk-based), `CHANGELOG.md`, `LICENSE`.
