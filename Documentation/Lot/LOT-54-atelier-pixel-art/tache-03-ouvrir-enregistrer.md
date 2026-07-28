# TACHE-03 — Ouvrir, créer, enregistrer et garde-fou d'écrasement {#lot-54-tache-03-ouvrir-enregistrer}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** non commencé

## Contexte
Le canevas (TACHE-02) travaille en mémoire ; il faut le relier aux fichiers d'assets. La
bibliothèque de LOT-43 fournit déjà la navigation, la validation de nom et la **détection des
références** — un asset peut être cité par plusieurs niveaux et par `skins.json`.

C'est cette détection qui rend l'enregistrement délicat : écraser un asset utilisé par cinq niveaux
n'est pas une opération anodine, et l'utilisateur n'a aucun moyen de le savoir sans qu'on le lui
dise.

## Travail à réaliser
- **Ouvrir** un asset existant depuis la bibliothèque (LOT-43), comme point d'entrée principal de
  l'atelier.
- **Créer** un nouvel asset, à une taille choisie **parmi celles admises** par le contrat d'asset de
  sa famille (LOT-40) — proposer les tailles valides plutôt que de laisser saisir n'importe quoi
  puis refuser au chargement.
- **Enregistrer** via l'écriture atomique de TACHE-01, en préservant les dimensions de l'image
  ouverte.
- **Garde-fou d'écrasement** : enregistrer par-dessus un asset **référencé** demande confirmation, en
  **nommant** les niveaux et les entrées de `skins.json` concernés (détection livrée en LOT-43).
- **Garde-fou de perte de travail** : fermer l'atelier ou ouvrir un autre asset avec des
  modifications non enregistrées demande confirmation — même patron que l'éditeur de niveaux
  (`EX-EDIT-021`).
- **Enregistrer sous** : créer une copie plutôt qu'écraser, la voie de sortie naturelle quand le
  garde-fou d'écrasement se déclenche.

## Fichiers impactés
- `Source/HMI/Editor/PixelCanvas.{h,cpp}`, `TexturePanel.{h,cpp}`, `AssetThumbnailView.{h,cpp}`.
- `Source/HMI/Editor/AssetReferences.{h,cpp}` (réutilisé), `AssetFileOperations.{h,cpp}`.
- `Source/Elements/Localization/fr.lang`, `en.lang`.

## Tests (obligatoires)
- **Préservation des dimensions** : ouvrir puis enregistrer sans modification produit une image de
  mêmes dimensions et de mêmes pixels.
- **Tailles proposées à la création** : conformes au contrat de chaque famille d'asset.
- **Détection d'écrasement** : un asset référencé déclenche la demande de confirmation, un asset non
  référencé ne la déclenche pas.
- Suivi de l'état « modifié » : un coup de pinceau le marque, un enregistrement le lève.
- Logique testée sans Qt.

## Points d'attention
- **Ne pas réimplémenter la détection des références** : elle existe depuis LOT-43 et doit rester
  unique, sinon les deux implémentations divergeront.
- L'enregistrement doit déclencher l'**invalidation ciblée** du cache (LOT-43, TACHE-03) pour que
  l'aperçu live (TACHE-04) se mette à jour — mais l'invalidation elle-même n'est pas le sujet de
  cette tâche.
- Une création à une taille non conforme ne doit pas être possible du tout, plutôt que refusée après
  coup : c'est la différence entre un outil qui guide et un outil qui punit.

## Définition de fait (DoD)
- Ouvrir, créer et enregistrer un asset fonctionne depuis la bibliothèque ; les dimensions sont
  préservées ; l'écrasement d'un asset référencé et la perte de travail sont couverts par des
  garde-fous nommant les éléments concernés ; chaînes traduites ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art) ; réutilise `EX-EDIT-026` (gestion des fichiers d'assets),
`EX-EDIT-021` (garde-fous contre la perte de travail), `EX-REN-007` (contrat d'asset),
`EX-REN-033` (traduction), `EX-NFR-040` (erreur récupérable).
