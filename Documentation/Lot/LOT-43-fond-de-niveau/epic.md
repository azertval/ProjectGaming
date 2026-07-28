# LOT-43 — Fond de niveau {#lot-43}

> Statut : **non commencé**. Prérequis : [LOT-40](@ref lot-40), [LOT-41](@ref lot-41).

## Objectif
Permettre d'associer une image de **fond** à un niveau donné, dessinée en dessous de toutes les
tuiles en mode Texture. Premier lot à toucher `Core` (un champ string/id, jamais un handle de
texture) et le format de niveau JSON.

## Périmètre

### Inclus
- **`Core`** : `std::optional<std::string> backgroundAsset` sur `core::Level`/`core::LevelDraft`
  (accesseur + `setBackground()`), ajouté à `LevelDraft::State` pour l'undo/redo — même patron que les
  champs optionnels existants (`jumpBudget`/`dashBudget`).
- **JSON** : champ racine optionnel (ex. `"background": "forest.png"`), lu/écrit dans
  `LevelLoader.cpp`/`LevelWriter.cpp` selon le patron déjà utilisé pour les champs racine optionnels —
  **rétrocompatible** : un niveau existant sans ce champ se charge sans aucun changement de
  comportement.
- **Résolution `HMI`** : `Assets/Backgrounds/*.png`, balayage de dossier, sélection dans le panneau
  « Textures » (section « Fond », créée dans le panneau introduit en LOT-42).
- **Rendu** : `RenderLayer::Background` (LOT-40), dessiné en mode Texture uniquement, **étiré** sur
  les bornes du niveau (un seul mode — pas de tuilage/parallax).
- Dossier `Assets/Backgrounds/` + commande `POST_BUILD`.

### Exclus (hors périmètre de ce lot)
- Fond par salle (`RoomGrid`) : le fond reste en **espace niveau**, simplement cadré par la caméra de
  la salle courante comme le reste du contenu — aucune donnée par salle.
- Fond animé, tuilage/scrolling parallax.
- Ombres portées sur le fond (LOT-46).

## Décisions de cadrage
- **Asymétrie manquant/introuvable** : un niveau **sans** fond configuré est un état normal (fond
  neutre, **pas** de damier magenta affiché) ; un fond **référencé mais introuvable** déclenche le
  damier + avertissement (`EX-NFR-040`) — cette distinction doit être explicite dans l'implémentation,
  ce n'est pas la même situation.
- **Étirement, pas de tuilage** : simplicité, correspond à l'intention « fond » plutôt que « tileset ».
- **Frontière `Core`** : `backgroundAsset` est une **string**, jamais un handle de texture — `Core`
  reste totalement ignorant du rendu.

## Exigences couvertes
- Nouvelle : `EX-REN-044` (fond de niveau optionnel, mode Texture).
- Réutilisées : `EX-REN-043` (rendu multi-textures), `EX-REN-046` (bascule), `EX-NFR-040` (repli),
  `EX-LVL-004`/format JSON existant.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | `backgroundAsset` sur `Level`/`LevelDraft` + JSON (lecture/écriture, rétrocompatible) + undo/redo | `Source/Core/Levels` | ⬜ |
| TACHE-02 | Rendu du fond (`RenderLayer::Background`, étiré, mode Texture) + repli damier si introuvable | `Source/HMI/Graphics`, `Source/HMI/Game` | ⬜ |
| TACHE-03 | Section « Fond » du panneau « Textures » (sélection parmi `Assets/Backgrounds/`) + dossier + `POST_BUILD` | `Source/HMI/Editor`, `Source/HMI/CMakeLists.txt` | ⬜ |

## Critères d'acceptation du lot
1. Un niveau sans fond configuré s'affiche exactement comme avant ce lot (mode Physique **et**
   Texture).
2. Un fond assigné et valide s'affiche en dessous de toutes les tuiles, en mode Texture uniquement.
3. Un fond référencé mais introuvable affiche le damier magenta + avertissement de log.
4. Un niveau existant (sans le nouveau champ) se charge sans erreur ni avertissement — rétrocompatible.
5. `backgroundAsset` testé (round-trip JSON, undo/redo) sans GPU ; build `/W4 /WX`, Doxygen, lint
   verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40), [LOT-41](@ref lot-41), et le panneau « Textures » de
[LOT-42](@ref lot-42). Première extension de `core::Level`/`LevelDraft` du programme. Prépare
[LOT-44](@ref lot-44), [LOT-45](@ref lot-45), [LOT-46](@ref lot-46).
