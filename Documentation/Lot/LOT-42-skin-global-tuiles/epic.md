# LOT-42 — Skin global : texture par type de tuile {#lot-42}

> Statut : **non commencé**. Prérequis : [LOT-40](@ref lot-40), [LOT-41](@ref lot-41). Répond à la
> demande d'origine (« définir les textures utilisées par un bloc »).

## Objectif
Permettre au level designer d'associer, **globalement** (tous niveaux confondus), une texture réelle
à chaque type de tuile — le cœur de la demande initiale. C'est aussi le lot qui introduit le panneau
Qt **« Textures »**, conçu dès maintenant pour être réutilisé (pas remplacé) par LOT-43/44/47.

## Périmètre

### Inclus
- **`skins.json`** : association `TileType → nom de fichier`, **globale et hors `Core`** — fichier
  côté `HMI` uniquement, jamais lu/écrit par `Level`/`LevelDraft`. Copié à côté de l'exécutable comme
  `Levels`/`Localization` (`Source/Elements/Assets/skins.json` = version livrée par défaut). L'éditeur
  **écrit** au chemin déployé (`executableDirectory()/Assets/skins.json`), exactement comme
  l'enregistrement d'un niveau (`Source/HMI/Editor/LevelFileOperations.cpp`) — aucun nouveau mécanisme
  d'écriture à inventer.
- **Sélection par fichiers indépendants** : `Assets/Skins/*.png`, listés par balayage de dossier —
  aucune saisie de chemin.
- **Masquage alpha automatique des pentes/arrondis** : pour les douze types à silhouette
  inclinée/courbe, le skin chargé est **détouré** à la bonne forme au chargement, via les fonctions
  pures déjà existantes (`core::slopeSurfaceHeight`/`ceilingSlopeHeight`, `core::isCeilingSlope`) —
  l'artiste peint un carré plein, le moteur applique le masque une fois (16×16 pixels, coût
  négligeable, résultat mis en cache comme toute texture de *TextureCache*).
- **Panneau Qt « Textures »** (nouveau dock, même patron d'enregistrement que `PalettePanel`/
  `LevelsPanel`/`ToolPanel`/`LinksPanel` dans `MainWindow`) : une section « Skins » listant chaque
  `TileType` avec un sélecteur parmi les fichiers de `Assets/Skins/`. Conçu en sections/onglets dès ce
  lot pour que LOT-43 (Fond) et LOT-44 (Objets) y ajoutent la leur, sans nouveau panneau.
- Résolution en mode Texture : type skinné → texture assignée ; type non skinné → repli en damier
  magenta (LOT-40) + `GRAPHICS_LOG_WARNING`.
- Toute UI de ce panneau passe par le catalogue de traduction (`_loc.text`, aucun libellé en dur).
- Dossier `Assets/Skins/` + commande `POST_BUILD` dans `Source/HMI/CMakeLists.txt`.

### Exclus (hors périmètre de ce lot)
- Fond de niveau, objets interactifs (LOT-43/44).
- Vignettes/aperçu visuel dans le panneau (liste texte pour ce lot ; upgrade en LOT-47).
- Rechargement à chaud (LOT-47).

## Décisions de cadrage
- **Frontière `Core`/`HMI` non négociable** : `skins.json` est une configuration globale, jamais une
  donnée de niveau — ne transite jamais par `LevelDraft` ni son undo/redo (à la différence de
  LOT-43/44, qui eux touchent `Level`).
- **Mise à l'échelle des blocs réduits gratuite** : `BlockHalf`/`BlockQuarter`
  (`core::tileVisualScale`) fonctionnent sans code supplémentaire — `SpriteRenderer` applique déjà
  l'échelle du `Transform` quelle que soit la source de la texture.
- **Un seul panneau « Textures »**, pas un panneau par couche : décision transverse au programme
  LOT-40→48, prise ici puisque c'est le premier panneau créé.

## Exigences couvertes
- Nouvelle : `EX-EDIT-042` (association globale type de tuile → texture).
- Réutilisées : `EX-REN-043` (rendu multi-textures), `EX-REN-046` (bascule Physique/Texture),
  `EX-NFR-040` (repli), `EX-REN-033` (traduction).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | `skins.json` : lecture/écriture, résolution `TileType → texture`, testable sans GPU | `Source/HMI/Graphics` ou `Source/HMI/Editor` | ⬜ |
| TACHE-02 | Masquage alpha automatique des pentes/arrondis au chargement d'un skin | `Source/HMI/Graphics` | ⬜ |
| TACHE-03 | Panneau Qt « Textures » (section Skins) + câblage `MainWindow` + dossier `Assets/Skins/` | `Source/HMI/Editor`, `Source/HMI/Interface`, `Source/HMI/CMakeLists.txt` | ⬜ |

## Critères d'acceptation du lot
1. Assigner un skin à un type de tuile depuis le panneau « Textures » le rend visible dans le niveau
   et dans l'éditeur, en mode Texture uniquement.
2. Un type sans skin assigné affiche le damier magenta + un avertissement de log nommant le type.
3. Les tuiles à silhouette inclinée/courbe affichent leur skin **découpé à la bonne forme**, pas un
   carré plein.
4. `skins.json` ne transite jamais par `Core`/`LevelDraft`.
5. Toute nouvelle UI passe par le catalogue de traduction.
6. Résolution `skins.json` testée sans GPU ; build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40) (rendu multi-textures), [LOT-41](@ref lot-41) (bascule). Réutilise
`hmi::AssetPaths`/*TextureCache* (LOT-39/40) et `hmi::slopeTileGridPosition`/géométrie de pentes
(`Core`). Ne modifie pas `Level`/`LevelDraft`. Prépare [LOT-43](@ref lot-43),
[LOT-44](@ref lot-44), [LOT-47](@ref lot-47) (panneau « Textures » réutilisé).
