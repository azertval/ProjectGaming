# Elements/Assets/Skins/

Skins de tuiles : les PNG que `skins.json` associe aux types de tuiles (`LOT-42`, `EX-EDIT-042`).

Déposer un fichier ici suffit à le rendre sélectionnable dans le panneau « Textures » de
l'éditeur : la liste est peuplée par **balayage de ce dossier**, jamais par saisie d'un chemin.

Dimensions attendues, validées au chargement (`EX-REN-007`) :

- **16×16 pixels** pour un skin en mode `single` ;
- **64×64 pixels** (4×4 cases de 16) pour une planche en mode `bitmask16`.

La convention de contenu de chaque case d'une planche est décrite dans le
[README du dossier parent](../README.md).

Un fichier aux mauvaises dimensions est refusé avec un message nommant le fichier, le trouvé et
l'attendu ; la tuile retombe alors sur le damier magenta plutôt que d'afficher des artefacts
silencieux.
