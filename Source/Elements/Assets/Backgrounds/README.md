# Elements/Assets/Backgrounds/

Fonds de niveau : les PNG qu'un niveau peut désigner comme image de dernier plan (`LOT-44`,
`EX-REN-044`).

Déposer un fichier ici suffit à le rendre sélectionnable dans la section « Fond » du panneau
« Textures » de l'éditeur : la liste est peuplée par **balayage de ce dossier**, jamais par saisie
d'un chemin (même patron que `Skins/`, `LOT-42`).

Dimensions **libres** (`hmi::AssetFamily::Background`, `EX-REN-007`) : le fond est étiré sur les
bornes du niveau au rendu, avec son ratio d'aspect **préservé** — l'image est recadrée par le
centre sur la dimension excédentaire plutôt que déformée. Un fichier illisible ou introuvable
retombe sur le damier magenta, avec un avertissement journalisé nommant l'asset (`EX-NFR-040`).

## Contenu actuel : des fonds de **test**

Trois fonds (320×180, ratio 16:9), **schématiques**, servent à vérifier le rendu du fond et à
offrir un minimum de variété d'ambiance pour la refonte du `LOT-65`, pas à habiller le jeu.
Générés par script, donc reproductibles et modifiables sans éditeur d'image :

```
python scripts/generate_test_backgrounds.py
```

| Fichier | Ambiance |
|---|---|
| `test_sky.png` | ciel diurne, soleil, bande de sol |
| `test_night.png` | ciel nocturne, lune, étoiles, silhouette de collines |
| `test_cave.png` | souterrain, stalactites, points de lueur |

Chacun partage le même cadre de bordure et les mêmes graduations centrées, qui rendent le
recadrage bien visible : sur un niveau d'un ratio différent, le cadre doit rester entier et
**centré**, jamais coupé de travers ni déformé — c'est le repère visuel pour vérifier
`hmi::computeBackgroundFit` (`LOT-44`, TACHE-02) à l'œil, dans l'éditeur (section « Fond » du
panneau « Textures », mode Texture, `F8`).

Un artiste remplacera ces fichiers par les vrais fonds, sans toucher au code.

## Fond `kenney_grass.png` : premier fond réel, sous licence libre (`LOT-65`)

Contrairement aux trois fonds ci-dessus (schématiques, générés par script), `kenney_grass.png`
(1024×1024) est une véritable illustration, publiée par **Kenney** (www.kenney.nl) sous licence
**CC0 1.0 Universal** (domaine public), utilisée telle quelle (dimensions déjà libres, aucune
retouche) — voir `../CREDITS.md` pour le détail. Il n'est associé à aucun jeu de skins
(`Backgrounds/` n'est pas structuré en `jeux` comme `skins.json`) : un niveau le désigne
directement par son nom de fichier.
