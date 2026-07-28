# Rendu 2D : de l'ECS à l'écran {#guide-rendu}

Cette page explique comment une entité ECS (@ref guide-ecs) — une simple combinaison de données —
finit par apparaître comme une image à l'écran, en partant des notions de base du rendu temps réel
pour qui n'en a jamais écrit. Tout le rendu vit dans `Source/HMI/Graphics`, sur une surface fournie
par le viewport Qt (`Source/HMI/Game`) ; c'est la seule partie du moteur qui dépend de Direct3D 11
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

## \ref hmi::GraphicsDevice "hmi::GraphicsDevice" : initialiser Direct3D 11 et présenter l'image

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

La swap chain utilise le **modèle de présentation flip** (`DXGI_SWAP_EFFECT_FLIP_DISCARD`, avec deux
back buffers — `EX-REN-004`, `LOT-33`) plutôt que l'ancien modèle *blt* (`DISCARD`, un seul buffer).
Sous Windows 10/11, le flip model présente le back buffer **directement** au compositeur (DWM), sans
la copie supplémentaire qu'imposait le modèle *blt* : moins de latence entre l'entrée du joueur et
l'image affichée, et une cadence plus régulière — y compris V-Sync activée. En contrepartie, `Present`
**dé-lie** la cible de rendu du back buffer à chaque frame ; `GraphicsDevice::clear()` la relie donc
(`OMSetRenderTargets`) en tête de chaque frame, avant tout dessin.

## La surface de dessin : le viewport Qt (`hmi::GameViewport`)

Direct3D a besoin d'une surface Windows où dessiner. Depuis la refonte Qt (@ref guide-ihm-qt), cette
surface est un **`QWindow` natif** embarqué, `hmi::GameViewport` : Qt en fournit le handle natif
(`HWND`, littéralement « *handle to a window* », l'identifiant opaque que Windows utilise pour
désigner une fenêtre) via `QWindow::winId()` — c'est ce `HWND` que `GraphicsDevice` reçoit à sa
construction pour savoir *où* dessiner, et sur lequel la swap chain présente **directement** (aucun
`QBackingStore`).

Le viewport a une seconde responsabilité : il possède l'**event loop** de rendu (tick cadencé par
`QEvent::UpdateRequest`) et traduit les événements clavier/souris **Qt** en `hmi::InputState` (@ref
guide-entrees) — c'est pour cela que la capture d'entrée vit au même endroit que le rendu, plutôt que
dans un module totalement séparé.

## Unités monde et pixels : \ref hmi::Camera2D "hmi::Camera2D"

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
l'écran. Ni le jeu (`GameSession`) ni l'éditeur (mode édition du viewport) ne font suivre ce centre en continu au personnage
(`EX-REN-013`) : il est recalculé par **cadrage**, sur le milieu du contenu à englober — le niveau
entier dans l'éditeur, ou la **salle courante** en jeu si le niveau en compte plusieurs (voir
ci-dessous). `projectionMatrix()` combine centre, échelle
et dimensions de la fenêtre (le *viewport*) en une **matrice de projection orthographique** : une
transformation mathématique standard en rendu 2D/3D qui convertit une position monde en position
« clip » — l'espace normalisé que le GPU attend en sortie du *vertex shader* (voir plus bas). C'est
cette matrice, et non une conversion manuelle pixel par pixel, que le pipeline de dessin applique à
chaque sommet ; `worldToScreen`/`screenToWorld` exposent la même conversion côté CPU, pour des
besoins hors dessin (par exemple convertir une position de souris en position monde).

### Cadrer un contenu plus grand que la fenêtre : `fitZoom` et `hmi::RoomGrid`

`Camera2D::fitZoom` (`LOT-16`) calcule le zoom qui fait tenir un rectangle donné (en unités monde)
dans une surface disponible (en pixels), sans jamais laisser de zone hors champ : zoom **entier**
tant que le rectangle tient déjà à l'échelle ×1 (netteté pixel art, `EX-ARCH-022`), fractionnaire
seulement si nécessaire pour l'englober malgré tout. Fonction pure, partagée par l'éditeur (niveau
entier) et le jeu.

En jeu (`GameSession`), pour un niveau plus grand qu'une **salle** (`hmi::RoomGrid`, `LOT-32`, taille
fixe en tuiles), ce rectangle n'est **plus le niveau entier** mais celui de la **salle** contenant
le personnage — façon *Celeste* : la caméra reste au zoom pixel art natif quelle que soit la taille
totale du niveau, et **bascule nettement** (un seul appel `setCenter`, pas d'interpolation) quand
`RoomGrid::roomIndexAt` désigne une salle différente de la précédente. Un niveau qui tient dans une
seule salle retombe exactement sur le cadrage « niveau entier » de LOT-16, sans branche spéciale :
`RoomGrid` produit alors une unique salle couvrant le niveau. L'éditeur, lui, garde son cadrage
« niveau entier » avec pan/zoom manuel (`EX-EDIT-013`) — seul un quadrillage superposé (`F10`)
indique les frontières de salles, sans changer sa caméra (@ref guide-editeur).

## Le pipeline de dessin de sprites : \ref hmi::SpriteBatch "hmi::SpriteBatch"

### Pourquoi « batcher » plutôt que dessiner un sprite à la fois

Chaque appel de dessin adressé au GPU (un *draw call*) a un coût fixe non négligeable, indépendant
du nombre de pixels dessinés — piloté par la communication CPU → GPU, pas par le travail du GPU
lui-même. Un niveau de plusieurs centaines de tuiles dessinées par des appels **individuels**
saturerait ce coût fixe avant même de saturer le GPU. Le **batching** (« dessin par lots ») regroupe
un grand nombre de sprites partageant la **même texture** en un minimum d'appels de dessin :
`SpriteBatch` accumule des quads (voir plus bas) dans un tampon CPU et ne les envoie au GPU qu'en un
seul appel, au moment de `flush()`/`end()` — l'usage est `begin(projection, texture)`, puis un ou
plusieurs `draw(quad)`, puis `end()`.

### \ref hmi::SpriteQuad "SpriteQuad" : un rectangle texturé

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

### \ref hmi::LineQuad "LineQuad" : un segment orienté (liens de mécanismes, `LOT-37`)

`SpriteQuad` décrit toujours un rectangle **aligné aux axes** (`x, y, width, height`) : impossible
d'en tirer un trait en diagonale, nécessaire pour relier deux cases quelconques de la grille (flèche
déclencheur → cible, @ref guide-editeur). `hmi::LineQuad` couvre ce cas sans nouveau pipeline ni
nouveau shader — même tampon, même `draw`/`flush`, juste une seconde façon de calculer les 4
sommets : au lieu d'un rectangle, deux **extrémités** (`ax, ay, bx, by`, unités monde) et une
**épaisseur** perpendiculaire au segment (`thickness`). `SpriteBatch::draw(const LineQuad&)`
calcule la direction normalisée du segment, en déduit une normale (perpendiculaire, longueur
`thickness / 2`), et pousse directement les 4 sommets décalés de part et d'autre des deux
extrémités — le même tampon d'indices (deux triangles par quad) s'applique sans changement, quelle
que soit l'orientation. Un segment dégénéré (les deux extrémités confondues) ne pousse aucun sommet.
Les liens de mécanismes réutilisent la région opaque de l'atlas comme UV (même technique que la
grille de repère, `DraftRenderer::drawGrid`) : la couleur vient uniquement de la teinte RVBA, pas
d'une texture dédiée.

## \ref hmi::TextureAtlas "hmi::TextureAtlas" : un spritesheet, généré en code

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

### Les images du personnage : pourquoi elles vivent dans le même atlas

Depuis LOT-17 (silhouette statique) puis LOT-18 (animation), `TextureAtlas` ne génère pas que la
grille de tuiles : une grille supplémentaire est ajoutée **sous** la grille de tuiles, où vivent les
**images de la silhouette humanoïde** (`playerFrameRegion(clip, frameIndex)`) — tête, cheveux,
torse/manches, mains, jambes, chaussures, chacun une couleur distincte, le reste transparent.
Chaque image reste **carrée** (16×16, exactement comme une tuile) : `SpriteRenderer::render`
multiplie ses dimensions en pixels par `Transform::scale` (cf. plus bas), et c'est cette échelle —
`core::playerSize()`, déjà non uniforme (0,4×0,8 unité monde) — qui donne à elle seule au
personnage sa proportion finale deux fois plus haute que large. Une région déjà non carrée
**doublerait** cet effet (bug réellement rencontré en LOT-17, corrigé aussitôt) : chaque image est
donc dessinée **pré-compressée** de moitié en hauteur dans son canevas carré, pour retrouver ses
proportions naturelles une fois étirée par l'échelle du `Transform`.

Ce choix — étendre l'atlas existant plutôt que placer le personnage dans une texture séparée —
découle directement de la contrainte de batching énoncée plus haut : le personnage est dessiné à
**chaque** frame parmi des centaines d'autres sprites, d'où l'obligation de partager la texture de
`TextureAtlas`. `SpriteBatch::begin` (et donc
`SpriteRenderer::render`, qui ne fait qu'**un seul** `begin`/`end` pour **toutes** les entités du
monde) ne lie qu'**une seule** texture par lot. Une région de personnage dans une texture séparée
aurait exigé de restructurer `SpriteRenderer` pour trier les entités par texture et faire plusieurs
passes — hors de proportion pour ce lot. En la plaçant dans la texture de `TextureAtlas`, aucune
ligne de `SpriteRenderer` n'a besoin de changer : la normalisation UV s'appuie déjà, génériquement,
sur `atlas.width()`/`height()` (devenus des membres stockés plutôt qu'une formule figée sur un atlas
carré, pour accueillir cette grille supplémentaire).

Chaque image est dessinée par **blocs rectangulaires** (comparaisons d'intervalles sur les
coordonnées de pixel) — direct à lire et à ajuster pour une forme humanoïde à cette résolution. Une pose (largeur des bras,
écartement des jambes) est un simple paramètre de la fonction de dessin : les 7 images de la grille
(2 `Idle`, 4 `Run`, 1 `Jump`) sont produites par la même logique, avec des paramètres différents —
pas 7 fonctions dupliquées.

### L'animation : une projection de l'état physique, pas un état séparé

`EX-REN-012` demande une animation par séquence d'images (repos, course, saut). Le clip actif et
l'image courante sont portés par un composant `core::Animation` (`clip`, `frameIndex`, `elapsed`)
et mis à jour chaque pas fixe par `core::AnimationSystem` (@ref guide-ecs) — **entièrement côté
`Core`**, sans dépendance aux pixels ni à `HMI` (`EX-ARCH-011`) : le système lit
`Player::grounded` et `Velocity::value.x` (déjà calculés par `CharacterPhysicsSystem` **pour le
même pas** — l'ordre d'appel dans `GameSession::update` est significatif, @ref guide-ecs) pour
déterminer si le personnage est en l'air, en train de courir, ou immobile ; aucun nouvel état n'est
ajouté à `core::Player`, l'animation est une pure **conséquence** de l'état physique existant.

Côté `HMI`, `GameSession::render` appelle `refreshPlayerSprite()` **à chaque frame** (pas seulement
au spawn, à la différence de LOT-17) : elle lit `core::Animation` du personnage et met à jour
`sprite.region = _atlas.playerFrameRegion(animation.clip, animation.frameIndex)`. C'est la même
séparation que partout ailleurs dans le rendu : `Core` décide **quoi** afficher (quel clip, quelle
image), `HMI` sait seule **à quoi ça ressemble** (quels pixels).

## \ref hmi::SpriteRenderer "hmi::SpriteRenderer" : le pont ECS → écran

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

### Interpoler le mouvement : `hmi::PreviousPosition` et le facteur d'interpolation

Ce découplage crée un artefact visuel dès qu'un écran dépasse 60 Hz : entre deux pas de simulation,
la position d'une entité mobile ne change pas, si bien qu'elle reste **figée** plusieurs frames de
rendu puis « saute » d'un coup au pas suivant — un *judder* en marches d'escalier. La parade,
**prévue dès le départ** dans l'architecture (`EX-ARCH-031`) et concrétisée en `LOT-33`, est
l'**interpolation** : dessiner l'entité entre sa position du **pas précédent** et celle du **pas
courant**, selon la fraction de pas déjà écoulée.

Concrètement, un composant de présentation `hmi::PreviousPosition` (rangé dans le `core::World` mais
écrit et lu par `HMI` seul — `Core` l'ignore, sa frontière reste intacte) conserve la position de
l'entité au pas précédent. `hmi::GameSession` la recopie depuis le `core::Transform` au **début** de
chaque pas fixe (`snapshotPreviousPositions`), avant que le pas ne modifie la position ; seules les
entités réellement mobiles (personnage, dangers mobiles, blocs poussables) reçoivent ce composant.
Au rendu, `SpriteRenderer::render` reçoit le **facteur d'interpolation** `[0, 1[` du cadenceur
(`core::FixedTimestep::interpolationAlpha`, passé en paramètre par `hmi::GameSession::render`) et dessine chaque
entité portant le composant à `lerp(position précédente, position courante, alpha)` ; les tuiles
fixes, sans le composant, sont dessinées à leur position courante, inchangées. La caméra, elle, n'est
**pas** interpolée : elle bascule déjà par coupure nette entre salles (`LOT-32`), sans suivi continu.
L'interpolation ne touche que l'**affichage** — la logique de jeu (collisions, fin de niveau)
continue de lire les positions **simulées** exactes, le déterminisme est préservé (`EX-NFR-002`).

## Le texte d'interface : côté Qt, plus dans le pipeline Direct3D

Le texte (menus, libellés, options) ne se dessine **pas** avec ce pipeline : c'est une préoccupation
entièrement différente, portée par les **widgets Qt** de l'IHM (@ref guide-ihm-qt). Direct3D 11 ne
rend donc que la **scène de jeu** (tuiles, personnage, décors) dans le viewport ; l'ancienne police
bitmap « maison » et sa projection écran ont été retirées avec l'IHM « maison » au `LOT-38`. Un
libellé de menu reste à la même position et à la même taille à l'écran parce que Qt le compose dans
une couche indépendante de la caméra du monde, sans passer par `SpriteBatch`.

## Assembler la frame complète

Dans le viewport (`hmi::GameViewport::renderFrame`), l'ordre d'une frame de rendu est :
`graphics.clear(...)` (vider le back buffer) → la scène courante dessine — `hmi::GameSession::render`
en jeu, `hmi::DraftRenderer` en édition, typiquement un ou plusieurs passages
`SpriteBatch::begin`/`draw`/`end` avec la projection de `Camera2D` → `graphics.present()` (échanger
les buffers). C'est la même boucle que celle décrite en @ref guide-boucle, dont le rendu n'est qu'une
étape — toujours exécutée **une fois par frame réelle**, après que tous les pas de simulation fixes
de cette frame ont eu lieu.

## Voir aussi
- `hmi::GraphicsDevice`, `hmi::GameViewport`, `hmi::Camera2D`.
- `hmi::SpriteBatch`, `hmi::SpriteQuad`, `hmi::LineQuad`, `hmi::TextureAtlas`, `hmi::SpriteRenderer`,
  `hmi::DraftRenderer`.
- `hmi::LinkGeometry`, `hmi::LinkGesture`, `hmi::LinkPanel` — liens de mécanismes (`LOT-37`, voir
  @ref guide-editeur).
- `core::Transform`, `core::Sprite`, `core::AtlasRegion`, `core::Color` — les composants lus par le rendu.
- @ref guide-ecs — le `World` et les vues que `SpriteRenderer` parcourt.
- @ref guide-boucle — où le rendu s'insère dans la boucle de jeu.
- @ref guide-maths — les unités monde converties en pixels par la caméra.
