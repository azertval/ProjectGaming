# LOT-46 — Ombres du plan physique sur le fond {#lot-46}

> Statut : **non commencé**. Prérequis : [LOT-41](@ref lot-41). Bénéficie de
> [LOT-43](@ref lot-43).

## Objectif
Aider visuellement le level designer (et, en mode Texture, le joueur) à distinguer ce qui est
**physique** (solide, collidable) de ce qui est **décor** : les tuiles solides projettent une ombre
douce sur le fond du niveau, purement visuelle.

## Périmètre

### Inclus
- Pour chaque tuile solide (`core::isSolid`/`TileMap::isSolid`, déjà existants dans `Core`, **aucun**
  nouvel accès `Core` requis au-delà de cette lecture), dessiner un quad sombre semi-transparent
  légèrement décalé, sur valeur *Shadow* de *RenderLayer* (LOT-40), **avant** le rendu de la tuile elle-même —
  visible uniquement là où le fond (LOT-43) est visible en dessous.
- Actif **uniquement** en mode Texture (LOT-41). Sans effet en mode Physique (déjà sans ambiguïté par
  la couleur plate) et **aucun** effet sur le gameplay (`EX-ARCH-012`).
- Dégradation propre : un niveau sans fond configuré n'a simplement aucune surface visible pour
  recevoir l'ombre (pas d'erreur, pas de cas particulier à gérer).

### Exclus (hors périmètre de ce lot)
- Éclairage dynamique, ombres portées entre objets, tout système d'éclairage général — ce lot reste
  un simple quad sombre décalé, pas un moteur d'ombres.
- Configuration par le designer (intensité, angle) — valeurs fixes pour ce premier lot.

## Décisions de cadrage
- **Purement `HMI`, zéro nouvelle surface `Core`** : réutilise `core::isSolid`/`TileMap::isSolid`
  existants, ne lit rien d'autre.
- **valeur *Shadow* de *RenderLayer*** déjà réservé en LOT-40 — ce lot est le premier à l'activer.

## Exigences couvertes
- Nouvelle : `EX-REN-045` (ombres du plan physique sur le fond, mode Texture, purement visuel).
- Réutilisées : `EX-REN-043` (calques), `EX-REN-044` (fond), `EX-REN-046` (bascule), `EX-ARCH-012`
  (rendu non mutant/sans effet gameplay).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | Génération du quad d'ombre par tuile solide sur valeur *Shadow* de *RenderLayer* | `Source/HMI/Graphics` | ⬜ |
| TACHE-02 | Vérification manuelle (mode Texture, avec et sans fond configuré) + documentation | `Documentation` | ⬜ |

## Critères d'acceptation du lot
1. Les tuiles solides projettent une ombre visible sur le fond, en mode Texture uniquement.
2. Aucun effet en mode Physique ni sur le gameplay (collisions inchangées).
3. Un niveau sans fond configuré ne provoque aucune erreur ni artefact visuel.
4. Build `/W4 /WX`, Doxygen, lint verts ; vérification manuelle.

## Dépendances
Bâtit sur [LOT-41](@ref lot-41) (bascule), [LOT-40](@ref lot-40) (valeur *Shadow* de *RenderLayer*). Bénéficie
de [LOT-43](@ref lot-43) (fond) sans en dépendre strictement — aucune dépendance sur
[LOT-42](@ref lot-42)/[LOT-44](@ref lot-44)/[LOT-45](@ref lot-45).
