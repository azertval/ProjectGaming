# Elements/Assets/Skins/

Skins de tuiles : les PNG que `skins.json` associe aux types de tuiles (`LOT-42`, `EX-EDIT-042`).

Déposer un fichier ici suffit à le rendre sélectionnable dans le panneau « Textures » de
l'éditeur : la liste est peuplée par **balayage de ce dossier**, jamais par saisie d'un chemin.

Dimensions attendues, validées au chargement (`EX-REN-007`) :

- **16×16 pixels** pour un skin en mode `single` fixe, ou **16×(16×N) pixels** (une case de haut,
  N cases de large) pour un skin en mode `single` **animé** — une spritesheet horizontale dont
  chaque case est une image, décrite par un fichier `<asset>.anim.json` à côté du PNG (`LOT-46`,
  voir `Documentation/Lot/LOT-46-moteur-animation/`) ;
- **64×64 pixels** (4×4 cases de 16) pour une planche en mode `bitmask16` — `bitmask16` exclut
  l'animation (limite assumée, `LOT-46` TACHE-05).

La convention de contenu de chaque case d'une planche est décrite dans le
[README du dossier parent](../README.md).

Un fichier aux mauvaises dimensions est refusé avec un message nommant le fichier, le trouvé et
l'attendu ; la tuile retombe alors sur le damier magenta plutôt que d'afficher des artefacts
silencieux.

## Contenu actuel : des skins de **test**

Les quatre images présentes sont **schématiques** et servent à vérifier que le moteur d'habillage
fonctionne, pas à habiller le jeu. Elles sont générées par script, donc reproductibles et
modifiables sans éditeur d'image :

```
python scripts/generate_test_skins.py
```

| Fichier | Contrat | Sert à vérifier |
|---|---|---|
| `stone.png` | planche 64×64 | les **raccords automatiques** : liseré clair sur les faces sans voisin solide, intérieur nu |
| `crate.png` | 16×16 | un skin `single` sur les trois tailles de bloc |
| `spikes.png` | 16×16 | un skin `single` **à transparence** (dangers) |
| `slope_stone.png` | 16×16 | le **détourage automatique** : carré plein, découpé par le moteur à la silhouette de chaque pente |

Le catalogue livré (`../skins.json`) les assigne dans un jeu nommé `test`, et propose un second jeu
`aucun`, vide, pour vérifier d'un coup d'œil le sélecteur de jeu et le repli en damier.

Un artiste remplacera ces fichiers par les vrais assets, sans toucher au code : c'est précisément
ce que le format permet.

## Skins **animés** de démonstration (`LOT-46`)

Trois assets animés, générés par script comme ci-dessus (même esprit, sans dépendance externe) :

```
python scripts/generate_test_animations.py
```

| Fichier | Clip | Sert à vérifier |
|---|---|---|
| `water.png` + `water.anim.json` | `wave`, 4 images, 0,15 s/image | une nappe d'eau ondulante |
| `lava.png` + `lava.anim.json` | `bubble`, 4 images, 0,2 s/image | une coulée de lave |
| `torch.png` + `torch.anim.json` | `flicker`, 3 images, 0,1 s/image | une flamme qui vacille |

Non assignés dans `skins.json` : les affecter à un type depuis le panneau « Textures » de
l'éditeur (mode `single` uniquement — `bitmask16` exclut l'animation, `LOT-46` TACHE-05) est un
choix de contenu, pas une conséquence du moteur.
