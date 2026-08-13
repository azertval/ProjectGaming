# Crédits graphiques

La plupart des images de `Source/Elements/Assets/` (`atlas.png`, les skins du jeu `test`, les
planches de test des dossiers `Objects/`/`Decors/`/`Backgrounds/`) sont **générées par script**
(schématiques, sans dépendance externe) : voir `Skins/README.md`, `Objects/README.md`,
`Decors/README.md`.

Le jeu de skins `kenney` (`skins.json`) est une exception : ses trois images sont issues de packs
**Kenney** (www.kenney.nl), publiés sous licence **CC0 1.0 Universal** (domaine public) :
<http://creativecommons.org/publicdomain/zero/1.0/>, retouchées pour respecter le contrat de
dimensions du projet (`Skins/README.md`).

Le CC0 n'exige aucune attribution, mais ce lot (`LOT-63`) s'engage à créditer les auteurs malgré
tout, même principe que `Source/Elements/Audio/CREDITS.md` (`LOT-60`).

| Fichier livré | Retouche | Sprite d'origine | Pack Kenney | Auteur | Licence |
|---|---|---|---|---|---|
| `Skins/key.png` (image 0/2) | recadrée, réduite à 16×16 | `keyYellow.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/locked_door.png` (image 0/2) | recadrée, réduite à 16×16 | `lockYellow.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/platform.png` | recadrée, réduite à 16×16 | `platformIndustrial_030.png` | Platformer Pack: Industrial | Kenney | CC0 1.0 |

Retouche appliquée à chaque sprite, identique pour les trois : détourage à la zone non
transparente, mise à l'échelle (`LANCZOS`) vers un carré de 16×16 pixels (ou deux cases de
16×16 côte à côte pour `key.png`/`locked_door.png`, la seconde image étant une case entièrement
transparente ajoutée par ce lot pour représenter l'état « ramassée »/« ouverte » — absente du pack
d'origine), sans autre retouche de couleur ou de forme.

Sources des packs :
- Platformer Pack Remastered — <https://kenney.nl/assets/platformer-pack-remastered>
- Platformer Pack: Industrial — <https://kenney.nl/assets/platformer-pack-industrial>
