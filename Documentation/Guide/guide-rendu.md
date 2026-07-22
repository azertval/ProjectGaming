# Rendu 2D : de l'ECS à l'écran {#guide-rendu}

Cette page explique comment une entité ECS (@ref guide-ecs) — une simple combinaison de données —
finit par apparaître comme une image à l'écran, en partant des notions de base du rendu temps réel
pour qui n'en a jamais écrit. Tout le rendu vit dans `Source/HMI/Graphics` et
`Source/HMI/Platform` ; c'est la seule partie du moteur qui dépend de Direct3D 11 et de Win32
(`Core` en reste totalement indépendant, @ref guide-boucle et `EX-ARCH-040`).

## Vocabulaire de base : GPU, swap chain, back buffer

Un jeu ne dessine pas directement sur l'écran : il dessine dans une zone mémoire dédiée sur la
carte graphique (le **GPU**, *Graphics Processing Unit*, un processeur spécialisé dans le calcul
massivement parallèle nécessaire pour colorier des millions de pixels par seconde), puis cette
image est transmise à l'écran. Dessiner directement dans l'image **actuellement affichée**
provoquerait un artefact visible (*tearing* : une moitié d'image montre l'ancien contenu, l'autre
le nouveau, si l'écran est en train de la rafraîchir pendant qu'on la modifie). La solution
standard, le **double buffering**, utilise **deux** images en mémoire :

- le **front buffer** : l'image actuellement montrée à l'écran, intouchable ;
- le **back buffer** : une image « en coulisse », sur laquelle le jeu dessine librement la frame
  suivante.

Une fois le back buffer entièrement dessiné, une opération de **présentation** échange les deux
rôles (le back buffer devient le front buffer et inversement) — idéalement au moment précis où
l'écran finit de rafraîchir l'image précédente, ce qui s'appelle la **synchronisation verticale**
(V-Sync) : elle évite le *tearing* en alignant l'échange sur le rythme de rafraîchissement de
l'écran, au prix d'attendre ce moment si le jeu est plus rapide que l'écran. L'ensemble
« back buffer(s) + mécanisme d'échange » s'appelle une **swap chain**.

## `hmi::GraphicsDevice` : initialiser Direct3D 11 et présenter l'image

**Direct3D 11** est l'API bas niveau, fournie par Windows, qui permet de piloter le GPU (créer des
ressources, envoyer des commandes de dessin, présenter l'image). `hmi::GraphicsDevice` encapsule les
trois objets fondamentaux que Direct3D 11 expose pour cela :

- le **device** (`ID3D11Device`) : sert à **créer** des ressources GPU (textures, shaders,
  buffers) — il ne dessine rien lui-même ;
- le **contexte immédiat** (`ID3D11DeviceContext`) : sert à **émettre les commandes** de dessin
  effectives (« dessine ces triangles avec cette texture ») ;
- la **swap chain** (`IDXGISwapChain`) : gère le back buffer et la présentation décrite ci-dessus.

`GraphicsDevice` les initialise à la construction (à partir du `HWND` — le handle natif Win32 de la
fenêtre, voir plus bas), expose `clear(r, g, b, a)` (remplir tout le back buffer d'une couleur
unie, l'étape qui précède tout dessin d'une frame — sans elle, chaque frame réafficherait par-dessus
les pixels de la précédente), `present()` (échanger front/back buffer, avec V-Sync), et `resize(w,
h)` (recréer les buffers à une nouvelle taille, nécessaire quand la fenêtre change de dimensions —
les buffers de la swap chain ont une taille fixe, ils ne « s'étirent » pas automatiquement).

Toutes les ressources Direct3D sont détenues via `Microsoft::WRL::ComPtr` (un pointeur intelligent
pour les objets **COM**, le mécanisme de gestion d'objets utilisé par les API Windows historiques) :
leur libération est automatique à la destruction, exactement comme un `std::unique_ptr` pour de la
mémoire ordinaire — c'est ce qui permet à `GraphicsDevice` de n'avoir aucun destructeur explicite à
écrire (`~GraphicsDevice() = default`).

## `hmi::Window` : la fenêtre, prérequis du rendu

Direct3D a besoin d'une surface Windows où dessiner : c'est `hmi::Window` (`Source/HMI/Platform`)
qui crée et possède cette fenêtre Win32, et fournit son handle natif (`HWND`, littéralement
« *handle to a window* », l'identifiant opaque que Windows utilise pour désigner une fenêtre) —
c'est ce `HWND` que `GraphicsDevice` reçoit à sa construction pour savoir *où* dessiner.

`Window` a une seconde responsabilité, sans rapport direct avec le rendu mais qui vit au même
endroit pour une raison pratique : Windows ne notifie les événements clavier/souris/fenêtre que par
une **pompe de messages** (*message pump*) attachée à une fenêtre précise ; `Window::pumpMessages()`
traite ces messages et, au passage, met à jour l'`hmi::InputState` de la fenêtre (@ref
guide-entrees) — c'est pour cela que la capture d'entrée est fenêtrée, plutôt qu'un module
totalement séparé.

## Unités monde et pixels : `hmi::Camera2D`

`Core` ne connaît que des **unités monde** (une tuile = 1 unité, @ref guide-maths) — jamais de
pixels. Le rendu doit donc **convertir** une position monde en position d'écran avant de dessiner
quoi que ce soit ; c'est le rôle de `hmi::Camera2D`. Deux paramètres gouvernent cette conversion :

- `PIXELS_PER_UNIT = 16` : l'échelle de base, fixée par convention du projet (`EX-ARCH-021`) — une
  entité de 1×1 unité (une tuile) occupe 16×16 pixels à l'écran avant tout zoom ;
- le **zoom** (`setZoom`) : un multiplicateur additionnel de cette échelle. Un zoom **entier** est
  recommandé (`EX-ARCH-022`) : avec un style *pixel art*, un zoom non entier (1,5× par exemple)
  étirerait certains pixels source sur 1 pixel écran et d'autres sur 2, brisant la netteté des
  contours voulue par ce style visuel.

La caméra a aussi un **centre** (`setCenter`, en unités monde) : le point qui apparaît au milieu de
l'écran — typiquement la position du personnage suivi. `projectionMatrix()` combine centre, échelle
et dimensions de la fenêtre (le *viewport*) en une **matrice de projection orthographique** : une
transformation mathématique standard en rendu 2D/3D qui convertit une position monde en position
« clip » — l'espace normalisé que le GPU attend en sortie du *vertex shader* (voir plus bas). C'est
cette matrice, et non une conversion manuelle pixel par pixel, que le pipeline de dessin applique à
chaque sommet ; `worldToScreen`/`screenToWorld` exposent la même conversion côté CPU, pour des
besoins hors dessin (par exemple convertir une position de souris en position monde).

## Le pipeline de dessin de sprites : `hmi::SpriteBatch`

### Pourquoi « batcher » plutôt que dessiner un sprite à la fois

Chaque appel de dessin adressé au GPU (un *draw call*) a un coût fixe non négligeable, indépendant
du nombre de pixels dessinés — piloté par la communication CPU → GPU, pas par le travail du GPU
lui-même. Un niveau de plusieurs centaines de tuiles dessinées par des appels **individuels**
saturerait ce coût fixe avant même de saturer le GPU. Le **batching** (« dessin par lots ») regroupe
un grand nombre de sprites partageant la **même texture** en un minimum d'appels de dessin :
`SpriteBatch` accumule des quads (voir plus bas) dans un tampon CPU et ne les envoie au GPU qu'en un
seul appel, au moment de `flush()`/`end()` — l'usage est `begin(projection, texture)`, puis un ou
plusieurs `draw(quad)`, puis `end()`.

### `SpriteQuad` : un rectangle texturé

Un **quad** est simplement un rectangle (deux triangles, en pratique — un GPU ne sait dessiner que
des triangles). `hmi::SpriteQuad` en décrit un par sa position/taille en **unités monde** (`x, y,
width, height`), la portion de texture à échantillonner en **coordonnées UV normalisées** (`u0, v0,
u1, v1`, chacune dans `[0, 1]` — la convention universelle du rendu temps réel pour désigner un
point dans une texture indépendamment de sa résolution en pixels), et une teinte RVBA (`r, g, b,
a`) multipliée avec la texture au dessin — une teinte blanche opaque (1,1,1,1) laisse la texture
inchangée, une teinte plus sombre ou colorée module son apparence sans créer de variante de texture
séparée.

### Sommets, shaders, et échantillonnage *nearest*

En interne, chaque quad devient 4 **sommets** (`Vertex` : position, UV, couleur), envoyés au GPU
avec deux petits programmes qui s'exécutent **sur le GPU** lui-même :

- le **vertex shader** transforme chaque position de sommet (unités monde) vers l'espace clip, via
  la matrice de projection de la caméra ;
- le **pixel shader** (aussi appelé *fragment shader*) calcule la couleur finale de chaque pixel
  couvert par les triangles, en échantillonnant la texture à la coordonnée UV interpolée et en la
  multipliant par la couleur du sommet.

L'échantillonnage utilise le mode ***nearest*** (au lieu du filtrage *bilinéaire*, plus courant
ailleurs) : il choisit le pixel de texture le **plus proche** de la coordonnée demandée, sans
mélanger ses voisins. C'est délibéré pour un rendu **pixel art** : le filtrage bilinéaire
adoucirait/flouterait les contours nets des sprites, un effet indésirable dans ce style visuel.

Le pipeline gère aussi la **transparence** (`_blendState`) : sans un état de *blending* configuré,
le canal alpha d'un quad (utile pour les zones transparentes de l'atlas, voir plus bas) serait
ignoré et chaque sprite dessinerait un rectangle plein.

## `hmi::TextureAtlas` : un spritesheet, généré en code

Un **atlas de texture** (ou *spritesheet*) regroupe **plusieurs** images dans une **seule** grande
texture, à des positions connues. C'est ce qui permet le batching décrit plus haut :
`SpriteBatch::begin` ne prend **qu'une seule** texture par lot, donc dessiner des sprites différents
dans le même appel exige qu'ils proviennent tous du même atlas — d'où l'intérêt de regrouper toutes
les tuiles d'un jeu dans un seul atlas plutôt qu'une texture par tuile.

Ce projet n'a pas (encore) d'atelier graphique fournissant des images dessinées à la main : l'atlas
de `hmi::TextureAtlas` est **généré en code**, de façon déterministe, en une grille de tuiles de 16
pixels de côté (`TILE_SIZE`) et de couleurs distinctes (dont une avec des zones transparentes, pour
exercer le canal alpha du pipeline). `tile(colonne, ligne)` renvoie la **région** (rectangle en
pixels) d'une tuile de cette grille — c'est cette région, convertie en UV normalisées, qu'un `Sprite`
(@ref guide-ecs, composant `core::Sprite`) référence via son champ `region` (en pixels, agnostique
de la résolution réelle de l'atlas — c'est le rendu qui la normalise). La classe est conçue pour
être **remplaçable** plus tard par un chargement de fichier réel sans changer son interface.

### La région du personnage : pourquoi elle vit dans le même atlas

Depuis LOT-17, `TextureAtlas` ne génère pas que la grille de tuiles : une bande supplémentaire de
32 pixels de haut est ajoutée **sous** la grille (colonnes 0-15), où vit une **silhouette
humanoïde** de 16×32 pixels (`playerRegion()`) — tête, cheveux, torse/manches, mains, jambes,
chaussures, chacun une couleur distincte, le reste transparent. Le ratio 16:32 (1:2) n'est pas
arbitraire : il reprend exactement celui de `core::playerSize()` (0,4×0,8 unité monde), pour que la
silhouette ne subisse **aucune déformation** à l'écran (`SpriteRenderer` multiplie directement les
pixels de la région par l'échelle du `Transform`, cf. plus bas).

Ce choix — étendre l'atlas existant plutôt que créer une classe séparée sur le modèle de
`SaveIcon`/`FlagIcons` (icônes de l'interface, @ref guide-journalisation, @ref guide-entrees) —
découle directement de la contrainte de batching énoncée plus haut : `SpriteBatch::begin` (et donc
`SpriteRenderer::render`, qui ne fait qu'**un seul** `begin`/`end` pour **toutes** les entités du
monde) ne lie qu'**une seule** texture par lot. Une région de personnage dans une texture séparée
aurait exigé de restructurer `SpriteRenderer` pour trier les entités par texture et faire plusieurs
passes — hors de proportion pour ajouter une seule silhouette. En la plaçant dans la texture de
`TextureAtlas`, aucune ligne de `SpriteRenderer` n'a besoin de changer : la normalisation UV s'appuie
déjà, génériquement, sur `atlas.width()`/`height()` (devenus des membres stockés plutôt qu'une
formule figée sur un atlas carré, pour accueillir cette bande supplémentaire).

La silhouette elle-même est dessinée par **blocs rectangulaires** (comparaisons d'intervalles sur
les coordonnées de pixel) plutôt que par les fonctions de distance géométrique de `FlagIcons` — plus
direct à lire et à ajuster pour une forme humanoïde à cette résolution. Ce sprite est **statique**
(une seule pose) : l'animation par séquence d'images (`EX-REN-012` — repos, course, saut) est
délibérément un lot séparé, pour ne concevoir la structure de séquence qu'une fois la pose de
référence validée visuellement.

## `hmi::SpriteRenderer` : le pont ECS → écran

C'est ici que les fils se rejoignent : `SpriteRenderer::render(world, camera)` parcourt le `World`
(@ref guide-ecs) par une **vue** sur les entités possédant à la fois `core::Transform` (position,
échelle, rotation) et `core::Sprite` (région d'atlas, couche, teinte) — exactement le motif vue +
lambda décrit dans @ref guide-ecs. Pour chacune, il résout la région d'atlas en UV, construit un
`SpriteQuad` en unités monde, et l'empile.

Un détail important : les sprites sont **triés par couche** (`Sprite::layer`, un entier — plus
grand = dessiné **au-dessus**) avant d'être soumis au `SpriteBatch`. Sans ce tri, l'ordre de dessin
suivrait l'ordre arbitraire d'itération de la vue ECS (@ref guide-ecs — le sparse set ne garantit
aucun ordre stable vis-à-vis du sens du jeu), et un élément de décor pourrait apparaître par-dessus
le personnage un pas sur deux.

`SpriteRenderer` **lit** l'ECS mais ne le modifie **jamais** (`EX-ARCH-012`) — le rendu est un
simple observateur de l'état de simulation, jamais une source de vérité. Ce n'est délibérément
**pas** un `core::ISystem` exécuté par `World::update` : le rendu est **découplé** de la simulation
au pas fixe (`EX-REN-021`), cohérent avec la séparation décrite en @ref guide-boucle — la simulation
avance par pas fixes, discrets ; le rendu, lui, redessine l'état courant une fois par **frame**
réelle, qu'un pas fixe ait eu lieu ou non entre deux frames.

## `hmi::BitmapFont` : dessiner du texte

Le texte (menus, libellés) ne peut pas se dessiner avec `sweepAabb` ni un `Transform` monde : c'est
une préoccupation entièrement différente, à la fois dans son espace de coordonnées et dans sa
représentation. `hmi::BitmapFont` est une **police bitmap** — chaque caractère est une petite image
de taille fixe (une **cellule**), plutôt qu'une police vectorielle calculée à la volée — générée en
code de la même façon que l'atlas, avec des glyphes blancs sur fond transparent (la teinte passée à
`drawText` les colore, par le même mécanisme de multiplication que les sprites).

Deux différences structurantes avec le rendu de sprites :

- **chasse fixe** (*monospace*) : chaque caractère occupe exactement la même largeur de cellule
  (`CELL_WIDTH`), ce qui simplifie le calcul de la largeur d'un texte (`textWidth`) à une simple
  multiplication, sans avoir à sommer des largeurs de glyphes variables ;
- **espace écran, pas espace monde** : `drawText` positionne le texte directement en **pixels**
  (coin haut-gauche `x, y`), indépendamment de la caméra du monde — un libellé de menu doit rester à
  la même position et à la même taille à l'écran, qu'importe où se trouve le personnage dans le
  niveau. `BitmapFont::screenProjection(largeur, hauteur)` construit une matrice de projection
  **différente** de celle de `Camera2D` : elle mappe directement `(0, 0)` au coin haut-gauche de
  l'écran et `(largeur, hauteur)` au coin bas-droit, sans notion de zoom ni de centre suivi. C'est
  cette projection, et non celle de la caméra de jeu, qu'il faut passer à `SpriteBatch::begin` pour
  dessiner de l'interface.

## Assembler la frame complète

Dans `Source/HMI/main.cpp`, l'ordre d'une frame de rendu est : `graphics.clear(...)` (vider le back
buffer) → `screens.render(context)` (l'écran courant dessine — typiquement un ou plusieurs passages
`SpriteBatch::begin`/`draw`/`end`, avec la projection de `Camera2D` pour la scène de jeu et celle de
`BitmapFont::screenProjection` pour le texte d'interface) → `graphics.present()` (échanger les
buffers). C'est la même boucle que celle décrite en @ref guide-boucle, dont le rendu n'est qu'une
étape — toujours exécutée **une fois par frame réelle**, après que tous les pas de simulation fixes
de cette frame ont eu lieu.

## Voir aussi
- `hmi::GraphicsDevice`, `hmi::Window`, `hmi::Camera2D`.
- `hmi::SpriteBatch`, `hmi::SpriteQuad`, `hmi::TextureAtlas`, `hmi::SpriteRenderer`, `hmi::BitmapFont`.
- `core::Transform`, `core::Sprite`, `core::AtlasRegion`, `core::Color` — les composants lus par le rendu.
- @ref guide-ecs — le `World` et les vues que `SpriteRenderer` parcourt.
- @ref guide-boucle — où le rendu s'insère dans la boucle de jeu.
- @ref guide-maths — les unités monde converties en pixels par la caméra.
