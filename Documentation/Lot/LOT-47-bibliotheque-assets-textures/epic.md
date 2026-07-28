# LOT-47 — Bibliothèque d'assets unifiée {#lot-47}

> Statut : **non commencé**. Prérequis : [LOT-42](@ref lot-42), [LOT-43](@ref lot-43),
> [LOT-44](@ref lot-44).

## Objectif
Remplacer les trois sélecteurs minimaux (liste de fichiers texte) construits ad hoc dans LOT-42/43/44
par un widget de **vignettes** partagé, et ajouter le **rechargement à chaud** des assets — deux
points explicitement laissés optionnels par LOT-39 lui-même, qui deviennent nécessaires une fois
trois sections du panneau « Textures » en usage réel.

## Périmètre

### Inclus
- **Widget de vignettes partagé**, utilisé par les trois sections existantes du panneau « Textures »
  (Skins, Fond, Objets — LOT-42/43/44) **en place**, sans changer l'identité ni le câblage du panneau
  ni son modèle de données : `hmi::decodeImageFile` → `QPixmap`, indépendant du
  *TextureCache* D3D11 (LOT-40) — deux consommateurs du même décodeur CPU, pas de nouveau décodeur.
- **Rechargement à chaud** : action « Recharger les textures » (ou surveillance de dossier) qui
  invalide le *TextureCache* (LOT-40) et régénère les vignettes, sans redémarrer l'application.

### Exclus (hors périmètre de ce lot)
- Aucun nouveau modèle de données, aucune nouvelle donnée `Level`/`skins.json` — pure consolidation
  d'UI sur les données déjà introduites par LOT-42/43/44.

## Décisions de cadrage
- **Évolution en place, pas un nouveau panneau** : réutilise le dock « Textures » créé en LOT-42 —
  cohérent avec la décision transverse du programme (« un seul panneau, pas trois jetables »).

## Exigences couvertes
- Réutilisées uniquement (amélioration d'UI) : `EX-EDIT-042`/`EX-EDIT-043` (sections concernées),
  `EX-REN-044` (fond).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | Widget de vignettes partagé (décodage CPU → `QPixmap`) | `Source/HMI/Editor` | ⬜ |
| TACHE-02 | Intégration aux sections Skins/Fond/Objets du panneau « Textures » | `Source/HMI/Editor` | ⬜ |
| TACHE-03 | Rechargement à chaud (invalidation *TextureCache* + vignettes) | `Source/HMI/Graphics`, `Source/HMI/Editor` | ⬜ |

## Critères d'acceptation du lot
1. Les trois sections du panneau « Textures » affichent des vignettes au lieu d'une liste texte, sans
   changement de comportement fonctionnel.
2. Une action « Recharger » reflète un asset modifié sur disque sans redémarrer l'application.
3. Build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-42](@ref lot-42), [LOT-43](@ref lot-43), [LOT-44](@ref lot-44) (panneau et sections
existantes). Réutilise *TextureLoader*/*TextureCache*. Prépare [LOT-48](@ref lot-48)
(point d'entrée « ouvrir dans l'éditeur pixel art »).
