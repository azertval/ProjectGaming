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

## Contenu actuel : un fond de **test**

`test_sky.png` (320×180, ratio 16:9) est **schématique** et sert à vérifier le rendu du fond, pas
à habiller le jeu. Généré par script, donc reproductible et modifiable sans éditeur d'image :

```
python scripts/generate_test_backgrounds.py
```

Son cadre de bordure et ses graduations centrées rendent le recadrage bien visible : sur un
niveau d'un ratio différent, le cadre doit rester entier et **centré**, jamais coupé de travers
ni déformé — c'est le repère visuel pour vérifier `hmi::computeBackgroundFit` (`LOT-44`,
TACHE-02) à l'œil, dans l'éditeur (section « Fond » du panneau « Textures », mode Texture, `F8`).

Un artiste remplacera ce fichier par le vrai fond, sans toucher au code.
