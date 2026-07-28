# LOT-48 — Éditeur de texture pixel art intégré {#lot-48}

> Statut : **non commencé**. Prérequis : [LOT-42](@ref lot-42) (convention `Assets/Skins/` etc.).
> Bénéficie de [LOT-47](@ref lot-47).

## Objectif
Permettre de créer/modifier les fichiers d'assets de texture (skins, fonds, objets) **sans quitter
l'application**, avec un canevas pixel art minimal.

## Périmètre

### Inclus
- **Canevas** `QWidget` + `QImage` + `QPainter` : peindre/effacer au pixel près (grille alignée),
  petite palette de couleurs, zoom, undo/redo **local** (propre au canevas, indépendant de
  `LevelDraft`).
- **Ouvrir/créer/enregistrer** : ouvrir un asset existant (`Assets/{Skins,Backgrounds,Objects}/*.png`,
  via le sélecteur de LOT-42/47) ou créer un nouveau fichier à une taille choisie ; enregistrer via un
  nouveau *saveImageFile*/*encodeImageFile*, symétrique de `hmi::decodeImageFile` (LOT-39).

### Exclus (hors périmètre de ce lot)
- Calques, images animées, outils avancés (sélection/déplacement, remplissage au-delà du seau de
  peinture simple) — périmètre minimal pour ce premier jet ; extensions possibles dans un lot
  ultérieur si le besoin se confirme à l'usage.
- Toute intégration d'une bibliothèque externe (cf. décision de cadrage).

## Décisions de cadrage
- **Développement interne, pas de bibliothèque externe.** La licence du projet (« tous droits
  réservés ») est incompatible avec les éditeurs pixel art matures existants (Aseprite, LibreSprite —
  tous deux sous une licence de lignée GPL) ; il n'existe pas de bibliothèque Qt/C++ légère et
  **permissive** équivalente pour ce besoin précis. Le périmètre réel (peindre/effacer/palette/zoom/
  annuler/sauver) est modeste et repose entièrement sur des primitives déjà en place (Qt Widgets
  depuis LOT-34, *TextureLoader*/`hmi::AssetPaths` depuis LOT-39) — cohérent avec la politique du
  projet de préférer Qt à une dépendance tierce (`External/` réservé aux cas où Qt ne suffit pas, cf.
  note `stb_image` de LOT-39, jamais adoptée).
- **Amende le non-objectif « pas d'édition d'assets graphiques »** de `editeur-niveaux.md` (section 5)
  de façon ciblée : l'éditeur de niveaux continue d'agencer des tuiles existantes sans générer de
  sprites procéduraux ; seul ce canevas dédié édite les fichiers d'assets eux-mêmes.

## Exigences couvertes
- Nouvelle : `EX-EDIT-045` (éditeur de texture pixel art minimal, intégré, sans dépendance externe).
- Réutilisées : `EX-EDIT-042` (convention de dossiers `Assets/`), `EX-REN-041` (décodage image,
  LOT-39).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | *saveImageFile*/*encodeImageFile* (symétrique de `decodeImageFile`) | `Source/HMI/Graphics` | ⬜ |
| TACHE-02 | Canevas pixel art (peindre/effacer/palette/zoom/undo-redo local) | `Source/HMI/Editor` | ⬜ |
| TACHE-03 | Ouvrir/créer/enregistrer, intégration au sélecteur de LOT-42/47 | `Source/HMI/Editor` | ⬜ |

## Critères d'acceptation du lot
1. Créer, peindre et enregistrer un nouvel asset PNG depuis le canevas, puis le retrouver assignable
   dans le panneau « Textures » sans redémarrer (avec le rechargement à chaud de LOT-47).
2. Modifier un asset existant et l'enregistrer préserve ses dimensions.
3. Undo/redo local du canevas fonctionnel, indépendant de celui de `LevelDraft`.
4. Build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-42](@ref lot-42) (convention de dossiers). Bénéficie de [LOT-47](@ref lot-47)
(point d'entrée depuis la bibliothèque de vignettes). Réutilise *TextureLoader*/
`hmi::AssetPaths` (LOT-39). Dernier lot du programme LOT-40→48.
