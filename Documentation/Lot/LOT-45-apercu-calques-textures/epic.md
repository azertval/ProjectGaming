# LOT-45 — Mode « définition des textures » : aperçu isolé par calque {#lot-45}

> Statut : **non commencé**. Prérequis : [LOT-41](@ref lot-41), [LOT-43](@ref lot-43),
> [LOT-44](@ref lot-44).

## Objectif
Livrer, précisément, la demande n°1 de l'utilisateur : visualiser **séparément**, dans l'éditeur,
chacun des trois calques de texture — Fond, Physique/skin, Interactif — à des fins d'inspection.
Ce lot est **délibérément distinct** de la bascule `F8` (LOT-41) : `F8` **compose** le rendu final
(override > skin > damier) ; ce mode **isole** un calque à la fois, pour auditer ce qui a été
configuré à cet endroit précis. Jamais exposé au joueur.

## Périmètre

### Inclus
- **Mode d'aperçu éditeur uniquement** (ex. contrôle segmenté ou raccourci de cycle dans l'éditeur,
  distinct de `F8`) avec trois états :
  - **Fond seul** : uniquement l'image de fond (LOT-43), aucune tuile, aucun objet.
  - **Physique seul** : le rendu flat-color existant (déjà disponible, ce lot l'ajoute simplement
    comme état sélectionnable dans ce nouveau mode plutôt qu'un renderer séparé).
  - **Interactif seul** : uniquement les cases portant un `TileTextureOverride` (LOT-44) — les autres
    cases restent transparentes/neutres. **Ne retombe volontairement pas** sur le skin global ni sur
    le damier magenta : le but est d'auditer *seulement* ce calque, pas le rendu composé.
- N'affecte ni `GameSession` (jeu réel), ni la bascule `F8`.

### Exclus (hors périmètre de ce lot)
- Tout ce qui touche au jeu réel ou au test « Essayer » — éditeur uniquement.
- Un 4ᵉ état « skin global seul » (peut être ajouté plus tard si le besoin apparaît ; hors périmètre
  initial pour rester au plus près de la demande formulée).

## Décisions de cadrage
- **Distinct de `F8` par construction** : les deux réutilisent le même résolveur de priorité
  (LOT-40/44) mais avec des règles d'affichage différentes (composition vs. isolement) — à documenter
  clairement dans l'UI pour ne pas confondre les deux (ex. libellés « Aperçu » vs. « Jeu »).
- Toute UI de ce mode passe par le catalogue de traduction.

## Exigences couvertes
- Nouvelle : `EX-EDIT-044` (aperçu isolé par calque, éditeur uniquement).
- Réutilisées : `EX-REN-043` (rendu multi-calques), `EX-REN-044` (fond), `EX-EDIT-043` (objets
  interactifs), `EX-REN-046` (distinct de la bascule).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | Contrôle d'aperçu (3 états) + câblage dans `GameViewport`/`DraftRenderer` (éditeur uniquement) | `Source/HMI/Game`, `Source/HMI/Graphics` | ⬜ |
| TACHE-02 | Rendu « Interactif seul » (n'affiche que les cases avec override, pas de repli) | `Source/HMI/Graphics` | ⬜ |
| TACHE-03 | Vérification manuelle des trois états + documentation | `Documentation` | ⬜ |

## Critères d'acceptation du lot
1. Les trois états (Fond seul, Physique seul, Interactif seul) sont sélectionnables indépendamment de
   `F8`.
2. Le mode « Interactif seul » n'affiche **que** les cases avec override — jamais de repli skin ni
   damier.
3. Aucun effet sur `GameSession`/le jeu réel.
4. Build `/W4 /WX`, Doxygen, lint verts ; vérification manuelle des trois états.

## Dépendances
Bâtit sur [LOT-41](@ref lot-41) (plomberie de mode), [LOT-43](@ref lot-43) (Fond),
[LOT-44](@ref lot-44) (Interactif). Éditeur uniquement (`Source/HMI/Game`, `Source/HMI/Graphics`,
`Source/HMI/Editor`).
