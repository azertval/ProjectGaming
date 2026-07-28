# TACHE-01 — *TextureCache* : registre de textures indépendantes par nom logique {#lot-40-tache-01-texture-cache}

**Lot :** [LOT-40](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** non commencé

## Contexte
`hmi::TextureAtlas` charge **une seule** texture fixe (`Assets/atlas.png`). Les lots suivants
(skin global, fond de niveau, objets interactifs) ont besoin de charger un **nombre variable** de
textures indépendantes, chacune identifiée par son nom de fichier logique (ex. `"forest.png"` dans
`Assets/Backgrounds/`). Cette tâche introduit le registre qui les charge à la demande et les met en
cache, en réutilisant telles quelles les primitives de LOT-39.

## Travail à réaliser
- ***TextureCache*** (`Source/HMI/Graphics/TextureCache.{h,cpp}`) :
  - Construit avec un `ID3D11Device*` et un `hmi::AssetPaths` (dossier de base).
  - `get(const std::string& fileName) -> const LoadedTexture*` (ou `std::optional<...>` selon
    convention) : résout via `AssetPaths::resolve`, charge via `hmi::loadTextureFromFile` au premier
    accès, met en cache par nom, renvoie le même pointeur/handle aux accès suivants ; renvoie
    `nullptr`/`nullopt` sans exception si l'asset est absent/illisible (`EX-NFR-040`) — **ne
    génère pas** elle-même le repli en damier magenta (TACHE-03 s'en charge, en amont de l'appelant).
  - RAII strict : toutes les textures possédées sont libérées à la destruction du cache
    (`Microsoft::WRL::ComPtr`, comme `TextureAtlas`/*TextureLoader*).
- Pas d'éviction (cf. décision de cadrage de l'epic) : le cache grossit avec les assets rencontrés,
  jamais libéré avant sa propre destruction.

## Fichiers impactés
- `Source/HMI/Graphics/TextureCache.{h,cpp}` (nouveau).

## Tests (obligatoires)
- Logique de résolution/mise en cache (nom → même handle réutilisé, asset absent → repli sans
  exception) : la partie sans GPU est testable via un `AssetPaths` pointant un dossier temporaire ;
  le chargement GPU effectif relève de la vérification visuelle (comme *TextureLoader*, LOT-39).

## Points d'attention
- Ne pas dupliquer la logique de `TextureAtlas::loadFromFile` : *TextureCache* et `TextureAtlas`
  partagent `hmi::loadTextureFromFile`/`hmi::AssetPaths`, aucun des deux ne réimplémente le
  décodage/upload.
- Chemins toujours résolus via `hmi::AssetPaths`, jamais de chemin en dur.

## Définition de fait (DoD)
- *TextureCache* charge et met en cache plusieurs textures indépendantes ; asset manquant géré sans
  exception ; `/W4 /WX` propre.

## Exigences
`EX-REN-043` (rendu multi-textures) ; réutilise `EX-NFR-040` (repli), `EX-NFR-010` (testable).
