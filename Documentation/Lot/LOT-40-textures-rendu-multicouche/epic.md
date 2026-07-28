# LOT-40 — Fondations : registre de textures indépendantes + rendu multi-textures {#lot-40}

> Statut : **non commencé**. Prérequis : [LOT-39](@ref lot-39) (chargement de textures depuis
> fichiers). Prépare LOT-41 → LOT-48 (mode de création de textures dans l'éditeur).

## Objectif
Lever le verrou technique qui bloque toute la suite du programme « textures » : aujourd'hui,
`hmi::SpriteBatch::begin(projection, texture)` ne lie **qu'une seule** texture par lot, et
`hmi::SpriteRenderer`/`hmi::DraftRenderer` pilotent chacun une unique `hmi::TextureAtlas` de bout en
bout. Aucune des couches à venir (skin global par type de tuile, fond de niveau, objets interactifs
texturés individuellement) n'est dessinable tant qu'un niveau ne peut afficher qu'une seule texture à
la fois. Ce lot introduit un registre de textures indépendantes et un rendu qui regroupe ses quads
par texture, **sans changer le rendu visible actuel**.

## Périmètre

### Inclus
- **`hmi::TextureCache`** : registre de textures chargées à la demande par **nom logique**, construit
  directement sur `hmi::TextureLoader`/`hmi::AssetPaths` (LOT-39) — décodage + upload D3D11 +
  résolution de chemin déjà génériques, seule la mise en cache par nom est nouvelle. RAII (`ComPtr`),
  repli sans exception si un asset est absent/illisible (`EX-NFR-040`).
- **`hmi::RenderLayer`** : un jeu de constantes/enum **unique**, réservé maintenant même si seule une
  valeur est utilisée avant LOT-43+ (`Background, Shadow, Tile, Object, EditorOverlay`) — évite que
  chaque lot suivant invente son propre ordonnancement de calques.
- **Rendu multi-textures** : `hmi::SpriteRenderer`/`hmi::DraftRenderer` regroupent leurs quads par
  `(RenderLayer, ID3D11ShaderResourceView*)` et émettent un `SpriteBatch::begin/end` par groupe
  contigu, **dans l'ordre des calques** — `SpriteBatch` lui-même n'est pas modifié (même décision que
  LOT-39 pour `TextureAtlas` : interface conservée).
- **Texture de repli en damier magenta** : générée procéduralement (sœur de
  `hmi::buildProceduralAtlasImage`), créée une fois, utilisée par tout lot futur ayant besoin
  d'indiquer « texture attendue mais absente », avec `GRAPHICS_LOG_WARNING`.
- Tests de la logique de regroupement et du cache, découplés du GPU (`EX-NFR-010`).

### Exclus (hors périmètre de ce lot)
- Aucune nouvelle couche visible (fond, skin, objets) — ce lot ne change **aucun** pixel affiché
  aujourd'hui.
- Aucun changement de `Level`/`LevelDraft` (`Core` non touché).
- Aucune UI éditeur.
- Optimisation par atlas dynamique/packing : hors périmètre, les niveaux de ce projet restent petits
  (quelques dizaines de tuiles/objets) — plusieurs passes `begin/end` par frame sont largement assez
  rapides ; à revisiter seulement si un profilage futur montre un problème réel.

## Décisions de cadrage
- **Plusieurs passes `begin/end` plutôt qu'un atlas partagé dynamique** : simplicité, coût des
  changements d'état D3D11 négligeable à l'échelle des niveaux du projet, `SpriteBatch` reste
  inchangé.
- **`TextureCache` sans éviction** : les niveaux et le nombre d'assets restent bornés ; pas de
  stratégie de libération mémoire pour ce lot — à revisiter si le nombre d'assets grandit
  significativement.
- **Tri stable** : à l'intérieur d'un calque, le tri par texture ne doit **jamais** réordonner deux
  sprites de calques différents — l'ordre par `RenderLayer` prime toujours sur le regroupement par
  texture (pas de régression sur `Sprite::layer`, déjà trié par `SpriteRenderer::render`).
- **`RenderLayer` réservé maintenant** : décision volontairement anticipée (cf. audit du programme
  LOT-40→48) pour qu'aucun lot suivant ne réinvente un ordonnancement de calques concurrent.

## Exigences couvertes
- Nouvelle : `EX-REN-043` (rendu multi-textures par calques).
- Réutilisées : `EX-NFR-040` (repli sans plantage), `EX-NFR-010` (logique testable sans GPU),
  `EX-ARCH-022` (nearest, inchangé).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-texture-cache.md) | `hmi::TextureCache` — registre de textures indépendantes par nom logique | `Source/HMI/Graphics` | ⬜ |
| [TACHE-02](tache-02-rendu-multicouche.md) | `hmi::RenderLayer` + regroupement des quads par `(layer, texture)` dans `SpriteRenderer`/`DraftRenderer` | `Source/HMI/Graphics` | ⬜ |
| [TACHE-03](tache-03-damier-magenta.md) | Texture de repli en damier magenta + tests de non-régression du rendu | `Source/HMI/Graphics`, `Source/Test` | ⬜ |

## Critères d'acceptation du lot
1. Le rendu (jeu et éditeur) est **pixel-identique** à avant ce lot — aucune régression visuelle,
   vérifiée manuellement.
2. Une scène de test peut afficher des sprites provenant d'**au moins deux textures distinctes** en
   une frame, dans l'ordre de calque attendu (test manuel/GPU).
3. `hmi::SpriteBatch` n'a **aucune** signature publique modifiée.
4. Le regroupement par `(layer, texture)` et le cache de textures sont **testés sans GPU**.
5. Build `/W4 /WX`, Doxygen et lint verts.

## Dépendances
Bâtit sur [LOT-39](@ref lot-39) (`hmi::TextureLoader`/`hmi::AssetPaths`/`hmi::buildProceduralAtlasImage`).
Ne modifie pas `Core`. Prérequis de [LOT-41](@ref lot-41) à [LOT-48](@ref lot-48).

## Navigation des tâches
- @subpage lot-40-tache-01-texture-cache
- @subpage lot-40-tache-02-rendu-multicouche
- @subpage lot-40-tache-03-damier-magenta
