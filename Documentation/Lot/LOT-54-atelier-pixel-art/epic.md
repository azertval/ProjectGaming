# LOT-54 — Atelier pixel art intégré {#lot-54}

> Statut : **non commencé**. Prérequis : [LOT-42](@ref lot-42) (convention `Assets/Skins/`, planches
> à raccords), [LOT-43](@ref lot-43) (bibliothèque et rechargement à chaud).

## Objectif
Permettre de créer et modifier les fichiers d'assets de texture (skins, planches, fonds, objets,
décors, images d'animation) **sans quitter l'application**, avec un canevas pixel art minimal — et
en voyant immédiatement le résultat dans le niveau.

## Périmètre

### Inclus
- **Canevas** `QWidget` + `QImage` + `QPainter` : peindre et effacer au pixel près (grille alignée),
  **pot de peinture** (remplissage par zone contiguë), pipette, palette de couleurs, zoom,
  annuler/refaire **local** — propre au canevas, totalement indépendant de l'historique de
  `LevelDraft`.
- **Ouvrir, créer, enregistrer** : ouvrir un asset existant depuis la bibliothèque (LOT-43), ou en
  créer un à une taille choisie parmi celles admises par le contrat d'asset (LOT-40) ; enregistrer
  via un encodeur d'image **symétrique** de `hmi::decodeImageFile` (LOT-39).
- **Aperçu live** : pendant l'édition, le niveau affiché reflète les modifications, en s'appuyant sur
  `TextureCache::invalidate` (API prévue dès LOT-40). C'est ce qui distingue un atelier intégré d'un
  éditeur d'images lancé à côté.
- **Mode planche à raccords** : lors de l'édition d'un asset en mode `bitmask16` (LOT-42), afficher
  les repères des seize cases et l'indication de ce que chaque case représente (bord haut, coin
  concave, intérieur…). Dessiner une planche cohérente sans repères est une source d'erreurs
  garantie.
- **Garde-fou d'écrasement** : enregistrer par-dessus un asset référencé par des niveaux ou par un
  jeu de skins demande confirmation, en nommant les références concernées (détection déjà livrée en
  LOT-43).
- Toute UI passe par le catalogue de traduction.

### Exclus (hors périmètre de ce lot)
- Calques dans le canevas, sélection et déplacement de région, transformations (rotation,
  symétrie), dégradés, anticrénelage — périmètre minimal pour un premier jet ; extensions possibles
  dans un lot ultérieur si le besoin se confirme à l'usage.
- Édition d'une animation image par image avec prévisualisation temporelle : le canevas édite une
  spritesheet comme une image ; l'affectation des clips relève de LOT-46/47.
- Conversion photo → pixel art (`EX-EDIT-041`, `EX-DEC-030` à `EX-DEC-032`) : hors programme.
- Toute intégration d'une bibliothèque externe (cf. décision de cadrage).

## Décisions de cadrage
- **Développement interne, pas de bibliothèque externe.** La licence du projet (« tous droits
  réservés ») est incompatible avec les éditeurs pixel art matures existants (Aseprite,
  LibreSprite — tous deux de lignée GPL) ; il n'existe pas de bibliothèque Qt/C++ légère et
  **permissive** équivalente pour ce besoin précis. Le périmètre réel (peindre, effacer, remplir,
  palette, zoom, annuler, enregistrer) est modeste et repose entièrement sur des primitives déjà en
  place (Qt Widgets depuis LOT-34, *TextureLoader*/`hmi::AssetPaths` depuis LOT-39) — cohérent avec
  la politique du projet de préférer Qt à une dépendance tierce.
- **Pot de peinture inclus, explicitement** : le cadrage initial excluait « le remplissage au-delà du
  seau de peinture simple », formulation ambiguë qui laissait ouvert le statut du seau lui-même.
  Tranché : remplissage par zone contiguë inclus, tout le reste exclu.
- **Aperçu live plutôt qu'aller-retour** : sans lui, l'atelier n'apporterait rien de plus qu'un
  éditeur d'images externe, et le programme aurait payé un lot pour une commodité.
- **Amende le non-objectif « pas d'édition d'assets graphiques »** de
  [`editeur-niveaux.md`](../../Specification/editeur-niveaux.md) (section 5) de façon ciblée :
  l'éditeur de niveaux continue d'agencer des tuiles existantes sans générer de sprites procéduraux ;
  seul ce canevas dédié édite les fichiers d'assets eux-mêmes.

## Exigences couvertes
- Amendée : `EX-EDIT-045` (outil de dessin pixel art intégré, avec aperçu du rendu dans le niveau).
- Réutilisées : `EX-EDIT-024`/`EX-EDIT-025` (jeux de skins, planches à raccords), `EX-EDIT-026`
  (gestion des fichiers d'assets), `EX-REN-041` (décodage image), `EX-REN-007` (contrat d'asset),
  `EX-REN-033` (traduction).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-encodage-image.md) | Encodage et enregistrement d'image, symétrique de `decodeImageFile` | `Source/HMI/Graphics` | ⬜ |
| [TACHE-02](tache-02-canevas.md) | Canevas pixel art (peindre, effacer, pot de peinture, pipette, palette, zoom, annuler/refaire local) | `Source/HMI/Editor` | ⬜ |
| [TACHE-03](tache-03-ouvrir-enregistrer.md) | Ouvrir, créer, enregistrer + garde-fou d'écrasement + intégration à la bibliothèque | `Source/HMI/Editor` | ⬜ |
| [TACHE-04](tache-04-apercu-live-planche.md) | Aperçu live (invalidation du cache) + mode planche à raccords avec repères | `Source/HMI/Editor`, `Source/HMI/Graphics` | ⬜ |

## Critères d'acceptation du lot
1. Créer, peindre et enregistrer un nouvel asset PNG depuis le canevas, puis le retrouver assignable
   dans le panneau « Textures » sans redémarrer.
2. Modifier un asset existant et l'enregistrer préserve ses dimensions.
3. Le niveau affiché reflète les modifications en cours d'édition.
4. L'édition d'une planche à raccords affiche les repères des seize cases et leur rôle.
5. Enregistrer par-dessus un asset référencé demande confirmation en nommant les références.
6. L'annuler/refaire du canevas est fonctionnel et **indépendant** de celui de `LevelDraft` :
   annuler un coup de pinceau n'annule jamais une action d'édition de niveau.
7. Build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-42](@ref lot-42) (convention de dossiers, planches à raccords) et
[LOT-43](@ref lot-43) (bibliothèque, détection de références, rechargement à chaud). Réutilise
*TextureLoader*/`hmi::AssetPaths` (LOT-39) et `TextureCache::invalidate` ([LOT-40](@ref lot-40)).

## Navigation des tâches
- @subpage lot-54-tache-01-encodage-image
- @subpage lot-54-tache-02-canevas
- @subpage lot-54-tache-03-ouvrir-enregistrer
- @subpage lot-54-tache-04-apercu-live-planche
