# LOT-44 — Objet interactif : texture par instance {#lot-44}

> Statut : **non commencé**. Prérequis : [LOT-40](@ref lot-40), [LOT-41](@ref lot-41),
> [LOT-42](@ref lot-42). Bénéficie de [LOT-43](@ref lot-43).

## Objectif
Permettre d'assigner une texture propre à **une case précise** d'un niveau (ex. « cette porte-là »),
prioritaire sur le skin global de son type (LOT-42). Seule vraie nouvelle donnée `Core` du programme
— toutes les autres restent globales (`skins.json`) ou déjà couvertes par le patron existant
(annexe de `Level`).

## Périmètre

### Inclus
- **`Core`** : `struct TileTextureOverride { GridPosition position; std::string assetName; };`,
  vecteur annexe sur `Level`/`LevelDraft` — **exactement** le patron déjà utilisé par `Mechanism`/
  `DangerLink`/`DangerMoverConfig`/`DangerBlinkConfig` (`Source/Core/Levels/Level.h`).
- **JSON** : champ par tuile optionnel (ex. `"texture": "door_red.png"`), même précédent que
  `switch.id`/`door.opensWith` dans `LevelLoader.cpp`/`LevelWriter.cpp` — **rétrocompatible**.
- **Nettoyage** : `TileTextureOverride` rejoint le funnel **déjà existant**
  `LevelDraft::removeLinkedDataAt` (appelé par `paintTileInternal`, donc par `paintTile` **et**
  `paintRegion`) — un override est retiré quand sa case reçoit un autre type de tuile. Participe
  aussi à la troncature silencieuse de `resize` (même principe que les autres données annexes).
  **Ne voyage pas** avec un copier-coller (`paintRegion` utilisé pour le collage n'importe **pas**
  les overrides de la source) — décision explicite, un override reste attaché à sa case d'origine.
- **Undo/redo** : gratuit via `LevelDraft::State` (même mécanisme que les autres champs annexes).
- **Éditeur** : nouvelle valeur `hmi::EditorTool` (ex. `TextureAssign`) + geste pur d'assignation
  modelé sur `hmi::resolveLinkClick`/`LinkGesture` (LOT-37) — clic sur une case → sélecteur parmi
  `Assets/Objects/*.png` → assignation/suppression. Section « Objets » du panneau « Textures »
  (LOT-42) listant les overrides du niveau courant, pas un panneau séparé.
- **Résolution en mode Texture** : **override > skin global (LOT-42) > damier magenta (LOT-40)**.
- Dossier `Assets/Objects/` + commande `POST_BUILD`.

### Exclus (hors périmètre de ce lot)
- Vignettes dans le sélecteur (LOT-47).
- Restriction du type de tuile éligible : **tout** type non-`Empty` peut recevoir un override (usage
  purement visuel, contrairement aux liens de mécanismes qui ont un rôle sémantique) — pas de liste
  blanche façon `isTriggerTile`/`isLinkTargetTile`.

## Décisions de cadrage
- **Geste dédié, pas une extension de `LinkGesture`** : la machine à états de `LinkGesture` est
  façonnée pour l'appariement déclencheur/cible, pas l'assignation sur une seule case — un module
  parallèle (`TextureAssignGesture`) évite de coupler deux outils sans rapport.
- **Priorité de résolution figée** : override > skin global > damier — cette hiérarchie est le
  contrat que LOT-45 (aperçu isolé) devra respecter en l'inversant délibérément pour son mode
  « Interactif seul ».
- **Un seul point de nettoyage** (`removeLinkedDataAt`) : pas de mécanisme parallèle, cf. audit du
  programme LOT-40→48.

## Exigences couvertes
- Nouvelle : `EX-EDIT-043` (assignation d'une texture par instance, priorité sur le skin global).
- Réutilisées : `EX-REN-043`/`EX-REN-046` (rendu, bascule), `EX-EDIT-042` (skin global, priorité
  inférieure), `EX-EDIT-010` (réutilisation du modèle `Core`), `EX-NFR-040` (repli).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | `TileTextureOverride` sur `Level`/`LevelDraft` + JSON + nettoyage (`removeLinkedDataAt`, `resize`) + undo/redo | `Source/Core/Levels` | ⬜ |
| TACHE-02 | `EditorTool::TextureAssign` + geste pur d'assignation (modelé sur `LinkGesture`) | `Source/HMI/Editor` | ⬜ |
| TACHE-03 | Section « Objets » du panneau « Textures » + résolution de priorité au rendu + dossier `Assets/Objects/` | `Source/HMI/Editor`, `Source/HMI/Graphics`, `Source/HMI/CMakeLists.txt` | ⬜ |

## Critères d'acceptation du lot
1. Assigner une texture à une case précise l'affiche avec cette texture, même si son type a un skin
   global différent (LOT-42).
2. Repeindre une autre type de tuile sur cette case retire l'override ; redimensionner hors de sa
   position aussi.
3. Copier-coller une région ne copie **pas** les overrides des cases source.
4. Un niveau existant (sans override) se charge sans changement de comportement.
5. Round-trip JSON, mutateurs `LevelDraft` (dont undo/redo) et geste d'assignation testés sans GPU ;
   build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40), [LOT-41](@ref lot-41), [LOT-42](@ref lot-42) (résolveur de
priorité). Bénéficie de [LOT-43](@ref lot-43) sans en dépendre strictement. Réutilise le patron
`Mechanism`/`DangerLink` (`Core`) et `LinkGesture`/`LinkGeometry`/`LinkPanel` (LOT-37, `HMI`).
Prépare [LOT-45](@ref lot-45).
