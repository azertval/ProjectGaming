# Mathématiques du moteur {#guide-maths}

Le moteur n'utilise **aucune** bibliothèque mathématique tierce dans `Core` : un type minimal suffit
et garde `Core` indépendant de DirectX (`EX-ARCH-040`).

## `Vector2`

`core::Vector2` est un vecteur 2D à composantes `float` (`x`, `y`). Il fournit l'algèbre usuelle :
addition/soustraction, produit par un scalaire, opposé, **produit scalaire**
([dot product](https://fr.wikipedia.org/wiki/Produit_scalaire) ⧉, `dot`), **longueur**
(`length`, `lengthSquared`) et **normalisation** (`normalized` — renvoie le vecteur nul si la
longueur est négligeable). L'égalité (`operator==`) est **approchée** (tolérance), car comparer des
flottants à l'exactitude est fragile.

`lengthSquared` évite une racine carrée quand on n'a besoin que de comparer des longueurs
(optimisation classique).

## Boîtes englobantes : `Aabb`

`core::Aabb` (dans `Core/Physics`) est un rectangle **aligné aux axes**
([AABB](https://en.wikipedia.org/wiki/Bounding_volume) ⧉), décrit par ses coins `min` (haut-gauche)
et `max` (bas-droite). `Aabb::fromTopLeftSize(topLeft, size)` le construit depuis la convention
« coin + taille » du `core::Collider`. C'est la brique du balayage (@ref guide-physique).

## Conventions d'unités et de repère

- **Une tuile = 1 unité monde.** Les positions, tailles et vitesses sont en unités monde
  (ou unités/seconde), **jamais** en pixels dans `Core`. La conversion en pixels se fait côté rendu
  (16 px/unité).
- **Origine haut-gauche**, `x` → droite, `y` → **bas** (`EX-ARCH-020`). Donc « monter » = `y` qui
  **diminue** ; la gravité pousse vers les `y` **positifs**.
- **Angles en radians** (`Transform::rotation`).

Garder ces conventions en tête explique tous les signes de la physique.

## Comparaison flottante

`core::approximatelyEqual` (dans `Core/Math`) compare deux `float` avec une tolérance **relative et
absolue** (robuste aux grandes magnitudes). Utilisée par `Vector2::operator==` et les tests.

## Voir aussi
- `core::Vector2`, `core::Aabb`, `core::Rect`, `core::approximatelyEqual`.
- @ref guide-physique.
