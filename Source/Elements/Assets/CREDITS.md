# Crédits graphiques

La plupart des images de `Source/Elements/Assets/` (`atlas.png`, les skins du jeu `test`, les
planches de test des dossiers `Objects/`/`Decors/`/`Backgrounds/`/`Player/`) sont **générées par
script** (schématiques, sans dépendance externe) : voir `Skins/README.md`, `Objects/README.md`,
`Decors/README.md`, `Player/README.md`.

Le jeu de skins `kenney` (`skins.json`) est une exception : ses images et le fond
`Backgrounds/kenney_grass.png` sont issus de packs **Kenney** (www.kenney.nl), publiés sous licence
**CC0 1.0 Universal** (domaine public) :
<http://creativecommons.org/publicdomain/zero/1.0/>, retouchés pour respecter le contrat de
dimensions du projet (`Skins/README.md`) — sauf le fond, dimensions libres, utilisé tel quel.

Le CC0 n'exige aucune attribution, mais ce dépôt s'engage à créditer les auteurs malgré tout, même
principe que `Source/Elements/Audio/CREDITS.md` (`LOT-60`).

| Fichier livré | Retouche | Sprite d'origine | Pack Kenney | Auteur | Licence |
|---|---|---|---|---|---|
| `Skins/key.png` (image 0/2) | recadrée, réduite à 16×16 | `keyYellow.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/locked_door.png` (image 0/2) | recadrée, réduite à 16×16 | `lockYellow.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/platform.png` | recadrée, réduite à 16×16 | `platformIndustrial_030.png` | Platformer Pack: Industrial | Kenney | CC0 1.0 |
| `Skins/kenney_spikes.png` | recadrée, réduite à 16×16 | `spikes.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/kenney_switch.png` (image 0/2) | recadrée, réduite à 16×16 | `switchYellow.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/kenney_switch.png` (image 1/2) | recadrée, réduite à 16×16 | `switchYellow_pressed.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/kenney_door.png` (image 0/2) | recadrée, réduite à 16×16 | `doorClosed_mid.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/kenney_door.png` (image 1/2) | recadrée, réduite à 16×16 | `doorOpen_mid.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Backgrounds/kenney_grass.png` | aucune (dimensions libres) | `colored_grass.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |

Retouche appliquée à chaque sprite de `Skins/` (hors `kenney_grass.png`, non retouché) : détourage
à la zone non transparente, mise à l'échelle (`LANCZOS`) vers un carré de 16×16 pixels (ou deux
cases de 16×16 côte à côte pour les assets à deux états — `key.png`/`locked_door.png` (`LOT-63`),
`kenney_switch.png`/`kenney_door.png` (`LOT-65`) —, la seconde image de `key.png`/`locked_door.png`
étant une case entièrement transparente ajoutée par le `LOT-63` pour représenter l'état
« ramassée »/« ouverte », absente du pack d'origine ; `kenney_switch.png`/`kenney_door.png`
utilisent chacun deux sprites distincts déjà présents dans le pack, un par état), sans autre
retouche de couleur ou de forme.

Sources des packs :
- Platformer Pack Remastered — <https://kenney.nl/assets/platformer-pack-remastered>
- Platformer Pack: Industrial — <https://kenney.nl/assets/platformer-pack-industrial>
