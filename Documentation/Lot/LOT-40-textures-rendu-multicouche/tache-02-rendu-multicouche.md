# TACHE-02 — `hmi::RenderLayer` + regroupement des quads par (calque, texture) {#lot-40-tache-02-rendu-multicouche}

**Lot :** [LOT-40](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** non commencé

## Contexte
`hmi::SpriteRenderer::render` trie déjà ses sprites par `core::Sprite::layer` (entier simple) avant
de les soumettre en **un seul** `SpriteBatch::begin/end`, puisqu'il n'existe aujourd'hui qu'une seule
texture (l'atlas). Cette tâche introduit un ordonnancement de calques **nommé** et fait émettre
**plusieurs** passes `begin/end`, une par groupe contigu de même texture, sans changer une seule
ligne de `SpriteBatch`.

## Travail à réaliser
- **`hmi::RenderLayer`** (`Source/HMI/Graphics/RenderLayer.h`) : `enum class RenderLayer { Background,
  Shadow, Tile, Object, EditorOverlay };` — ordre de déclaration = ordre de dessin. Seule `Tile` est
  utilisée par ce lot (comportement actuel) ; les autres valeurs sont réservées pour LOT-43/44/45/46.
- **`SpriteRenderer`** : la structure interne `LayeredQuad` gagne un `RenderLayer` (en plus de
  `core::Sprite::layer`, qui reste le tri **fin** à l'intérieur d'un calque) et une référence de
  texture. Le tri devient : `RenderLayer` (ordre fixe), puis texture (regroupement, ordre stable),
  puis `Sprite::layer` (comme aujourd'hui) — jamais l'inverse, pour ne pas régresser l'ordre visuel
  actuel. `render()` émet un `begin/end` par groupe contigu de même texture, dans cet ordre.
- **`DraftRenderer`** : même traitement pour la scène de l'éditeur (`rebuild()`).
- Toute entité/tuile existante reste implicitement sur `RenderLayer::Tile` avec la texture de l'atlas
  — **aucun changement de comportement** tant qu'aucun appelant ne pousse un sprite sur un autre
  calque/texture (ce qui n'arrive qu'à partir de LOT-42+).

## Fichiers impactés
- `Source/HMI/Graphics/RenderLayer.h` (nouveau).
- `Source/HMI/Graphics/SpriteRenderer.{h,cpp}`, `Source/HMI/Graphics/DraftRenderer.{h,cpp}`
  (regroupement, plusieurs passes).

## Tests (obligatoires)
- Logique de regroupement/tri (ordre de calque > texture > `Sprite::layer`) testable sans GPU avec
  des textures/handles factices (pointeurs distincts, pas de vraies ressources D3D11).
- **Non-régression visuelle** : vérification manuelle que le rendu actuel (jeu et éditeur) est
  identique avant/après.

## Points d'attention
- Le tri doit rester **stable** (comme aujourd'hui, `std::stable_sort`) : à texture et calque égaux,
  l'ordre de soumission d'origine est préservé.
- `SpriteBatch::begin/end` : contrat public strictement inchangé (aucune signature modifiée).

## Définition de fait (DoD)
- Rendu multi-textures fonctionnel, pixel-identique au rendu actuel tant qu'une seule texture est
  utilisée ; `/W4 /WX` propre ; logique de regroupement testée sans GPU.

## Exigences
`EX-REN-043` (rendu multi-textures par calques).
