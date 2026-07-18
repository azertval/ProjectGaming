# Physique du personnage {#guide-physique}

Toute la physique vit dans `core::CharacterPhysicsSystem` (système ECS pur) et s'appuie sur la
primitive de collision `core::sweepAabb`. Elle s'exécute au **pas de temps fixe** (@ref
guide-boucle), donc **déterministe** : mêmes entrées → même trajectoire (`EX-NFR-002`).

Repère du projet : origine **haut-gauche**, `x` vers la droite, `y` vers le **bas**. Une tuile = **1
unité monde**. « Monter » est donc une vitesse **négative** en `y`.

## 1. Collision par balayage continu (swept AABB)

`core::sweepAabb(box, delta, tiles)` déplace une boîte `core::Aabb` de `delta` **sans jamais
traverser** une tuile solide, quelle que soit la vitesse (`EX-GP-014`). C'est le cœur de la
robustesse « pas de tunneling ».

### Pourquoi « continu »
Un test de recouvrement en **fin** de pas raterait un mur plus fin que le déplacement d'une frame
(l'objet « saute » par-dessus). Le balayage teste **tout le trajet**.

### Méthode retenue : balayage **par axe** avec clamp direct
On résout **X puis Y** séparément (`sweepX`, `sweepY` dans `SweptCollision.cpp`). Pour chaque axe :

1. on parcourt **toutes les cellules** entre le bord d'attaque courant et le bord d'attaque visé ;
2. au **premier** solide rencontré, on **cale** la position sur la coordonnée entière du mur (ex.
   `col - size.x`).

Le clamp **direct** (et non `position += delta·t`) évite toute **dérive flottante** : caler sur la
coordonnée du mur donne une position exacte, ce qui supprime le bug du « bord interne » où le
personnage *colle* au mur lors d'un glissement.

> Note historique (voir le commit de la TACHE-02) : une première version « diagonale » par
> [somme de Minkowski](https://fr.wikipedia.org/wiki/Somme_de_Minkowski) ⧉ +
> [méthode des *slabs*](https://en.wikipedia.org/wiki/Slab_method) ⧉ a été abandonnée — élégante
> mais victime du bord interne à cause de la dérive flottante. Le balayage par axe est plus robuste
> et tout aussi continu. Une fine « peau » (`kSkin`) sur l'axe perpendiculaire évite de confondre
> *marcher sur* un sol et *buter contre* lui.

Le résultat `core::SweepResult` porte la **position** finale et une **normale** indicatrice par axe
(`-1`/`+1`/`0`) que le système lit pour annuler la vitesse et déduire l'appui (sol, mur, plafond).

## 2. Gravité et intégration

À chaque pas, la vitesse est intégrée par [intégration d'Euler
explicite](https://fr.wikipedia.org/wiki/M%C3%A9thode_d%27Euler) ⧉ : `velocity += g·dt`, puis
`delta = velocity·dt`, puis balayage. La gravité est **effective** (`EX-GP-018`) — elle dépend de la
phase du saut (`core::PhysicsConfig`) :

- **montée** : gravité de base ;
- **chute** : × `fallGravityMultiplier` (la chute est plus « lourde » que la montée) ;
- **fast-fall** : × `fastFallMultiplier` supplémentaire si « bas » est maintenu ;
- **apex** : × `apexGravityMultiplier` quand `|vy| < apexThreshold` (flottement au sommet).

La vitesse de chute est bornée par `maxFallSpeed`.

## 3. Saut et *game feel*

Le déclenchement du saut est **contextuel** — plusieurs sources d'autorisation, dans l'ordre :

1. **sol / coyote time** : `coyoteTimer` est rechargé au sol et décompté en l'air, autorisant un
   saut un court instant **après** avoir quitté un bord (pardonne l'erreur d'une frame) ;
2. **wall jump** (`EX-GP-016`) : contre un mur (voir §5), éjection en diagonale opposée ;
3. **saut aérien** (`EX-GP-015`, double saut) : `airJumpsRemaining`, rechargé au sol.

Deux tolérances complètent le ressenti :

- **jump buffering** : `jumpBufferTimer`, rechargé à l'appui, honore un saut **pré-appuyé** juste
  avant l'atterrissage ;
- **hauteur variable** : relâcher le bouton pendant la montée **plafonne** la vitesse ascendante
  (`jumpCutFactor`) → petit saut vs saut complet.

Le **budget** de sauts du tableau (`EX-GP-024`) peut refuser le saut une fois épuisé
(`jumpsRemaining`, `-1` = illimité).

## 4. Dash 8 directions

`EX-GP-017`. Sur front d'appui, si le dash est **disponible** (`dashAvailable`, rechargé au sol) et
le budget non épuisé (`dashesRemaining`), le personnage part en **ruée** : direction `(moveX,
moveY)` **normalisée** (8 directions ; à défaut l'orientation `facing`), vitesse `dashSpeed` pendant
`dashDuration`. Pendant le dash, la **gravité et l'entrée sont suspendues** (trajectoire nette) ; le
balayage l'arrête sur un mur. La normalisation garantit qu'une diagonale ne va pas plus vite qu'une
cardinale.

## 5. Wall jump et wall slide

`EX-GP-016`. Après le balayage, un **contact horizontal** en l'air alors que le personnage **pousse
vers** le mur fixe `wallDirection` (sens du mur). Conséquences :

- **wall slide** : en descente, la vitesse de chute est **plafonnée** à `wallSlideSpeed` (glisse
  lente le long du mur) ;
- **wall jump** : un saut éjecte en **diagonale opposée** (`wallJumpSpeedX/Y`) et **verrouille** le
  contrôle horizontal (`wallJumpLockTimer`) le temps que l'éjection porte le personnage loin du mur
  (sinon l'appui vers le mur annulerait aussitôt l'éjection).

## Ordre d'un pas (résumé)

Pour chaque personnage, `CharacterPhysicsSystem::update` fait, dans l'ordre : orientation → minuteries
(coyote/buffer/verrou) → **dash** (démarrage ou maintien) → sinon **saut** → hauteur variable →
vitesse horizontale → **gravité effective** → wall slide → **balayage** → position → annulation des
vitesses bloquées → `grounded` → détection du **mur**. Cet ordre est le contrat à respecter pour
préserver le déterminisme et la compatibilité entre mécaniques.

## Voir aussi
- `core::sweepAabb`, `core::SweepResult`, `core::Aabb` — la primitive de collision.
- `core::CharacterPhysicsSystem`, `core::PhysicsConfig`, `core::Player`, `core::PlayerInput`.
- @ref guide-maths pour `Vector2` et les conventions d'unités.
