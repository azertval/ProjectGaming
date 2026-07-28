# LOT-41 — Bascule Physique/Texture (`F8`, non remappable) {#lot-41}

> Statut : **non commencé**. Prérequis : [LOT-40](@ref lot-40) (rendu multi-textures). Prépare
> LOT-42 → LOT-48.

## Objectif
Introduire la commande qui bascule, en édition **et** en jeu réel, entre le rendu **Physique**
(couleur plate par type de tuile, celui d'aujourd'hui — lecture directe des collisions) et le rendu
**Texture** (habillage complet, construit lot après lot à partir de LOT-42). Sans cette bascule, rien
de ce que les lots suivants ajoutent n'est consultable ni comparable au rendu physique existant.

## Périmètre

### Inclus
- ***RenderMode { Physique, Texture }*** (namespace `hmi`, nom équivalent possible), propagé depuis
  `hmi::GameViewport` (widget unique derrière édition, essai et jeu réel, `EX-IHM-002`) jusqu'à
  `DraftRenderer::render` et `GameSession::render`/`SpriteRenderer::render`.
- **`F8`** géré **en dur** dans `GameViewport::keyPressEvent`, précédent direct : `Qt::Key_F10`
  (bascule de la grille de repère, déjà non remappable) — **hors** des tables remappables
  (`GameKeyBindings`/`EditorKeyBindings`) : aucune entrée dans leurs enums, leur JSON ou leur UI de
  remappage.
- **Défaut selon la configuration de build** : `if constexpr (core::kDeveloperBuild)` (précédent :
  `Source/HMI/main.cpp`, sinks de log) → Physique par défaut en Debug, Texture par défaut en
  Release. Le joueur peut ensuite basculer librement dans les deux configurations (`F8` reste actif
  dans les deux).
- En mode Texture, **tout** s'affiche avec le repli en damier magenta (LOT-40, TACHE-03) tant
  qu'aucun skin n'existe encore — comportement honnête et attendu, pas un bug, jusqu'à LOT-42.
- Le mode Physique reste **identique bit à bit** au rendu actuel, dans tous les cas.

### Exclus (hors périmètre de ce lot)
- Aucune donnée de skin/fond/objet — ce lot ne fait que câbler la bascule et le point de résolution
  déjà préparé en LOT-40.
- Persistance du choix du joueur entre deux sessions : **non retenu** pour ce lot (chaque lancement
  repart du défaut de build) — à revisiter seulement si demandé explicitement.
- Aucun 3ᵉ mode (ex. superposition physique+texture) : binaire pour ce lot.

## Décisions de cadrage
- **`GameViewport` unique** porte la bascule : couvre automatiquement édition, essai et jeu réel sans
  double implémentation (`EX-IHM-002`).
- **Touche fixe, jamais remappable** : cohérent avec `F10` déjà non remappable ; une bascule de rendu
  n'est pas une action de gameplay remappable au même titre que sauter/courir.
- **Pas de persistance inter-session** (choisi explicitement, cf. « Exclus ») : simplicité, le défaut
  de build reste la source de vérité à chaque lancement.
- **Résolution de calque non concernée** : ce lot introduit uniquement le drapeau de mode ; le point
  de résolution « override > skin > damier » (LOT-40) n'a encore qu'une seule branche active
  (damier), les branches suivantes arrivent lot après lot sans changer la forme de ce résolveur.

## Exigences couvertes
- Nouvelle : `EX-REN-046` (bascule Physique/Texture, F8, non remappable, défaut par build).
- Réutilisées : `EX-REN-043` (rendu multi-textures, LOT-40), `EX-NFR-040` (repli), `EX-IHM-002`
  (viewport unique).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | *RenderMode* + câblage `GameViewport` → `DraftRenderer`/`GameSession` | `Source/HMI/Game`, `Source/HMI/Graphics` | ⬜ |
| TACHE-02 | `F8` en dur, hors tables de remappage ; défaut par `core::kDeveloperBuild` | `Source/HMI/Game` | ⬜ |
| TACHE-03 | Vérification manuelle (édition, essai, jeu réel) + documentation (guide-rendu.md) | `Documentation` | ⬜ |

## Critères d'acceptation du lot
1. `F8` bascule le rendu en édition, en essai et en jeu réel, sans passer par les tables de
   remappage.
2. Le mode Physique est **identique bit à bit** au rendu d'avant ce lot.
3. Le défaut au lancement correspond à la configuration de build (Debug → Physique, Release →
   Texture).
4. Build `/W4 /WX`, Doxygen et lint verts ; bascule vérifiée visuellement dans les trois contextes
   (édition, essai, jeu réel).

## Dépendances
Bâtit sur [LOT-40](@ref lot-40) (rendu multi-textures, résolveur de texture). Ne modifie pas `Core`.
Prérequis de [LOT-42](@ref lot-42) à [LOT-48](@ref lot-48).
