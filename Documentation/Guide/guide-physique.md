# Physique du personnage {#guide-physique}

Toute la physique vit dans `core::CharacterPhysicsSystem` (système ECS pur, @ref guide-ecs) et
s'appuie sur la primitive de collision `core::sweepAabb`. Elle s'exécute au **pas de temps fixe**
(@ref guide-boucle), donc **déterministe** : mêmes entrées → même trajectoire, toujours
(`EX-NFR-002`).

Repère du projet (détaillé en @ref guide-maths) : origine **haut-gauche**, `x` vers la droite, `y`
vers le **bas**. Une tuile = **1 unité monde**. « Monter » est donc une vitesse **négative** en
`y` ; la gravité est une accélération **positive** en `y`.

## 1. Collision par balayage continu (swept AABB)

### Le problème : le *tunneling*

Un objet qui se déplace vite peut, en un seul pas de simulation, traverser entièrement un obstacle
fin sans jamais être détecté « dedans ». C'est le phénomène de **tunneling**. Exemple chiffré : un
mur a une épaisseur de 1 unité. Le personnage se déplace à 20 unités/seconde et le pas fixe dure
1/60 s (@ref guide-boucle) : il avance donc de `20 × 1/60 ≈ 0,33` unité par pas — moins que
l'épaisseur du mur, donc ce cas précis serait détecté. Mais à 80 unités/seconde (un dash rapide, par
exemple), l'avancée par pas est de `80 × 1/60 ≈ 1,33` unité : **plus** que l'épaisseur du mur. Une
approche naïve qui ne testerait le recouvrement qu'**après** avoir déplacé la boîte à sa position
finale ne verrait jamais l'objet « dans » le mur : à la fin du pas, il serait déjà passé de l'autre
côté. Le mur, pourtant infranchissable en théorie, laisserait tout traverser en pratique.

### La solution : tester tout le trajet, pas seulement l'arrivée

Le **balayage continu** (*swept collision*) évite ce piège en testant **tout le trajet** parcouru
pendant le pas, pas seulement le point de départ et le point d'arrivée. `core::sweepAabb(box, delta,
tiles)` déplace une boîte `core::Aabb` de `delta` en garantissant qu'elle ne **traverse jamais** une
tuile solide, quelle que soit la vitesse (`EX-GP-014`) — c'est le cœur de la robustesse « pas de
tunneling » du moteur.

### Méthode retenue : balayage **par axe** avec clamp direct

On résout le déplacement **X puis Y**, séparément (fonctions internes `sweepX` puis `sweepY`, dans
`SweptCollision.cpp`). Pour chaque axe, dans l'ordre :

1. on parcourt **toutes les cellules de la grille** situées entre le bord d'attaque courant de la
   boîte (le bord qui avance en premier dans le sens du mouvement) et le bord d'attaque **visé** —
   jamais seulement la cellule de départ et celle d'arrivée, ce qui est précisément ce qui évite le
   tunneling décrit plus haut ;
2. au **premier** solide rencontré en parcourant ces cellules, on **cale** (« clampe ») la position
   de la boîte exactement sur la coordonnée entière du mur (par exemple `col - size.x` pour un
   mouvement vers la droite qui bute sur la colonne `col`), au lieu de la laisser à mi-chemin dans
   le mur.

La passe Y part de la position **X déjà résolue** par la passe précédente : c'est ce qui produit
naturellement le **glissement** le long d'un mur — un personnage qui avance en diagonale contre un
mur vertical voit sa composante X bloquée mais continue de tomber en Y, sans effet de « collage »
artificiel à coder séparément.

### Pourquoi caler directement plutôt que d'interpoler

Une alternative plus « naturelle » consisterait à calculer le paramètre `t ∈ [0, 1]` du point exact
de contact le long du trajet, puis à poser `position = position + delta * t`. Ce moteur ne fait
**pas** cela : il cale directement `position` sur la coordonnée entière du mur touché. La raison est
la **dérive flottante** (@ref guide-maths) : `position + delta * t` est un calcul flottant qui peut
laisser un résidu minuscule (par exemple `4.99999998` au lieu de `5.0` exactement), suffisant pour
que le test de collision du pas **suivant** considère encore la boîte comme légèrement à l'intérieur
du mur — un bug de « bord interne » où le personnage semble *coller* au mur plutôt que d'y glisser
librement. Caler sur la coordonnée **entière** du mur donne une position **exacte**, sans résidu, et
élimine cette classe de bug par construction plutôt que par correction a posteriori.

> **Note historique** (voir le commit de la TACHE-02) : une première version « diagonale », résolvant
> X et Y simultanément par [somme de Minkowski](https://fr.wikipedia.org/wiki/Somme_de_Minkowski) ⧉
> et [méthode des *slabs*](https://en.wikipedia.org/wiki/Slab_method) ⧉, a été abandonnée — élégante
> sur le papier, mais victime du même bug de bord interne à cause de la dérive flottante. Le
> balayage par axe s'est révélé plus robuste, tout en restant tout aussi **continu** (aucune
> réintroduction du tunneling). Une fine « peau » de tolérance (`kSkin`) sur l'axe perpendiculaire au
> mouvement évite de confondre deux situations proches : *marcher sur* un sol (contact attendu, pas
> un blocage) et *buter contre* lui (blocage réel).

### Lire le résultat : \ref core::SweepResult "core::SweepResult"

`SweepResult` porte la **position** finale de la boîte après résolution, et une **normale
indicatrice** par axe (`normal.x`, `normal.y`), chacune valant `-1`, `+1` ou `0` — pas un vecteur
unitaire au sens géométrique habituel, mais un indicateur de **quel axe** a été bloqué et dans
**quel sens** (signe « surface → boîte » : un sol sous la boîte, avec `y` vers le bas, donne
`normal.y < 0`). Le système de physique lit cet indicateur pour deux choses : **annuler** la
composante de vitesse correspondante (on ne peut pas continuer d'accélérer contre un mur) et
**déduire l'appui** — un contact bloquant sous la boîte signifie qu'elle repose au sol
(`grounded`).

## 2. Suivi de pente (EX-GP-003)

### Pourquoi une pente n'est jamais solide

Contrairement à un bloc plein, une tuile de pente (`core::TileType::SlopeUpRight` /
`SlopeUpLeft`) renvoie **toujours** `false` pour `core::isSolid` — y compris pour le balayage
classique du §1. Une pente couvre une case dont la hauteur **varie** entre `0` (bord haut) et `1`
(bord bas) selon la position horizontale ; si elle était solide au sens classique, le balayage par
axe la traiterait comme un bloc plein occupant **toute** la case, et son bord **haut** agirait comme
un mur invisible bloquant le personnage bien avant qu'il n'atteigne la pente elle-même. Rendre la
pente non solide et lui substituer un **suivi de surface** dédié (ci-dessous) évite ce piège par
construction.

### `core::slopeSurfaceHeight` et `core::resolveSlopeFollow`

`Core/Physics/SlopeGeometry.h/.cpp` définit la géométrie d'une pente à 45° (montée pleine sur toute
la largeur d'une case) :

```
slopeSurfaceHeight(SlopeUpRight, localX) = 1 - localX;  // haut a droite, bas a gauche
slopeSurfaceHeight(SlopeUpLeft,  localX) = localX;      // haut a gauche, bas a droite
```

`localX ∈ [0, 1[` est la position horizontale **dans la case** (0 = bord gauche) ; le résultat, dans
`[0, 1]`, se lit depuis le **haut** de la case (0 = haut, 1 = bas) — il faut l'additionner à la ligne
(`row`) de la case pour obtenir la hauteur monde de la surface (`EX-ARCH-020`).

`core::resolveSlopeFollow` est appelé **après** le balayage classique (§1), à chaque pas : il compare
le bord bas de la boîte **avant** ce pas et **après**, et parcourt **toutes** les lignes traversées
entre les deux (pas seulement la position finale) — le même principe anti-tunneling que le balayage
lui-même, indispensable car une chute rapide pourrait sinon « sauter » une pente entière en un seul
pas, la grille classique ne la voyant jamais comme solide. Si le bord bas se trouve **à ou sous** la
surface suivable à sa colonne courante (`newBottomY >= surfaceY - tolérance`), la position verticale
est **calée** exactement sur cette surface et la vitesse verticale annulée. Deux exceptions
protègent les autres mécaniques :

- **saut actif** (`velocityY < 0`, le personnage **monte**) : le suivi est **entièrement désactivé**
  — sans cette exception, sauter depuis une pente serait immédiatement annulé par le calage,
  neutralisant l'impulsion du saut (§4) ;
- **colonne hors grille** : aucune surface n'est cherchée, le personnage tombe normalement.

### Le piège du mur adjacent (correction du balayage horizontal)

Suivre une pente signifie que le bord bas de la boîte peut se retrouver **à l'intérieur** d'une case
(par exemple à mi-hauteur), et non plus seulement **effleurer** sa frontière comme sur un sol plat.
Ce nouvel invariant a révélé un piège pendant les tests d'intégration de la TACHE-02 : une pente
immédiatement suivie d'un bloc plein de **même hauteur** (le cas normal d'un raccord pente → palier)
bloquait le personnage à mi-montée. La cause : le balayage horizontal (`sweepX`,
`SweptCollision.cpp`) considère bloquante toute case solide chevauchée **verticalement** par la
boîte ; or, à mi-pente, la boîte chevauche déjà la ligne de la pente — la même ligne que le bloc
plein voisin — et s'y retrouve donc bloquée avant même d'avoir fini de la gravir. La fine « peau »
`kSkin` qui règle ce problème pour un sol plat (§1) ne suffit pas ici : elle n'exclut que la ligne
**exactement effleurée** à la frontière, alors qu'une pente fait chevaucher la boîte **franchement**
à l'intérieur de sa case.

La correction (`rowIsSlopeGround` dans `SweptCollision.cpp`) généralise le principe de `kSkin` : une
ligne où l'empreinte horizontale **courante** de la boîte repose sur une pente n'est **jamais**
considérée comme un mur par `sweepX`, même si un bloc plein voisin partage cette ligne. La
résolution verticale (§1, `sweepY`) reste inchangée et corrige naturellement, une fois la case
voisine atteinte, le léger reliquat d'enfoncement — un personnage nettement plus étroit qu'une case
(taille réelle 0,4 × 0,8, @ref guide-niveaux) ne produit alors qu'un **rattrapage** borné et non
bloquant au raccord, pas un à-coup perceptible ni un blocage (validé par les tests d'intégration
`SuitUnePenteAscendanteEnMarchant`, `TransitionPenteSolPlatSansAACoup`).

## 3. Gravité et intégration

À chaque pas, la vitesse **verticale** est d'abord mise à jour sous l'effet de la gravité, puis
cette vitesse sert à calculer le déplacement, qui est enfin résolu par balayage. Ce schéma —
vitesse d'abord, position ensuite, à partir de la vitesse **déjà mise à jour** — s'appelle
l'[intégration d'Euler explicite](https://fr.wikipedia.org/wiki/M%C3%A9thode_d%27Euler) ⧉ (la
méthode numérique la plus simple pour approximer l'évolution d'un système physique continu par des
pas discrets) :

```
velocity += gravity * dt;   // 1. la gravité accélère la chute
delta = velocity * dt;      // 2. le déplacement du pas découle de cette vitesse
box = sweepAabb(box, delta, tiles);  // 3. le déplacement est résolu contre les murs/sols
```

C'est un choix délibéré de simplicité : des méthodes d'intégration plus précises existent (Euler
semi-implicite, Verlet, Runge-Kutta), mais à pas fixe et petit (1/60 s), Euler explicite est
largement suffisant pour un plateformer, et surtout le plus simple à raisonner et à déboguer.

La gravité n'est cependant pas une simple constante : elle est **effective** (`EX-GP-018`), c'est-
à-dire modulée selon la **phase** du saut (les multiplicateurs vivent dans `core::PhysicsConfig`) —
une pratique courante en *game design* pour obtenir une trajectoire qui « a l'air bien » plutôt
qu'une parabole physiquement pure et perçue comme molle :

- **montée** : gravité de base (`gravity`) ;
- **chute** : gravité de base × `fallGravityMultiplier` (> 1) — la chute est délibérément plus
  « lourde », donc plus rapide, que la montée. Une parabole symétrique (montée et chute à la même
  vitesse) est perçue comme flottante et peu réactive ; accélérer la chute rend les sauts plus
  « nets » sans changer leur hauteur ;
- **fast-fall** : un multiplicateur `fastFallMultiplier` **supplémentaire** s'ajoute en chute si le
  joueur maintient la direction « bas » — permet d'écourter volontairement une chute ;
- **apex** : près du **sommet** de la trajectoire (quand `|vitesse verticale| < apexThreshold`, un
  seuil proche de zéro), un multiplicateur `apexGravityMultiplier` (< 1) **réduit** temporairement la
  gravité — quelques instants de « flottement » perceptible au sommet du saut, qui laissent au
  joueur une fenêtre plus généreuse pour ajuster sa trajectoire horizontale ou viser une plateforme
  précisément.

### Vitesse terminale newtonienne (EX-GP-019)

Sans borne, une chute prolongée accélérerait indéfiniment sous gravité constante — injouable (le
personnage traverserait plusieurs tuiles par pas, au risque de *tunneling* malgré le balayage
continu, et le ressenti deviendrait imprévisible). Plutôt qu'un plafond arbitraire (`std::min`), la
chute intègre une **traînée** (résistance de l'air, proportionnelle à la vitesse) qui s'oppose au
**poids** (masse `core::Player::mass` × gravité effective) — la même idée que la chute d'un
parachutiste : au début, peu de vitesse, donc peu de traînée, donc la gravité l'emporte largement et
la chute accélère franchement ; puis, la vitesse augmentant, la traînée grandit avec elle jusqu'à
**égaler** le poids — l'accélération nette tombe alors à zéro, et la vitesse cesse de croître (la
**vitesse terminale**) :

```
effectiveGravity = ...;  // multiplicateurs de chute/fast-fall/apex, inchanges (ci-dessus)
netAcceleration = effectiveGravity - (fallDragCoefficient * velocity.y) / mass;
velocity.y += netAcceleration * dt;   // seulement en chute (velocity.y >= 0) ; la montee du
                                       // saut reste a gravite simple, sans trainee (LOT-11
                                       // inchange, voir decision de cadrage de l'epic LOT-19)
```

La vitesse terminale émerge de l'équation à l'équilibre (`netAcceleration = 0`) :
`vitesseTerminale = (masse × effectiveGravity) / fallDragCoefficient`. Le coefficient par défaut
(`fallDragCoefficient = 3.6`) est calibré pour retomber sur l'ancienne borne fixe (25 unités/s) à
masse par défaut (1,0) — la **valeur** finale ne change pas, seule la **courbe** pour y parvenir
devient progressive plutôt que coupée net. Une masse plus grande fait pencher la balance vers le
poids (traînée relativement plus faible) : elle tombe donc plus vite, à traînée égale — cohérent
avec l'intuition (un objet lourd est moins freiné par l'air, à forme égale, que sa masse ne le fait
tomber).

## 4. Saut et *game feel*

Le terme *game feel* désigne l'ensemble des petits ajustements, souvent invisibles et parfois
« physiquement faux », qui rendent un contrôle agréable et réactif plutôt que strictement réaliste.
Le saut de ce moteur en est l'exemple central : il n'est **jamais** un simple « si au sol et bouton
pressé, alors vitesse verticale = -jumpSpeed » — plusieurs tolérances, chacune corrigeant un défaut
de perception précis, s'y ajoutent.

Le déclenchement du saut est **contextuel** : plusieurs sources d'autorisation sont testées, dans
cet ordre de priorité :

1. **sol / coyote time** — `coyoteTimer` (dans `core::Player`) est **rechargé** à sa valeur maximale
   tant que le personnage est au sol, et **décompté** dès qu'il quitte le sol (en l'air). Le saut
   reste autorisé tant que ce minuteur n'est pas à zéro. **Pourquoi** : un joueur qui appuie sur
   « sauter » une fraction de seconde **après** avoir quitté un rebord (par exemple en courant hors
   d'une plateforme) perçoit intuitivement qu'il était « encore sur le bord » au moment de l'appui,
   même si la simulation, au pixel près, l'en a déjà fait quitter. Sans cette tolérance, ce cas —
   pourtant fréquent et perçu comme injuste par le joueur — refuserait le saut. Le nom vient d'un
   gag de dessin animé : le coyote qui continue de courir dans le vide un instant avant de tomber ;
2. **wall jump** (`EX-GP-016`, détaillé au §6) — contre un mur, en l'air, une éjection en diagonale
   opposée au mur ;
3. **saut aérien** (`EX-GP-015`, double/multi-saut) — `airJumpsRemaining` autorise un nombre
   configurable de sauts supplémentaires **sans** retoucher le sol, rechargé uniquement au contact
   du sol.

Deux tolérances supplémentaires, indépendantes de la source d'autorisation, complètent le ressenti :

- **jump buffering** — `jumpBufferTimer` est **rechargé** au moment où le bouton de saut est
  **pressé** (front montant), puis **décompté**. Tant qu'il n'est pas à zéro, un atterrissage
  déclenche **immédiatement** le saut, même si le bouton a été pressé légèrement **avant**
  l'atterrissage effectif. **Pourquoi** : à l'inverse du coyote time (tolérance après le sol), celle-
  ci tolère une pression **anticipée** — un joueur qui vise un timing serré (sauter juste en touchant
  le sol après une chute) appuie souvent une frame ou deux trop tôt ; sans buffering, cet appui
  serait perdu (le bouton n'est déjà plus « pressé » cette frame-là) et le saut raté malgré une
  intention et un timing perçus comme corrects ;
- **hauteur de saut variable** — relâcher le bouton de saut **pendant la montée** plafonne
  immédiatement la vitesse ascendante à une fraction `jumpCutFactor` de sa valeur courante, ce qui
  écourte la trajectoire. **Pourquoi** : sans ce mécanisme, la hauteur du saut serait **fixe**,
  quelle que soit la durée d'appui — un jeu où « tapoter » et « maintenir » le bouton produisent le
  même saut paraît rigide. Ici, un appui bref donne un petit saut, un appui maintenu un saut complet
  : c'est un unique degré de liberté supplémentaire pour le joueur, à coût d'implémentation minime.

Le **budget** de sauts du tableau (`EX-GP-024`, @ref guide-niveaux) s'ajoute par-dessus toutes ces
règles : `jumpsRemaining` peut **refuser** un saut par ailleurs autorisé, une fois épuisé (`-1`
signifie illimité — aucune limite de budget, indépendamment des règles de *game feel* ci-dessus).

## 5. Dash 8 directions

`EX-GP-017`. Un **dash** est une ruée brève, à vitesse constante et élevée, dans une direction
choisie par le joueur. Sur le **front** d'appui du bouton dédié (voir @ref guide-entrees), si le
dash est **disponible** (`dashAvailable`, rechargé uniquement au contact du sol) et que le budget
n'est pas épuisé (`dashesRemaining`), le personnage part dans la direction `(moveX, moveY)` de
l'intention de déplacement, **normalisée** (@ref guide-maths — sans cette étape, une diagonale
irait `√2` fois plus vite qu'un dash cardinal). Cette direction couvre 8 orientations possibles
(les 4 cardinales et les 4 diagonales) ; à défaut de direction pressée, le dash part dans
l'orientation courante du personnage (`facing`). Pendant sa durée (`dashDuration`), à vitesse
constante `dashSpeed` :

- la **gravité est suspendue** — le dash suit une trajectoire **rectiligne** nette, sans être
  infléchi par la chute, ce qui le rend lisible et prévisible à l'écran ;
- l'**entrée du joueur est ignorée** pour le mouvement — le dash « prend la main » entièrement
  pendant sa courte durée ;
- le **balayage continu** (§1) s'applique normalement et **arrête** le dash net sur un mur, sans
  jamais le traverser, quelle que soit sa vitesse (c'est d'ailleurs l'exemple numérique du
  tunneling au §1 : le dash est précisément le cas de vitesse élevée que le balayage continu doit
  couvrir).

## 6. Wall jump et wall slide

`EX-GP-016`. **Après** résolution du balayage du pas (§1), le système détecte un **contact
horizontal** : le personnage est en l'air et son mouvement voulu le pousse **contre** un mur
vertical. Ce contact fixe `wallDirection` (le **sens** du mur touché : `-1` à gauche, `+1` à
droite). Deux comportements en découlent :

- **wall slide** (glissade contre le mur) : tant que ce contact persiste et que le personnage
  **descend**, sa vitesse de chute est **plafonnée** à `wallSlideSpeed` — nettement plus lente que
  la chute libre normale (qui ne plafonne plus, elle s'approche d'une vitesse terminale émergente,
  §3). Cela donne au joueur le temps de réagir
  (viser un wall jump, ou simplement ralentir sa chute contre un mur) plutôt que de tomber à pleine
  vitesse le long d'une paroi ;
- **wall jump** : un saut déclenché dans cet état éjecte le personnage en **diagonale opposée** au
  mur (composantes `wallJumpSpeedX` horizontale et `wallJumpSpeedY` verticale, dans le sens **opposé**
  à `wallDirection`), et **verrouille** le contrôle horizontal du joueur pendant `wallJumpLockTimer`
  secondes. **Pourquoi verrouiller** : sans ce verrou, un joueur qui maintient la direction
  « vers le mur » (par exemple pour rester collé et enchaîner un wall slide) annulerait
  instantanément l'éjection horizontale du wall jump en la contrant par son intention — l'action
  paraîtrait ne rien faire. Le verrou impose que l'impulsion produise son effet, le temps de
  porter le personnage suffisamment loin du mur, avant de rendre la main au joueur.

## Ordre d'un pas (résumé)

Pour chaque personnage, `CharacterPhysicsSystem::update` exécute, dans cet **ordre précis**, à
chaque pas fixe :

1. orientation (`facing`) ;
2. minuteries (coyote time, jump buffer, verrou de wall jump) ;
3. **dash** — démarrage sur front d'appui, ou maintien s'il est en cours ;
4. sinon **saut** (selon les sources d'autorisation du §4) ;
5. hauteur de saut variable (coupe de vitesse si le bouton est relâché) ;
6. vitesse horizontale (intention du joueur) ;
7. **gravité effective** (intégration, phases du §3) ;
8. wall slide (plafond de vitesse de chute contre un mur) ;
9. **balayage** (résolution des collisions, §1) ;
10. mise à jour de la position ;
11. **suivi de pente** (§2) — cale la position verticale sur une surface suivable franchie ce pas,
    sauf en cas de saut (`velocity.y < 0`) ;
12. annulation des composantes de vitesse bloquées par le balayage (sauf axe Y si calé sur une
    pente) ;
13. recalcul de `grounded` (contact bloquant sous la boîte **ou** calé sur une pente) ;
14. détection du contact avec un mur (pour le wall jump/slide du pas **suivant**).

Cet ordre précis est un **contrat** à respecter scrupuleusement dans toute évolution du système :
c'est lui qui garantit à la fois le déterminisme (`EX-NFR-002`, @ref guide-boucle) et la
compatibilité entre mécaniques qui, sans lui, pourraient s'annuler ou se combiner de façon
incohérente d'un pas à l'autre (par exemple, appliquer la gravité **avant** de traiter un dash en
cours romprait la suspension de gravité du §4).

## Voir aussi
- `core::sweepAabb`, `core::SweepResult`, `core::Aabb` — la primitive de collision.
- `core::slopeSurfaceHeight`, `core::resolveSlopeFollow`, `core::isFollowableSurface` — le suivi de
  pente (§2).
- `core::CharacterPhysicsSystem`, `core::PhysicsConfig`, `core::Player`, `core::PlayerInput`.
- @ref guide-maths pour `Vector2`, `Aabb` et les conventions d'unités/repère.
- @ref guide-niveaux pour le budget de sauts/dashs et les mécanismes qui influent sur la grille de
  collision consommée par le balayage.
