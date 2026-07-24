# Éditeur de niveaux {#spec-editeur}

> Statut : **édition de tuiles de base et robustesse/confort d'édition validés et livrés**
> (LOT-14 : peinture, mécanismes, entrée/sortie, redimensionnement, undo/redo,
> enregistrement/validation, essai immédiat ; LOT-15 : nommage, garde-fous, caméra, outils de
> zone, découvrabilité ; LOT-16 : niveaux de grande taille, section 7 ; LOT-27 : palette organisée
> par catégories repliables, section 8).
> Dépend de [`niveaux.md`](niveaux.md).

## Objectif
Permettre la **création et la modification de niveaux sans écrire de code**, afin que des membres de l'équipe **non-développeurs** (game design, level design) contribuent directement au contenu du jeu.

## 1. Exigences fonctionnelles
- \anchor EX-EDIT-001 **EX-EDIT-001** — L'éditeur doit permettre de créer et modifier un niveau **sans compétence en programmation** ni ligne de commande.
- \anchor EX-EDIT-002 **EX-EDIT-002** — L'édition doit être **WYSIWYG** : une grille visuelle où l'on peint les tuiles (vide, solide, danger…) à la souris, depuis une **palette** de types.
- \anchor EX-EDIT-003 **EX-EDIT-003** — L'éditeur doit permettre de placer et **relier visuellement les mécanismes** (interrupteur ↔ porte ; clé ↔ porte verrouillée et blocs poussables **dès que `Core` les implémentera côté gameplay** — absents du moteur à ce jour, donc hors périmètre de l'éditeur jusque-là, cf. `epic.md` du lot LOT-14).
- \anchor EX-EDIT-004 **EX-EDIT-004** — L'éditeur doit permettre de définir l'**entrée** et la **sortie** du niveau.
- \anchor EX-EDIT-005 **EX-EDIT-005** — L'éditeur doit permettre de **redimensionner** la grille et de gérer **annuler/refaire** (undo/redo).
- \anchor EX-EDIT-006 **EX-EDIT-006** — L'éditeur doit **enregistrer et charger** au format JSON défini par `EX-LVL-003` (celui réellement implémenté par `LevelLoader`, pas un format hybride ASCII — l'édition texte brut n'est pas visée, cf. `EX-EDIT-001`), en produisant des fichiers **valides**.
- \anchor EX-EDIT-007 **EX-EDIT-007** — L'éditeur doit **valider** le niveau avant enregistrement (présence entrée/sortie, dimensions cohérentes, liaisons de mécanismes valides — `EX-LVL-004`) et signaler les erreurs de façon compréhensible par un non-codeur.
- \anchor EX-EDIT-008 **EX-EDIT-008** — L'éditeur doit permettre de **tester le niveau** immédiatement (le lancer dans le jeu depuis l'éditeur), pour un cycle création → essai rapide.
- \anchor EX-EDIT-009 **EX-EDIT-009** — L'éditeur doit permettre de **nommer** un niveau à sa création et de le **renommer**, et **avertir avant d'écraser** un fichier existant différent du niveau en cours d'édition lors de l'enregistrement.

## 2. Réutilisation & cohérence
- \anchor EX-EDIT-010 **EX-EDIT-010** — L'éditeur doit **réutiliser le modèle de niveau et la validation de `Core`** — aucune duplication de la logique de niveau entre le jeu et l'éditeur (source unique de vérité).
- \anchor EX-EDIT-011 **EX-EDIT-011** — Un niveau enregistré par l'éditeur doit être **directement jouable** par le jeu sans conversion, et réciproquement (round-trip fiable).

## 3. Distribution & collaboration
- \anchor EX-EDIT-020 **EX-EDIT-020** — L'éditeur doit être fourni comme un **outil exécutable** que les non-codeurs lancent sans étape de build.
- \anchor EX-EDIT-021 **EX-EDIT-021** — Les niveaux sont des **fichiers** rangés dans `Source/Elements` et versionnés ; l'éditeur enregistre directement à cet emplacement.
- \anchor EX-EDIT-022 **EX-EDIT-022** — Le partage des niveaux passe par **Git via une interface graphique** (type GitHub Desktop) : les niveaux sont versionnés dans le dépôt au même titre que le reste du projet. Le level designer publie et récupère les niveaux en quelques clics, **sans ligne de commande**. Un court guide d'utilisation (installation + flux publier/mettre à jour) doit être fourni dans `Documentation/` à destination des non-codeurs.

## 4. Approche d'implémentation (décidée)
**Option retenue : éditeur intégré.** L'édition est un **mode de l'application** de jeu, réutilisant le rendu Direct3D 11 et le modèle de niveau de `Core`.

- \anchor EX-EDIT-030 **EX-EDIT-030** — L'éditeur est intégré à l'application (mode éditeur), et non un outil séparé.
- \anchor EX-EDIT-031 **EX-EDIT-031** — Le mode éditeur réutilise le **rendu D3D11** de `HMI` et le **modèle/validation de niveau** de `Core` (pas de duplication).

Justification : un seul codebase, un rendu identique au jeu, un cycle **création → essai** immédiat et un round-trip garanti avec le format de niveau. *(Repli documenté si le temps manque : l'éditeur libre Tiled avec une couche d'import vers notre format — non retenu par défaut.)*

## 4bis. Décors & pixel art (post-MVP, intégré à l'éditeur)
- \anchor EX-EDIT-040 **EX-EDIT-040** — L'éditeur doit permettre de **placer et transformer des décors** (position, échelle, superposition par couches) — cf. [`decors.md`](decors.md).
- \anchor EX-EDIT-041 **EX-EDIT-041** — L'éditeur doit intégrer la **conversion photo → pixel art** (chargement d'une photo, pixellisation, réduction de palette, paramètres ajustables) et enregistrer l'asset résultant dans `Source/Elements` — cf. `EX-DEC-030/031/032`.

Ces capacités sont livrées **après** l'édition de tuiles de base, mais l'architecture les accommode dès le départ (cf. [`architecture.md`](architecture.md)).

## 5. Non-objectifs (éditeur, MVP)
- Édition collaborative en temps réel (plusieurs personnes sur le même niveau simultanément).
- Édition des assets graphiques/sonores (l'éditeur agence des tuiles existantes, il ne dessine pas les sprites).
- Sélection multiple non contiguë et historique annuler/refaire par delta (l'historique par
  snapshots complets, retenu en LOT-14, reste adapté à la taille des niveaux du projet).
- Palette pilotée par un fichier de configuration externe (la liste de types gérés par `Core`
  reste petite et change rarement ; une couche de données externes serait une abstraction non
  justifiée à ce stade).

## 6. Robustesse et confort d'édition (LOT-15)
Une fois l'édition de tuiles de base livrée (LOT-14), l'usage réel fait ressortir des besoins
complémentaires pour rapprocher l'éditeur d'un outil de production : éviter la perte de travail,
éditer confortablement des niveaux plus grands, et rendre les commandes découvrables sans dépendre
uniquement de la documentation externe.

- \anchor EX-EDIT-012 **EX-EDIT-012** — L'éditeur doit **demander confirmation** avant toute action
  destructrice : un redimensionnement qui supprimerait l'entrée, la sortie ou une liaison de
  mécanisme, et la fermeture de l'éditeur alors que des modifications ne sont **pas enregistrées**.
- \anchor EX-EDIT-013 **EX-EDIT-013** — L'éditeur doit permettre de **déplacer (pan)** et de
  **zoomer** la vue indépendamment du cadrage automatique, pour éditer confortablement des niveaux
  de toute taille.
- \anchor EX-EDIT-014 **EX-EDIT-014** — Au-delà de la peinture case par case, l'éditeur doit fournir
  un **outil de remplissage rectangulaire** et un **outil de sélection** avec **copier/coller** d'une
  zone de tuiles.
- \anchor EX-EDIT-015 **EX-EDIT-015** — L'éditeur doit exposer ses commandes de façon
  **découvrable** à l'écran : une barre d'outils pour changer d'outil, un aperçu des raccourcis
  clavier, et des libellés sur les entrées de la palette.
- \anchor EX-EDIT-016 **EX-EDIT-016** — Lorsque plusieurs liaisons interrupteur ↔ porte sont
  visibles simultanément, chacune doit être **visuellement distinguable** des autres (et non une
  teinte unique partagée par toutes les liaisons).

## 7. Niveaux de grande taille (LOT-16)
Le redimensionnement (`EX-EDIT-005`) se faisait jusqu'ici uniquement case par case (flèches),
praticable pour de petits ajustements mais pas pour viser directement une grande taille. Aucune
limite technique n'existe dans `Core` (`TileMap`/`LevelDraft` acceptent toute dimension positive) —
seule l'ergonomie manquait.

- \anchor EX-EDIT-017 **EX-EDIT-017** — L'éditeur doit permettre de **saisir directement** une
  largeur et une hauteur cibles (plutôt que d'incrémenter case par case), sous un **plafond
  généreux** (très au-delà des tailles livrées à ce jour) qui reste configurable au niveau du code,
  pas une limite arbitraire de `Core`.

## 8. Palette organisée par catégories (LOT-27)
La palette (`EX-EDIT-002`) affichait ses types de tuiles en **liste plate** : 19 lignes, chacune
son propre libellé, où les huit variantes de pente/arrondi et les trois tailles de bloc poussable
noyaient les huit types simples au milieu d'orientations presque identiques visuellement. Aucun
souci de compréhension du type lui-même (chaque icône reste le rendu réel de la tuile), mais un
défilement long avant de repérer le bon type.

- \anchor EX-EDIT-018 **EX-EDIT-018** — La palette doit regrouper les types de tuiles en
  **catégories repliables** plutôt qu'en liste plate : quatre catégories (Tuile, Interactif,
  Piège, Jalon) plus une entrée autonome (Vide), chacune dépliable indépendamment sans jamais
  recouvrir le canevas d'édition. Une famille regroupant **plusieurs formes ou tailles d'un même
  type d'usage** (Pente, Arrondi, Bloc poussable) doit être elle-même un **sous-groupe repliable**
  imbriqué dans sa catégorie — trois niveaux d'accordéon au maximum jusqu'à une variante précise,
  plutôt qu'une seconde liste plate cachée sous la première. Si tout déplier en même temps dépasse
  la hauteur de fenêtre disponible, la palette doit rester entièrement accessible par
  **défilement** (molette, barre de défilement) plutôt que de rendre ses dernières entrées
  inaccessibles à la souris ; déplier ou replier un en-tête ne doit jamais le faire disparaître de
  la fenêtre visible.

## Traçabilité
L'éditeur s'appuie sur `Core` (modèle et validation de niveau, `niveaux.md`) et sur le rendu de
`HMI` (`rendu-technique.md`). L'édition de tuiles de base a fait l'objet du lot **LOT-14** (terminé) ;
la robustesse et le confort d'édition (section 6) du lot **LOT-15** (terminé) ; la saisie directe de
grandes tailles (section 7) du lot **LOT-16** (terminé) ; la palette organisée par catégories
(section 8) du lot **LOT-27** (terminé).
