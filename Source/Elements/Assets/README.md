# Elements/Assets/

**Assets graphiques éditables hors code** (`LOT-39`, `EX-REN-042`) : atlas de tuiles au format PNG,
copié à côté de l'exécutable au build (patron `Levels`/`Localization`) et chargé au démarrage par
`hmi::TextureAtlas` (`hmi::AssetPaths`/`hmi::TextureLoader`).

## Contenu

- `atlas.png` — l'atlas de tuiles + images d'animation du personnage. Convention de grille
  (inchangée depuis la génération procédurale historique, `hmi::TextureAtlas::TILE_SIZE`/
  `TILES_PER_SIDE`/`PLAYER_FRAME_SIZE`/`PLAYER_FRAME_COLUMNS`) :
  - une grille de tuiles carrées de 16×16 pixels, `TILES_PER_SIDE` (5) tuiles par côté, dans le
    coin haut-gauche de l'image ;
  - sous cette grille, une ou plusieurs lignes d'images d'animation du personnage (16×16 chacune,
    même largeur de grille), dans l'ordre `Idle` puis `Run` puis `Jump`
    (`hmi::flatPlayerFrameIndex`).
  - `hmi::TextureAtlas::tile(colonne, ligne)`/`playerFrameRegion(clip, index)` calculent la région
    à partir de ces seules constantes — **remplacer le fichier suffit** à changer l'apparence, sans
    toucher au code, tant que la grille (dimensions, position des cases utilisées par
    `hmi::TileVisuals::regionForTile`) reste respectée.

## Éditer / régénérer

- **Remplacer une tuile** : éditer `atlas.png` directement (n'importe quel éditeur d'image), en
  respectant la grille ci-dessus. Relancer l'éditeur pour voir le résultat (pas de rechargement à
  chaud à ce stade).
- **Régénérer `atlas.png` à partir de la génération procédurale historique** (référence, en cas de
  divergence ou pour repartir d'une base propre) :
  `ProjectGaming.exe --export-atlas=Source/Elements/Assets/atlas.png` (option de développement,
  n'ouvre aucune fenêtre — voir `Documentation/Guide/guide-rendu.md`).
- **Asset absent ou illisible** : `hmi::TextureAtlas` retombe automatiquement sur la génération
  procédurale (`EX-NFR-040`), sans bloquer le rendu — utile pour développer sans art final.

> Anciennement `Textures/` (réservé, jamais peuplé) — ce dossier est le point d'entrée réel des
> assets graphiques depuis le LOT-39, nommé `Assets/` pour suivre le même patron que `Levels/` et
> `Localization/`.

## `Skins/` — skins de tuiles et planches à raccords (`LOT-42`)

`skins.json` associe chaque type de tuile à un fichier de `Skins/` et à un **mode** de découpage.
Le fichier porte un numéro de `version` et regroupe les associations en **jeux nommés** (`jeux`),
pour qu'un même jeu puisse proposer plusieurs ambiances. Il s'édite depuis le panneau « Textures »
de l'éditeur ; l'écrire à la main reste possible.

### Mode `single` — une image, une case

Un PNG de **16×16 pixels exactement**, utilisé tel quel. C'est le mode qui convient à tout type
dont le voisinage n'a pas de sens : dangers, interrupteurs, portes, blocs, pentes et arrondis.

Pour les douze types à **silhouette inclinée ou courbe** (pentes, arrondis convexes et concaves,
au sol comme au plafond), dessiner un **carré plein** : le moteur détoure automatiquement l'image
à la forme exacte de la hitbox, de sorte que l'affichage ne puisse pas mentir sur la géométrie de
collision.

### Mode `bitmask16` — une planche de raccords

Un PNG de **4×4 cases de 16×16 pixels** (64×64 au total). La case affichée est choisie selon les
**quatre voisins orthogonaux solides** de la tuile, ce qui donne des bords et des coins distincts
de l'intérieur — sans quoi un mur de vingt tuiles répéterait vingt fois le même motif et la grille
resterait visible.

Contenu attendu de chaque case (colonne 0 à gauche, ligne 0 en haut) :

| Ligne | Colonne 0 | Colonne 1 | Colonne 2 | Colonne 3 |
|:-----:|-----------|-----------|-----------|-----------|
| **0** | isolée (bordée partout) | extrémité basse de colonne | extrémité gauche de rangée | coin extérieur bas-gauche |
| **1** | extrémité haute de colonne | segment vertical | coin extérieur haut-gauche | bord gauche |
| **2** | extrémité droite de rangée | coin extérieur bas-droite | segment horizontal | bord bas |
| **3** | coin extérieur haut-droite | bord droit | **bord haut** (dessus de plateforme) | **intérieur plein** |

Deux conventions, explicites parce qu'elles se voient à l'écran :

- **L'extérieur du niveau compte comme solide.** Un mur de bordure ne dessine donc pas de contour
  sur sa face invisible, hors du niveau.
- **Le raccord suit la solidité, pas le type.** Un mur et un bloc poussable se raccordent entre
  eux : ils forment visuellement la même matière, et exiger le même type laisserait une couture
  partout où ils se touchent. Les pentes et arrondis, jamais solides, ne participent pas au
  voisinage.

## `Backgrounds/` — fonds de niveau (`LOT-44`)

Un niveau peut désigner une image de `Backgrounds/` comme fond (`core::Level::background`,
`EX-REN-044`), sélectionnée depuis la section « Fond » du panneau « Textures ». Dimensions
**libres** : le rendu étire l'image sur les bornes du niveau en préservant son ratio d'aspect
(recadrage par le centre sur la dimension excédentaire, jamais de déformation).

## À venir (programme `LOT-40` → `LOT-55`)

Ce dossier accueillera d'autres **sous-dossiers par famille d'asset**, chacun avec ses dimensions
attendues **validées au chargement** (`EX-REN-007`) : `Objects/` (`LOT-45`), `Player/` (`LOT-48`),
`Decors/` (`LOT-49`), plus des fichiers `<asset>.anim.json` décrivant les animations (`LOT-46`).

Le **rechargement à chaud**, absent aujourd'hui, arrive au `LOT-43` : éditer un asset se reflétera
sans relancer l'application.
