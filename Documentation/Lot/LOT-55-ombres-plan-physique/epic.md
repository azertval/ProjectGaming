# LOT-55 — Ombres du plan physique {#lot-55}

> Statut : **non commencé**. Prérequis : [LOT-41](@ref lot-41) (bascule). Bénéficie de
> [LOT-44](@ref lot-44) (fond) et [LOT-49](@ref lot-49) (décors). Dernier lot du programme
> d'habillage.

## Objectif
Aider visuellement le level designer — et, en mode Texture, le joueur — à distinguer ce qui est
**physique** (solide, collidable) de ce qui est **décor** : les tuiles solides projettent une ombre
douce sur ce qui se trouve derrière elles, purement visuelle.

C'est le complément du calque de premier plan (LOT-49) : le premier plan dit « ceci passe devant
vous, donc ne vous porte pas » ; l'ombre dit « ceci est en relief sur le décor, donc vous porte ».

Placé en dernier parce que c'est du polissage : il n'est prérequis d'aucun autre lot et n'apporte
rien tant que le reste de l'habillage n'existe pas.

## Périmètre

### Inclus
- Pour chaque tuile solide (`core::isSolid`/`TileMap::isSolid`, déjà existants dans `Core`,
  **aucun** nouvel accès requis au-delà de cette lecture), dessiner un quad sombre semi-transparent
  légèrement décalé, sur le calque *Shadow* de *RenderLayer* (réservé en LOT-40), **avant** le rendu
  de la tuile elle-même — visible uniquement là où le fond ou un décor d'arrière-plan est visible en
  dessous.
- Actif **uniquement** en mode Texture (LOT-41). Sans effet en mode Physique (déjà sans ambiguïté
  par la couleur plate) et **aucun** effet sur le gameplay (`EX-ARCH-012`).
- Dégradation propre : un niveau sans fond ni décor d'arrière-plan n'a simplement aucune surface
  visible pour recevoir l'ombre (pas d'erreur, pas de cas particulier à gérer).
- Les quads d'ombre passent par le culling comme tout le reste (LOT-40, TACHE-05).

### Exclus (hors périmètre de ce lot)
- Éclairage dynamique, ombres portées entre objets, tout système d'éclairage général — ce lot reste
  un simple quad sombre décalé, pas un moteur d'ombres.
- Ombres projetées par le personnage, les objets interactifs ou les décors.
- Configuration par le designer (intensité, angle, portée) : valeurs fixes, constantes nommées.

## Décisions de cadrage
- **Purement `HMI`, zéro nouvelle surface `Core`** : réutilise `core::isSolid`/`TileMap::isSolid`
  existants, ne lit rien d'autre.
- **Calque *Shadow* déjà réservé en LOT-40** — ce lot est le premier et le seul à l'activer.
- **Placé en dernier** : aucun lot n'en dépend, et son intérêt est nul tant qu'il n'y a rien
  derrière les tuiles.
- **Valeurs fixes** : ouvrir le réglage au designer supposerait de le stocker (où ? par niveau ? par
  jeu de skins ?) pour un bénéfice non établi.

## Exigences couvertes
- Nouvelle : `EX-REN-045` (ombres du plan physique, mode Texture, purement visuel).
- Réutilisées : `EX-REN-043` (calques), `EX-REN-044` (fond), `EX-REN-046` (bascule), `EX-DEC-002`
  (couches de décor), `EX-ARCH-012` (rendu non mutant, sans effet gameplay), `EX-NFR-005` (culling),
  `EX-NFR-004` (vérification sans GPU).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-quad-ombre.md) | Génération du quad d'ombre par tuile solide sur le calque *Shadow* | `Source/HMI/Graphics` | ⬜ |
| [TACHE-02](tache-02-verification-documentation.md) | Vérification finale du programme + documentation | `Source/Test`, `Documentation` | ⬜ |

## Critères d'acceptation du lot
1. Les tuiles solides projettent une ombre visible sur le fond et sur les décors d'arrière-plan, en
   mode Texture uniquement.
2. Aucun quad d'ombre n'est émis en mode Physique — asserté via le *QuadRecorder*.
3. Le gameplay est inchangé (collisions identiques, tests existants inchangés).
4. Un niveau sans fond ni décor d'arrière-plan ne provoque aucune erreur ni artefact visuel.
5. Build `/W4 /WX`, Doxygen, lint verts ; vérification manuelle.

## Dépendances
Bâtit sur [LOT-41](@ref lot-41) (bascule) et [LOT-40](@ref lot-40) (calque *Shadow*, culling,
*QuadRecorder*). Bénéficie de [LOT-44](@ref lot-44) (fond) et [LOT-49](@ref lot-49) (décors) sans en
dépendre strictement. Aucun lot ne dépend de celui-ci.

## Navigation des tâches
- @subpage lot-55-tache-01-quad-ombre
- @subpage lot-55-tache-02-verification-documentation
